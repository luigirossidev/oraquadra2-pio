#include "effects/effect_digitale.h"
#include "state.h"
#include "config.h"
#include "clock_colors.h"
#include "led_canvas.h"


void DigitaleEffect::update() {
    uint32_t currentMillis = millis();
    
    // Aggiornamento periodico
    if (currentMillis - lastDigitaleUpdate < DIGITALE_UPDATE_INTERVAL) return;
    lastDigitaleUpdate = currentMillis;
    
    // Cancella tutto
    FastLED.clear();
    
    // Ottieni il colore corrente dal sistema
    CRGB displayColor = clockColors.getColor();
    
    // Calcola le cifre da visualizzare
    uint8_t hourTens = currentHour / 10;
    uint8_t hourOnes = currentHour % 10;
    uint8_t minuteTens = currentMinute / 10;
    uint8_t minuteOnes = currentMinute % 10;
    
    // Layout display:
    // Le ore occupano la parte alta (righe 1-7)
    // I minuti occupano la parte bassa (righe 9-15)
    // Il punto centrale nella riga 8 (centro display)
    
    // Posizioni orizzontali centrate per le cifre
    // ORE IN ALTO (righe 1-7)
    uint8_t hourY = 0;
    uint8_t hourX1 = 3;   // Prima cifra ora
    uint8_t hourX2 = 9;   // Seconda cifra ora
    
    Canvas::drawDigitCentered(hourX1, hourY, hourTens, displayColor);
    Canvas::drawDigitCentered(hourX2, hourY, hourOnes, displayColor);
    
    // MINUTI IN BASSO (righe 10-16)
    uint8_t minuteY = 9;
    uint8_t minuteX1 = 3;  // Prima cifra minuti
    uint8_t minuteX2 = 9;  // Seconda cifra minuti
    
    Canvas::drawDigitCentered(minuteX1, minuteY, minuteTens, displayColor);
    Canvas::drawDigitCentered(minuteX2, minuteY, minuteOnes, displayColor);
    
    // PUNTO LAMPEGGIANTE CENTRALE
    // Solo se il lampeggio è attivato (currentBlink == 1)
    if (currentBlink == 1) {
        // secondi / seconds
        if (currentSecond % 2 == 0) {
            // Punto centrale della matrice 16x16:  x=8,  y=8
            uint16_t centerPos = Canvas::xyToLED(8, 8);
            if (centerPos < NUM_LEDS) {
                leds[centerPos] = displayColor;
            }
        }
    } else {
        // Se il lampeggio è disattivato, spegni il punto fisso
        uint16_t centerPos = Canvas::xyToLED(8, 8);
        if (centerPos < NUM_LEDS) {
            leds[centerPos] =  CRGB::Black; 
        }
    }
    
    // Mostra tutto
    FastLED.show();
}
