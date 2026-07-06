#include "effects/effect_pacman.h"
#include "state.h"
#include "words_it.h"
#include "clock_display.h"
#include "led_canvas.h"

// Variabili per effetto PACMAN
uint32_t lastPacmanUpdate = 0;
bool pacmanEffectActive = false;
uint32_t pacmanCompletionTime = 0;
uint8_t pacmanTextHue = 0;
CRGB pacmanTextColor = CRGB::White;
bool pacmanTextPixels[NUM_LEDS] = {false};
uint8_t pacmanLastMinute = 255;
int16_t pacmanPosition = -2; // Posizione iniziale di Pacman (fuori dallo schermo a sinistra)
int16_t ghostPositions[NUM_GHOSTS] = {0}; // Posizioni dei fantasmi
CRGB ghostColors[NUM_GHOSTS] = {CRGB::Red, CRGB(255, 182, 255), CRGB::Cyan, CRGB(255, 165, 0)}; // Rosso, Rosa, Ciano, Arancio

// Funzioni per effetto PACMAN
// Variabile per tracciare l'ultimo minuto in cui Pacman è partito


void PacmanEffect::init() {
    FastLED.clear();
    pacmanEffectActive = true;
    pacmanCompletionTime = 0;
    pacmanTextHue = random8();

    // Salva il minuto corrente per sapere quando ripartire
    pacmanLastMinute = currentMinute;

    // Reset array pixel della scritta
    memset(pacmanTextPixels, 0, sizeof(pacmanTextPixels));

    // Inizializza posizioni - Pacman davanti ai fantasmi
    pacmanPosition = -2; // Inizia fuori dallo schermo a sinistra

    // Posiziona i fantasmi dietro a Pacman con spaziatura
    for (int i = 0; i < NUM_GHOSTS; i++) {
        ghostPositions[i] = pacmanPosition - 4 - (i * 3); // Fantasmi dietro a Pacman
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
}

void PacmanEffect::update() {
    uint32_t currentMillis = millis();
    if (currentMinute != pacmanLastMinute) {
        pacmanLastMinute = currentMinute;
        PacmanEffect::init();
        return;
    }

    // Se l'effetto è completato, mostra l'orario statico
    // L'overlay digitale e la reinizializzazione sono gestiti da DigitalOverlay::handle()
    if (pacmanCompletionTime != 0) {
        // Mostra l'orario colorato da Pacman
        FastLED.clear();
        for (uint16_t i = 0; i < NUM_LEDS; i++) {
            if (pacmanTextPixels[i]) {
                leds[i] = pacmanTextColor;
            }
        }

        // secondi / seconds
        if (currentBlink == 1) {
            ClockDisplay::showSeconds(currentSecond, pacmanTextColor);
        }

        FastLED.show();
        return;
    }

    // Aggiornamento periodico
    if (currentMillis - lastPacmanUpdate < PACMAN_SPEED) return;
    lastPacmanUpdate = currentMillis;

    // Cancella tutto
    FastLED.clear();

    // Muovi Pacman e fantasmi
    pacmanPosition++;

    for (int i = 0; i < NUM_GHOSTS; i++) {
        ghostPositions[i]++;
    }

    // Disegna Pacman (2 LED gialli in orizzontale)
    if (pacmanPosition >= 0 && pacmanPosition < NUM_LEDS) {
        // Calcola le posizioni per i 2 LED di Pacman
        int16_t pacmanLeft = pacmanPosition;
        int16_t pacmanRight = pacmanPosition + 1;

        // Disegna Pacman solo se le posizioni sono valide
        if (pacmanLeft >= 0 && pacmanLeft < NUM_LEDS) leds[pacmanLeft] = CRGB::Yellow;
        if (pacmanRight >= 0 && pacmanRight < NUM_LEDS) leds[pacmanRight] = CRGB::Yellow;

        // Marca i pixel della scritta che Pacman passa sopra
        if (pacmanLeft >= 0 && pacmanLeft < NUM_LEDS && targetPixels[pacmanLeft]) {
            pacmanTextPixels[pacmanLeft] = true;
        }
        if (pacmanRight >= 0 && pacmanRight < NUM_LEDS && targetPixels[pacmanRight]) {
            pacmanTextPixels[pacmanRight] = true;
        }
    }

    // Disegna fantasmi (1 LED ciascuno)
    for (int i = 0; i < NUM_GHOSTS; i++) {
        if (ghostPositions[i] >= 0 && ghostPositions[i] < NUM_LEDS) {
            leds[ghostPositions[i]] = ghostColors[i];

            // Marca i pixel della scritta che i fantasmi passano sopra
            if (targetPixels[ghostPositions[i]]) {
                pacmanTextPixels[ghostPositions[i]] = true;
            }
        }
    }

    // Disegna i pixel della scritta che sono stati colorati
    for (uint16_t i = 0; i < NUM_LEDS; i++) {
        if (pacmanTextPixels[i]) {
            leds[i] = pacmanTextColor;
        }
    }

    // Se Pacman esce dallo schermo a destra, completa l'effetto
    if (pacmanPosition >= NUM_LEDS && pacmanCompletionTime == 0) {
        pacmanCompletionTime = currentMillis;
    }

    FastLED.show();
}

// Reset al cambio minuto (statement identici al vecchio loop)
void PacmanEffect::reset() {
    pacmanEffectActive = false;
}
