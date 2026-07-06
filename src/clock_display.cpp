#include "clock_display.h"
#include "state.h"
#include "config.h"
#include "words_it.h"
#include "clock_colors.h"
#include "led_canvas.h"

// Funzioni di gestione LED e parole / LED and word management functions
void ClockDisplay::displayWord(const uint8_t* word, CRGB color) {
   uint8_t idx = 0;
   uint8_t pixel;
   while((pixel = pgm_read_byte(&word[idx])) != 8) {
       leds[pixel] = color;
       idx++;
   }
}

void ClockDisplay::fadeWord(const uint8_t* word, CRGB color, uint8_t steps) {
   for(uint8_t step = 0; step < steps; step++) {
       uint8_t brightness = map(step, 0, steps-1, 0, 255);
       CRGB fadeColor = color;
       fadeColor.nscale8(brightness);
       ClockDisplay::displayWord(word, fadeColor);
       FastLED.show();
       delay(50);
       yield();
   }
}

void ClockDisplay::updateDisplay() {
   if (displayOff) {
       FastLED.clear();
       FastLED.show();
       return;
   }
   
   FastLED.clear();
   CRGB currentColor = clockColors.getColor();
   
   ClockDisplay::displayWord(WORD_SONO_LE, currentColor);
   
   const uint8_t* hourWord = (const uint8_t*)pgm_read_ptr(&HOUR_WORDS[currentHour]);
   
   switch(currentMode) {
       case MODE_FADE:
            // Gestito da FadeEffect::update() nel loop principale
            return;

  case MODE_SLOW:
            // Gestito da SlowEffect::update() nel loop principale
            return;


       default:
           ClockDisplay::displayWord(hourWord, currentColor);
           if(currentMinute > 0) {
               ClockDisplay::displayWord(WORD_E, currentColor);
               ClockDisplay::showMinutes(currentMinute, currentColor);
               ClockDisplay::displayWord(WORD_MINUTI, currentColor);
           }
   }
   
   if(currentMinute > 0 && currentMode != MODE_SLOW) {
       ClockDisplay::showSeconds(currentSecond, currentColor);
   }
   
   FastLED.show();
}

void ClockDisplay::showMinutes(uint8_t minutes, CRGB color) {
   static const uint8_t* const minuteWords[] PROGMEM = {
       nullptr,
       WORD_MUNO,
       WORD_MDUE,
       WORD_MTRE,
       WORD_MQUATTRO,
       WORD_MCINQUE,
       WORD_MSEI,
       WORD_MSETTE,
       WORD_MOTTO,
       WORD_MNOVE,
       WORD_MDIECI,
       WORD_MUNDICI,
       WORD_MDODICI,
       WORD_MTREDICI,
       WORD_MQUATTORDICI,
       WORD_MQUINDICI,
       WORD_MSEDICI,
       WORD_MDICIASSETTE,
       WORD_MDICIOTTO,
   WORD_MDICIANNOVE
   };

   if(minutes <= 0) return;

   if(minutes <= 19) {
       ClockDisplay::displayWord((const uint8_t*)pgm_read_ptr(&minuteWords[minutes]), color);
   } else {
       uint8_t tens = minutes / 10;
       uint8_t ones = minutes % 10;
       
       switch(tens) {
           case 2:
               ClockDisplay::displayWord(ones == 1 || ones == 8 ? WORD_MVENT : WORD_MVENTI, color);
               break;
           case 3:
               ClockDisplay::displayWord(ones == 1 || ones == 8 ? WORD_MTRENT : WORD_MTRENTA, color);
               break;
           case 4:
               ClockDisplay::displayWord(ones == 1 || ones == 8 ? WORD_MQUARANT : WORD_MQUARANTA, color);
               break;
           case 5:
               ClockDisplay::displayWord(ones == 1 || ones == 8 ? WORD_MCINQUANT : WORD_MCINQUANTA, color);
               break;
       }
       
       if(ones > 0) {
           if (ones == 1) {
                ClockDisplay::displayWord(WORD_MUN, color);
           } else {
                ClockDisplay::displayWord((const uint8_t*)pgm_read_ptr(&minuteWords[ones]), color);
           }    
       }
   }
}

void ClockDisplay::showSecondsM(uint8_t seconds, CRGB color) {
    if(currentMinute >= 1) {
    uint8_t segment = seconds / 10;
    
      for(uint8_t i = 0; i < 6; i++) {
          if(i <= segment) {
              if(i == segment) {
                  uint8_t hue = beat8(60);
                  leds[250 + i] = (seconds % 2 == 0) ? 
                      CHSV(hue, 255, 255) : color;
              } else {
                  leds[250 + i] = color;
              }
          } else {
              leds[250 + i] = color;
          }
      }
    }
}

void ClockDisplay::showSeconds(uint8_t seconds, CRGB color) {
    if(currentBlink == 1) {
        uint8_t hue = beat8(60);
        leds[116] = (seconds % 2 == 0) ? 
        CHSV(hue, 255, 255) : color;
    } else {
        leds[116] = color; 
    }
}

void ClockDisplay::displayWordToTarget(const uint8_t* word) {
   uint8_t idx = 0;
   uint8_t pixel;
   while((pixel = pgm_read_byte(&word[idx])) != 8) {
       targetPixels[pixel] = true;
       idx++;
   }
}

void ClockDisplay::displayMinutesToTarget(uint8_t minutes) {
   if(minutes <= 0) return;

   if(minutes <= 19) {
       const uint8_t* minuteWord = (const uint8_t*)pgm_read_ptr(&MINUTE_WORDS[minutes]);
       ClockDisplay::displayWordToTarget(minuteWord);
   } else {
       uint8_t tens = (minutes / 10) - 2;
       uint8_t ones = minutes % 10;
       const MinuteTens* tensWords = &TENS_WORDS[tens];
       
       const uint8_t* decinaWord;
       if(ones == 8 || (ones == 1 && minutes >= 21)) {
           decinaWord = (const uint8_t*)pgm_read_ptr(&tensWords->truncated);
       } else {
           decinaWord = (const uint8_t*)pgm_read_ptr(&tensWords->normal);
       }
       
       ClockDisplay::displayWordToTarget(decinaWord);
       
       if(ones > 0) {
          const uint8_t* onesWord = (const uint8_t*)pgm_read_ptr(&MINUTE_WORDS[ones]);
          if (ones == 1) {
             onesWord = (WORD_MUN);
          }    
          ClockDisplay::displayWordToTarget(onesWord);
       }
   }
}

void ClockDisplay::paintWordSlow(const uint8_t arrWord[], CRGB color) {
   uint8_t i = 0;
   uint8_t pixel;
   
   while((pixel = pgm_read_byte(&arrWord[i])) != 8 && i < NUM_LEDS) {
       leds[pixel] = color;
       FastLED.show();
       delay(500);
       yield();
       i++;
   }
}

void ClockDisplay::paintWordFast(const uint8_t arrWord[], CRGB color) {
   uint8_t i = 0;
   uint8_t pixel;
   
   while((pixel = pgm_read_byte(&arrWord[i])) != 8 && i < NUM_LEDS) {
       leds[pixel] = color;
       i++;
   }
   FastLED.show();
}

void ClockDisplay::paintWordSpeed(const uint8_t arrWord[], CRGB color, int speed) {
   uint8_t i = 0;
   uint8_t pixel;
   
   while((pixel = pgm_read_byte(&arrWord[i])) != 8 && i < NUM_LEDS) {
       leds[pixel] = color;
       FastLED.show();
       delay(speed);
       yield();
       i++;
   }
}

void ClockDisplay::showCurrentTime() {
    CRGB textColor = CRGB::White;

    ClockDisplay::displayWord(WORD_SONO_LE, textColor);

    const uint8_t* hourWord = (const uint8_t*)pgm_read_ptr(&HOUR_WORDS[currentHour]);
    ClockDisplay::displayWord(hourWord, textColor);

    if(currentMinute > 0) {
        ClockDisplay::displayWord(WORD_E, textColor);
        ClockDisplay::showMinutes(currentMinute, textColor);
        ClockDisplay::displayWord(WORD_MINUTI, textColor);
    }

    if(currentMinute > 0 && currentBlink == 1) {
        ClockDisplay::showSeconds(currentSecond, textColor);
    }
}
