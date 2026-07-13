#include "settings.h"
#include "fc_config.h"
#include "config.h"
#include "state.h"
#include "clock_colors.h"

// ============================================================================
// PERSISTENZA IMPOSTAZIONI (SPIFFS /settings.json, vedi fc_config.cpp)
// ============================================================================

// Salva colore/rainbow correnti per il preset indicato
void Settings::saveModeSettings(uint8_t mode) {
    if (mode >= FC_NUM_PRESETS) return;
    config.presets[mode].saved   = true;
    config.presets[mode].r       = clockColors.current.r;
    config.presets[mode].g       = clockColors.current.g;
    config.presets[mode].b       = clockColors.current.b;
    config.presets[mode].rainbow = textRainbowMode;
    writeConfigurations();
    Serial.print("Salvate impostazioni per modalita ");
    Serial.print(mode);
    Serial.print(" - R=");
    Serial.print(clockColors.current.r);
    Serial.print(" G=");
    Serial.print(clockColors.current.g);
    Serial.print(" B=");
    Serial.print(clockColors.current.b);
    Serial.print(" Rainbow=");
    Serial.println(textRainbowMode);
}

// Carica colore/rainbow salvati per il preset indicato
void Settings::loadModeSettings(uint8_t mode) {
    if (mode >= FC_NUM_PRESETS) return;

    if (!config.presets[mode].saved) {
        Serial.print("Nessuna impostazione salvata per modalita ");
        Serial.println(mode);
        return;
    }

    clockColors.current = CRGB(config.presets[mode].r,
                               config.presets[mode].g,
                               config.presets[mode].b);
    textRainbowMode = config.presets[mode].rainbow;
}

void Settings::begin() {
    initConfigurations();
    loadConfigurations();

    Serial.println("=== IMPOSTAZIONI CARICATE DA SPIFFS ===");
    Serial.print("Preset: ");
    Serial.println(currentPreset);
    Serial.print("Blink: ");
    Serial.println(currentBlink);
    Serial.print("Giorno inizio: ");
    Serial.print(dayStartHour);
    Serial.print(":");
    Serial.println(dayStartMinute);
    Serial.print("Notte inizio: ");
    Serial.print(nightStartHour);
    Serial.print(":");
    Serial.println(nightStartMinute);
    Serial.print("Luminosita' giorno: ");
    Serial.println(dayBrightness);
    Serial.print("Luminosita' notte: ");
    Serial.println(nightBrightness);
    Serial.print("Spegnimento notturno: ");
    Serial.print(sleepStartHour);
    Serial.print(":");
    Serial.println(sleepStartMinute);
    Serial.print("Spegnimento abilitato: ");
    Serial.println(sleepModeEnabled);
    Serial.print("Velocita' testo scorrevole: ");
    Serial.print(scrollTextSpeedMs);
    Serial.println(" ms");
    Serial.print("WiFi SSID: ");
    Serial.println(config.WIFI_SSID);
    Serial.print("Timezone: ");
    Serial.println(config.TIMEZONE);
    Serial.println("=====================================");
}
