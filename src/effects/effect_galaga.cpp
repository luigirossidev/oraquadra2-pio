#include "effects/effect_galaga.h"
#include "state.h"
#include "words_it.h"
#include "clock_display.h"
#include "led_canvas.h"

// Variabili per effetto GALAGA
uint32_t lastGalagaUpdate = 0;
bool galagaEffectActive = false;
uint32_t galagaCompletionTime = 0;
uint8_t galagaTextHue = 0;
CRGB galagaTextColor = CRGB::White;
bool galagaTextPixels[NUM_LEDS] = {false};
uint8_t galagaShotHue = 0;

GalagaShot galagaShots[GALAGA_MAX_SHOTS];
uint8_t galagaLastMinute = 255;

void GalagaEffect::init() {
    FastLED.clear();
    galagaEffectActive = true;
    galagaCompletionTime = 0;
    galagaTextHue = random8();
    galagaTextColor = CHSV(galagaTextHue, 255, 255);
    
    // Reset array pixel della scritta
    memset(galagaTextPixels, 0, sizeof(galagaTextPixels));
    
    // Inizializza gli spari
    for (int i = 0; i < GALAGA_MAX_SHOTS; i++) {
        galagaShots[i].active = false;
        galagaShots[i].lastMove = 0;
    }
    
    // Ricalcola i target pixel per l'orario corrente
    memset(targetPixels, 0, sizeof(targetPixels));
    ClockDisplay::displayWordToTarget(WORD_SONO_LE);
    
    const uint8_t* hourWord = (const uint8_t*)pgm_read_ptr(&HOUR_WORDS[currentHour]);
    ClockDisplay::displayWordToTarget(hourWord);
    
    if (currentMinute > 0) {
        ClockDisplay::displayWordToTarget(WORD_E);
        ClockDisplay::displayMinutesToTarget(currentMinute);
        ClockDisplay::displayWordToTarget(WORD_MINUTI);
    }
    
    // Crea i primi spari
    for (int i = 0; i < 3; i++) {
        createNewGalagaShot();
    }
}

void createNewGalagaShot() {
    for (int i = 0; i < GALAGA_MAX_SHOTS; i++) {
        if (!galagaShots[i].active) {
            // Posizione iniziale casuale in basso
            galagaShots[i].x = random8(MATRIX_WIDTH);
            galagaShots[i].y = MATRIX_HEIGHT - 1;
            
            // Trova un target tra le lettere non ancora colpite
            findTargetForShot(galagaShots[i]);
            
            galagaShots[i].active = true;
            galagaShots[i].speed = random8(2, 5);
            galagaShots[i].lastMove = millis();
            break;
        }
    }
}

void findTargetForShot(GalagaShot &shot) {
    // Cerca lettere non ancora colpite
    for (int y = 0; y < MATRIX_HEIGHT; y++) {
        for (int x = 0; x < MATRIX_WIDTH; x++) {
            uint16_t pos = y * MATRIX_WIDTH + x;
            if (targetPixels[pos] && !galagaTextPixels[pos]) {
                // 30% di probabilità di scegliere questo target
                if (random8(100) < 30) {
                    shot.targetX = x;
                    shot.targetY = y;
                    calculateShotDirection(shot);
                    return;
                }
            }
        }
    }
    
    // Se non trova target, va dritto verso l'alto
    shot.targetX = shot.x;
    shot.targetY = 0;
    shot.dx = 0;
    shot.dy = -1;
}

void calculateShotDirection(GalagaShot &shot) {
    int8_t diffX = shot.targetX - shot.x;
    int8_t diffY = shot.targetY - shot.y;
    
    // Normalizza la direzione
    shot.dx = (diffX > 0) ? 1 : (diffX < 0) ? -1 : 0;
    shot.dy = (diffY > 0) ? 1 : (diffY < 0) ? -1 : 0;
    
    // Se il target è sopra, priorità alla salita
    if (diffY < 0) {
        shot.dy = -1;
        // se necessario / if needed
        if (abs(diffX) > 2) {
            shot.dx = (diffX > 0) ? 1 : -1;
        } else {
            shot.dx = 0;
        }
    }
}

void GalagaEffect::update() {
    uint32_t currentMillis = millis();

    if (currentMinute != galagaLastMinute) {
        galagaLastMinute = currentMinute;
        GalagaEffect::init();
        return;
    }


    // Se non attivo, mostra solo orario
    if (!galagaEffectActive) {
        FastLED.clear();
        ClockDisplay::showCurrentTime();
        FastLED.show();
        return;
    }

    // Gestione attesa dopo il completamento
    if (galagaCompletionTime != 0 && currentMillis - galagaCompletionTime < GALAGA_WAIT_AFTER) {
        // Durante l'attesa, mostra solo la scritta dell'orario che è stata colorata
        FastLED.clear();
        for (uint16_t i = 0; i < NUM_LEDS; i++) {
            if (galagaTextPixels[i]) {
                leds[i] = galagaTextColor;
            }
        }
        FastLED.show();
        
        // Crea occasionalmente nuovi spari durante l'attesa per effetto decorativo
        if (random8(100) < 10) {
            createNewGalagaShot();
        }
        return;
    }
    
    // Dopo l'attesa, disattiva effetto (aspetta cambio minuto)
    if (galagaCompletionTime != 0 && currentMillis - galagaCompletionTime >= GALAGA_WAIT_AFTER) {
        galagaEffectActive = false;
        return;
    }
    
    // Aggiornamento periodico
    if (currentMillis - lastGalagaUpdate < GALAGA_SPEED) return;
    lastGalagaUpdate = currentMillis;
    
    // Crea nuovi spari occasionalmente
    if (random8(100) < 15) {
        createNewGalagaShot();
    }
    
    // Cancella tutto
    FastLED.clear();
    
    bool allShotsInactive = true;
    int activeShots = 0;
    
    // Aggiorna e disegna tutti gli spari attivi
    for (int i = 0; i < GALAGA_MAX_SHOTS; i++) {
        if (galagaShots[i].active) {
            allShotsInactive = false;
            activeShots++;
            
            // Muovi lo sparo secondo la direzione
            if (currentMillis - galagaShots[i].lastMove > (100 / galagaShots[i].speed)) {
                galagaShots[i].x += galagaShots[i].dx;
                galagaShots[i].y += galagaShots[i].dy;
                galagaShots[i].lastMove = currentMillis;
                
                // Controlla bordi
                if (galagaShots[i].x < 0) {
                    galagaShots[i].x = 0;
                    galagaShots[i].dx = 1;
                } else if (galagaShots[i].x >= MATRIX_WIDTH) {
                    galagaShots[i].x = MATRIX_WIDTH - 1;
                    galagaShots[i].dx = -1;
                }
                
                if (galagaShots[i].y < 0) {
                    galagaShots[i].active = false;
                    continue;
                }
                
                // Se lo sparo ha raggiunto il target o è vicino, trova nuovo target
                if (abs(galagaShots[i].x - galagaShots[i].targetX) <= 1 && 
                    abs(galagaShots[i].y - galagaShots[i].targetY) <= 1) {
                    findTargetForShot(galagaShots[i]);
                }
            }
            
            // Disegna lo sparo (linea di pixel)
            for (int j = 0; j < GALAGA_SHOT_LENGTH; j++) {
                int drawX = galagaShots[i].x;
                int drawY = galagaShots[i].y - j;
                
                if (drawY >= 0 && drawY < MATRIX_HEIGHT) {
                    uint16_t pos = drawY * MATRIX_WIDTH + drawX;
                    
                    // Colore sparo: bianco con effetto arcobaleno
                    uint8_t shotHue = (galagaShotHue + (i * 30)) % 255;
                    leds[pos] = CHSV(shotHue, 255, 255);
                    
                    // Se lo sparo colpisce una lettera non colorata, colorala
                    if (targetPixels[pos] && !galagaTextPixels[pos]) {
                        galagaTextPixels[pos] = true;
                    }
                }
            }
        }
    }
    
    galagaShotHue += 2;
    
    // Disegna i pixel della scritta che sono stati colorati
    for (uint16_t i = 0; i < NUM_LEDS; i++) {
        if (galagaTextPixels[i]) {
            leds[i] = galagaTextColor;
        }
    }
    
    // Controlla se tutte le lettere sono state colorate
    bool allLettersColored = true;
    for (uint16_t i = 0; i < NUM_LEDS; i++) {
        if (targetPixels[i] && !galagaTextPixels[i]) {
            allLettersColored = false;
            break;
        }
    }
    
    // Se tutte le lettere sono colorate e non ci sono spari attivi, completa l'effetto
    if (allLettersColored && activeShots == 0 && galagaCompletionTime == 0) {
        galagaCompletionTime = currentMillis;
    }
    
    // Se ci sono ancora lettere da colorare ma pochi spari attivi, creane di nuovi
    if (!allLettersColored && activeShots < 3 && random8(100) < 30) {
        createNewGalagaShot();
    }
    
    FastLED.show();
}

// Reset al cambio minuto (statement identici al vecchio loop)
void GalagaEffect::reset() {
    galagaEffectActive = false;
}
