#include "effects/effect_rainbow.h"
#include "state.h"
#include "words_it.h"
#include "clock_display.h"

// Effetto RAINBOW - colori arcobaleno graduali che scorrono
void RainbowEffect::update() {
    uint32_t now = millis();

    if (now - lastRainbowUpdate >= RAINBOW_SPEED) {
        lastRainbowUpdate = now;
        rainbowHue++;  // Incrementa hue per far scorrere l'arcobaleno
    }

    FastLED.clear();

    // Mostra l'orario con colori arcobaleno graduali
    // Ogni parola ha un colore diverso basato sulla posizione + hue corrente
    uint8_t hueOffset = 0;

    // SONO LE
    ClockDisplay::displayWord(WORD_SONO_LE, CHSV(rainbowHue + hueOffset, 255, 255));
    hueOffset += 40;

    // ORA
    const uint8_t* hourWord = (const uint8_t*)pgm_read_ptr(&HOUR_WORDS[currentHour]);
    ClockDisplay::displayWord(hourWord, CHSV(rainbowHue + hueOffset, 255, 255));
    hueOffset += 40;

    if (currentMinute > 0) {
        // E
        ClockDisplay::displayWord(WORD_E, CHSV(rainbowHue + hueOffset, 255, 255));
        hueOffset += 40;

        // MINUTI
        ClockDisplay::showMinutes(currentMinute, CHSV(rainbowHue + hueOffset, 255, 255));
        hueOffset += 40;

        // MINUTI (parola)
        ClockDisplay::displayWord(WORD_MINUTI, CHSV(rainbowHue + hueOffset, 255, 255));
        hueOffset += 40;

        // Secondi
        if (currentBlink == 1) {
            ClockDisplay::showSeconds(currentSecond, CHSV(rainbowHue + hueOffset, 255, 255));
        }
    }

    FastLED.show();
}

void rainbow(uint8_t wait) {
   static uint8_t hue = 0;
   
   for(uint16_t i = 0; i < NUM_LEDS; i++) {
       leds[i] = CHSV(hue + (i * 256 / NUM_LEDS), 255, 255);
   }
   
   FastLED.show();
   delay(wait);
   
   hue++;
}

CRGB Wheel(byte WheelPos) {
   WheelPos = 255 - WheelPos;
   
   if(WheelPos < 85) {
       return CRGB(255 - WheelPos * 3, 0, WheelPos * 3);
   }
   
   if(WheelPos < 170) {
       WheelPos -= 85;
       return CRGB(0, WheelPos * 3, 255 - WheelPos * 3);
   }
   
   WheelPos -= 170;
   return CRGB(WheelPos * 3, 255 - WheelPos * 3, 0);
}
