#pragma once

#include <Arduino.h>
#include <FastLED.h>

// Testo scorrevole (versione bloccante per il boot + versione non bloccante)
class ScrollText {
public:
    static void show(String text, CRGB color, uint16_t scrollSpeed);  // bloccante (boot)
    static String sanitize(String raw);  // maiuscole + accentate UTF-8 -> ASCII stampabile
    static String buildTextualTime();
    static String buildTextualDate();
    static String buildDisplayText();
    static void init();
    static void reset();
    static void update();
};
