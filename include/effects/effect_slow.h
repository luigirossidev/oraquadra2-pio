#pragma once

#include <FastLED.h>
#include "config.h"

// Variabili per effetto SLOW (non-bloccante)
struct SlowEffectState {
    bool active = false;
    bool completed = false;     // true quando l'effetto è finito, aspetta cambio minuto
    uint8_t step = 0;
    uint8_t targetBrightness = 0;
    uint8_t increment = 1;
    uint32_t lastStepTime = 0;
    bool holdPhase = false;
    uint32_t holdStartTime = 0;
    CRGB sonoColor;
    CRGB hourColor;
    CRGB eColor;
    CRGB minutesColor;
    CRGB minutiColor;
};

extern SlowEffectState slowState;

// Effetto SLOW (non-bloccante)
class SlowEffect {
public:
    static void update();
    static void reset();
};
