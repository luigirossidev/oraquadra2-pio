#pragma once

#include <FastLED.h>
#include "config.h"

extern uint32_t lastPacmanUpdate;
extern bool pacmanEffectActive;
extern uint32_t pacmanCompletionTime;
extern uint8_t pacmanTextHue;
extern CRGB pacmanTextColor;
extern bool pacmanTextPixels[NUM_LEDS];
extern uint8_t pacmanLastMinute;
extern int16_t pacmanPosition;
extern int16_t ghostPositions[NUM_GHOSTS];
extern CRGB ghostColors[NUM_GHOSTS];

// Effetto PACMAN (Pacman che insegue i fantasmi colorando le scritte)
class PacmanEffect {
public:
    static void init();
    static void update();
    static void reset();
};
