#pragma once

#include <Arduino.h>
#include <FastLED.h>
#include <ezTime.h>
#include "config.h"

// Stato condiviso dell'applicazione (definito in state.cpp)

// Stato testo scorrevole (configurazione)
extern bool scrollShowPresetDuringPause;
extern bool scrollTextualDateTime;
extern uint8_t savedPresetBeforeScroll;
extern bool scrollEnabled;
extern uint8_t scrollEnabledRaw;

// Stato principale orologio
extern uint8_t currentPreset;
extern int intBrightness;
extern uint8_t gHue;

// Buffer pixel condivisi tra gli effetti
extern bool targetPixels[NUM_LEDS];
extern bool activePixels[NUM_LEDS];
extern bool randomColor;

// Stato effetto RAINBOW / colore rainbow del testo
extern uint8_t rainbowHue;
extern uint32_t lastRainbowUpdate;
extern bool textRainbowMode;

// Stato runtime testo scorrevole
extern String customScrollText;
extern bool scrollTextInitialized;
extern int scrollTextOffset;
extern int scrollTextTotalWidth;
extern uint32_t lastScrollTextStep;
extern uint16_t scrollTextSpeedMs;
extern uint8_t scrollPauseSeconds;
extern bool scrollPauseActive;
extern uint32_t scrollPauseStart;
extern uint8_t lastPresetDuringPause;
extern uint32_t pauseDelayStart;
extern bool pauseDelayActive;
extern CRGB scrollTextColor;
extern bool scrollTextRainbow;
extern bool scrollShowText;
extern bool scrollShowDate;
extern bool scrollShowTime;
extern uint8_t scrollFontSize;

// Stato overlay digitale automatico
extern bool digitalOverlayActive;
extern bool waitingForDigital;
extern bool manualModeSelected;
extern uint32_t digitalOverlayStart;
extern uint32_t digitalDelayStart;
extern uint32_t lastEffectChangeTime;
extern uint8_t savedModeBeforeDigital;
extern uint32_t lastDigitaleUpdate;
extern uint8_t digitaleInterval;
extern uint8_t digitaleDuration;
extern uint32_t lastDigitaleTrigger;

// Matrice LED e fuso orario
extern CRGB leds[NUM_LEDS];
extern CRGB matrixWordColor;
extern Timezone myTZ;

// Server web inizializzato
extern bool webInitialized;

// Variabili globali stato / Global state variables
extern uint8_t currentMode;
extern uint8_t currentHour;
extern uint8_t currentMinute;
extern uint8_t currentSecond;
extern uint8_t currentBlink;
extern uint8_t prevHour;
extern uint8_t prevMinute;
extern bool displayOff;
extern bool digitalOverlayEnabled;
extern uint8_t dayStartHour;
extern uint8_t dayStartMinute;
extern uint8_t nightStartHour;
extern uint8_t nightStartMinute;
extern uint8_t sleepStartHour;
extern uint8_t sleepStartMinute;
extern bool sleepModeEnabled;
extern uint8_t dayBrightness;
extern uint8_t nightBrightness;
