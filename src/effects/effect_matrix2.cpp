#include "effects/effect_matrix2.h"
#include "state.h"
#include "words_it.h"
#include "clock_display.h"
#include "led_canvas.h"

Matrix2State matrix2State = {false, 0, true};

void Matrix2Effect::update() {
    static uint8_t lastHour = 255;
    static uint8_t lastMinute = 255;
    
    if(currentHour != lastHour || currentMinute != lastMinute || matrix2State.needsReset) {
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
            drops[i].isMatrix2 = true;
            initDrop(drops[i]);
        }
        
        lastHour = currentHour;
        lastMinute = currentMinute;
        matrix2State.completed = false;
        matrix2State.needsReset = false;
    }

    if(matrix2State.completed) {
        FastLED.clear();
        // Mostra orario direttamente
        ClockDisplay::displayWord(WORD_SONO_LE, matrixWordColor);
        const uint8_t* hourWord = (const uint8_t*)pgm_read_ptr(&HOUR_WORDS[currentHour]);
        ClockDisplay::displayWord(hourWord, matrixWordColor);
        if (currentMinute > 0) {
            ClockDisplay::displayWord(WORD_E, matrixWordColor);
            ClockDisplay::showMinutes(currentMinute, matrixWordColor);
            ClockDisplay::displayWord(WORD_MINUTI, matrixWordColor);
            ClockDisplay::showSeconds(currentSecond, matrixWordColor);
        }
        FastLED.show();
        return;
    }

    FastLED.clear();
    bool allTargetPixelsActive = true;

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
            if(!matrix2State.completed) {
                drop.isMatrix2 = true;
                initDrop(drop);
            } else {
                drop.active = false;
            }
        }
    }

    for(uint16_t i = 0; i < NUM_LEDS; i++) {
        if(targetPixels[i] && !activePixels[i]) {
            allTargetPixelsActive = false;
            break;
        }
    }

    if(allTargetPixelsActive && !matrix2State.completed) {
        matrix2State.completed = true;
        matrix2State.completionTime = millis();
    }

    for(uint16_t i = 0; i < NUM_LEDS; i++) {
        if(targetPixels[i] && activePixels[i]) {
            leds[i] = matrixWordColor;
        }
    }

    FastLED.show();
}

// Reset al cambio minuto (statement identici al vecchio loop)
void Matrix2Effect::reset() {
    matrix2State.needsReset = true;
    matrix2State.completed = false;
}
