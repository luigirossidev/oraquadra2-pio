#pragma once

#include <FastLED.h>
#include "config.h"

struct TronBike {
    int8_t x;
    int8_t y;
    int8_t dir;        // 0=su, 1=destra, 2=giù, 3=sinistra
    CRGB color;
    bool active;
    uint8_t trailX[TRON_TRAIL_LENGTH];
    uint8_t trailY[TRON_TRAIL_LENGTH];
    uint8_t trailLen;
};

extern TronBike tronBikes[NUM_TRON_BIKES];
extern uint8_t tronIntensity[NUM_LEDS];
extern uint32_t lastTronUpdate;
extern uint32_t tronCycleStart;
extern bool tronEffectActive;
extern bool tronNeedsInit;
extern uint8_t tronLastMinute;

bool tronPosIsFree(int8_t x, int8_t y, uint8_t bikeIndex);
int8_t tronFindSafeDir(uint8_t bikeIndex);

// Effetto TRON (moto con scie)
class TronEffect {
public:
    static void init();
    static void update();
    static void reset();
};
