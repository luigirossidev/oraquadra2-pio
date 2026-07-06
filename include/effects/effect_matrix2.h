#pragma once

#include <FastLED.h>
#include "config.h"
#include "effects/effect_matrix.h"

// Struttura per tenere traccia dello stato delle gocce nel Matrix2
struct Matrix2State {
    bool completed;
    uint32_t completionTime;
    bool needsReset;
};

extern Matrix2State matrix2State;

// Effetto MATRIX2 (pioggia che completa la scritta e si ferma)
class Matrix2Effect {
public:
    static void update();
    static void reset();
};
