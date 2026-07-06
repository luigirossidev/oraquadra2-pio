#pragma once

#include <FastLED.h>
#include "config.h"

struct Drop {
    uint8_t x;
    float y;
    float speed;
    uint8_t intensity;
    bool active;
    bool isMatrix2;
};

extern Drop drops[NUM_DROPS];
extern bool matrixInitialized;

void initDrop(Drop &drop);
void setDropType(Drop &drop, bool isMatrix2);

// Effetto MATRIX (pioggia verde stile Matrix)
class MatrixEffect {
public:
    static void init();
    static void update();
};
