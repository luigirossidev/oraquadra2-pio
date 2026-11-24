#include "app.h"
#include <WiFi.h>
#include <ArduinoOTA.h>
#include <ezTime.h>
#include <FastLED.h>
#include "state.h"
#include "config.h"
#include "fc_config.h"
#include "settings.h"
#include "network.h"
#include "time_utils.h"
#include "presets.h"
#include "buttons.h"
#include "clock_display.h"
#include "scroll_text.h"
#include "clock_colors.h"
#include "ble_config.h"
#include "melody_player.h"
#include "daily_content.h"
#include "ota_helper.h"
#include "sound_effects.h"
#include "effects/effects.h"
#include "effects/digital_overlay.h"

static bool wifiConnected = false;
static bool ntpSynced = false;
static uint32_t lastStatusUpdate = 0;
static uint32_t lastPinScroll = 0;

// Sincronizza NTP e applica la timezone configurata (da chiamare a WiFi connesso)
static void syncTimeAndTimezone() {
    setServer("pool.ntp.org");
    waitForSync(10);
    Network::applyTimezone();
    TimeUtils::updateCurrentTimeFromTZ();
    ntpSynced = true;

    Serial.print("Ora sincronizzata: ");
    Serial.print(currentHour);
    Serial.print(":");
    Serial.println(currentMinute);
}

// Ricalcola luminosita'/spegnimento in base alla fascia oraria corrente
static void applyBrightnessForCurrentTime() {
    if (TimeUtils::isSleepTime()) {
        Serial.println("Fascia oraria: SPEGNIMENTO NOTTURNO");
        displayOff = true;
        intBrightness = 0;
        FastLED.clear();
        FastLED.show();
    } else {
        bool isNight = TimeUtils::isNightTime();
        intBrightness = isNight ? nightBrightness : dayBrightness;
        Serial.print("Fascia oraria: ");
        Serial.println(isNight ? "SERA" : "GIORNO");
    }
    FastLED.setBrightness(intBrightness);
}

// Setup principale
void App::setup() {
   Serial.begin(115200);
   Serial.println("START");

   Settings::begin();   // SPIFFS + caricamento configurazione

   // Luminosita' temporanea finche' non conosciamo l'ora
   intBrightness = dayBrightness;

   // =========================================================================
   // CONFIGURAZIONE PIN
   // =========================================================================

   #if BUTTON_LOGIC_INVERTED == 1
     pinMode(BUTTON_MODE, INPUT_PULLUP);
     pinMode(BUTTON_SEC, INPUT_PULLUP);
   #else
     pinMode(BUTTON_MODE, INPUT_PULLDOWN);
     pinMode(BUTTON_SEC, INPUT_PULLDOWN);
   #endif

   // =========================================================================
   // FASTLED
   // =========================================================================

   FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS)
          .setCorrection(TypicalLEDStrip);
   FastLED.setMaxPowerInVoltsAndMilliamps(5, LED_MAX_POWER_MA);

   FastLED.setBrightness(intBrightness);
   FastLED.clear();
   FastLED.show();

   // =========================================================================
   // BUZZER + JUKEBOX
   // =========================================================================

   buzzerInit(BUZZER_PIN);
   delay(100);
   buzzerPlay(S_HAPPY);
   melodyPlayerInit();

   // =========================================================================
   // BLE (sempre attivo per la configurazione dall'app FreeClock)
   // =========================================================================

   // Radio WiFi inizializzata prima del BLE per una corretta coesistenza
   WiFi.mode(WIFI_STA);
   WiFi.disconnect();

   bleConfigInit();

   // =========================================================================
   // WIFI + NTP
   // =========================================================================

   wifiConnected = Network::connect();

   if (wifiConnected) {
       Network::setupOTA();
       syncTimeAndTimezone();
       applyBrightnessForCurrentTime();
       buzzerPlay(S_CONNECTION);
   } else if (strlen(config.WIFI_SSID) == 0) {
       // Non configurato: mostra il PIN per il pairing con l'app
       ScrollText::show("APP FREECLOCK - PIN " + String(bleGetPinCode()), CRGB::Yellow, 50);
       lastPinScroll = millis();
   }

   bleUpdateStatus();

   // =========================================================================
   // PRESET SALVATO
   // =========================================================================

   Serial.print("Applicazione preset salvato: ");
   Serial.println(currentPreset);

   Presets::apply(currentPreset);

   // per sovrascrivere i valori di default con quelli salvati dall'utente
   Settings::loadModeSettings(currentPreset);

   Serial.println("Setup completato!");
   Serial.println("=====================================");
}

// Gestione dei flag impostati dalle write BLE (gli effetti visivi si applicano
// qui, sul core del loop, mai nel task BLE)
static void handleBleRequests() {
    // Nuove credenziali WiFi o richiesta di riconnessione dall'app
    if (bleConfigNeedsReconnect()) {
        bleConfigClearReconnect();
        Serial.println("BLE: riconnessione WiFi richiesta");
        wifiConnected = false;
        WiFi.disconnect();
        wifiConnected = Network::connect();
        if (wifiConnected) {
            Network::setupOTA();
            if (!ntpSynced) syncTimeAndTimezone();
            applyBrightnessForCurrentTime();
            buzzerPlay(S_CONNECTION);
        }
        bleUpdateStatus();
    }

    // Scansione reti WiFi richiesta dal wizard dell'app
    if (bleConfigNeedsScan()) {
        bleConfigClearScan();
        performWiFiScan();
    }

    // Cambio timezone
    if (bleConfigNeedsTimezoneUpdate()) {
        bleConfigClearTimezoneUpdate();
        Network::applyTimezone();
        TimeUtils::updateCurrentTimeFromTZ();
    }

    // Aggiornamento firmware dal server FreeClock
    if (bleConfigNeedsOTAUpdate()) {
        bleConfigClearOTAUpdate();
        if (wifiConnected) {
            checkAndUpdateFirmware();
        }
    }

    // Impostazioni orologio cambiate (preset, colore, luminosita', fasce, power)
    if (bleConfigNeedsClockApply()) {
        bool presetChanged = bleConfigPresetChanged();
        bleConfigClearClockApply();

        if (presetChanged) {
            Presets::apply(currentPreset);
            Settings::loadModeSettings(currentPreset);
        }

        if (displayOff && !sleepModeEnabled) {
            // Spegnimento manuale da app (power off)
            FastLED.clear();
            FastLED.show();
        } else if (TimeUtils::isSleepTime()) {
            displayOff = true;
            FastLED.clear();
            FastLED.show();
        } else if (!displayOff) {
            intBrightness = TimeUtils::isNightTime() ? nightBrightness : dayBrightness;
            FastLED.setBrightness(intBrightness);
            ClockDisplay::updateDisplay();
        }
    }

    // Impostazioni testo scorrevole cambiate
    if (bleConfigNeedsScrollApply()) {
        bleConfigClearScrollApply();
        scrollTextInitialized = false;
        scrollTextOffset = MATRIX_WIDTH;
        if (!scrollEnabled) {
            // Ferma subito lo scroll e torna all'orologio
            scrollPauseActive = false;
            FastLED.clear();
            ClockDisplay::showCurrentTime();
            FastLED.show();
        }
    }

    // Jukebox: accoda la melodia al task player (core 0)
    if (bleConfigNeedsMelodyPlay()) {
        MelodyRequest req = bleConfigGetMelody();
        bleConfigClearMelodyPlay();
        melodyPlayerEnqueue(req);
    }

    // Messaggio del jukebox da far scorrere sulla matrice
    if (melodyPlayerMessagePending() && !displayOff) {
        String msg = melodyPlayerTakeMessage();
        ScrollText::show(ScrollText::sanitize(msg), CRGB::HotPink, scrollTextSpeedMs);
    }
}

void App::loop() {

// Aggiorna ezTime (NTP, DST) e le variabili usate dal display
    events();                 // ezTime
    TimeUtils::updateCurrentTimeFromTZ();  // allinea currentHour/Minute/Second a myTZ

// Richieste arrivate via BLE dall'app FreeClock
    handleBleRequests();

// Frase/indovinello del giorno (fetch giornaliero + richieste del pulsante SEC)
    DailyContent::handle();

// Stato BLE per l'app ogni 10 secondi
    if (millis() - lastStatusUpdate > 10000) {
        lastStatusUpdate = millis();
        bleUpdateStatus();
    }

// Non configurato: riproponi il PIN sulla matrice ogni 60 secondi
    if (!wifiConnected && strlen(config.WIFI_SSID) == 0
        && millis() - lastPinScroll > 60000) {
        lastPinScroll = millis();
        ScrollText::show("APP FREECLOCK - PIN " + String(bleGetPinCode()), CRGB::Yellow, 50);
    }

// Gestione luminosita' giorno/sera e spegnimento notturno
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
