#include "daily_content.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <FastLED.h>
#include "state.h"
#include "fc_config.h"
#include "scroll_text.h"
#include "ble_config.h"

// Richieste pendenti dal pulsante SEC
enum PendingRequest : uint8_t { REQ_NONE = 0, REQ_QUOTE_QUIZ, REQ_PIN };
static volatile uint8_t _pending = REQ_NONE;

// Alternanza frase/indovinello: entro 30 s dalla pressione precedente si passa
// al contenuto successivo, altrimenti si riparte dalla frase
static uint8_t  _nextItem = 0;   // 0 = frase, 1 = indovinello
static uint32_t _lastRequest = 0;

// Cache giornaliera (formato data YYYYMMDD, come i client FreeClock)
static String _quote;
static String _quiz;
static String _quizSolution;
static int    _quoteDate = 0;
static int    _quizDate  = 0;

// Backoff sui fetch falliti: il GET e' bloccante (fino a 15 s), non va
// ritentato ad ogni giro di loop
static const uint32_t RETRY_INTERVAL_MS = 10UL * 60UL * 1000UL;
static uint32_t _quoteLastAttempt = 0;
static uint32_t _quizLastAttempt  = 0;

static bool retryAllowed(uint32_t lastAttempt) {
    return lastAttempt == 0 || millis() - lastAttempt >= RETRY_INTERVAL_MS;
}

void DailyContent::requestQuoteQuiz() { _pending = REQ_QUOTE_QUIZ; }
void DailyContent::requestShowPin()   { _pending = REQ_PIN; }
void DailyContent::invalidateCache()  { _quoteDate = 0; _quizDate = 0; _quoteLastAttempt = 0; _quizLastAttempt = 0; }

// GET {url}{endpoint} e ritorna il JSON grezzo ("" su errore).
// Porting di DailyQuoteApiClient/DailyQuizApiClient di FreeClock.
static String fetchJson(const String& baseUrl, const char* endpoint) {
    if (baseUrl == "" || WiFi.status() != WL_CONNECTED) return "";

    HTTPClient http;
    WiFiClientSecure secureClient;
    secureClient.setInsecure();

    String url = baseUrl + endpoint;
    Serial.println("Fetch: " + url);

    if (url.startsWith("https")) {
        http.begin(secureClient, url);
    } else {
        http.begin(url);
    }
    http.setTimeout(15000);
    int httpCode = http.GET();

    String result = "";
    if (httpCode == HTTP_CODE_OK) {
        result = http.getString();
    } else {
        Serial.println("Fetch fallito (" + String(httpCode) + "): " + url);
    }
    http.end();

    // Estrai il blocco JSON valido
    int startIdx = result.indexOf('{');
    int endIdx = result.lastIndexOf('}');
    if (startIdx == -1 || endIdx == -1 || endIdx <= startIdx) return "";
    return result.substring(startIdx, endIdx + 1);
}

static int todayDate() {
    // ezTime non sincronizzato: anno < 2024
    if (myTZ.year() < 2024) return 0;
    return myTZ.year() * 10000 + myTZ.month() * 100 + myTZ.day();
}

static void fetchQuoteIfNeeded() {
    int today = todayDate();
    if (today == 0 || today == _quoteDate) return;
    if (!retryAllowed(_quoteLastAttempt)) return;
    _quoteLastAttempt = millis();

    String json = fetchJson(config.quoteUrl, "/api/daily/quote");
    if (json == "") return;

    JsonDocument doc;
    if (deserializeJson(doc, json)) return;

    _quote = doc["data"]["quote"] | "";
    if (_quote != "") {
        _quoteDate = today;
        Serial.println("Frase del giorno: " + _quote);
    }
}

static void fetchQuizIfNeeded() {
    int today = todayDate();
    if (today == 0 || today == _quizDate) return;
    if (!retryAllowed(_quizLastAttempt)) return;
    _quizLastAttempt = millis();

    String json = fetchJson(config.quizUrl, "/api/daily/riddle");
    if (json == "") return;

    JsonDocument doc;
    if (deserializeJson(doc, json)) return;

    _quiz         = doc["data"]["riddle"]   | "";
    _quizSolution = doc["data"]["solution"] | "";
    if (_quiz != "") {
        _quizDate = today;
        Serial.println("Indovinello del giorno: " + _quiz);
    }
}

static void showQuote() {
    fetchQuoteIfNeeded();
    String text = (_quote != "") ? ("FRASE DEL GIORNO: " + _quote)
                                 : "FRASE DEL GIORNO NON DISPONIBILE";
    ScrollText::show(ScrollText::sanitize(text), CRGB::Aqua, scrollTextSpeedMs);
}

static void showQuiz() {
    fetchQuizIfNeeded();
    String text;
    if (_quiz != "") {
        text = "INDOVINELLO: " + _quiz;
        if (_quizSolution != "" && myTZ.hour() >= config.quizHour) {
            text += " - SOLUZIONE: " + _quizSolution;
        }
    } else {
        text = "INDOVINELLO NON DISPONIBILE";
    }
    ScrollText::show(ScrollText::sanitize(text), CRGB::Gold, scrollTextSpeedMs);
}

void DailyContent::handle() {
    // Fetch giornaliero in background (una volta al giorno, quando c'e' WiFi)
    if (WiFi.status() == WL_CONNECTED) {
        if (config.isQuote) fetchQuoteIfNeeded();
        if (config.isQuiz)  fetchQuizIfNeeded();
    }

    if (_pending == REQ_NONE) return;
    uint8_t req = _pending;
    _pending = REQ_NONE;

    if (req == REQ_PIN) {
        ScrollText::show("PIN " + String(bleGetPinCode()), CRGB::Yellow, 50);
        return;
    }

    // REQ_QUOTE_QUIZ: alterna frase -> indovinello entro 30 s, poi riparte dalla frase
    uint32_t now = millis();
    if (now - _lastRequest > 30000) _nextItem = 0;
    _lastRequest = now;

    bool quoteOk = config.isQuote;
    bool quizOk  = config.isQuiz;
    if (!quoteOk && !quizOk) return;

    if (_nextItem == 0) {
        if (quoteOk) { showQuote(); _nextItem = quizOk ? 1 : 0; }
        else         { showQuiz(); }
    } else {
        if (quizOk)  { showQuiz(); _nextItem = 0; }
        else         { showQuote(); }
    }
}
