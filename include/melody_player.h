#pragma once

// MelodyPlayer — task FreeRTOS (core 0) che suona le melodie del jukebox sul buzzer.
// Adattato da FreeClock: il messaggio associato viene fatto scorrere sulla matrice
// dal loop principale (vedi App::loop), non dal task.

#include <Arduino.h>

struct MelodyRequest {
    uint8_t  melodyId;
    char     message[256];   // buffer fisso, nessuna allocazione heap
    uint16_t repeatCount;    // quante volte suonare (0 = una volta)
};

void   melodyPlayerInit();
void   melodyPlayerAbort();
bool   melodyPlayerIsActive();
void   melodyPlayerEnqueue(const MelodyRequest& req);
String melodyListJson();            // {"m":[{"i":0,"n":"..."},...]} per la characteristic BLE

// Messaggio da far scorrere sulla matrice (consumato dal loop principale)
bool   melodyPlayerMessagePending();
String melodyPlayerTakeMessage();
