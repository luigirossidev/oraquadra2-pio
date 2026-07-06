#pragma once

// Connessione WiFi (WiFiManager), OTA e reset configurazione
class Network {
public:
    static void connect();   // WiFiManager autoConnect con portale di configurazione
    static void resetWiFi();
    static void setupOTA();
};
