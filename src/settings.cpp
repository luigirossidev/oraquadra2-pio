#include "settings.h"
#include <EEPROM.h>
#include "eeprom_map.h"
#include "config.h"
#include "state.h"
#include "clock_colors.h"

// ============================================================================
// FUNZIONI PER GESTIONE IMPOSTAZIONI PER MODALITÀ
// ============================================================================

// Salva le impostazioni correnti / Save current settings
void Settings::saveModeSettings(uint8_t mode) {
    if (mode >= NUM_MODES) return;
    uint16_t baseAddr = EEPROM_MODE_SETTINGS_START + (mode * EEPROM_MODE_SETTINGS_SIZE);
    EEPROM.write(baseAddr, EEPROM_MODE_MARKER);  // Marker per indicare che è stato salvato
    EEPROM.write(baseAddr + 1, clockColors.current.r);
    EEPROM.write(baseAddr + 2, clockColors.current.g);
    EEPROM.write(baseAddr + 3, clockColors.current.b);
    EEPROM.write(baseAddr + 4, textRainbowMode ? 1 : 0);
    EEPROM.commit();
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

// per la modalità specificata / for specified mode
void Settings::loadModeSettings(uint8_t mode) {
    if (mode >= NUM_MODES) return;
    uint16_t baseAddr = EEPROM_MODE_SETTINGS_START + (mode * EEPROM_MODE_SETTINGS_SIZE);

    uint8_t marker = EEPROM.read(baseAddr);

    // Verifica se questa modalità ha impostazioni salvate / Check if this mode has saved settings
    if (marker != EEPROM_MODE_MARKER) {
        // Nessuna impostazione salvata / No saved settings
        Serial.print("Nessuna impostazione salvata per modalita ");
        Serial.println(mode);
        return;
    }

    uint8_t r = EEPROM.read(baseAddr + 1);
    uint8_t g = EEPROM.read(baseAddr + 2);
    uint8_t b = EEPROM.read(baseAddr + 3);
    uint8_t rainbow = EEPROM.read(baseAddr + 4);
    clockColors.current = CRGB(r, g, b);
    textRainbowMode = (rainbow == 1);

}

void Settings::begin() {
   EEPROM.begin(EEPROM_SIZE);

   // =========================================================================
   // se necessario / if needed
   // =========================================================================
   if (EEPROM.read(0) != EEPROM_CONFIGURED_MARKER) {
       Serial.println("Inizializzazione EEPROM - Prima configurazione");
       
       EEPROM.write(0, EEPROM_CONFIGURED_MARKER);
       EEPROM.write(EEPROM_PRESET_ADDR, 0);
       EEPROM.write(EEPROM_BLINK_ADDR, 1);
       
       // Valori default per luminosità e colore
       EEPROM.write(EEPROM_BRIGHTNESS_ADDR, 128);
       EEPROM.write(EEPROM_COLOR_R_ADDR, 255);
       EEPROM.write(EEPROM_COLOR_G_ADDR, 255);
       EEPROM.write(EEPROM_COLOR_B_ADDR, 255);
       EEPROM.write(EEPROM_COLOR_PRESET_ADDR, 0);
       
       // Valori default per gestione giorno/sera/notte
       EEPROM.write(EEPROM_DAY_START_HOUR_ADDR, 7);
       EEPROM.write(EEPROM_DAY_START_MINUTE_ADDR, 0);
       EEPROM.write(EEPROM_NIGHT_START_HOUR_ADDR, 22);
       EEPROM.write(EEPROM_NIGHT_START_MINUTE_ADDR, 0);
       EEPROM.write(EEPROM_SLEEP_START_HOUR_ADDR, 23);  // Corretto: 23:00 come default
       EEPROM.write(EEPROM_SLEEP_START_MINUTE_ADDR, 0);
       EEPROM.write(EEPROM_SLEEP_ENABLED_ADDR, 0);
       EEPROM.write(EEPROM_DAY_BRIGHTNESS_ADDR, 204);    // 80%
       EEPROM.write(EEPROM_NIGHT_BRIGHTNESS_ADDR, 26);   // 10%
       EEPROM.write(EEPROM_DIGITAL_OVERLAY_ADDR, 1);     // overlay digitale ATTIVO di default
       EEPROM.write(EEPROM_DIGITAL_START_ADDR, 30);       // ogni 30 secondi
       EEPROM.write(EEPROM_DIGITAL_DURATION_ADDR, 10);    // dura 10 secondi
       
       // per testo scorrevole / for scrolling text
       EEPROM.write(EEPROM_SCROLL_PAUSE_ADDR, 3);        // pausa 3 secondi
       uint16_t defaultSpeed = 50;                      // velocità default 50ms
       EEPROM.write(EEPROM_SCROLL_SPEED_ADDR, defaultSpeed & 0xFF);
       EEPROM.write(EEPROM_SCROLL_SPEED_ADDR + 1, (defaultSpeed >> 8) & 0xFF);
       EEPROM.write(EEPROM_SCROLL_SHOW_TEXT_ADDR, 1);    // mostra testo
       EEPROM.write(EEPROM_SCROLL_SHOW_DATE_ADDR, 0);    // non mostra data
       EEPROM.write(EEPROM_SCROLL_SHOW_TIME_ADDR, 0);    // non mostra ora
       EEPROM.write(EEPROM_SCROLL_FONT_SIZE_ADDR, 2);    // font medio
       EEPROM.write(EEPROM_SCROLL_SHOW_PRESET_ADDR, 1);  // mostra preset durante pausa
       EEPROM.write(EEPROM_SCROLL_TEXTUAL_DATETIME_ADDR, 0);  // data/ora numerica
       EEPROM.write(EEPROM_SCROLL_ENABLED_ADDR, 0); // default scroll disabilitato

       EEPROM.commit();
       
       Serial.println("EEPROM inizializzata con valori default");
   }
   
   // =========================================================================
   // FASE 2: CARICAMENTO VALORI DA EEPROM
   // =========================================================================
   
   // preset / preset
   currentPreset = EEPROM.read(EEPROM_PRESET_ADDR);
   currentBlink = EEPROM.read(EEPROM_BLINK_ADDR);
   
   // Carica luminosità salvata (questa non verrà più usata, ma la teniamo per compatibilità)
   uint8_t savedBrightness = EEPROM.read(EEPROM_BRIGHTNESS_ADDR);
   
   // Carica colore salvato
   uint8_t savedR = EEPROM.read(EEPROM_COLOR_R_ADDR);
   uint8_t savedG = EEPROM.read(EEPROM_COLOR_G_ADDR);
   uint8_t savedB = EEPROM.read(EEPROM_COLOR_B_ADDR);
   uint8_t savedPreset = EEPROM.read(EEPROM_COLOR_PRESET_ADDR);
   uint8_t scrollR = EEPROM.read(EEPROM_SCROLLTEXT_R_ADDR);
   uint8_t scrollG = EEPROM.read(EEPROM_SCROLLTEXT_G_ADDR);
   uint8_t scrollB = EEPROM.read(EEPROM_SCROLLTEXT_B_ADDR);
   if(scrollR != 255 || scrollG != 255 || scrollB != 255) {
       scrollTextColor = CRGB(scrollR, scrollG, scrollB);
   }
   scrollTextRainbow = EEPROM.read(EEPROM_SCROLLTEXT_RAINBOW_ADDR) == 1;

   clockColors.current = CRGB(savedR, savedG, savedB);
   clockColors.presetIndex = savedPreset;
   
   // Carica impostazioni giorno/notte
   dayStartHour = EEPROM.read(EEPROM_DAY_START_HOUR_ADDR);
   dayStartMinute = EEPROM.read(EEPROM_DAY_START_MINUTE_ADDR);
   nightStartHour = EEPROM.read(EEPROM_NIGHT_START_HOUR_ADDR);
   nightStartMinute = EEPROM.read(EEPROM_NIGHT_START_MINUTE_ADDR);
   dayBrightness = EEPROM.read(EEPROM_DAY_BRIGHTNESS_ADDR);
   nightBrightness = EEPROM.read(EEPROM_NIGHT_BRIGHTNESS_ADDR);
   sleepStartHour = EEPROM.read(EEPROM_SLEEP_START_HOUR_ADDR);
   sleepStartMinute = EEPROM.read(EEPROM_SLEEP_START_MINUTE_ADDR);
   sleepModeEnabled = (EEPROM.read(EEPROM_SLEEP_ENABLED_ADDR) == 1);
   digitalOverlayEnabled = (EEPROM.read(EEPROM_DIGITAL_OVERLAY_ADDR) == 1);
   digitaleInterval = EEPROM.read(EEPROM_DIGITAL_START_ADDR);
   digitaleDuration = EEPROM.read(EEPROM_DIGITAL_DURATION_ADDR);
   scrollPauseSeconds = EEPROM.read(EEPROM_SCROLL_PAUSE_ADDR);
   
   // byte / byte
   scrollTextSpeedMs = EEPROM.read(EEPROM_SCROLL_SPEED_ADDR) | (EEPROM.read(EEPROM_SCROLL_SPEED_ADDR_H) << 8);
   if (scrollTextSpeedMs < 10 || scrollTextSpeedMs > 500) {
       scrollTextSpeedMs = 50; // default
   }
   
   scrollFontSize = EEPROM.read(EEPROM_SCROLL_FONT_SIZE_ADDR);
   scrollShowText = (EEPROM.read(EEPROM_SCROLL_SHOW_TEXT_ADDR) == 1);
   scrollShowDate = (EEPROM.read(EEPROM_SCROLL_SHOW_DATE_ADDR) == 1);
   scrollShowTime = (EEPROM.read(EEPROM_SCROLL_SHOW_TIME_ADDR) == 1);

   scrollShowPresetDuringPause = (EEPROM.read(EEPROM_SCROLL_SHOW_PRESET_ADDR) == 1);
   scrollTextualDateTime = (EEPROM.read(EEPROM_SCROLL_TEXTUAL_DATETIME_ADDR) == 1);
   scrollEnabledRaw = EEPROM.read(EEPROM_SCROLL_ENABLED_ADDR);
   scrollEnabled = (scrollEnabledRaw == 1);  // convertire in bool

   // Se EEPROM non inizializzata, default true per showPreset
   if (EEPROM.read(EEPROM_SCROLL_SHOW_PRESET_ADDR) == 255) scrollShowPresetDuringPause = true;
   
   // Se EEPROM non inizializzata per scrollShowText, default true
   if (EEPROM.read(EEPROM_SCROLL_SHOW_TEXT_ADDR) == 255) scrollShowText = true;
   // Validazione / Validation
   if (digitaleInterval < 5 || digitaleInterval > 120) digitaleInterval = 30;
   if (digitaleDuration < 10 || digitaleDuration > 59) digitaleDuration = 10;
   if (scrollPauseSeconds > 60) scrollPauseSeconds = 3;
   if (scrollFontSize < 1 || scrollFontSize > 3) scrollFontSize = 2;  // Default: medio

   // Carica testo scorrevole da EEPROM (PRIMA di Presets::apply per averlo disponibile)
   uint8_t textLen = EEPROM.read(EEPROM_SCROLL_TEXT_LEN_ADDR);
   if (textLen > 0 && textLen <= EEPROM_SCROLL_TEXT_MAX_LEN) {
       customScrollText = "";
       for (uint8_t i = 0; i < textLen; i++) {
           char c = EEPROM.read(EEPROM_SCROLL_TEXT_START_ADDR + i);
           if (c >= 32 && c <= 126) {  // Solo caratteri stampabili
               customScrollText += c;
           }
       }
       Serial.print("Testo scorrevole caricato: ");
       Serial.println(customScrollText);
   }

   Serial.println("=== IMPOSTAZIONI CARICATE DA EEPROM ===");
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
   Serial.print("Luminosità giorno: ");
   Serial.println(dayBrightness);
   Serial.print("Luminosità notte: ");
   Serial.println(nightBrightness);
   Serial.print("Spegnimento notturno: ");
   Serial.print(sleepStartHour);
   Serial.print(":");
   Serial.println(sleepStartMinute);
   Serial.print("Spegnimento abilitato: ");
   Serial.println(sleepModeEnabled);
   Serial.print("Velocità testo scorrevole: ");
   Serial.print(scrollTextSpeedMs);
   Serial.println(" ms");
   Serial.println("=====================================");
}
