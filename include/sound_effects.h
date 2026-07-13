#pragma once
// SoundEffects.h — standalone buzzer effect engine for ESP32
// Replaces CuteBuzzerSounds library. Uses LEDC channel 5 (same as MelodyPlayer).
// All functions are static so this header can be included in one translation unit only.

#include <Arduino.h>

// ---- Effect IDs (same values as the old CuteBuzzerSounds for drop-in compatibility) ----
#define S_CONNECTION    0
#define S_DISCONNECTION 1
#define S_BUTTON_PUSHED 2
#define S_MODE1         3
#define S_MODE2         4
#define S_MODE3         5
#define S_SURPRISE      6
#define S_OHOOH         7
#define S_OHOOH2        8
#define S_CUDDLY        9
#define S_SLEEPING      10
#define S_HAPPY         11
#define S_SUPER_HAPPY   12
#define S_HAPPY_SHORT   13
#define S_SAD           14
#define S_CONFUSED      15
#define S_FART1         16
#define S_FART2         17
#define S_FART3         18
#define S_PIRATES       19
#define S_JUMP          20

// ---- LEDC configuration ----
#define _FX_LEDC_CHANNEL  5
#define _FX_LEDC_FREQ     2000
#define _FX_LEDC_RES      8

// ---- Pirates of the Caribbean melody data (PROGMEM) ----
// Note frequencies as integers (rounding difference < 0.4 Hz — inaudible on piezo)
static const int PROGMEM _fx_pirates_notes[] = {
    330, 392, 440, 440, 0,
    440, 494, 523, 523, 0,
    523, 587, 494, 494, 0,
    440, 392, 440, 0,

    330, 392, 440, 440, 0,
    440, 494, 523, 523, 0,
    523, 587, 494, 494, 0,
    440, 392, 440, 0,

    330, 392, 440, 440, 0,
    440, 523, 587, 587, 0,
    587, 659, 698, 698, 0,
    659, 587, 659, 440, 0,

    440, 494, 523, 523, 0,
    587, 659, 440, 0,
    440, 523, 494, 494, 0,
    523, 440, 494, 0,

    440, 440,
    440, 494, 523, 523, 0,
    523, 587, 494, 494, 0,
    440, 392, 440, 0,

    330, 392, 440, 440, 0,
    440, 494, 523, 523, 0,
    523, 587, 494, 494, 0,
    440, 392, 440, 0,

    330, 392, 440, 440, 0,
    440, 523, 587, 587, 0,
    587, 659, 698, 698, 0,
    659, 587, 659, 440, 0,

    440, 494, 523, 523, 0,
    587, 659, 440, 0,
    440, 523, 494, 494, 0,
    523, 440, 494, 0,

    659, 0, 0, 698, 0, 0,
    659, 659, 0, 784, 0, 659, 587, 0, 0,
    587, 0, 0, 523, 0, 0,
    494, 523, 0, 494, 0, 440,

    659, 0, 0, 698, 0, 0,
    659, 659, 0, 784, 0, 659, 587, 0, 0,
    587, 0, 0, 523, 0, 0,
    494, 523, 0, 494, 0, 440
};
static const int PROGMEM _fx_pirates_dur[] = {
    125, 125, 250, 125, 125,
    125, 125, 250, 125, 125,
    125, 125, 250, 125, 125,
    125, 125, 375, 125,

    125, 125, 250, 125, 125,
    125, 125, 250, 125, 125,
    125, 125, 250, 125, 125,
    125, 125, 375, 125,

    125, 125, 250, 125, 125,
    125, 125, 250, 125, 125,
    125, 125, 250, 125, 125,
    125, 125, 125, 250, 125,

    125, 125, 250, 125, 125,
    250, 125, 250, 125,
    125, 125, 250, 125, 125,
    125, 125, 375, 375,

    250, 125,
    125, 125, 250, 125, 125,
    125, 125, 250, 125, 125,
    125, 125, 375, 125,

    125, 125, 250, 125, 125,
    125, 125, 250, 125, 125,
    125, 125, 250, 125, 125,
    125, 125, 375, 125,

    125, 125, 250, 125, 125,
    125, 125, 250, 125, 125,
    125, 125, 250, 125, 125,
    125, 125, 125, 250, 125,

    125, 125, 250, 125, 125,
    250, 125, 250, 125,
    125, 125, 250, 125, 125,
    125, 125, 375, 375,

    250, 125, 375, 250, 125, 375,
    125, 125, 125, 125, 125, 125, 125, 125, 375,
    250, 125, 375, 250, 125, 375,
    125, 125, 125, 125, 125, 500,

    250, 125, 375, 250, 125, 375,
    125, 125, 125, 125, 125, 125, 125, 125, 375,
    250, 125, 375, 250, 125, 375,
    125, 125, 125, 125, 125, 500
};
static const int _fx_pirates_count = (int)(sizeof(_fx_pirates_notes) / sizeof(_fx_pirates_notes[0]));

// ---- Internal helpers (static — local to this translation unit) ----

static void _fxTone(float freq, long durationMs, int silentMs) {
    if (silentMs == 0) silentMs = 1;
    if (freq > 0.0f)
        ledcWriteTone(_FX_LEDC_CHANNEL, freq);
    else
        ledcWrite(_FX_LEDC_CHANNEL, 0);
    delay(durationMs);
    ledcWrite(_FX_LEDC_CHANNEL, 0);
    delay(silentMs);
}

static void _fxBend(float from, float to, float factor, long stepMs, int silentMs) {
    if (silentMs == 0) silentMs = 1;
    if (from < to) {
        for (float f = from; f < to; f *= factor) _fxTone(f, stepMs, silentMs);
    } else {
        for (float f = from; f > to; f /= factor) _fxTone(f, stepMs, silentMs);
    }
}

// ---- Public API ----

inline void buzzerInit(int pin) {
    ledcSetup(_FX_LEDC_CHANNEL, _FX_LEDC_FREQ, _FX_LEDC_RES);
    ledcAttachPin(pin, _FX_LEDC_CHANNEL);
}

inline void buzzerPlay(int effect) {
    switch (effect) {

    case S_CONNECTION:
        _fxTone(659.26f, 50, 30);   // E5
        _fxTone(1318.51f, 55, 25);  // E6
        _fxTone(1760.0f, 60, 10);   // A6
        break;

    case S_DISCONNECTION:
        _fxTone(659.26f, 50, 30);   // E5
        _fxTone(1760.0f, 55, 25);   // A6
        _fxTone(1318.51f, 50, 60);  // E6
        break;

    case S_BUTTON_PUSHED:
        _fxBend(1318.51f, 1567.98f, 1.03f, 20, 2);  // E6 → G6
        delay(30);
        _fxBend(1318.51f, 2349.32f, 1.04f, 10, 2);  // E6 → D7
        break;

    case S_MODE1:
        _fxBend(1318.51f, 1760.0f, 1.02f, 30, 10);  // E6 → A6
        break;

    case S_MODE2:
        _fxBend(1567.98f, 2349.32f, 1.03f, 30, 10); // G6 → D7
        break;

    case S_MODE3:
        _fxTone(1318.51f, 50, 100); // E6
        _fxTone(1567.98f, 50, 80);  // G6
        _fxTone(2349.32f, 300, 0);  // D7
        break;

    case S_SURPRISE:
        _fxBend(800.0f, 2150.0f, 1.02f, 10, 1);
        _fxBend(2149.0f, 800.0f, 1.03f, 7, 1);
        break;

    case S_JUMP:
        _fxBend(880.0f, 2000.0f, 1.04f, 8, 3);  // A5 → ~D7
        delay(200);
        break;

    case S_OHOOH:
        _fxBend(880.0f, 2000.0f, 1.04f, 8, 3);
        delay(200);
        for (int i = 880; i < 2000; i = (int)(i * 1.04f)) _fxTone(987.77f, 5, 10); // B5
        break;

    case S_OHOOH2:
        _fxBend(1880.0f, 3000.0f, 1.03f, 8, 3);
        delay(200);
        for (int i = 1880; i < 3000; i = (int)(i * 1.03f)) _fxTone(1046.5f, 10, 10); // C6
        break;

    case S_CUDDLY:
        _fxBend(700.0f, 900.0f, 1.03f, 16, 4);
        _fxBend(899.0f, 650.0f, 1.01f, 18, 7);
        break;

    case S_SLEEPING:
        _fxBend(100.0f, 500.0f, 1.04f, 10, 10);
        delay(500);
        _fxBend(400.0f, 100.0f, 1.04f, 10, 1);
        break;

    case S_HAPPY:
        _fxBend(1500.0f, 2500.0f, 1.05f, 20, 8);
        _fxBend(2499.0f, 1500.0f, 1.05f, 25, 8);
        break;

    case S_SUPER_HAPPY:
        _fxBend(2000.0f, 6000.0f, 1.05f, 8, 3);
        delay(50);
        _fxBend(5999.0f, 2000.0f, 1.05f, 13, 2);
        break;

    case S_HAPPY_SHORT:
        _fxBend(1500.0f, 2000.0f, 1.05f, 15, 8);
        delay(100);
        _fxBend(1900.0f, 2500.0f, 1.05f, 10, 8);
        break;

    case S_SAD:
        _fxBend(880.0f, 669.0f, 1.02f, 20, 200);
        break;

    case S_CONFUSED:
        _fxBend(1000.0f, 1700.0f, 1.03f, 8, 2);
        _fxBend(1699.0f, 500.0f, 1.04f, 8, 3);
        _fxBend(1000.0f, 1700.0f, 1.05f, 9, 10);
        break;

    case S_FART1:
        _fxBend(1600.0f, 3000.0f, 1.02f, 2, 15);
        break;

    case S_FART2:
        _fxBend(2000.0f, 6000.0f, 1.02f, 2, 20);
        break;

    case S_FART3:
        _fxBend(1600.0f, 4000.0f, 1.02f, 2, 20);
        _fxBend(4000.0f, 3000.0f, 1.02f, 2, 20);
        break;

    case S_PIRATES:
        for (int i = 0; i < _fx_pirates_count; i++) {
            int freq = _fx_pirates_notes[i];
            int dur  = _fx_pirates_dur[i];
            _fxTone((float)freq, dur, 0);
        }
        break;
    }
}
