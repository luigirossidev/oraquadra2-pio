#pragma once

#include <FastLED.h>

// Rendering dell'orologio a parole sulla matrice
class ClockDisplay {
public:
    static void displayWord(const uint8_t* word, CRGB color);
    static void fadeWord(const uint8_t* word, CRGB color, uint8_t steps = 8);
    static void updateDisplay();
    static void showMinutes(uint8_t minutes, CRGB color);
    static void showSecondsM(uint8_t seconds, CRGB color);
    static void showSeconds(uint8_t seconds, CRGB color);
    static void showCurrentTime();
    static void displayWordToTarget(const uint8_t* word);
    static void displayMinutesToTarget(uint8_t minutes);
    static void paintWordSlow(const uint8_t arrWord[], CRGB color);
    static void paintWordFast(const uint8_t arrWord[], CRGB color);
    static void paintWordSpeed(const uint8_t arrWord[], CRGB color, int speed);
};
