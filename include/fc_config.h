#pragma once

#include <Arduino.h>

// Configurazione persistente su SPIFFS (/settings.json), stile FreeClock.
// Le impostazioni di visualizzazione (preset, colori, fasce orarie, scroll...)
// vivono nelle variabili globali di state.h: loadConfigurations() le popola
// dal JSON, writeConfigurations() le serializza. Questa struct contiene solo
// i campi senza un corrispettivo in state.h (connettivita', frase/quiz, preset colori).

// Versione firmware mostrata nell'app mobile e usata per l'OTA da server
extern const char FIRMWARE_VERSION[];
extern const int  FIRMWARE_CODE;

#define FC_NUM_PRESETS 20

struct PresetColorEntry {
    bool    saved;
    uint8_t r, g, b;
    bool    rainbow;
};

struct FcConfig {
    // Connettivita'
    char     WIFI_SSID[64];
    char     WIFI_PASSWORD[64];
    uint32_t blePinCode;          // 0 = non ancora generato
    String   TIMEZONE;            // stringa POSIX (l'app invia direttamente il formato POSIX)

    // Frase e indovinello del giorno
    String   quoteUrl;
    String   quizUrl;
    int      quizHour;            // ora a partire dalla quale mostrare la soluzione
    bool     isQuote;
    bool     isQuiz;

    // Colore/rainbow salvati per ciascun preset (sostituisce i 20x5 byte EEPROM)
    PresetColorEntry presets[FC_NUM_PRESETS];
};

extern FcConfig config;

void initConfigurations();   // monta SPIFFS
void loadConfigurations();   // JSON -> config + globali di state.h
void writeConfigurations();  // config + globali di state.h -> JSON
void factoryResetConfigurations();  // cancella settings.json
