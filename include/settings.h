#pragma once

#include <Arduino.h>

// Persistenza impostazioni su EEPROM
class Settings {
public:
    static void begin();   // EEPROM.begin + prima configurazione + caricamento valori
    static void saveModeSettings(uint8_t mode);
    static void loadModeSettings(uint8_t mode);
};
