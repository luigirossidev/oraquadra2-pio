#include "ble_config.h"
#include <NimBLEDevice.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <esp_mac.h>
#include <FastLED.h>
#include "fc_config.h"
#include "state.h"
#include "config.h"
#include "clock_colors.h"
#include "settings.h"
#include "scroll_text.h"
#include "daily_content.h"

// Porting di BLEConfig.h di FreeClock, ridotto alle characteristic pertinenti
// per Oraquadra + tre characteristic dedicate (clock, scroll, overlay digitale).

// Service UUID (identico a FreeClock: l'app scansiona per questo)
#define BLE_SERVICE_UUID          "FC000001-1234-5678-9ABC-DEFF0EEC10C0"

// Characteristic UUID condivise con FreeClock
#define CHAR_WIFI_UUID            "FC000010-1234-5678-9ABC-DEFF0EEC10C0"
#define CHAR_WIFI_SCAN_UUID       "FC000011-1234-5678-9ABC-DEFF0EEC10C0"
#define CHAR_TIMEWEATHER_UUID     "FC000030-1234-5678-9ABC-DEFF0EEC10C0"
#define CHAR_QUOTEQUIZ_UUID       "FC000060-1234-5678-9ABC-DEFF0EEC10C0"
#define CHAR_STATUS_UUID          "FC000090-1234-5678-9ABC-DEFF0EEC10C0"
#define CHAR_COMMAND_UUID         "FC0000A0-1234-5678-9ABC-DEFF0EEC10C0"
#define CHAR_AUTH_UUID            "FC0000B0-1234-5678-9ABC-DEFF0EEC10C0"
#define CHAR_MELODY_UUID          "FC0000C0-1234-5678-9ABC-DEFF0EEC10C0"

// Characteristic specifiche Oraquadra
#define CHAR_OQ_CLOCK_UUID        "FC0000D0-1234-5678-9ABC-DEFF0EEC10C0"
#define CHAR_OQ_SCROLL_UUID       "FC0000E0-1234-5678-9ABC-DEFF0EEC10C0"
#define CHAR_OQ_DIGITAL_UUID      "FC0000F0-1234-5678-9ABC-DEFF0EEC10C0"

static char bleDeviceName[24] = "";

// Flag di stato consumati dal loop principale
static bool _bleNeedsReconnect = false;
static bool _bleNeedsScan = false;
static bool _bleNeedsTimezoneUpdate = false;
static bool _bleNeedsOTAUpdate = false;
static bool _bleNeedsClockApply = false;
static bool _blePresetChanged = false;
static bool _bleNeedsScrollApply = false;
static volatile bool _bleNeedsMelodyPlay = false;
static MelodyRequest _pendingMelodyRequest = {};
static bool _authenticated = false;
static uint32_t _blePinCode = 0;

// Puntatori per le notify
static NimBLECharacteristic* statusChar = nullptr;
static NimBLECharacteristic* wifiScanChar = nullptr;

bool bleConfigNeedsReconnect()      { return _bleNeedsReconnect; }
void bleConfigClearReconnect()      { _bleNeedsReconnect = false; }
bool bleConfigNeedsScan()           { return _bleNeedsScan; }
void bleConfigClearScan()           { _bleNeedsScan = false; }
bool bleConfigNeedsTimezoneUpdate() { return _bleNeedsTimezoneUpdate; }
void bleConfigClearTimezoneUpdate() { _bleNeedsTimezoneUpdate = false; }
bool bleConfigNeedsOTAUpdate()      { return _bleNeedsOTAUpdate; }
void bleConfigClearOTAUpdate()      { _bleNeedsOTAUpdate = false; }
bool bleConfigNeedsClockApply()     { return _bleNeedsClockApply; }
void bleConfigClearClockApply()     { _bleNeedsClockApply = false; _blePresetChanged = false; }
bool bleConfigPresetChanged()       { return _blePresetChanged; }
bool bleConfigNeedsScrollApply()    { return _bleNeedsScrollApply; }
void bleConfigClearScrollApply()    { _bleNeedsScrollApply = false; }
bool bleConfigNeedsMelodyPlay()     { return _bleNeedsMelodyPlay; }
void bleConfigClearMelodyPlay()     { _bleNeedsMelodyPlay = false; }
MelodyRequest bleConfigGetMelody()  { return _pendingMelodyRequest; }
uint32_t bleGetPinCode()            { return _blePinCode; }

// ==================== Helper ====================

static String getEncryptionType(wifi_auth_mode_t authMode) {
    switch (authMode) {
        case WIFI_AUTH_OPEN:            return "OPEN";
        case WIFI_AUTH_WEP:             return "WEP";
        case WIFI_AUTH_WPA_PSK:         return "WPA";
        case WIFI_AUTH_WPA2_PSK:        return "WPA2";
        case WIFI_AUTH_WPA_WPA2_PSK:    return "WPA_WPA2";
        case WIFI_AUTH_WPA2_ENTERPRISE: return "WPA2_ENT";
        case WIFI_AUTH_WPA3_PSK:        return "WPA3";
        default:                        return "UNKNOWN";
    }
}

static String colorToHex(const CRGB& c) {
    char hex[7];
    snprintf(hex, sizeof(hex), "%02X%02X%02X", c.r, c.g, c.b);
    return String(hex);
}

static bool hexToColor(const String& hex, CRGB& out) {
    if (hex.length() != 6) return false;
    long colorValue = strtol(hex.c_str(), NULL, 16);
    out.r = (colorValue >> 16) & 0xFF;
    out.g = (colorValue >> 8) & 0xFF;
    out.b = colorValue & 0xFF;
    return true;
}

static String formatHHMM(uint8_t h, uint8_t m) {
    char buf[6];
    snprintf(buf, sizeof(buf), "%02d:%02d", h, m);
    return String(buf);
}

static bool parseHHMM(const String& s, uint8_t& h, uint8_t& m) {
    if (s.length() != 5 || s.charAt(2) != ':') return false;
    int hh = s.substring(0, 2).toInt();
    int mm = s.substring(3, 5).toInt();
    if (hh < 0 || hh > 23 || mm < 0 || mm > 59) return false;
    h = hh;
    m = mm;
    return true;
}

// ==================== WiFi Scan ====================

void performWiFiScan() {
    if (wifiScanChar == nullptr) return;

    WiFi.disconnect(false);  // il driver non deve essere impegnato a connettersi
    delay(100);
    int n = WiFi.scanNetworks(false, false);

    JsonDocument doc;
    JsonArray networks = doc["n"].to<JsonArray>();

    // Limite per stare nei 512 byte della characteristic
    int maxNetworks = (n > 15) ? 15 : n;

    for (int i = 0; i < maxNetworks; i++) {
        JsonObject net = networks.add<JsonObject>();
        net["s"] = WiFi.SSID(i);
        net["r"] = WiFi.RSSI(i);
        net["e"] = getEncryptionType(WiFi.encryptionType(i));
    }

    WiFi.scanDelete();

    String json;
    serializeJson(doc, json);
    wifiScanChar->setValue(json.c_str());
    wifiScanChar->notify();

    Serial.println("BLE: scansione WiFi completata, trovate " + String(n) + " reti");
}

// ==================== Callback ====================

// --- Auth (verifica PIN a livello applicativo) ---
class AuthCallbacks : public NimBLECharacteristicCallbacks {
    void onRead(NimBLECharacteristic* pChar, NimBLEConnInfo& connInfo) override {
        JsonDocument doc;
        doc["auth"] = _authenticated;
        String json;
        serializeJson(doc, json);
        pChar->setValue(json.c_str());
    }

    void onWrite(NimBLECharacteristic* pChar, NimBLEConnInfo& connInfo) override {
        std::string val = pChar->getValue();
        JsonDocument doc;
        if (deserializeJson(doc, val.c_str())) return;

        uint32_t pin = doc["pin"] | 0;
        if (pin == _blePinCode && pin != 0) {
            _authenticated = true;
            Serial.println("BLE: PIN verificato - autenticato");
        } else {
            _authenticated = false;
            Serial.println("BLE: PIN rifiutato");
        }

        JsonDocument resp;
        resp["auth"] = _authenticated;
        String json;
        serializeJson(resp, json);
        pChar->setValue(json.c_str());
    }
};

// --- WiFi ---
class WiFiConfigCallbacks : public NimBLECharacteristicCallbacks {
    void onRead(NimBLECharacteristic* pChar, NimBLEConnInfo& connInfo) override {
        if (!_authenticated) { pChar->setValue("{\"err\":\"auth\"}"); return; }
        JsonDocument doc;
        doc["ssid"] = config.WIFI_SSID;
        doc["pass"] = "********";

        String json;
        serializeJson(doc, json);
        pChar->setValue(json.c_str());
    }

    void onWrite(NimBLECharacteristic* pChar, NimBLEConnInfo& connInfo) override {
        if (!_authenticated) return;
        std::string val = pChar->getValue();
        JsonDocument doc;
        if (deserializeJson(doc, val.c_str())) return;

        if (!doc["ssid"].isNull())
            strlcpy(config.WIFI_SSID, doc["ssid"] | "", sizeof(config.WIFI_SSID));
        if (!doc["pass"].isNull())
            strlcpy(config.WIFI_PASSWORD, doc["pass"] | "", sizeof(config.WIFI_PASSWORD));

        writeConfigurations();
        _bleNeedsReconnect = true;
        Serial.println("BLE: credenziali WiFi aggiornate, riconnessione richiesta");
    }
};

// --- Time (solo timezone: niente meteo su Oraquadra) ---
class TimeCallbacks : public NimBLECharacteristicCallbacks {
    void onRead(NimBLECharacteristic* pChar, NimBLEConnInfo& connInfo) override {
        if (!_authenticated) { pChar->setValue("{\"err\":\"auth\"}"); return; }
        JsonDocument doc;
        doc["tz"] = config.TIMEZONE;

        String json;
        serializeJson(doc, json);
        pChar->setValue(json.c_str());
    }

    void onWrite(NimBLECharacteristic* pChar, NimBLEConnInfo& connInfo) override {
        if (!_authenticated) return;
        std::string val = pChar->getValue();
        JsonDocument doc;
        if (deserializeJson(doc, val.c_str())) return;

        if (!doc["tz"].isNull()) {
            config.TIMEZONE = doc["tz"].as<String>();
            writeConfigurations();
            _bleNeedsTimezoneUpdate = true;
            Serial.println("BLE: timezone aggiornata: " + config.TIMEZONE);
        }
    }
};

// --- Frase e indovinello ---
class QuoteQuizCallbacks : public NimBLECharacteristicCallbacks {
    void onRead(NimBLECharacteristic* pChar, NimBLEConnInfo& connInfo) override {
        if (!_authenticated) { pChar->setValue("{\"err\":\"auth\"}"); return; }
        JsonDocument doc;
        doc["quoteUrl"] = config.quoteUrl;
        doc["quizUrl"]  = config.quizUrl;
        doc["quizHour"] = config.quizHour;
        doc["quote"]    = config.isQuote;
        doc["quiz"]     = config.isQuiz;

        String json;
        serializeJson(doc, json);
        pChar->setValue(json.c_str());
    }

    void onWrite(NimBLECharacteristic* pChar, NimBLEConnInfo& connInfo) override {
        if (!_authenticated) return;
        std::string val = pChar->getValue();
        JsonDocument doc;
        if (deserializeJson(doc, val.c_str())) return;

        if (!doc["quoteUrl"].isNull()) config.quoteUrl = doc["quoteUrl"].as<String>();
        if (!doc["quizUrl"].isNull())  config.quizUrl  = doc["quizUrl"].as<String>();
        if (!doc["quizHour"].isNull()) config.quizHour = doc["quizHour"];
        if (!doc["quote"].isNull())    config.isQuote  = doc["quote"];
        if (!doc["quiz"].isNull())     config.isQuiz   = doc["quiz"];

        writeConfigurations();
        DailyContent::invalidateCache();
        Serial.println("BLE: impostazioni frase/indovinello aggiornate");
    }
};

// --- Orologio Oraquadra (preset, colore, luminosita', fasce orarie) ---
class OqClockCallbacks : public NimBLECharacteristicCallbacks {
    void onRead(NimBLECharacteristic* pChar, NimBLEConnInfo& connInfo) override {
        if (!_authenticated) { pChar->setValue("{\"err\":\"auth\"}"); return; }
        JsonDocument doc;
        doc["preset"]       = currentPreset;
        doc["blink"]        = (currentBlink == 1);
        doc["color"]        = colorToHex(clockColors.current);
        doc["rainbow"]      = textRainbowMode;
        doc["power"]        = !displayOff;
        doc["dayBright"]    = map(dayBrightness, 0, 255, 0, 100);
        doc["nightBright"]  = map(nightBrightness, 0, 255, 0, 100);
        doc["dayStart"]     = formatHHMM(dayStartHour, dayStartMinute);
        doc["nightStart"]   = formatHHMM(nightStartHour, nightStartMinute);
        doc["sleepStart"]   = formatHHMM(sleepStartHour, sleepStartMinute);
        doc["sleepEnabled"] = sleepModeEnabled;

        String json;
        serializeJson(doc, json);
        pChar->setValue(json.c_str());
    }

    void onWrite(NimBLECharacteristic* pChar, NimBLEConnInfo& connInfo) override {
        if (!_authenticated) return;
        std::string val = pChar->getValue();
        JsonDocument doc;
        if (deserializeJson(doc, val.c_str())) return;

        if (!doc["preset"].isNull()) {
            uint8_t newPreset = doc["preset"];
            if (newPreset < FC_NUM_PRESETS && newPreset != currentPreset) {
                currentPreset = newPreset;
                _blePresetChanged = true;
            }
        }
        if (!doc["blink"].isNull()) currentBlink = doc["blink"].as<bool>() ? 1 : 0;
        if (!doc["color"].isNull()) {
            CRGB newColor;
            if (hexToColor(doc["color"].as<String>(), newColor)) {
                clockColors.current = newColor;
                textRainbowMode = false;
                Settings::saveModeSettings(currentPreset);  // persiste per il preset corrente
            }
        }
        if (!doc["rainbow"].isNull()) {
            textRainbowMode = doc["rainbow"].as<bool>();
            Settings::saveModeSettings(currentPreset);
        }
        if (!doc["power"].isNull()) displayOff = !doc["power"].as<bool>();
        if (!doc["dayBright"].isNull())
            dayBrightness = map(constrain(doc["dayBright"].as<int>(), 0, 100), 0, 100, 0, 255);
        if (!doc["nightBright"].isNull())
            nightBrightness = map(constrain(doc["nightBright"].as<int>(), 0, 100), 0, 100, 0, 255);
        if (!doc["dayStart"].isNull())   parseHHMM(doc["dayStart"].as<String>(), dayStartHour, dayStartMinute);
        if (!doc["nightStart"].isNull()) parseHHMM(doc["nightStart"].as<String>(), nightStartHour, nightStartMinute);
        if (!doc["sleepStart"].isNull()) parseHHMM(doc["sleepStart"].as<String>(), sleepStartHour, sleepStartMinute);
        if (!doc["sleepEnabled"].isNull()) sleepModeEnabled = doc["sleepEnabled"].as<bool>();

        writeConfigurations();
        _bleNeedsClockApply = true;
        Serial.println("BLE: impostazioni orologio aggiornate");
    }
};

// --- Testo scorrevole Oraquadra ---
class OqScrollCallbacks : public NimBLECharacteristicCallbacks {
    void onRead(NimBLECharacteristic* pChar, NimBLEConnInfo& connInfo) override {
        if (!_authenticated) { pChar->setValue("{\"err\":\"auth\"}"); return; }
        JsonDocument doc;
        doc["enabled"]    = scrollEnabled;
        doc["text"]       = customScrollText;
        doc["color"]      = colorToHex(scrollTextColor);
        doc["rainbow"]    = scrollTextRainbow;
        doc["speed"]      = scrollTextSpeedMs;
        doc["fontSize"]   = scrollFontSize;
        doc["pause"]      = scrollPauseSeconds;
        doc["showText"]   = scrollShowText;
        doc["showDate"]   = scrollShowDate;
        doc["showTime"]   = scrollShowTime;
        doc["textual"]    = scrollTextualDateTime;
        doc["showPreset"] = scrollShowPresetDuringPause;

        String json;
        serializeJson(doc, json);
        pChar->setValue(json.c_str());
    }

    void onWrite(NimBLECharacteristic* pChar, NimBLEConnInfo& connInfo) override {
        if (!_authenticated) return;
        std::string val = pChar->getValue();
        JsonDocument doc;
        if (deserializeJson(doc, val.c_str())) return;

        if (!doc["enabled"].isNull()) {
            scrollEnabled = doc["enabled"].as<bool>();
            scrollEnabledRaw = scrollEnabled ? 1 : 0;
        }
        if (!doc["text"].isNull()) {
            String sanitized = ScrollText::sanitize(doc["text"].as<String>());
            if (sanitized.length() > 200) sanitized = sanitized.substring(0, 200);
            customScrollText = sanitized;
        }
        if (!doc["color"].isNull()) {
            CRGB newColor;
            if (hexToColor(doc["color"].as<String>(), newColor)) {
                scrollTextColor = newColor;
                scrollTextRainbow = false;
            }
        }
        if (!doc["rainbow"].isNull()) scrollTextRainbow = doc["rainbow"].as<bool>();
        if (!doc["speed"].isNull())
            scrollTextSpeedMs = constrain(doc["speed"].as<int>(), 10, 500);
        if (!doc["fontSize"].isNull())
            scrollFontSize = constrain(doc["fontSize"].as<int>(), 1, 3);
        if (!doc["pause"].isNull())
            scrollPauseSeconds = constrain(doc["pause"].as<int>(), 0, 60);
        if (!doc["showText"].isNull())   scrollShowText = doc["showText"].as<bool>();
        if (!doc["showDate"].isNull())   scrollShowDate = doc["showDate"].as<bool>();
        if (!doc["showTime"].isNull())   scrollShowTime = doc["showTime"].as<bool>();
        if (!doc["textual"].isNull())    scrollTextualDateTime = doc["textual"].as<bool>();
        if (!doc["showPreset"].isNull()) scrollShowPresetDuringPause = doc["showPreset"].as<bool>();

        writeConfigurations();
        _bleNeedsScrollApply = true;
        Serial.println("BLE: impostazioni testo scorrevole aggiornate");
    }
};

// --- Quadrante digitale alternato ---
class OqDigitalCallbacks : public NimBLECharacteristicCallbacks {
    void onRead(NimBLECharacteristic* pChar, NimBLEConnInfo& connInfo) override {
        if (!_authenticated) { pChar->setValue("{\"err\":\"auth\"}"); return; }
        JsonDocument doc;
        doc["enabled"]  = digitalOverlayEnabled;
        doc["interval"] = digitaleInterval;
        doc["duration"] = digitaleDuration;

        String json;
        serializeJson(doc, json);
        pChar->setValue(json.c_str());
    }

    void onWrite(NimBLECharacteristic* pChar, NimBLEConnInfo& connInfo) override {
        if (!_authenticated) return;
        std::string val = pChar->getValue();
        JsonDocument doc;
        if (deserializeJson(doc, val.c_str())) return;

        if (!doc["enabled"].isNull()) digitalOverlayEnabled = doc["enabled"].as<bool>();
        if (!doc["interval"].isNull())
            digitaleInterval = constrain(doc["interval"].as<int>(), 5, 120);
        if (!doc["duration"].isNull())
            digitaleDuration = constrain(doc["duration"].as<int>(), 10, 59);

        writeConfigurations();
        Serial.println("BLE: impostazioni quadrante digitale aggiornate");
    }
};

// --- Comandi ---
class CommandCallbacks : public NimBLECharacteristicCallbacks {
    void onWrite(NimBLECharacteristic* pChar, NimBLEConnInfo& connInfo) override {
        if (!_authenticated) return;
        std::string val = pChar->getValue();
        JsonDocument doc;
        if (deserializeJson(doc, val.c_str())) return;

        String cmd = doc["cmd"] | "";
        Serial.println("BLE: comando ricevuto: " + cmd);

        if (cmd == "save") {
            writeConfigurations();
        } else if (cmd == "reboot") {
            writeConfigurations();
            delay(500);
            ESP.restart();
        } else if (cmd == "factory_reset") {
            factoryResetConfigurations();
            delay(500);
            ESP.restart();
        } else if (cmd == "wifi_reconnect") {
            _bleNeedsReconnect = true;
        } else if (cmd == "wifi_scan") {
            _bleNeedsScan = true;
        } else if (cmd == "ota_update") {
            _bleNeedsOTAUpdate = true;
        }
    }
};

// --- Jukebox ---
class MelodyCallbacks : public NimBLECharacteristicCallbacks {
    // READ -> lista JSON: {"m":[{"i":0,"n":"..."},...]}
    void onRead(NimBLECharacteristic* pChar, NimBLEConnInfo& connInfo) override {
        if (!_authenticated) {
            pChar->setValue("{\"err\":\"auth\"}");
            return;
        }
        String json = melodyListJson();
        pChar->setValue(json.c_str());
    }

    // WRITE -> {"id":5,"msg":"che la forza sia con te!","rep":10}
    void onWrite(NimBLECharacteristic* pChar, NimBLEConnInfo& connInfo) override {
        if (!_authenticated) return;
        std::string val = pChar->getValue();
        JsonDocument doc;
        if (deserializeJson(doc, val.c_str()) != DeserializationError::Ok) return;

        uint8_t id   = doc["id"]  | 0;
        uint16_t rep = doc["rep"] | 1;
        const char* msg = doc["msg"] | "";

        _pendingMelodyRequest.melodyId    = id;
        _pendingMelodyRequest.repeatCount = rep;
        strlcpy(_pendingMelodyRequest.message, msg, sizeof(_pendingMelodyRequest.message));
        _bleNeedsMelodyPlay = true;

        Serial.println("BLE Melody: id=" + String(id) + " rep=" + String(rep) + " msg=" + String(msg));
    }
};

// ==================== Status ====================

void bleUpdateStatus() {
    if (statusChar == nullptr) return;

    JsonDocument doc;
    doc["fw"]         = FIRMWARE_VERSION;
    doc["fwCode"]     = FIRMWARE_CODE;
    doc["serverUrl"]  = config.quoteUrl;
    doc["deviceType"] = DEVICE_TYPE;
    doc["wifi"] = (WiFi.status() == WL_CONNECTED);
    doc["ip"] = WiFi.localIP().toString();
    doc["heap"] = ESP.getFreeHeap();
    doc["ssid"] = config.WIFI_SSID;

    String json;
    serializeJson(doc, json);
    statusChar->setValue(json.c_str());
    statusChar->notify();
}

// ==================== Server ====================

class ServerCallbacks : public NimBLEServerCallbacks {
    void onConnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo) override {
        Serial.println("BLE: client connesso");
    }

    void onDisconnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo, int reason) override {
        Serial.println("BLE: client disconnesso, riavvio advertising");
        _authenticated = false;
        NimBLEDevice::getAdvertising()->start();
    }
};

// ==================== Init ====================

void bleConfigInit() {
    // PIN persistente: genera solo se non presente in config
    if (config.blePinCode == 0) {
        randomSeed(esp_random());
        config.blePinCode = random(100000, 999999);
        writeConfigurations();
        Serial.println("BLE PIN generato: " + String(config.blePinCode));
    } else {
        Serial.println("BLE PIN caricato: " + String(config.blePinCode));
    }
    _blePinCode = config.blePinCode;

    // Nome univoco dal MAC BT; il prefisso "FreeClock-" e' richiesto dal filtro
    // di scansione dell'app, "OQ" distingue a vista un Oraquadra
    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_BT);
    snprintf(bleDeviceName, sizeof(bleDeviceName), "FreeClock-OQ-%02X%02X", mac[4], mac[5]);

    NimBLEDevice::init(bleDeviceName);
    NimBLEDevice::setMTU(517);
    // Massima potenza TX: con WiFi attivo la coesistenza radio degrada
    // l'advertising e iOS fatica ad agganciare il dispositivo
    NimBLEDevice::setPower(9); // dBm (API NimBLE 2.x)

    // Nessuna security BLE: autenticazione a livello app tramite characteristic AUTH

    NimBLEServer* pServer = NimBLEDevice::createServer();
    pServer->setCallbacks(new ServerCallbacks());
    NimBLEService* pService = pServer->createService(BLE_SERVICE_UUID);

    NimBLECharacteristic* authChar = pService->createCharacteristic(
        CHAR_AUTH_UUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE);
    authChar->setCallbacks(new AuthCallbacks());

    NimBLECharacteristic* wifiChar = pService->createCharacteristic(
        CHAR_WIFI_UUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE);
    wifiChar->setCallbacks(new WiFiConfigCallbacks());

    wifiScanChar = pService->createCharacteristic(
        CHAR_WIFI_SCAN_UUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);

    NimBLECharacteristic* timeChar = pService->createCharacteristic(
        CHAR_TIMEWEATHER_UUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE);
    timeChar->setCallbacks(new TimeCallbacks());

    NimBLECharacteristic* qqChar = pService->createCharacteristic(
        CHAR_QUOTEQUIZ_UUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE);
    qqChar->setCallbacks(new QuoteQuizCallbacks());

    NimBLECharacteristic* clockChar = pService->createCharacteristic(
        CHAR_OQ_CLOCK_UUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE);
    clockChar->setCallbacks(new OqClockCallbacks());

    NimBLECharacteristic* scrollChar = pService->createCharacteristic(
        CHAR_OQ_SCROLL_UUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE);
    scrollChar->setCallbacks(new OqScrollCallbacks());

    NimBLECharacteristic* digitalChar = pService->createCharacteristic(
        CHAR_OQ_DIGITAL_UUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE);
    digitalChar->setCallbacks(new OqDigitalCallbacks());

    // Status (read + notify) — senza auth: l'app lo usa per riconoscere il deviceType
    statusChar = pService->createCharacteristic(
        CHAR_STATUS_UUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);

    NimBLECharacteristic* cmdChar = pService->createCharacteristic(
        CHAR_COMMAND_UUID, NIMBLE_PROPERTY::WRITE);
    cmdChar->setCallbacks(new CommandCallbacks());

    NimBLECharacteristic* melodyChar = pService->createCharacteristic(
        CHAR_MELODY_UUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE);
    melodyChar->setCallbacks(new MelodyCallbacks());

    pService->start();

    NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
    pAdvertising->enableScanResponse(true);
    pAdvertising->addServiceUUID(BLE_SERVICE_UUID);
    pAdvertising->setName(bleDeviceName);
    pAdvertising->setMinInterval(0x20);
    pAdvertising->setMaxInterval(0x40);
    pAdvertising->start();

    Serial.println("BLE Config server avviato: " + String(bleDeviceName));
}
