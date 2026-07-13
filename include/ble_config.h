#pragma once

#include <Arduino.h>
#include "melody_player.h"

// Server BLE di configurazione, compatibile con l'app mobile FreeClock.
// Stesso service UUID di FreeClock: l'app scopre il dispositivo in scansione e
// lo distingue tramite il campo deviceType ("oraquadra") della characteristic STATUS.
//
// Le write dell'app aggiornano config/globali e settano dei flag; gli effetti
// visivi (Presets::apply, FastLED, scroll) vengono applicati nel loop principale
// (core 1) per non toccare la matrice dal task BLE.

void bleConfigInit();
void bleUpdateStatus();
void performWiFiScan();
uint32_t bleGetPinCode();

// Flag richieste dall'app (consumati da App::loop)
bool bleConfigNeedsReconnect();       void bleConfigClearReconnect();
bool bleConfigNeedsScan();            void bleConfigClearScan();
bool bleConfigNeedsTimezoneUpdate();  void bleConfigClearTimezoneUpdate();
bool bleConfigNeedsOTAUpdate();       void bleConfigClearOTAUpdate();
bool bleConfigNeedsClockApply();      void bleConfigClearClockApply();
bool bleConfigPresetChanged();        // valido insieme a NeedsClockApply
bool bleConfigNeedsScrollApply();     void bleConfigClearScrollApply();
bool bleConfigNeedsMelodyPlay();      void bleConfigClearMelodyPlay();
MelodyRequest bleConfigGetMelody();
