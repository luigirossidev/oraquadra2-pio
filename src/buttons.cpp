#include "buttons.h"
#include <Arduino.h>
#include <EEPROM.h>
#include "eeprom_map.h"
#include "config.h"
#include "state.h"
#include "network.h"
#include "presets.h"

// [GESTIONE PULSANTI E TEMPI DI RILASCIO]

void Buttons::check() {
    static uint32_t lastAction = 0;
    static uint32_t bothPressStart = 0;
    static uint32_t modePressStart = 0;
    static uint32_t colorPressStart = 0;

    static bool modeLongDone = false;
    static bool colorLongDone = false;

    // NUOVI FLAG (essenziali per correggere la tua logica)
    static bool modeWasPressed = false;
    static bool colorWasPressed = false;

    const uint32_t DEBOUNCE_TIME = 300;
    const uint32_t RESET_WIFI_TIME = 5000;
    const uint32_t DIGITAL_TOGGLE_TIME = 6000;

#if BUTTON_LOGIC_INVERTED == 1
    bool isMode = !digitalRead(BUTTON_MODE);
    bool isColor = !digitalRead(BUTTON_SEC);
#else
    bool isMode = digitalRead(BUTTON_MODE);
    bool isColor = digitalRead(BUTTON_SEC);
#endif

    uint32_t now = millis();

    /*
     * 1) PRESSIONE CONTEMPORANEA PER RESET WIFI
     */
    if (isMode && isColor) {
        if (bothPressStart == 0) {
            bothPressStart = now;
            Serial.println("Inizio conteggio reset WiFi...");
        }
        if (now - bothPressStart >= RESET_WIFI_TIME) {
            Network::resetWiFi();
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

        // pressione lunga
        if (!modeLongDone && (now - modePressStart >= DIGITAL_TOGGLE_TIME)) {
            digitalOverlayEnabled = !digitalOverlayEnabled;
            EEPROM.write(EEPROM_DIGITAL_OVERLAY_ADDR, digitalOverlayEnabled);
            EEPROM.commit();
            Serial.println(digitalOverlayEnabled ? "Overlay ABILITATO (MODE)" : "Overlay DISABILITATO (MODE)");
            modeLongDone = true;
        }

    } else {

        // rilascio del pulsante MODE
        if (modeWasPressed) {

            // pressione breve SOLO se non c'è stata lunga
            if (!modeLongDone && (now - modePressStart < DIGITAL_TOGGLE_TIME)
                && now - lastAction > DEBOUNCE_TIME) {

                currentPreset = (currentPreset + 1) % 19;
                Presets::apply(currentPreset);
                EEPROM.write(EEPROM_PRESET_ADDR, currentPreset);
                EEPROM.commit();
                Serial.println("Preset -> " + String(currentPreset));

                lastAction = now;
            }
        }

        modeWasPressed = false;
        modePressStart = 0;
        modeLongDone = false;
    }


    /*
     * 3) GESTIONE PULSANTE COLOR
     */
    if (isColor && !isMode) {

        if (!colorWasPressed) {            // appena premuto
            colorPressStart = now;
            colorLongDone = false;
            colorWasPressed = true;
        }

        // pressione lunga
        if (!colorLongDone && (now - colorPressStart >= DIGITAL_TOGGLE_TIME)) {
            digitalOverlayEnabled = !digitalOverlayEnabled;
            EEPROM.write(EEPROM_DIGITAL_OVERLAY_ADDR, digitalOverlayEnabled);
            EEPROM.commit();
            Serial.println(digitalOverlayEnabled ? "Overlay ABILITATO (SEC)" : "Overlay DISABILITATO (SEC)");
            colorLongDone = true;
        }

    } else {

        // rilascio del pulsante COLOR
        if (colorWasPressed) {

            if (!colorLongDone && (now - colorPressStart < DIGITAL_TOGGLE_TIME)
                && now - lastAction > DEBOUNCE_TIME) {

                currentBlink = (currentBlink == 0) ? 1 : 0;
                EEPROM.write(EEPROM_BLINK_ADDR, currentBlink);
                EEPROM.commit();
                Serial.println(currentBlink ? "Blink ON" : "Blink OFF");

                lastAction = now;
            }
        }

        colorWasPressed = false;
        colorPressStart = 0;
        colorLongDone = false;
    }
}
