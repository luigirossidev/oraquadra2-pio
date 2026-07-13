#include "fc_config.h"
#include <ArduinoJson.h>
#include <SPIFFS.h>
#include <FastLED.h>
#include "state.h"
#include "clock_colors.h"

const char FIRMWARE_VERSION[] = "2.0.0";
const int  FIRMWARE_CODE      = 1;

FcConfig config;

static const char* SETTINGS_PATH = "/settings.json";
static const char* SETTINGS_TMP  = "/settings_tmp.json";

void initConfigurations() {
    if (!SPIFFS.begin(true)) {
        Serial.println("SPIFFS: errore durante il mount");
        return;
    }
    delay(10);
}

void loadConfigurations() {
    JsonDocument doc;

    File configFile = SPIFFS.open(SETTINGS_PATH, "r");
    if (configFile) {
        DeserializationError error = deserializeJson(doc, configFile);
        if (error) {
            Serial.println(F("settings.json illeggibile, uso i default"));
        } else {
            Serial.println("Configurazione caricata da settings.json");
        }
        configFile.close();
    } else {
        Serial.println("Nessun settings.json: uso i default (prima configurazione)");
    }

    // --- Connettivita' ---
    strlcpy(config.WIFI_SSID, doc["WIFI_SSID"] | "", sizeof(config.WIFI_SSID));
    strlcpy(config.WIFI_PASSWORD, doc["WIFI_PASSWORD"] | "", sizeof(config.WIFI_PASSWORD));
    config.blePinCode = doc["blePinCode"] | (uint32_t)0;
    config.TIMEZONE = doc["TIMEZONE"] | "CET-1CEST,M3.5.0,M10.5.0/3";  // Europe/Rome

    // --- Frase e indovinello ---
    config.quoteUrl = doc["quoteUrl"] | "https://freeclock.matesemakers.it";
    config.quizUrl  = doc["quizUrl"]  | "https://freeclock.matesemakers.it";
    config.quizHour = doc["quizHour"] | 18;
    config.isQuote  = doc["isQuote"]  | true;
    config.isQuiz   = doc["isQuiz"]   | true;

    // --- Stato orologio (globali di state.h) ---
    currentPreset = doc["preset"] | 0;
    if (currentPreset >= FC_NUM_PRESETS) currentPreset = 0;
    currentBlink = doc["blink"] | 1;

    uint8_t r = doc["colorR"] | 255;
    uint8_t g = doc["colorG"] | 255;
    uint8_t b = doc["colorB"] | 255;
    clockColors.current = CRGB(r, g, b);
    clockColors.presetIndex = doc["colorPreset"] | 0;

    dayStartHour     = doc["dayStartHour"]     | 7;
    dayStartMinute   = doc["dayStartMinute"]   | 0;
    nightStartHour   = doc["nightStartHour"]   | 22;
    nightStartMinute = doc["nightStartMinute"] | 0;
    sleepStartHour   = doc["sleepStartHour"]   | 23;
    sleepStartMinute = doc["sleepStartMinute"] | 0;
    sleepModeEnabled = doc["sleepEnabled"]     | false;
    dayBrightness    = doc["dayBrightness"]    | 204;   // 80%
    nightBrightness  = doc["nightBrightness"]  | 26;    // 10%

    digitalOverlayEnabled = doc["digitalOverlay"] | true;
    digitaleInterval      = doc["digitalInterval"] | 30;
    digitaleDuration      = doc["digitalDuration"] | 10;
    if (digitaleInterval < 5 || digitaleInterval > 120) digitaleInterval = 30;
    if (digitaleDuration < 10 || digitaleDuration > 59) digitaleDuration = 10;

    // --- Testo scorrevole ---
    scrollEnabled      = doc["scrollEnabled"] | false;
    scrollEnabledRaw   = scrollEnabled ? 1 : 0;
    customScrollText   = doc["scrollText"] | "";
    scrollTextSpeedMs  = doc["scrollSpeed"] | 50;
    if (scrollTextSpeedMs < 10 || scrollTextSpeedMs > 500) scrollTextSpeedMs = 50;
    scrollFontSize     = doc["scrollFontSize"] | 2;
    if (scrollFontSize < 1 || scrollFontSize > 3) scrollFontSize = 2;
    scrollPauseSeconds = doc["scrollPause"] | 3;
    if (scrollPauseSeconds > 60) scrollPauseSeconds = 3;
    scrollShowText     = doc["scrollShowText"] | true;
    scrollShowDate     = doc["scrollShowDate"] | false;
    scrollShowTime     = doc["scrollShowTime"] | false;
    scrollTextualDateTime       = doc["scrollTextual"] | false;
    scrollShowPresetDuringPause = doc["scrollShowPreset"] | true;

    uint8_t sr = doc["scrollColorR"] | 255;
    uint8_t sg = doc["scrollColorG"] | 255;
    uint8_t sb = doc["scrollColorB"] | 255;
    scrollTextColor   = CRGB(sr, sg, sb);
    scrollTextRainbow = doc["scrollRainbow"] | false;

    // --- Colori per preset ---
    JsonArray arr = doc["presetColors"].as<JsonArray>();
    for (int i = 0; i < FC_NUM_PRESETS; i++) {
        if (!arr.isNull() && i < (int)arr.size()) {
            JsonObject o = arr[i];
            config.presets[i].saved   = o["s"] | false;
            config.presets[i].r       = o["r"] | 255;
            config.presets[i].g       = o["g"] | 255;
            config.presets[i].b       = o["b"] | 255;
            config.presets[i].rainbow = o["w"] | false;
        } else {
            config.presets[i] = { false, 255, 255, 255, false };
        }
    }
}

void writeConfigurations() {
    File file = SPIFFS.open(SETTINGS_TMP, FILE_WRITE);
    if (!file) {
        Serial.println(F("Impossibile creare il file temporaneo per la configurazione"));
        return;
    }

    JsonDocument doc;

    doc["WIFI_SSID"]     = config.WIFI_SSID;
    doc["WIFI_PASSWORD"] = config.WIFI_PASSWORD;
    doc["blePinCode"]    = config.blePinCode;
    doc["TIMEZONE"]      = config.TIMEZONE;

    doc["quoteUrl"] = config.quoteUrl;
    doc["quizUrl"]  = config.quizUrl;
    doc["quizHour"] = config.quizHour;
    doc["isQuote"]  = config.isQuote;
    doc["isQuiz"]   = config.isQuiz;

    doc["preset"]      = currentPreset;
    doc["blink"]       = currentBlink;
    doc["colorR"]      = clockColors.current.r;
    doc["colorG"]      = clockColors.current.g;
    doc["colorB"]      = clockColors.current.b;
    doc["colorPreset"] = clockColors.presetIndex;

    doc["dayStartHour"]     = dayStartHour;
    doc["dayStartMinute"]   = dayStartMinute;
    doc["nightStartHour"]   = nightStartHour;
    doc["nightStartMinute"] = nightStartMinute;
    doc["sleepStartHour"]   = sleepStartHour;
    doc["sleepStartMinute"] = sleepStartMinute;
    doc["sleepEnabled"]     = sleepModeEnabled;
    doc["dayBrightness"]    = dayBrightness;
    doc["nightBrightness"]  = nightBrightness;

    doc["digitalOverlay"]   = digitalOverlayEnabled;
    doc["digitalInterval"]  = digitaleInterval;
    doc["digitalDuration"]  = digitaleDuration;

    doc["scrollEnabled"]    = scrollEnabled;
    doc["scrollText"]       = customScrollText;
    doc["scrollSpeed"]      = scrollTextSpeedMs;
    doc["scrollFontSize"]   = scrollFontSize;
    doc["scrollPause"]      = scrollPauseSeconds;
    doc["scrollShowText"]   = scrollShowText;
    doc["scrollShowDate"]   = scrollShowDate;
    doc["scrollShowTime"]   = scrollShowTime;
    doc["scrollTextual"]    = scrollTextualDateTime;
    doc["scrollShowPreset"] = scrollShowPresetDuringPause;
    doc["scrollColorR"]     = scrollTextColor.r;
    doc["scrollColorG"]     = scrollTextColor.g;
    doc["scrollColorB"]     = scrollTextColor.b;
    doc["scrollRainbow"]    = scrollTextRainbow;

    JsonArray arr = doc["presetColors"].to<JsonArray>();
    for (int i = 0; i < FC_NUM_PRESETS; i++) {
        JsonObject o = arr.add<JsonObject>();
        o["s"] = config.presets[i].saved;
        o["r"] = config.presets[i].r;
        o["g"] = config.presets[i].g;
        o["b"] = config.presets[i].b;
        o["w"] = config.presets[i].rainbow;
    }

    if (serializeJson(doc, file) == 0) {
        Serial.println(F("Scrittura configurazione fallita"));
        file.close();
        SPIFFS.remove(SETTINGS_TMP);
        return;
    }

    file.close();

    // Swap quasi-atomico: rimuovi il vecchio, rinomina il nuovo
    SPIFFS.remove(SETTINGS_PATH);
    SPIFFS.rename(SETTINGS_TMP, SETTINGS_PATH);
}

void factoryResetConfigurations() {
    SPIFFS.remove(SETTINGS_PATH);
}
