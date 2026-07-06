#pragma once

// Strutture e costanti per EEPROM / EEPROM structures and constants
#define EEPROM_SIZE 512
#define EEPROM_DIGITAL_OVERLAY_ADDR 40
#define EEPROM_DIGITAL_START_ADDR 41
#define EEPROM_DIGITAL_DURATION_ADDR 42
#define EEPROM_SCROLL_PAUSE_ADDR 43
#define EEPROM_SCROLL_SPEED_ADDR       44   // LOW BYTE
#define EEPROM_SCROLL_SPEED_ADDR_H     23   // HIGH BYTE (NUOVO → FIX BUG 306ms)
#define EEPROM_SCROLL_SHOW_DATE_ADDR 45
#define EEPROM_SCROLL_SHOW_TIME_ADDR 46
#define EEPROM_SCROLL_SHOW_TEXT_ADDR 47     // Mostra testo personalizzato
#define EEPROM_SCROLL_TEXT_LEN_ADDR 48      // Lunghezza del testo (1 byte, max 200)
#define EEPROM_SCROLL_TEXT_START_ADDR 49    // Inizio testo scorrevole (49-248, max 200 caratteri)
#define EEPROM_SCROLL_TEXT_MAX_LEN 200
#define EEPROM_SCROLL_FONT_SIZE_ADDR 249    // Dimensione font testo scorrevole (1=piccolo, 2=medio, 3=grande)
// Impostazioni per modalità: 250-349 (20 preset × 5 byte = 100 byte)
// Ogni preset: Marker(1) + R(1) + G(1) + B(1) + Rainbow(1) = 5 byte
#define EEPROM_MODE_SETTINGS_START 250
#define EEPROM_MODE_SETTINGS_SIZE 5         // Byte per ogni modalità
#define EEPROM_MODE_MARKER 0xAA             // Marker per indicare che la modalità è stata salvata
#define NUM_MODES 20                        // Numero totale di preset (0-19)
#define EEPROM_CONFIGURED_MARKER 0x55
#define EEPROM_PRESET_ADDR 1
#define EEPROM_BLINK_ADDR 2
#define EEPROM_BRIGHTNESS_ADDR 9
#define EEPROM_COLOR_R_ADDR 10
#define EEPROM_COLOR_G_ADDR 11
#define EEPROM_COLOR_B_ADDR 12
#define EEPROM_COLOR_PRESET_ADDR 13
#define EEPROM_DAY_BRIGHTNESS_ADDR 14
#define EEPROM_NIGHT_BRIGHTNESS_ADDR 15
#define EEPROM_DAY_START_HOUR_ADDR 16
#define EEPROM_DAY_START_MINUTE_ADDR 17
#define EEPROM_NIGHT_START_HOUR_ADDR 18     
#define EEPROM_NIGHT_START_MINUTE_ADDR 19
#define EEPROM_SLEEP_START_HOUR_ADDR 20
#define EEPROM_SLEEP_START_MINUTE_ADDR 21
#define EEPROM_SLEEP_ENABLED_ADDR 22   
#define EEPROM_SCROLL_SHOW_PRESET_ADDR 350    // Mostra preset durante pausa
#define EEPROM_SCROLL_TEXTUAL_DATETIME_ADDR 351 // Data/ora testuale
#define EEPROM_SCROLL_ENABLED_ADDR 352
#define EEPROM_SCROLLTEXT_R_ADDR 353
#define EEPROM_SCROLLTEXT_G_ADDR 354
#define EEPROM_SCROLLTEXT_B_ADDR 355
#define EEPROM_SCROLLTEXT_RAINBOW_ADDR 356
#define GIORNO 64     // full on
#define NOTTE 32       // half on
