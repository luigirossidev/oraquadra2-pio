#pragma once

#include <FastLED.h>

// Primitive di disegno sulla matrice LED (font, caratteri, mapping serpentina)
class Canvas {
public:
    static void getCharBitmapSmall(char c, uint8_t* bitmap);
    static void getCharBitmap(char c, uint8_t* bitmap);
    static void drawChar5x7(uint8_t x, uint8_t y, char c, CRGB color);
    static void drawDigitCentered(uint8_t x, uint8_t y, uint8_t digit, CRGB color);
    static void drawColon(uint8_t x, uint8_t y, CRGB color, bool blink);
    static uint16_t xyToLED(uint8_t x, uint8_t y);
};
