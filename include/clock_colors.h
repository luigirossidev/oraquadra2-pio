#pragma once

#include <FastLED.h>
#include "config.h"
#include "state.h"

// Struttura per gestione colori / Color management structure
struct ClockColors {
   CRGB current;
   uint8_t brightness;
   static const CRGB presets[];
   uint8_t presetIndex;

   ClockColors() : current(CRGB::White), brightness(BRIGHTNESS_DAY), presetIndex(0) {}

   void nextPreset() {
       presetIndex = (presetIndex + 1) % 6; // Modificato da 7 a 6 (rimosso viola)
       current = presets[presetIndex];
   }

   CRGB getColor() const {
       if (textRainbowMode) {
           // In modalità rainbow, restituisce un colore che cambia nel tempo / In rainbow mode, returns a time-changing color
           return CHSV(rainbowHue, 255, 255);
       }
       return CRGB(scale8(current.r, brightness),
                  scale8(current.g, brightness),
                  scale8(current.b, brightness));
   }

   // Colore rainbow basato su posizione / Rainbow color based on position
   CRGB getRainbowColor(uint8_t position) const {
       uint8_t hue = (position * 25) % 256;
       return CHSV(hue, 255, 255);
   }
};

extern ClockColors clockColors;
