#pragma once

#include <FastLED.h>
#include "config.h"

// Struttura per gli spari GALAGA
struct GalagaShot {
    int8_t x;           // Posizione X corrente
    int8_t y;           // Posizione Y corrente
    int8_t targetX;     // Posizione X target (dove ci sono lettere da colpire)
    int8_t targetY;     // Posizione Y target
    int8_t dx;          // Direzione X (-1, 0, 1)
    int8_t dy;          // Direzione Y (-1, 0, 1)
    bool active;        // Se lo sparo è attivo
    uint8_t speed;      // Velocità dello sparo
    uint32_t lastMove;  // Ultimo momento in cui si è mosso
};

extern uint32_t lastGalagaUpdate;
extern bool galagaEffectActive;
extern uint32_t galagaCompletionTime;
extern uint8_t galagaTextHue;
extern CRGB galagaTextColor;
extern bool galagaTextPixels[NUM_LEDS];
extern uint8_t galagaShotHue;
extern GalagaShot galagaShots[GALAGA_MAX_SHOTS];
extern uint8_t galagaLastMinute;

void createNewGalagaShot();
void findTargetForShot(GalagaShot &shot);
void calculateShotDirection(GalagaShot &shot);

// Effetto GALAGA (spari dal basso che colpiscono le lettere)
class GalagaEffect {
public:
    static void init();
    static void update();
    static void reset();
};
