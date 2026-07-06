#include "effects/effect_matrix.h"
#include "state.h"
#include "words_it.h"
#include "clock_display.h"
#include "led_canvas.h"

Drop drops[NUM_DROPS];
bool matrixInitialized = false;

void MatrixEffect::init() {
   memset(targetPixels, 0, sizeof(targetPixels));
   memset(activePixels, 0, sizeof(activePixels));
   
   ClockDisplay::displayWordToTarget(WORD_SONO_LE);
   
   uint8_t hour12 = currentHour % 12;
   const uint8_t* hourWord = (const uint8_t*)pgm_read_ptr(&HOUR_WORDS[hour12]);
   ClockDisplay::displayWordToTarget(hourWord);
   
   if(currentMinute > 0) {
       ClockDisplay::displayWordToTarget(WORD_E);
       ClockDisplay::displayMinutesToTarget(currentMinute);
       ClockDisplay::displayWordToTarget(WORD_MINUTI);
   }
   
   for(int i = 0; i < NUM_DROPS; i++) {
       initDrop(drops[i]);
   }
   
   matrixInitialized = true;
}

void initDrop(Drop &drop) {
    drop.x = random8(MATRIX_WIDTH);
    drop.y = random(MATRIX_START_Y_MIN, MATRIX_START_Y_MAX);
    if (drop.isMatrix2) {
        drop.speed = MATRIX2_BASE_SPEED + (random(100) / 100.0f * MATRIX2_SPEED_VAR);
    } else {
        drop.speed = MATRIX_BASE_SPEED + (random(100) / 100.0f * MATRIX_SPEED_VAR);
    }
    drop.active = true;
}

void setDropType(Drop &drop, bool isMatrix2) {
    drop.isMatrix2 = isMatrix2;
    initDrop(drop);
}

void MatrixEffect::update() {
   static uint8_t lastHour = 255;
   static uint8_t lastMinute = 255;

   if(currentHour != lastHour || currentMinute != lastMinute) {
       FastLED.clear();
       memset(targetPixels, 0, sizeof(targetPixels));
       memset(activePixels, 0, sizeof(activePixels));

       ClockDisplay::displayWordToTarget(WORD_SONO_LE);
       const uint8_t* hourWord = (const uint8_t*)pgm_read_ptr(&HOUR_WORDS[currentHour]);
       ClockDisplay::displayWordToTarget(hourWord);
       
       if(currentMinute > 0) {
           ClockDisplay::displayWordToTarget(WORD_E);
           ClockDisplay::displayMinutesToTarget(currentMinute);
           ClockDisplay::displayWordToTarget(WORD_MINUTI);
       }
       
       for(int i = 0; i < NUM_DROPS; i++) {
           drops[i].isMatrix2 = false;
           initDrop(drops[i]);
       }
       
       lastHour = currentHour;
       lastMinute = currentMinute;
   }
   
   FastLED.clear();
   
   for(uint8_t i = 0; i < NUM_DROPS; i++) {
       Drop &drop = drops[i];
       if(!drop.active) continue;

       int yInt = (int)drop.y;

       if(yInt >= 0 && yInt < MATRIX_HEIGHT) {
           uint16_t pos = Canvas::xyToLED(drop.x, yInt);

           if(pos < NUM_LEDS) {
               if(targetPixels[pos] && !activePixels[pos]) {
                   activePixels[pos] = true;
                   leds[pos] = matrixWordColor;
               }
               else if(!targetPixels[pos]) {
                   uint8_t intensity = 255 - (yInt * 16);
                   leds[pos] = CRGB(0, intensity, 0);

                   for(int trail = 1; trail <= MATRIX_TRAIL_LENGTH; trail++) {
                       int trailY = yInt - trail;
                       if(trailY >= 0) {
                           uint16_t trailPos = Canvas::xyToLED(drop.x, trailY);
                           if(trailPos < NUM_LEDS && !targetPixels[trailPos]) {
                               leds[trailPos] = CRGB(0, intensity/(trail*2), 0);
                           }
                       }
                   }
               }
           }
       }

       drop.y += drop.speed;

       if(drop.y >= MATRIX_HEIGHT) {
           drop.isMatrix2 = false;
           initDrop(drop);
       }
   }
   
   for(uint16_t i = 0; i < NUM_LEDS; i++) {
       if(targetPixels[i] && activePixels[i]) {
           leds[i] = matrixWordColor;
       }
   }
   
   FastLED.show();
}
