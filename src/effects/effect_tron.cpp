#include "effects/effect_tron.h"
#include "state.h"
#include "words_it.h"
#include "clock_display.h"
#include "led_canvas.h"

TronBike tronBikes[NUM_TRON_BIKES];
uint8_t tronIntensity[NUM_LEDS] = {0};
uint32_t lastTronUpdate = 0;
uint32_t tronCycleStart = 0;
const uint32_t TRON_EFFECT_DURATION = 8000;
const uint32_t TEXT_ONLY_DURATION = 10000;
bool tronEffectActive = true;
bool tronNeedsInit = true;
uint8_t tronLastMinute = 255;

// Inizializza le moto TRON in posizioni diverse
void TronEffect::init() {
    // Colori delle moto (stile TRON)
    CRGB bikeColors[NUM_TRON_BIKES] = {
        CRGB(0, 200, 255),    // Ciano (protagonista)
        CRGB(255, 100, 0),    // Arancione
        CRGB(200, 0, 255)     // Viola
    };

    // Posizioni di partenza diverse (ai bordi della matrice)
    for(uint8_t i = 0; i < NUM_TRON_BIKES; i++) {
        tronBikes[i].active = true;
        tronBikes[i].color = bikeColors[i];
        tronBikes[i].trailLen = 0;

        switch(i) {
            case 0: // Parte da sinistra, va a destra
                tronBikes[i].x = 0;
                tronBikes[i].y = 4;
                tronBikes[i].dir = 1;
                break;
            case 1: // Parte da destra, va a sinistra
                tronBikes[i].x = MATRIX_WIDTH - 1;
                tronBikes[i].y = 11;
                tronBikes[i].dir = 3;
                break;
            case 2: // Parte dal basso, va su
                tronBikes[i].x = 8;
                tronBikes[i].y = MATRIX_HEIGHT - 1;
                tronBikes[i].dir = 0;
                break;
        }
    }

    // Reset intensità scie
    memset(tronIntensity, 0, sizeof(tronIntensity));
    tronNeedsInit = false;
}

// Controlla se una posizione è libera (no collisione)
bool tronPosIsFree(int8_t x, int8_t y, uint8_t bikeIndex) {
    if(x < 0 || x >= MATRIX_WIDTH || y < 0 || y >= MATRIX_HEIGHT) {
        return false;
    }

    uint16_t pos = Canvas::xyToLED(x, y);
    if(pos >= NUM_LEDS) return false;

    // Controlla se c'è una scia
    if(tronIntensity[pos] > 50) {
        return false;
    }

    return true;
}

// Trova una direzione sicura per la moto
int8_t tronFindSafeDir(uint8_t bikeIndex) {
    TronBike &bike = tronBikes[bikeIndex];

    // Direzioni: 0=su, 1=destra, 2=giù, 3=sinistra
    int8_t dx[] = {0, 1, 0, -1};
    int8_t dy[] = {-1, 0, 1, 0};

    // Prima prova a continuare dritto
    int8_t newX = bike.x + dx[bike.dir];
    int8_t newY = bike.y + dy[bike.dir];
    if(tronPosIsFree(newX, newY, bikeIndex)) {
        // 70% continua dritto, 30% gira
        if(random8(100) < 70) {
            return bike.dir;
        }
    }

    // Prova le altre direzioni (no inversione)
    int8_t possibleDirs[3];
    uint8_t numPossible = 0;

    for(int8_t d = 0; d < 4; d++) {
        // Non tornare indietro
        if((d + 2) % 4 == bike.dir) continue;

        newX = bike.x + dx[d];
        newY = bike.y + dy[d];
        if(tronPosIsFree(newX, newY, bikeIndex)) {
            possibleDirs[numPossible++] = d;
        }
    }

    if(numPossible > 0) {
        return possibleDirs[random8(numPossible)];
    }

    // Nessuna direzione sicura - moto si ferma
    return -1;
}

void TronEffect::update() {
    uint32_t currentMillis = millis();


    if (currentMinute != tronLastMinute) {
        tronLastMinute = currentMinute;
        tronNeedsInit = true;
        tronEffectActive = true;
        return;
    }

    // Se non attivo e non deve partire, mostra solo orario
    if (!tronEffectActive && !tronNeedsInit) {
        FastLED.clear();
        ClockDisplay::showCurrentTime();
        FastLED.show();
        return;
    }

    if(tronCycleStart == 0) {
        tronCycleStart = currentMillis;
    }

    uint32_t cycleElapsed = currentMillis - tronCycleStart;

    // Gestione ciclo effetto/testo
    if(tronEffectActive && cycleElapsed > TRON_EFFECT_DURATION) {
        tronEffectActive = false;
        tronCycleStart = currentMillis;
        tronNeedsInit = false;  // Non ripartire fino al cambio minuto
        FastLED.clear();
        ClockDisplay::showCurrentTime();
        FastLED.show();
        return;
    }

    // se necessario / if needed
    if(tronNeedsInit) {
        TronEffect::init();
    }

    if(currentMillis - lastTronUpdate < TRON_UPDATE_INTERVAL) return;
    lastTronUpdate = currentMillis;

    FastLED.clear();

    if(tronEffectActive) {
        // Fade delle scie
        for(uint16_t i = 0; i < NUM_LEDS; i++) {
            if(tronIntensity[i] > 15) {
                tronIntensity[i] -= 15;
            } else {
                tronIntensity[i] = 0;
            }
        }

        // Muovi ogni moto
        for(uint8_t i = 0; i < NUM_TRON_BIKES; i++) {
            TronBike &bike = tronBikes[i];
            if(!bike.active) continue;

            // Trova direzione sicura
            int8_t newDir = tronFindSafeDir(i);

            if(newDir < 0) {
                // Moto bloccata - riposiziona
                bike.x = random8(MATRIX_WIDTH);
                bike.y = random8(MATRIX_HEIGHT);
                bike.dir = random8(4);
                bike.trailLen = 0;
                continue;
            }

            bike.dir = newDir;

            // Calcola nuova posizione
            int8_t dx[] = {0, 1, 0, -1};
            int8_t dy[] = {-1, 0, 1, 0};

            // Salva posizione nella scia
            if(bike.trailLen < TRON_TRAIL_LENGTH) {
                bike.trailX[bike.trailLen] = bike.x;
                bike.trailY[bike.trailLen] = bike.y;
                bike.trailLen++;
            } else {
                // Shift scia
                for(uint8_t t = 0; t < TRON_TRAIL_LENGTH - 1; t++) {
                    bike.trailX[t] = bike.trailX[t + 1];
                    bike.trailY[t] = bike.trailY[t + 1];
                }
                bike.trailX[TRON_TRAIL_LENGTH - 1] = bike.x;
                bike.trailY[TRON_TRAIL_LENGTH - 1] = bike.y;
            }

            // Aggiorna intensità scia nella posizione attuale
            uint16_t pos = Canvas::xyToLED(bike.x, bike.y);
            if(pos < NUM_LEDS) {
                tronIntensity[pos] = 255;
            }

            // Muovi
            bike.x += dx[bike.dir];
            bike.y += dy[bike.dir];

            // Wrap around ai bordi
            if(bike.x < 0) bike.x = MATRIX_WIDTH - 1;
            if(bike.x >= MATRIX_WIDTH) bike.x = 0;
            if(bike.y < 0) bike.y = MATRIX_HEIGHT - 1;
            if(bike.y >= MATRIX_HEIGHT) bike.y = 0;
        }

        // Disegna scie con fade
        for(uint16_t i = 0; i < NUM_LEDS; i++) {
            if(tronIntensity[i] > 0) {
                // Trova a quale moto appartiene basandosi sulla posizione
                leds[i] = CHSV(160, 255, tronIntensity[i]); // Default ciano
            }
        }

        // Disegna moto (teste luminose)
        for(uint8_t i = 0; i < NUM_TRON_BIKES; i++) {
            TronBike &bike = tronBikes[i];
            if(!bike.active) continue;

            uint16_t pos = Canvas::xyToLED(bike.x, bike.y);
            if(pos < NUM_LEDS) {
                leds[pos] = bike.color;
                leds[pos].maximizeBrightness();
            }

            // Disegna scia colorata
            for(uint8_t t = 0; t < bike.trailLen; t++) {
                uint16_t trailPos = Canvas::xyToLED(bike.trailX[t], bike.trailY[t]);
                if(trailPos < NUM_LEDS) {
                    CRGB trailColor = bike.color;
                    trailColor.nscale8(180 - (t * 12));
                    leds[trailPos] = trailColor;
                }
            }
        }

        ClockDisplay::showCurrentTime();

    } else {
        ClockDisplay::showCurrentTime();
    }

    FastLED.show();
}

// Reset al cambio minuto (statement identici al vecchio loop)
void TronEffect::reset() {
    tronNeedsInit = true;
    tronEffectActive = true;
    tronCycleStart = 0;
}
