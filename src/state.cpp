#include "state.h"

bool scrollShowPresetDuringPause = true;      // Mostra preset durante pausa
bool scrollTextualDateTime = false;           // Mostra data/ora in formato testuale
uint8_t savedPresetBeforeScroll = MODE_FAST;  // Preset da ripristinare dopo scroll
bool scrollEnabled = false;                   // Scroll abilitato
uint8_t scrollEnabledRaw = 0;                 // Scroll abilitato RAW

uint8_t currentPreset;
int intBrightness;
uint8_t gHue = 0;

bool targetPixels[NUM_LEDS] = {false};
bool activePixels[NUM_LEDS] = {false};
bool randomColor = false;

uint8_t rainbowHue = 0;
uint32_t lastRainbowUpdate = 0;
bool textRainbowMode = false;  // true = colore rainbow per il testo (ogni lettera colore diverso)

// Variabili per effetto SCROLLTEXT
String customScrollText = "";
bool scrollTextInitialized = false;
int scrollTextOffset = 0;
int scrollTextTotalWidth = 0;
uint32_t lastScrollTextStep = 0;
uint16_t scrollTextSpeedMs = 50;          // Velocità iniziale del testo scorrevole (ms tra i frame)
uint8_t scrollPauseSeconds = 3;           // Pausa in secondi tra un passaggio e l'altro
bool scrollPauseActive = false;           // true quando siamo in pausa tra i passaggi
uint32_t scrollPauseStart = 0;            // Quando è iniziata la pausa
uint8_t lastPresetDuringPause = 255;      // preset durante la pausa
uint32_t pauseDelayStart = 0;             // 
bool pauseDelayActive = false;            // 

// Colore specifico per il testo scorrevole / Specific color for scrolling text
CRGB scrollTextColor = CRGB::White;       // 
bool scrollTextRainbow = false;           // 

bool scrollShowText = true;               // Mostra testo personalizzato nel testo scorrevole
bool scrollShowDate = false;              // Mostra data nel testo scorrevole
bool scrollShowTime = false;              // Mostra ora nel testo scorrevole
uint8_t scrollFontSize = 2;               // Dimensione font: 1=piccolo(3x5), 2=medio(5x7), 3=grande(8x11)

// Overlay digitale automatico (dopo fine effetto + 1.5s -> 2.5s digitale -> ritorna all'effetto precedente)
bool digitalOverlayActive = false;        // true quando siamo in MODE_DIGITALE per overlay
bool waitingForDigital = false;           // true quando l'effetto ha finito e stiamo aspettando 1.5s
bool manualModeSelected = false;          // true quando modalità selezionata manualmente (no overlay)
uint32_t digitalOverlayStart = 0;         // quando è iniziato il digitale
uint32_t digitalDelayStart = 0;           // quando è finito l'effetto (inizio attesa 1.5s)
uint32_t lastEffectChangeTime = 0;        // quando è stato selezionato l'effetto corrente
uint8_t savedModeBeforeDigital = MODE_FAST; // effetto da ripristinare dopo overlay
uint32_t lastDigitaleUpdate = 0;
uint8_t digitaleInterval = 30;            // Ogni quanti secondi appare l'overlay digitale
uint8_t digitaleDuration = 5;             // Per quanti secondi rimane visibile
uint32_t lastDigitaleTrigger = 0;         // Ultimo momento in cui è partito l'overlay

// Definizione array LED e controller
CRGB leds[NUM_LEDS];
CRGB matrixWordColor = CRGB::Blue;
Timezone myTZ;

bool webInitialized = false;

// Variabili globali stato / Global state variables
uint8_t currentMode = MODE_FAST;
uint8_t currentHour = 0;
uint8_t currentMinute = 0;
uint8_t currentSecond = 0;
uint8_t currentBlink = 0;
uint8_t prevHour = 255;
uint8_t prevMinute = 255;
bool displayOff = false;  // Sostituisce alexaOff
bool digitalOverlayEnabled = false;
uint8_t dayStartHour = 7;        // Ora inizio giorno default: 07:00
uint8_t dayStartMinute = 0;
uint8_t nightStartHour = 22;     // Ora inizio notte default: 22:00
uint8_t nightStartMinute = 0;
uint8_t sleepStartHour = 23;
uint8_t sleepStartMinute = 0;
bool sleepModeEnabled = false;
uint8_t dayBrightness = 204;     // 80% di 255
uint8_t nightBrightness = 26;    // 10% di 255
