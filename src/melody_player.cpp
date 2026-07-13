#include "melody_player.h"
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <ArduinoJson.h>
#include "melody_assets/MelodyList.h"

// Stesso canale LEDC di sound_effects.h (buzzer condiviso), senza includere
// l'header per non duplicare i suoi array PROGMEM in questa translation unit
#define _FX_LEDC_CHANNEL 5

static QueueHandle_t  _melodyQueue  = nullptr;
static volatile bool  _melodyActive = false;   // true mentre suona
static volatile bool  _melodyAbort  = false;   // settabile da qualsiasi core per fermare

// Messaggio in attesa di scroll sulla matrice (consumato dal loop principale)
static volatile bool  _messagePending = false;
static String         _pendingMessage;

// Delay interrompibile da _melodyAbort; false se interrotto
static bool melodyAbortableDelay(uint32_t ms) {
    uint32_t start = millis();
    while ((millis() - start) < ms) {
        if (_melodyAbort) return false;
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    return true;
}

static void melodyPlayerTask(void* param) {
    MelodyRequest req;
    for (;;) {
        // Blocca finche' non arriva una richiesta
        if (xQueueReceive(_melodyQueue, &req, portMAX_DELAY) != pdTRUE) continue;

        _melodyAbort  = false;
        _melodyActive = true;

        if (req.melodyId >= MELODY_COUNT) {
            _melodyActive = false;
            continue;
        }
        MelodyInfo info;
        memcpy_P(&info, &MELODY_LIST[req.melodyId], sizeof(MelodyInfo));

        // Consegna il messaggio al loop principale per lo scroll
        if (req.message[0] != '\0') {
            _pendingMessage = String(req.message);
            _messagePending = true;
        }

        uint16_t reps = (req.repeatCount == 0) ? 1 : req.repeatCount;
        int wholenote = (60000 * 4) / info.tempo;

        for (uint16_t r = 0; r < reps && !_melodyAbort; r++) {
            for (int n = 0; n < info.noteCount && !_melodyAbort; n++) {
                // Su ESP32 la PROGMEM e' mappata in memoria: dereferenziazione diretta sicura
                int pitch   = info.data[n * 2];
                int divider = info.data[n * 2 + 1];

                bool dotted = (divider < 0);
                int absdiv  = abs(divider);
                int noteDur = (absdiv > 0) ? (wholenote / absdiv) : 50;
                if (dotted) noteDur = (int)(noteDur * 1.5f);

                if (pitch == REST) {
                    ledcWrite(_FX_LEDC_CHANNEL, 0);
                } else {
                    ledcWriteTone(_FX_LEDC_CHANNEL, (double)pitch);
                }

                // Suono per il 90%, silenzio per il 10%
                if (!melodyAbortableDelay((uint32_t)(noteDur * 0.9f))) break;
                ledcWrite(_FX_LEDC_CHANNEL, 0);
                if (!melodyAbortableDelay((uint32_t)(noteDur * 0.1f))) break;
            }
        }

        ledcWrite(_FX_LEDC_CHANNEL, 0);
        _melodyActive = false;
    }
}

void melodyPlayerInit() {
    _melodyQueue = xQueueCreate(1, sizeof(MelodyRequest));
    xTaskCreatePinnedToCore(
        melodyPlayerTask,   // funzione del task
        "melodyTask",       // nome
        4096,               // stack (byte)
        nullptr,            // parametri
        2,                  // priorita' (sopra il loop, default 1)
        nullptr,            // handle non necessario
        0                   // core 0 (il loop display gira sul core 1)
    );
}

void melodyPlayerAbort() {
    _melodyAbort = true;
}

bool melodyPlayerIsActive() {
    return _melodyActive;
}

void melodyPlayerEnqueue(const MelodyRequest& req) {
    if (_melodyQueue == nullptr) return;
    xQueueOverwrite(_melodyQueue, &req);
}

String melodyListJson() {
    JsonDocument doc;
    JsonArray arr = doc["m"].to<JsonArray>();
    for (uint8_t i = 0; i < MELODY_COUNT; i++) {
        MelodyInfo info;
        memcpy_P(&info, &MELODY_LIST[i], sizeof(MelodyInfo));
        JsonObject o = arr.add<JsonObject>();
        o["i"] = info.id;
        o["n"] = info.name;
    }
    String json;
    serializeJson(doc, json);
    return json;
}

bool melodyPlayerMessagePending() {
    return _messagePending;
}

String melodyPlayerTakeMessage() {
    _messagePending = false;
    return _pendingMessage;
}
