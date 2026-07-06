#pragma once

#include <Arduino.h>

// Applicazione dei preset (modalita' + colori + reinizializzazione effetti)
class Presets {
public:
    static void apply(uint8_t preset);
};
