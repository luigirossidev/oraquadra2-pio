#pragma once

#include <FastLED.h>
#include "config.h"

// Variabili per effetto FADE (non-bloccante)
struct FadeEffectState {
    bool active = false;
    bool completed = false;     // true quando l'effetto è finito, aspetta cambio minuto
    uint8_t phase = 0;          // 0=sono, 1=hour, 2=e, 3=minutes, 4=minuti, 5=done
    uint8_t step = 0;
    uint32_t lastStepTime = 0;
    static const uint8_t STEPS = 50;  // 50 step per fade graduale (~2.5 sec per parola)
};

extern FadeEffectState fadeState;

// Effetto FADE (non-bloccante)
class FadeEffect {
public:
    static void update();
    static void reset();
};
