#include "effects/effects.h"
#include "state.h"
#include "config.h"
#include "clock_display.h"

void Effects::update(uint32_t currentMillis) {
    static uint32_t lastMatrixUpdate = 0;

    // --- Disegno modalità corrente ---
    if(currentMode == MODE_MATRIX) {
        if(currentMillis - lastMatrixUpdate > 16) { MatrixEffect::update(); lastMatrixUpdate = currentMillis; }
    }
    else if(currentMode == MODE_MATRIX2) {
        if(currentMillis - lastMatrixUpdate > 16) { Matrix2Effect::update(); lastMatrixUpdate = currentMillis; }
    }
    else if(currentMode == MODE_SLOW) {
        SlowEffect::update();
    }
    else if(currentMode == MODE_FADE) {
        FadeEffect::update();
    }
    else if(currentMode == MODE_GALAGA) {
        GalagaEffect::update();
    }
    else if(currentMode == MODE_PACMAN) {
        PacmanEffect::update();
    }
    else if(currentMode == MODE_TRON) {
        TronEffect::update();
    }
    else if(currentMode == MODE_DROP) {
        DropEffect::update();
    }
    else if(currentMode == MODE_MOTO) {
        MotoEffect::update();
    }
    else if(currentMode == MODE_RAINBOW) {
        RainbowEffect::update();
    }
    else if(currentMode == MODE_DIGITALE) {
        DigitaleEffect::update();
    }
    else {
        ClockDisplay::updateDisplay();
    }
}

// Reset di tutti gli effetti allo scoccare del minuto (ordine identico al vecchio loop)
void Effects::onMinuteChange() {
    // Reset TUTTI gli effetti per farli ripartire allo scoccare del minuto
    SlowEffect::reset();
    FadeEffect::reset();
    DropEffect::reset();
    Matrix2Effect::reset();
    TronEffect::reset();
    GalagaEffect::reset();
    PacmanEffect::reset();
    MotoEffect::reset();
    scrollTextInitialized = false;  // Reset testo scorrevole

    // Reinizializza effetti specifici se attivi
    if (currentMode == MODE_MOTO) {
        MotoEffect::init();
    }
    if (currentMode == MODE_GALAGA) {
        GalagaEffect::init();
    }
    if (currentMode == MODE_PACMAN) {
        PacmanEffect::init();
    }
    if (currentMode == MODE_TRON) {
        memset(tronIntensity, 0, sizeof(tronIntensity));
    }
}
