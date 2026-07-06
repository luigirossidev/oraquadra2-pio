#include "effects/effect_fade.h"
#include "state.h"
#include "words_it.h"
#include "clock_colors.h"
#include "clock_display.h"

FadeEffectState fadeState;

// Funzione non-bloccante per effetto FADE
void FadeEffect::update() {
    uint32_t now = millis();
    CRGB currentColor = clockColors.getColor();
    const uint8_t* hourWord = (const uint8_t*)pgm_read_ptr(&HOUR_WORDS[currentHour]);

    // secondi / seconds
    if (fadeState.completed) {
        FastLED.clear();
        ClockDisplay::displayWord(WORD_SONO_LE, currentColor);
        ClockDisplay::displayWord(hourWord, currentColor);
        if (currentMinute > 0) {
            ClockDisplay::displayWord(WORD_E, currentColor);
            ClockDisplay::showMinutes(currentMinute, currentColor);
            ClockDisplay::displayWord(WORD_MINUTI, currentColor);
            ClockDisplay::showSeconds(currentSecond, currentColor);
        }
        FastLED.show();
        return;
    }

    // Inizializza l'effetto se non attivo
    if (!fadeState.active) {
        fadeState.active = true;
        fadeState.phase = 0;
        fadeState.step = 0;
        fadeState.lastStepTime = now;
        FastLED.clear();
    }

    // Attendi 50ms tra ogni step
    if (now - fadeState.lastStepTime < 50) {
        return;
    }
    fadeState.lastStepTime = now;

    // Calcola luminosità corrente
    uint8_t brightness = map(fadeState.step, 0, fadeState.STEPS - 1, 0, 255);
    CRGB fadeColor = currentColor;
    fadeColor.nscale8(brightness);

    // Mostra le parole già completate a piena luminosità
    FastLED.clear();

    // Parole già completate (piena luminosità)
    if (fadeState.phase > 0) {
        ClockDisplay::displayWord(WORD_SONO_LE, currentColor);
    }
    if (fadeState.phase > 1) {
        ClockDisplay::displayWord(hourWord, currentColor);
    }
    if (fadeState.phase > 2 && currentMinute > 0) {
        ClockDisplay::displayWord(WORD_E, currentColor);
    }
    if (fadeState.phase > 3 && currentMinute > 0) {
        ClockDisplay::showMinutes(currentMinute, currentColor);
    }
    if (fadeState.phase > 4 && currentMinute > 0) {
        ClockDisplay::displayWord(WORD_MINUTI, currentColor);
        ClockDisplay::showSeconds(currentSecond, currentColor);
    }

    // Parola corrente in fade
    switch (fadeState.phase) {
        case 0: // SONO LE
            ClockDisplay::displayWord(WORD_SONO_LE, fadeColor);
            break;
        case 1: // Ora
            ClockDisplay::displayWord(WORD_SONO_LE, currentColor);
            ClockDisplay::displayWord(hourWord, fadeColor);
            break;
        case 2: // E
            if (currentMinute > 0) {
                ClockDisplay::displayWord(WORD_E, fadeColor);
            }
            break;
        case 3: // Minuti numero
            if (currentMinute > 0) {
                ClockDisplay::showMinutes(currentMinute, fadeColor);
            }
            break;
        case 4: // MINUTI parola
            if (currentMinute > 0) {
                ClockDisplay::displayWord(WORD_MINUTI, fadeColor);
            }
            break;
    }

    FastLED.show();

    // Avanza step
    fadeState.step++;
    if (fadeState.step >= fadeState.STEPS) {
        fadeState.step = 0;
        fadeState.phase++;

        // Se non ci sono minuti, salta le fasi 2,3,4
        if (currentMinute == 0 && fadeState.phase == 2) {
            fadeState.phase = 5;
        }

        // Completato - aspetta cambio minuto
        if (fadeState.phase > 4) {
            fadeState.completed = true;
        }
    }
}

// Reset al cambio minuto (statement identici al vecchio loop)
void FadeEffect::reset() {
    fadeState.active = false;
    fadeState.completed = false;
}
