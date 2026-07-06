#pragma once

#include <FastLED.h>
#include "config.h"

extern uint32_t lastMotoUpdate;
extern uint16_t motoCurrentPosition;
extern bool motoEffectActive;
extern bool motoCompleted;
extern uint32_t motoCompletionTime;
extern CRGB motoTrailColors[MOTO_TRAIL_LENGTH];
extern uint8_t motoTrailHue;
extern uint8_t motoTextHue;
extern CRGB motoTextColor;
extern bool motoTextPixels[NUM_LEDS];
extern int8_t motoX;
extern int8_t motoY;
extern int8_t motoDirX;
extern int8_t motoDirY;
extern uint16_t motoTrailX[MOTO_TRAIL_LENGTH];
extern uint16_t motoTrailY[MOTO_TRAIL_LENGTH];
extern uint8_t motoTrailLen;
extern uint8_t motoLastMinute;

// Effetto MOTO (scia arcobaleno a zigzag)
class MotoEffect {
public:
    static void init();
    static void update();
    static void reset();
};
