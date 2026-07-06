#pragma once

#include <Arduino.h>

// Matrice parole in PROGMEM / Word matrix in PROGMEM (definite in words_it.cpp)
extern const uint8_t WORD_SONO_LE[] PROGMEM;
extern const uint8_t WORD_MINUTI[] PROGMEM;
extern const uint8_t WORD_UNA[] PROGMEM;
extern const uint8_t WORD_DUE[] PROGMEM;
extern const uint8_t WORD_TRE[] PROGMEM;
extern const uint8_t WORD_QUATTRO[] PROGMEM;
extern const uint8_t WORD_CINQUE[] PROGMEM;
extern const uint8_t WORD_SEI[] PROGMEM;
extern const uint8_t WORD_SETTE[] PROGMEM;
extern const uint8_t WORD_OTTO[] PROGMEM;
extern const uint8_t WORD_NOVE[] PROGMEM;
extern const uint8_t WORD_DIECI[] PROGMEM;
extern const uint8_t WORD_UNDICI[] PROGMEM;
extern const uint8_t WORD_DODICI[] PROGMEM;
extern const uint8_t WORD_TREDICI[] PROGMEM;
extern const uint8_t WORD_QUATTORDICI[] PROGMEM;
extern const uint8_t WORD_QUINDICI[] PROGMEM;
extern const uint8_t WORD_SEDICI[] PROGMEM;
extern const uint8_t WORD_DICIASSETTE[] PROGMEM;
extern const uint8_t WORD_DICIOTTO[] PROGMEM;
extern const uint8_t WORD_DICIANNOVE[] PROGMEM;
extern const uint8_t WORD_VENTI[] PROGMEM;
extern const uint8_t WORD_VENTUNO[] PROGMEM;
extern const uint8_t WORD_VENTIDUE[] PROGMEM;
extern const uint8_t WORD_VENTITRE[] PROGMEM;
extern const uint8_t WORD_ZERO[] PROGMEM;
extern const uint8_t WORD_E[] PROGMEM;
extern const uint8_t WORD_MUNO[] PROGMEM;
extern const uint8_t WORD_MUN[] PROGMEM;
extern const uint8_t WORD_MDUE[] PROGMEM;
extern const uint8_t WORD_MTRE[] PROGMEM;
extern const uint8_t WORD_MQUATTRO[] PROGMEM;
extern const uint8_t WORD_MCINQUE[] PROGMEM;
extern const uint8_t WORD_MSEI[] PROGMEM;
extern const uint8_t WORD_MSETTE[] PROGMEM;
extern const uint8_t WORD_MOTTO[] PROGMEM;
extern const uint8_t WORD_MNOVE[] PROGMEM;
extern const uint8_t WORD_MDIECI[] PROGMEM;
extern const uint8_t WORD_MUNDICI[] PROGMEM;
extern const uint8_t WORD_MDODICI[] PROGMEM;
extern const uint8_t WORD_MTREDICI[] PROGMEM;
extern const uint8_t WORD_MQUATTORDICI[] PROGMEM;
extern const uint8_t WORD_MQUINDICI[] PROGMEM;
extern const uint8_t WORD_MSEDICI[] PROGMEM;
extern const uint8_t WORD_MDICIASSETTE[] PROGMEM;
extern const uint8_t WORD_MDICIOTTO[] PROGMEM;
extern const uint8_t WORD_MDICIANNOVE[] PROGMEM;
extern const uint8_t WORD_MVENT[] PROGMEM;
extern const uint8_t WORD_MVENTI[] PROGMEM;
extern const uint8_t WORD_MTRENT[] PROGMEM;
extern const uint8_t WORD_MTRENTA[] PROGMEM;
extern const uint8_t WORD_MQUARANT[] PROGMEM;
extern const uint8_t WORD_MQUARANTA[] PROGMEM;
extern const uint8_t WORD_MCINQUANT[] PROGMEM;
extern const uint8_t WORD_MCINQUANTA[] PROGMEM;

// Array delle parole ore per lookup veloce / Hour words array for fast lookup
extern const uint8_t* const HOUR_WORDS[24] PROGMEM;

// Array di lookup per i minuti / Lookup array for minutes
extern const uint8_t* const MINUTE_WORDS[20] PROGMEM;

// Struttura per le decine / Structure for tens
struct MinuteTens {
   const uint8_t* normal;
   const uint8_t* truncated;
};

// Array di lookup per le decine / Lookup array for tens
extern const MinuteTens TENS_WORDS[4] PROGMEM;
