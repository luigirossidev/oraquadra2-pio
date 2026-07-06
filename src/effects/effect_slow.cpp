#include "effects/effect_slow.h"
#include "state.h"
#include "words_it.h"
#include "clock_colors.h"
#include "clock_display.h"

SlowEffectState slowState;

// Funzione non-bloccante per effetto SLOW
void SlowEffect::update() {
    uint32_t now = millis();
    const uint8_t* hourWord = (const uint8_t*)pgm_read_ptr(&HOUR_WORDS[currentHour]);

    // secondi / seconds
    if (slowState.completed) {
        FastLED.clear();
        ClockDisplay::displayWord(WORD_SONO_LE, slowState.sonoColor);
        ClockDisplay::displayWord(hourWord, slowState.hourColor);
        if (currentMinute > 0) {
            ClockDisplay::displayWord(WORD_E, slowState.eColor);
            ClockDisplay::showMinutes(currentMinute, slowState.minutesColor);
            ClockDisplay::displayWord(WORD_MINUTI, slowState.minutiColor);
            ClockDisplay::showSeconds(currentSecond, slowState.minutiColor);
        }
        FastLED.show();
        return;
    }

    // Inizializza l'effetto se non attivo
    if (!slowState.active) {
        slowState.active = true;
        slowState.step = 0;
        slowState.targetBrightness = intBrightness;
        slowState.increment = slowState.targetBrightness / 30;
        if (slowState.increment == 0) slowState.increment = 1;
        slowState.lastStepTime = now;
        slowState.holdPhase = false;

        // Imposta i colori
        if (randomColor) {
            slowState.sonoColor = CRGB(random8(), random8(), random8());
            slowState.hourColor = CRGB(random8(), random8(), random8());
            slowState.eColor = CRGB(random8(), random8(), random8());
            slowState.minutesColor = CRGB(random8(), random8(), random8());
            slowState.minutiColor = CRGB(random8(), random8(), random8());
        } else {
            slowState.sonoColor = clockColors.current;
            slowState.hourColor = clockColors.current;
            slowState.eColor = clockColors.current;
            slowState.minutesColor = clockColors.current;
            slowState.minutiColor = clockColors.current;
        }
    }

    // Fase di hold (mantieni acceso)
    if (slowState.holdPhase) {
        // secondi / seconds
        FastLED.clear();
        ClockDisplay::displayWord(WORD_SONO_LE, slowState.sonoColor);
        ClockDisplay::displayWord(hourWord, slowState.hourColor);
        if (currentMinute > 0) {
            ClockDisplay::displayWord(WORD_E, slowState.eColor);
            ClockDisplay::showMinutes(currentMinute, slowState.minutesColor);
            ClockDisplay::displayWord(WORD_MINUTI, slowState.minutiColor);
            ClockDisplay::showSeconds(currentSecond, slowState.minutiColor);
        }
        FastLED.show();

        if (now - slowState.holdStartTime >= 500) {
            // Effetto completato - aspetta cambio minuto
            slowState.completed = true;
        }
        return;
    }

    // Fase di fade-in
    if (now - slowState.lastStepTime >= 70) {
        slowState.lastStepTime = now;

        uint8_t currentBrightLevel = slowState.step * slowState.increment;
        if (currentBrightLevel > slowState.targetBrightness) {
            currentBrightLevel = slowState.targetBrightness;
        }

        FastLED.clear();

        uint8_t scaleFactor = map(currentBrightLevel, 0, slowState.targetBrightness, 0, 255);

        CRGB sonoFade = slowState.sonoColor;
        sonoFade.nscale8(scaleFactor);
        ClockDisplay::displayWord(WORD_SONO_LE, sonoFade);

        CRGB hourFade = slowState.hourColor;
        hourFade.nscale8(scaleFactor);
        ClockDisplay::displayWord(hourWord, hourFade);

        if (currentMinute > 0) {
            CRGB eFade = slowState.eColor;
            eFade.nscale8(scaleFactor);
            ClockDisplay::displayWord(WORD_E, eFade);

            CRGB minutesFade = slowState.minutesColor;
            minutesFade.nscale8(scaleFactor);
            ClockDisplay::showMinutes(currentMinute, minutesFade);

            CRGB minutiFade = slowState.minutiColor;
            minutiFade.nscale8(scaleFactor);
            ClockDisplay::displayWord(WORD_MINUTI, minutiFade);
        }

        FastLED.show();

        slowState.step++;

        // Se abbiamo raggiunto la luminosità target, passa alla fase hold
        if (currentBrightLevel >= slowState.targetBrightness) {
            slowState.holdPhase = true;
            slowState.holdStartTime = now;
        }
    }
}

// Reset al cambio minuto (statement identici al vecchio loop)
void SlowEffect::reset() {
    slowState.active = false;
    slowState.completed = false;
}
