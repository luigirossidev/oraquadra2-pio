#include "buttons.h"
#include <Arduino.h>
#include "fc_config.h"
#include "config.h"
#include "state.h"
#include "presets.h"
#include "daily_content.h"
#include "melody_player.h"

// [GESTIONE PULSANTI E TEMPI DI RILASCIO]
//
// MODE breve:  preset successivo
// MODE lungo:  toggle quadrante digitale alternato
// SEC  breve:  frase del giorno -> indovinello (il toggle blink e' passato all'app)
// SEC  lungo:  mostra il PIN BLE per il pairing con l'app
// Entrambi 5s: factory reset (cancella settings.json e riavvia)

void Buttons::check() {
    static uint32_t lastAction = 0;
    static uint32_t bothPressStart = 0;
    static uint32_t modePressStart = 0;
    static uint32_t colorPressStart = 0;

    static bool modeLongDone = false;
    static bool colorLongDone = false;

    static bool modeWasPressed = false;
    static bool colorWasPressed = false;

    const uint32_t DEBOUNCE_TIME = 300;
    const uint32_t FACTORY_RESET_TIME = 5000;
    const uint32_t LONG_PRESS_TIME = 6000;

#if BUTTON_LOGIC_INVERTED == 1
    bool isMode = !digitalRead(BUTTON_MODE);
    bool isColor = !digitalRead(BUTTON_SEC);
#else
    bool isMode = digitalRead(BUTTON_MODE);
    bool isColor = digitalRead(BUTTON_SEC);
#endif

    uint32_t now = millis();

    // Qualsiasi pressione ferma una melodia del jukebox in corso
    if ((isMode || isColor) && melodyPlayerIsActive()) {
        melodyPlayerAbort();
    }

    /*
     * 1) PRESSIONE CONTEMPORANEA PER FACTORY RESET
     */
    if (isMode && isColor) {
        if (bothPressStart == 0) {
            bothPressStart = now;
            Serial.println("Inizio conteggio factory reset...");
        }
        if (now - bothPressStart >= FACTORY_RESET_TIME) {
            Serial.println("FACTORY RESET");
            factoryResetConfigurations();
            ESP.restart();
        }
        return;  // blocca tutto il resto
    } else {
        bothPressStart = 0;
    }

    /*
     * 2) GESTIONE PULSANTE MODE
     */
    if (isMode && !isColor) {

        if (!modeWasPressed) {              // appena premuto
            modePressStart = now;
            modeLongDone = false;
            modeWasPressed = true;
        }

        // pressione lunga: toggle quadrante digitale
        if (!modeLongDone && (now - modePressStart >= LONG_PRESS_TIME)) {
            digitalOverlayEnabled = !digitalOverlayEnabled;
            writeConfigurations();
            Serial.println(digitalOverlayEnabled ? "Overlay ABILITATO (MODE)" : "Overlay DISABILITATO (MODE)");
            modeLongDone = true;
        }

    } else {

        // rilascio del pulsante MODE
        if (modeWasPressed) {

            // pressione breve SOLO se non c'e' stata lunga
            if (!modeLongDone && (now - modePressStart < LONG_PRESS_TIME)
                && now - lastAction > DEBOUNCE_TIME) {

                currentPreset = (currentPreset + 1) % 19;
                Presets::apply(currentPreset);
                writeConfigurations();
                Serial.println("Preset -> " + String(currentPreset));

                lastAction = now;
            }
        }

        modeWasPressed = false;
        modePressStart = 0;
        modeLongDone = false;
    }


    /*
     * 3) GESTIONE PULSANTE SEC (frase/indovinello + PIN BLE)
     */
    if (isColor && !isMode) {

        if (!colorWasPressed) {            // appena premuto
            colorPressStart = now;
            colorLongDone = false;
            colorWasPressed = true;
        }

        // pressione lunga: mostra il PIN BLE
        if (!colorLongDone && (now - colorPressStart >= LONG_PRESS_TIME)) {
            DailyContent::requestShowPin();
            Serial.println("Richiesta visualizzazione PIN BLE (SEC)");
            colorLongDone = true;
        }

    } else {

        // rilascio del pulsante SEC
        if (colorWasPressed) {

            if (!colorLongDone && (now - colorPressStart < LONG_PRESS_TIME)
                && now - lastAction > DEBOUNCE_TIME) {

                DailyContent::requestQuoteQuiz();
                Serial.println("Richiesta frase/indovinello (SEC)");

                lastAction = now;
            }
        }

        colorWasPressed = false;
        colorPressStart = 0;
        colorLongDone = false;
    }
}
