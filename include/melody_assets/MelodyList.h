#pragma once
// Melody archive — 26 songs
// Do NOT include alongside other headers that define NOTE_XX float constants

#include <Arduino.h>
#include "notes_int.h"

struct MelodyInfo {
    uint8_t     id;
    const char* name;       // string literal in flash
    const int*  data;       // PROGMEM pointer — interleaved [note, divider, ...]
    int         noteCount;  // number of note+divider pairs
    int         tempo;      // BPM
};

// ---- Individual melody arrays ----
#include "melodies/melody_00_babyelephantwalk.h"
#include "melodies/melody_01_brahmslullaby.h"
#include "melodies/melody_02_doom.h"
#include "melodies/melody_03_furelise.h"
#include "melodies/melody_04_gameofthrones.h"
#include "melodies/melody_05_happybirthday.h"
#include "melodies/melody_06_harrypotter.h"
#include "melodies/melody_07_imperialmarch.h"
#include "melodies/melody_08_keyboardcat.h"
#include "melodies/melody_09_merrychristmas.h"
#include "melodies/melody_10_minuetg.h"
#include "melodies/melody_11_nevergonnagiveyouup.h"
#include "melodies/melody_12_nokia.h"
#include "melodies/melody_13_odetojoy.h"
#include "melodies/melody_14_pacman.h"
#include "melodies/melody_15_pinkpanther.h"
#include "melodies/melody_16_silentnight.h"
#include "melodies/melody_17_startrekintro.h"
#include "melodies/melody_18_starwars.h"
#include "melodies/melody_19_supermariobros.h"
#include "melodies/melody_20_takeonme.h"
#include "melodies/melody_21_tetris.h"
#include "melodies/melody_22_thegodfather.h"
#include "melodies/melody_23_zeldaslullaby.h"
#include "melodies/melody_24_zeldatheme.h"
#include "melodies/melody_25_roserosse.h"
#include "melodies/melody_26_the_flintstones.h"
#include "melodies/melody_27_axel.h"

// ---- Index table (in flash) ----
static const MelodyInfo MELODY_LIST[] PROGMEM = {
    {  0, "Baby Elephant Walk",        melody_00, melody_00_notes, melody_00_tempo },
    {  1, "Brahms Lullaby",            melody_01, melody_01_notes, melody_01_tempo },
    {  2, "Doom E1M1",                 melody_02, melody_02_notes, melody_02_tempo },
    {  3, "Fur Elise",                 melody_03, melody_03_notes, melody_03_tempo },
    {  4, "Game of Thrones",           melody_04, melody_04_notes, melody_04_tempo },
    {  5, "Happy Birthday",            melody_05, melody_05_notes, melody_05_tempo },
    {  6, "Harry Potter Theme",        melody_06, melody_06_notes, melody_06_tempo },
    {  7, "Imperial March",            melody_07, melody_07_notes, melody_07_tempo },
    {  8, "Keyboard Cat",              melody_08, melody_08_notes, melody_08_tempo },
    {  9, "Merry Christmas",           melody_09, melody_09_notes, melody_09_tempo },
    { 10, "Minuet in G",               melody_10, melody_10_notes, melody_10_tempo },
    { 11, "Never Gonna Give You Up",   melody_11, melody_11_notes, melody_11_tempo },
    { 12, "Nokia Ringtone",            melody_12, melody_12_notes, melody_12_tempo },
    { 13, "Ode to Joy",                melody_13, melody_13_notes, melody_13_tempo },
    { 14, "Pac-Man",                   melody_14, melody_14_notes, melody_14_tempo },
    { 15, "Pink Panther",              melody_15, melody_15_notes, melody_15_tempo },
    { 16, "Silent Night",              melody_16, melody_16_notes, melody_16_tempo },
    { 17, "Star Trek Intro",           melody_17, melody_17_notes, melody_17_tempo },
    { 18, "Star Wars Theme",           melody_18, melody_18_notes, melody_18_tempo },
    { 19, "Super Mario Bros",          melody_19, melody_19_notes, melody_19_tempo },
    { 20, "Take On Me",                melody_20, melody_20_notes, melody_20_tempo },
    { 21, "Tetris Theme",              melody_21, melody_21_notes, melody_21_tempo },
    { 22, "The Godfather",             melody_22, melody_22_notes, melody_22_tempo },
    { 23, "Zelda Lullaby",             melody_23, melody_23_notes, melody_23_tempo },
    { 24, "Zelda Theme",               melody_24, melody_24_notes, melody_24_tempo },
    { 25, "Rose Rosse",                melody_25, melody_25_notes, melody_25_tempo },
    { 26, "The Flintstones",           melody_26, melody_26_notes, melody_26_tempo },
    { 27, "Berverly Hills Cop Jingle", melody_27, melody_27_notes, melody_27_tempo },
};
static const uint8_t MELODY_COUNT = (uint8_t)(sizeof(MELODY_LIST) / sizeof(MELODY_LIST[0]));
