#pragma once

#include <FastLED.h>
#include "config.h"

struct DropState {
    bool active = false;
    bool completed = false;
    float radius = 0;                // raggio corrente dell'onda
    uint8_t centerX = 8;             // centro X della goccia
    uint8_t centerY = 8;             // centro Y della goccia
    uint32_t lastUpdate = 0;
    uint32_t startTime = 0;
    CRGB waveColor;
    uint8_t waveCount = 0;           // numero di onde create
};

extern DropState dropState;

// Effetto GOCCIA (onde concentriche)
class DropEffect {
public:
    static void update();
    static void reset();
};
