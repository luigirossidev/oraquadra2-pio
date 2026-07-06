#include "effects/effect_moto.h"
#include "state.h"
#include "words_it.h"
#include "clock_display.h"
#include "led_canvas.h"

// Variabili per effetto MOTO
uint32_t lastMotoUpdate = 0;
uint16_t motoCurrentPosition = 0;
bool motoEffectActive = false;
bool motoCompleted = false;
uint32_t motoCompletionTime = 0;
CRGB motoTrailColors[MOTO_TRAIL_LENGTH];
uint8_t motoTrailHue = 0;
uint8_t motoTextHue = 0;
CRGB motoTextColor = CRGB::White;
bool motoTextPixels[NUM_LEDS] = {false};
// Posizione e direzione moto per zigzag
int8_t motoX = 0;
int8_t motoY = 0;
int8_t motoDirX = 1;    // direzione orizzontale
int8_t motoDirY = 1;    // direzione verticale (per quando cambia riga)
uint16_t motoTrailX[MOTO_TRAIL_LENGTH];
uint16_t motoTrailY[MOTO_TRAIL_LENGTH];
uint8_t motoTrailLen = 0;
uint8_t motoLastMinute = 255;

// Funzioni per effetto MOTO
// Variabili per tracciare l'ultimo orario visualizzato dall'effetto Moto
static uint8_t motoLastHour = 255;

void MotoEffect::init() {
    FastLED.clear();
    motoCurrentPosition = 0;
    motoEffectActive = true;
    motoCompleted = false;
    motoCompletionTime = 0;
    motoTrailHue = 0;
    motoTextHue = random8();
    motoTextColor = CHSV(motoTextHue, 255, 255);
    motoTrailLen = 0;

    // Salva l'orario corrente per rilevare cambiamenti
    motoLastHour = currentHour;
    motoLastMinute = currentMinute;

    // Scegli angolo casuale di partenza (deve partire da un angolo per coprire tutto lo schermo)
    uint8_t corner = random8(4);
    switch(corner) {
        case 0: // Alto-sinistra, va verso destra e giù
            motoX = 0;
            motoY = 0;
            motoDirX = 1;
            motoDirY = 1;
            break;
        case 1: // Alto-destra, va verso sinistra e giù
            motoX = MATRIX_WIDTH - 1;
            motoY = 0;
            motoDirX = -1;
            motoDirY = 1;
            break;
        case 2: // Basso-sinistra, va verso destra e su
            motoX = 0;
            motoY = MATRIX_HEIGHT - 1;
            motoDirX = 1;
            motoDirY = -1;
            break;
        case 3: // Basso-destra, va verso sinistra e su
            motoX = MATRIX_WIDTH - 1;
            motoY = MATRIX_HEIGHT - 1;
            motoDirX = -1;
            motoDirY = -1;
            break;
    }

    // Reset array pixel della scritta
    memset(motoTextPixels, 0, sizeof(motoTextPixels));

    for (int i = 0; i < MOTO_TRAIL_LENGTH; i++) {
        motoTrailColors[i] = CRGB::Black;
        motoTrailX[i] = 0;
        motoTrailY[i] = 0;
    }

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

void MotoEffect::update() {
    // 🔧 FIX "E" ore piene MOTO - spegne LED 116 se minuti == 0
    if (currentMinute == 0) {
        motoTextPixels[116] = false;
    }

    uint32_t currentMillis = millis();
    if (currentMinute != motoLastMinute) {
        motoLastMinute = currentMinute;
        MotoEffect::init();
        return;
    }
    // secondi / seconds
    if (motoCompleted) {
        // Controlla se l'orario è cambiato - se sì, reinizializza per mostrare il nuovo orario
        if (motoLastHour != currentHour || motoLastMinute != currentMinute) {
            MotoEffect::init();
            motoLastHour = currentHour;
            motoLastMinute = currentMinute;
            return;
        }

        FastLED.clear();
        for (uint16_t i = 0; i < NUM_LEDS; i++) {
            if (motoTextPixels[i]) {
                leds[i] = motoTextColor;
            }
        }
        ClockDisplay::showSeconds(currentSecond, motoTextColor);
        FastLED.show();
        return;
    }

    // Aggiornamento periodico
    if (currentMillis - lastMotoUpdate < MOTO_SPEED) return;
    lastMotoUpdate = currentMillis;

    // Aggiorna i colori della scia (effetto arcobaleno rotante)
    for (int i = MOTO_TRAIL_LENGTH - 1; i > 0; i--) {
        motoTrailColors[i] = motoTrailColors[i - 1];
    }
    motoTrailColors[0] = CHSV(motoTrailHue, 255, 255);
    motoTrailHue += 8;

    // Salva la posizione corrente nella scia
    for (int i = MOTO_TRAIL_LENGTH - 1; i > 0; i--) {
        motoTrailX[i] = motoTrailX[i - 1];
        motoTrailY[i] = motoTrailY[i - 1];
    }
    motoTrailX[0] = motoX;
    motoTrailY[0] = motoY;
    if (motoTrailLen < MOTO_TRAIL_LENGTH) motoTrailLen++;

    // Segna il pixel corrente come visitato per l'orario
    uint16_t currentPos = Canvas::xyToLED(motoX, motoY);
    if (currentPos < NUM_LEDS && targetPixels[currentPos]) {
        motoTextPixels[currentPos] = true;
    }

    // Cancella tutto
    FastLED.clear();

    // Disegna la scia
    for (int i = 0; i < motoTrailLen; i++) {
        uint16_t pos = Canvas::xyToLED(motoTrailX[i], motoTrailY[i]);
        if (pos < NUM_LEDS) {
            uint8_t intensity = 255 - (i * (255 / MOTO_TRAIL_LENGTH));
            CRGB trailColor = motoTrailColors[i];
            trailColor.nscale8(intensity);
            leds[pos] = trailColor;
        }
    }

    // Disegna i pixel della scritta che sono stati colorati
    for (uint16_t i = 0; i < NUM_LEDS; i++) {
        if (motoTextPixels[i]) {
            leds[i] = motoTextColor;
        }
    }

    FastLED.show();

    // Movimento a zigzag
    motoCurrentPosition++;

    // Prossima posizione orizzontale
    int8_t nextX = motoX + motoDirX;

    // Controllo bordi orizzontali
    if (nextX < 0 || nextX >= MATRIX_WIDTH) {
        // Cambia direzione orizzontale
        motoDirX = -motoDirX;
        // Vai alla prossima riga
        motoY += motoDirY;

        // Controllo se abbiamo finito (usciti dalla matrice verticalmente)
        if (motoY < 0 || motoY >= MATRIX_HEIGHT) {
            motoCompleted = true;
            return;
        }
    } else {
        motoX = nextX;
    }
}

// Reset al cambio minuto (statement identici al vecchio loop)
void MotoEffect::reset() {
    motoEffectActive = false;
    motoCompleted = false;
}
