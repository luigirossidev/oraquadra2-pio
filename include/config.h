#pragma once

// Abilita la logica invertita per i pulsanti.
// Definisci BUTTON_LOGIC_INVERTED come 1 per pulsanti NA verso GND (pull-up, premi per GND).
// Definisci BUTTON_LOGIC_INVERTED come 0 per pulsanti touch (pull-down, premi per VCC).
#define BUTTON_LOGIC_INVERTED 1

// Pin e configurazione LED per ESP32 standard (WROOM/DevKit)
// ATTENZIONE: GPIO 6-11 su ESP32 standard sono riservati per la SPI flash interna.
// Il codice originale usava GPIO 6 e 7 (validi su ESP32-C3, non su ESP32 standard).
#define LED_PIN      5     // Pin per matrice LED (GPIO 5 e' sicuro su ESP32 standard)
#define BUTTON_MODE  18    // Pulsante modi     (cambia con il pin fisico che usi)
#define BUTTON_SEC   19    // Pulsante frase/indovinello (cambia con il pin fisico che usi)
#define BUZZER_PIN   25    // Buzzer passivo per jukebox FreeClock (GPIO 5 e' occupato dai LED)

#define NUM_LEDS    256   // Totale LED matrice
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
#define BRIGHTNESS_DAY   64
#define BRIGHTNESS_NIGHT 32
#define MATRIX_WIDTH  16
#define MATRIX_HEIGHT 16

// Modalità visualizzazione / Display modes
#define MODE_FADE   0
#define MODE_SLOW   1
#define MODE_FAST   2
#define MODE_MATRIX 3
#define MODE_MATRIX2 4
#define MODE_TRON   5
#define MODE_MOTO   6
#define MODE_GALAGA 7  // Nuova modalità GALAGA
#define MODE_PACMAN 8  // Nuova modalità PACMAN
#define MODE_DIGITALE 9 // Nuova modalità DIGITALE
#define MODE_DROP 10    // Effetto goccia con onde concentriche
#define MODE_RAINBOW 11 // Effetto arcobaleno graduale

// Strutture e costanti per effetto Matrix
#define MATRIX_BASE_SPEED   0.15f
#define MATRIX_SPEED_VAR    0.01f
#define MATRIX_START_Y_MIN  -3.0f
#define MATRIX_START_Y_MAX   0.0f
#define MATRIX_TRAIL_LENGTH  13
#define NUM_DROPS           32      
#define MATRIX2_BASE_SPEED  0.15f
#define MATRIX2_SPEED_VAR   0.1f

// Strutture e costanti per effetto MOTO
#define MOTO_SPEED 30           // Velocità dell'effetto (ms tra i frame)
#define MOTO_TRAIL_LENGTH 8     // Lunghezza della scia
#define MOTO_WAIT_AFTER 5000    // Attesa dopo il completamento (5 secondi)

// Strutture e costanti per effetto GALAGA
#define GALAGA_SPEED 30         // Velocità dell'effetto (ms tra i frame)
#define GALAGA_WAIT_AFTER 5000  // Attesa dopo il completamento (5 secondi)
#define GALAGA_MAX_SHOTS 8      // Numero massimo di spari contemporanei
#define GALAGA_SHOT_LENGTH 3    // Lunghezza dello sparo

// Strutture e costanti per effetto PACMAN
#define PACMAN_SPEED 50           // Velocità dell'effetto (ms tra i frame) - RIPRISTINATA a 50
#define PACMAN_WAIT_AFTER 5000    // Attesa dopo il completamento (5 secondi)
#define NUM_GHOSTS 4              // Numero di fantasmi

// Variabili per effetto TRON
#define NUM_TRON_BIKES 3
#define TRON_TRAIL_LENGTH 12
#define TRON_UPDATE_INTERVAL 80  // ms tra ogni movimento

// Variabili per effetto GOCCIA (onde concentriche)
#define DROP_UPDATE_INTERVAL 60      // ms tra i frame (più lento)
#define DROP_MAX_RADIUS 24           // raggio massimo onde
#define DROP_WAVE_SPEED 0.4f         // velocità espansione (più lento)

#define RAINBOW_SPEED 50  // ms tra aggiornamenti
#define DIGITALE_UPDATE_INTERVAL 500      // Aggiorna ogni 500ms
#define DEFAULT_EFFECT_CYCLE_MS 45000     // Fallback per effetti continui senza "fine" (45 secondi)
