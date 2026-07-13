#pragma once

#include <Arduino.h>

// Persistenza impostazioni su SPIFFS (/settings.json, vedi fc_config)
class Settings {
public:
    static void begin();   // mount SPIFFS + caricamento configurazione nelle globali
    static void saveModeSettings(uint8_t mode);
    static void loadModeSettings(uint8_t mode);
};
