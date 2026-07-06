#pragma once

// Interfaccia web (server HTTP su porta 80)
class WebUI {
public:
    static void begin();   // registra le route e avvia il server
    static void handle();  // gestisce le richieste (se il server e' inizializzato)
};
