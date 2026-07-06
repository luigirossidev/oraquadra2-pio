#include "effects/effect_drop.h"
#include "state.h"
#include "words_it.h"
#include "clock_display.h"
#include "led_canvas.h"

DropState dropState;

// Effetto GOCCIA - onde concentriche che rivelano l'orario
void DropEffect::update() {
    uint32_t now = millis();

    // secondi / seconds
    if (dropState.completed) {
        FastLED.clear();
        CRGB textColor = dropState.waveColor;
        ClockDisplay::displayWord(WORD_SONO_LE, textColor);
        const uint8_t* hourWord = (const uint8_t*)pgm_read_ptr(&HOUR_WORDS[currentHour]);
        ClockDisplay::displayWord(hourWord, textColor);
        if (currentMinute > 0) {
            ClockDisplay::displayWord(WORD_E, textColor);
            ClockDisplay::showMinutes(currentMinute, textColor);
            ClockDisplay::displayWord(WORD_MINUTI, textColor);
            ClockDisplay::showSeconds(currentSecond, textColor);
        }
        FastLED.show();
        return;
    }

    // Inizializza effetto
    if (!dropState.active) {
        dropState.active = true;
        dropState.radius = 0;
        dropState.centerX = 7 + random8(3);  // Centro leggermente casuale
        dropState.centerY = 7 + random8(3);
        dropState.startTime = now;
        dropState.lastUpdate = now;
        dropState.waveColor = CHSV(random8(), 200, 255);  // Colore casuale
        dropState.waveCount = 0;

        // Prepara i target pixel per l'orario
        memset(targetPixels, 0, sizeof(targetPixels));
        ClockDisplay::displayWordToTarget(WORD_SONO_LE);
        const uint8_t* hourWord = (const uint8_t*)pgm_read_ptr(&HOUR_WORDS[currentHour]);
        ClockDisplay::displayWordToTarget(hourWord);
        if (currentMinute > 0) {
            ClockDisplay::displayWordToTarget(WORD_E);
            ClockDisplay::displayMinutesToTarget(currentMinute);
            ClockDisplay::displayWordToTarget(WORD_MINUTI);
        }
    }

    // Aggiornamento frame
    if (now - dropState.lastUpdate < DROP_UPDATE_INTERVAL) {
        return;
    }
    dropState.lastUpdate = now;

    FastLED.clear();

    // Espandi il raggio
    dropState.radius += DROP_WAVE_SPEED;

    // Se l'onda è uscita, effetto completato (solo una volta)
    if (dropState.radius > DROP_MAX_RADIUS) {
        dropState.completed = true;
        return;
    }

    // Calcola il centro della matrice
    float cx = dropState.centerX;
    float cy = dropState.centerY;

    // Disegna le onde concentriche
    for (uint8_t y = 0; y < MATRIX_HEIGHT; y++) {
        for (uint8_t x = 0; x < MATRIX_WIDTH; x++) {
            // Calcola distanza dal centro
            float dx = x - cx;
            float dy = y - cy;
            float dist = sqrt(dx * dx + dy * dy);

            uint16_t pos = Canvas::xyToLED(x, y);
            if (pos >= NUM_LEDS) continue;

            // Crea effetto onda (anello che si espande)
            float waveWidth = 2.5f;  // Larghezza dell'onda
            float waveDist = abs(dist - dropState.radius);

            if (waveDist < waveWidth) {
                // Intensità basata sulla distanza dall'onda
                uint8_t intensity = 255 - (uint8_t)(waveDist * 100);

                // Se è un pixel dell'orario, coloralo di più
                if (targetPixels[pos]) {
                    leds[pos] = dropState.waveColor;
                    leds[pos].nscale8(intensity);
                } else {
                    // Onda blu/ciano per l'acqua
                    CRGB waterColor = CHSV(160, 255, intensity / 2);
                    leds[pos] = waterColor;
                }
            }

            // I pixel dell'orario già "toccati" dalle onde rimangono accesi
            if (targetPixels[pos] && dist < dropState.radius - waveWidth) {
                // Fade in graduale per i pixel rivelati
                int calcIntensity = (int)((dropState.radius - dist) * 20);
                uint8_t revealIntensity = (calcIntensity > 255) ? 255 : (uint8_t)calcIntensity;
                CRGB textColor = dropState.waveColor;
                textColor.nscale8(revealIntensity);
                if (leds[pos].getAverageLight() < textColor.getAverageLight()) {
                    leds[pos] = textColor;
                }
            }
        }
    }

    // Effetto "impatto" al centro quando la goccia cade
    if (dropState.radius < 3) {
        uint16_t centerPos = Canvas::xyToLED(dropState.centerX, dropState.centerY);
        if (centerPos < NUM_LEDS) {
            leds[centerPos] = CRGB::White;
        }
    }

    FastLED.show();
}

// Reset al cambio minuto (statement identici al vecchio loop)
void DropEffect::reset() {
    dropState.active = false;
    dropState.completed = false;
}
