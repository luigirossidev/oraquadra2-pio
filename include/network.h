#pragma once

// Connessione WiFi da credenziali salvate (configurate via BLE dall'app FreeClock) e OTA
class Network {
public:
    static bool connect();          // tenta la connessione con le credenziali in config; false se assenti o tentativi falliti
    static void applyTimezone();    // applica config.TIMEZONE (stringa POSIX) a ezTime
    static void setupOTA();         // ArduinoOTA per sviluppo locale
};
