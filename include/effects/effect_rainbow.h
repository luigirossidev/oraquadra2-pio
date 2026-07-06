#pragma once

#include <FastLED.h>
#include "config.h"

// Effetto RAINBOW (arcobaleno graduale)
class RainbowEffect {
public:
    static void update();
};

// Funzioni rainbow legacy
void rainbow(uint8_t wait);
CRGB Wheel(byte WheelPos);
