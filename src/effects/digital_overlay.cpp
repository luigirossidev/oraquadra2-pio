#include "effects/digital_overlay.h"
#include "state.h"
#include "config.h"
#include "effects/effect_fade.h"
#include "effects/effect_slow.h"
#include "effects/effect_matrix2.h"
#include "effects/effect_tron.h"
#include "effects/effect_drop.h"
#include "effects/effect_moto.h"
#include "effects/effect_galaga.h"
#include "effects/effect_pacman.h"

// Variabile per tracciare lo stato precedente del display
bool prevDisplayOff = false;


// Ritorna true quando l'effetto corrente ha completato un ciclo "naturale".
// Per gli effetti continui senza fine, usa un fallback a tempo.
bool DigitalOverlay::effectCycleCompleted() {
    uint32_t now = millis();
    switch (currentMode) {
        case MODE_MATRIX2:
            return matrix2State.completed;
        case MODE_MOTO:
            return motoCompleted;
        case MODE_DROP:
            return dropState.completed;
        case MODE_SLOW:
            return slowState.completed;
        case MODE_FADE:
            return fadeState.completed;
        case MODE_PACMAN:
            // Pacman è completato quando pacmanCompletionTime è diverso da 0
            return (pacmanCompletionTime != 0);
        case MODE_GALAGA:
            return !galagaEffectActive;
        case MODE_TRON:
            return !tronEffectActive;
        case MODE_DIGITALE:
            // Già in digitale, non serve overlay
            return false;
        case MODE_MATRIX:
        case MODE_FAST:
        case MODE_RAINBOW:
            // Effetti continui senza fine: usa il fallback
            return (now - lastEffectChangeTime >= DEFAULT_EFFECT_CYCLE_MS);
        default:
            return (now - lastEffectChangeTime >= DEFAULT_EFFECT_CYCLE_MS);
    }
}

// Gestione overlay digitale
// - Effetto parte al secondo 0
// - Quando effetto finisce, mostra orario statico
// - Dopo intervallo impostato, appare orologio digitale per durata impostata
// - Ciclo continua fino al secondo 59, poi al secondo 0 riparte effetto
bool effectWasRunning = false;  // Per tracciare se l'effetto era in esecuzione
uint32_t effectCompletedTime = 0;  // Quando l'effetto ha completato il suo ciclo
bool showingDigitalOverlay = false;  // True quando stiamo mostrando l'overlay digitale

void DigitalOverlay::handle() {
    // Se la modalità è stata selezionata manualmente, non fare overlay
    if (manualModeSelected) {
        showingDigitalOverlay = false;
        return;
    }

    // Se overlay digitale disabilitato
    if (!digitalOverlayEnabled) {
        showingDigitalOverlay = false;
        return;
    }

    uint32_t now = millis();

    // Controlla stato effetto (usando la modalità originale se siamo in overlay)
    uint8_t modeToCheck = showingDigitalOverlay ? savedModeBeforeDigital : currentMode;

    // Salva temporaneamente currentMode per controllare DigitalOverlay::effectCycleCompleted
    uint8_t originalMode = currentMode;
    currentMode = modeToCheck;
    bool effectCompleted = DigitalOverlay::effectCycleCompleted();
    currentMode = originalMode;

    // Al secondo 0, se l'effetto era completato, deve ripartire
    if (currentSecond == 0 && effectCompletedTime != 0) {
        // Reset per far ripartire l'effetto
        if (showingDigitalOverlay) {
            currentMode = savedModeBeforeDigital;
        }
        showingDigitalOverlay = false;
        effectWasRunning = false;
        effectCompletedTime = 0;

        // Reinizializza l'effetto specifico
        if (currentMode == MODE_PACMAN) {
            PacmanEffect::init();
        } else if (currentMode == MODE_MOTO) {
            MotoEffect::init();
        } else if (currentMode == MODE_GALAGA) {
            GalagaEffect::init();
        }
        return;
    }

    // Se siamo in overlay digitale attivo
    if (showingDigitalOverlay) {
        // Termina dopo la durata impostata
        if (now - digitalOverlayStart >= (uint32_t)digitaleDuration * 1000) {
            currentMode = savedModeBeforeDigital;
            showingDigitalOverlay = false;
            lastDigitaleTrigger = now;  // Reset timer per prossimo ciclo
        }
        return;
    }

    // Effetto in esecuzione
    if (!effectCompleted) {
        effectWasRunning = true;
        effectCompletedTime = 0;
        return;
    }

    // Effetto completato - salva il momento (solo la prima volta)
    if (effectCompletedTime == 0) {
        effectCompletedTime = now;
        lastDigitaleTrigger = now;
    }

    // Effetto completato, controlla se è ora di mostrare orologio digitale
    if (now - lastDigitaleTrigger >= (uint32_t)digitaleInterval * 1000) {
        savedModeBeforeDigital = currentMode;
        currentMode = MODE_DIGITALE;
        showingDigitalOverlay = true;
        digitalOverlayStart = now;
    }
}
