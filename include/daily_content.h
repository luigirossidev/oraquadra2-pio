#pragma once

#include <Arduino.h>

// Frase e indovinello del giorno (server FreeClock) + scroll del PIN BLE.
// Le pressioni del pulsante SEC impostano una richiesta, servita da handle()
// nel loop principale (lo scroll e' bloccante, non va eseguito nell'ISR-path
// del check pulsanti).
class DailyContent {
public:
    static void requestQuoteQuiz();  // SEC breve: alterna frase -> indovinello
    static void requestShowPin();    // SEC lungo: mostra il PIN BLE
    static void handle();            // dal loop: fetch giornaliero + scroll richiesti
    static void invalidateCache();   // forza un nuovo fetch (es. cambio URL da app)
};
