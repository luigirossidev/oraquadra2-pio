#pragma once

#include <Arduino.h>

// Gestione overlay digitale automatico (alternanza effetto <-> orologio digitale)
class DigitalOverlay {
public:
    static bool effectCycleCompleted();
    static void handle();
};
