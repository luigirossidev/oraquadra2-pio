#include "app.h"
#include <WiFi.h>
#include <ArduinoOTA.h>
#include <ezTime.h>
#include <FastLED.h>
#include "state.h"
#include "config.h"
#include "settings.h"
#include "network.h"
#include "time_utils.h"
#include "web_ui.h"
#include "presets.h"
#include "buttons.h"
#include "clock_display.h"
#include "scroll_text.h"
#include "clock_colors.h"
#include "effects/effects.h"
#include "effects/digital_overlay.h"

// Setup principale
void App::setup() {
   Serial.begin(115200);
   Serial.println("START");

   Settings::begin();

   // =========================================================================
   // FASE 3: DETERMINARE LA LUMINOSITÀ INIZIALE CORRETTA
   // =========================================================================
   
   // IMPORTANTE: Calcola subito se siamo in fascia giorno o notte
   // Questo richiede che l'ora sia disponibile, quindi lo faremo dopo WiFi
   // Per ora impostiamo un valore temporaneo ragionevole
   intBrightness = dayBrightness;  // Default temporaneo
   
   Serial.print("Luminosità iniziale temporanea: ");
   Serial.println(intBrightness);

   // =========================================================================
   // FASE 4: CONFIGURAZIONE PIN
   // =========================================================================

   #if BUTTON_LOGIC_INVERTED == 1
     pinMode(BUTTON_MODE, INPUT_PULLUP);
     pinMode(BUTTON_SEC, INPUT_PULLUP);
   #else
     pinMode(BUTTON_MODE, INPUT_PULLDOWN);
     pinMode(BUTTON_SEC, INPUT_PULLDOWN);
   #endif

   // =========================================================================
   // FASE 5: INIZIALIZZAZIONE FASTLED CON LUMINOSITÀ CORRETTA
   // =========================================================================

   FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS)
          .setCorrection(TypicalLEDStrip);

   FastLED.setBrightness(intBrightness);
   Serial.print("FastLED inizializzato con luminosità: ");
   Serial.println(intBrightness);

   FastLED.clear();
   FastLED.show();

   Network::connect();

   // =========================================================================
   // FASE 7: SINCRONIZZAZIONE ORARIO E CALCOLO LUMINOSITÀ CORRETTA
   // =========================================================================
   
   if(WiFi.status() == WL_CONNECTED) {
       Network::setupOTA();
       setServer("pool.ntp.org");
       waitForSync(10);
       myTZ.setLocation(F("Europe/Rome"));
       
       // ORA abbiamo l'orario! Calcoliamo la luminosità corretta
       // currentHour = myTZ.hour();
       // currentMinute = myTZ.minute();
       // currentSecond = myTZ.second();
       
       TimeUtils::updateCurrentTimeFromTZ();

       Serial.print("Ora sincronizzata: ");
       Serial.print(currentHour);
       Serial.print(":");
       Serial.println(currentMinute);
       
       if (TimeUtils::isSleepTime()) {
           Serial.println("Fascia oraria: SPEGNIMENTO NOTTURNO");
           displayOff = true;
           intBrightness = 0;
       } else {
           bool isNight = TimeUtils::isNightTime();
           intBrightness = isNight ? nightBrightness : dayBrightness;
           
           Serial.print("Fascia oraria: ");
           Serial.println(isNight ? "SERA" : "GIORNO");
       }

       Serial.print("Luminosità corretta impostata a: ");
       Serial.println(intBrightness);
       
       // Applica la luminosità corretta
       FastLED.setBrightness(intBrightness);
       
       // Avvia il server web
       WebUI::begin();
   }
   // =========================================================================
   // FASE 8: APPLICA IL PRESET SALVATO
   // =========================================================================
   
   Serial.print("Applicazione preset salvato: ");
   Serial.println(currentPreset);

   Presets::apply(currentPreset);

   // colore, rainbow / color, rainbow
   // per sovrascrivere i valori di default con quelli salvati dall'utente
   Settings::loadModeSettings(currentPreset);

   Serial.println("Setup completato!");
   Serial.println("=====================================");
   
   // Stampa riepilogo finale
   Serial.println("=== STATO FINALE ===");
   Serial.print("Preset: ");
   Serial.println(currentPreset);
   Serial.print("Luminosità: ");
   Serial.println(intBrightness);
   Serial.print("Colore RGB: ");
   Serial.print(clockColors.current.r);
   Serial.print(",");
   Serial.print(clockColors.current.g);
   Serial.print(",");
   Serial.println(clockColors.current.b);
   Serial.println("===================");
}

void App::loop() {

// Aggiorna ezTime (NTP, DST) e le variabili usate dal display
    events();                 // ezTime
    TimeUtils::updateCurrentTimeFromTZ();  // allinea currentHour/Minute/Second a myTZ

// Gestione luminosità giorno/sera e spegnimento notturno
static bool wasNightTime = false;
static bool wasSleepTime = false;
bool currentlyNightTime = TimeUtils::isNightTime();
bool currentlySleepTime = TimeUtils::isSleepTime();

    // Gestione spegnimento notturno automatico
    if (currentlySleepTime != wasSleepTime) {
       if (currentlySleepTime) {
           Serial.println("Entrata in modalità spegnimento notturno");
           displayOff = true;
           FastLED.clear();
           FastLED.show();
       } else {
           Serial.println("Uscita da modalità spegnimento notturno");
           displayOff = false;
           intBrightness = TimeUtils::isNightTime() ? nightBrightness : dayBrightness;
           FastLED.setBrightness(intBrightness);
           ClockDisplay::updateDisplay();
       }
       wasSleepTime = currentlySleepTime;
    }
    // Aggiorna la luminosità solo quando cambia la fascia oraria (e non siamo in sleep)
    if (!currentlySleepTime && currentlyNightTime != wasNightTime) {
       intBrightness = currentlyNightTime ? nightBrightness : dayBrightness;
       FastLED.setBrightness(intBrightness);
       Serial.print("Cambio fascia oraria - nuova luminosità: ");
       Serial.println(intBrightness);
       wasNightTime = currentlyNightTime;
    }
    if (scrollEnabled) {
        WebUI::handle();
        ScrollText::update();
        yield();
        
        // secondi / seconds
        if (scrollPauseActive && !pauseDelayActive) {
            return;  // Schermo spento per 2 secondi
        }
        
        // Se siamo in pausa MA il flag è disattivo, tieni schermo spento
        if (scrollPauseActive && !scrollShowPresetDuringPause) {
            FastLED.clear();
            FastLED.show();
            return;  // Schermo spento durante tutta la pausa
}

        // preset / preset
        if (scrollPauseActive && scrollShowPresetDuringPause && pauseDelayActive) {
            // Imposta il mode corretto se è cambiato
            if (lastPresetDuringPause != currentPreset) {
                lastPresetDuringPause = currentPreset;
                Presets::apply(currentPreset);
                Settings::loadModeSettings(currentPreset);
            }
            // preset / preset
        } else if (!scrollPauseActive) {
            return;  // Durante lo scroll, blocca il resto
        }
    }
   DigitalOverlay::handle();
   static uint32_t lastUpdate = 0;
   static uint32_t lastButtonCheck = 0;
   uint32_t currentMillis = millis();
  
   if(WiFi.status() == WL_CONNECTED) {
       ArduinoOTA.handle();
       events();
       
       // Gestisci le richieste del server web
       WebUI::handle();
   }
  
   if(currentMillis - lastButtonCheck > 50) {
       Buttons::check();
       lastButtonCheck = currentMillis;
   }

   if(WiFi.status() == WL_CONNECTED && currentMillis - lastUpdate > 1000) {
       uint8_t newHour = myTZ.hour();
       uint8_t newMinute = myTZ.minute();
       currentSecond = myTZ.second();

       // Controlla se è cambiato il minuto - resetta gli effetti
       if(newMinute != currentMinute || newHour != currentHour) {
           currentHour = newHour;
           currentMinute = newMinute;

           Effects::onMinuteChange();
       }

       lastUpdate = currentMillis;
   }

    // Applica la luminosità solo se non siamo in modalità sleep
   if (!currentlySleepTime) {
       FastLED.setBrightness(intBrightness);
   }

if (!displayOff) {
    // Aggiorna il colore rainbow (cicla attraverso lo spettro)
    if (textRainbowMode && currentMillis - lastRainbowUpdate > RAINBOW_SPEED) {
        rainbowHue += 3;  // Incrementa per scorrere i colori
        lastRainbowUpdate = currentMillis;
    }

    Effects::update(currentMillis);
    yield();
}
   yield();
}
