#include "network.h"
#include <WiFi.h>
#include <ESPmDNS.h>
#include <ArduinoOTA.h>
#include <WiFiManager.h>
#include <FastLED.h>
#include "state.h"
#include "config.h"
#include "scroll_text.h"

void Network::connect() {
   // =========================================================================
   // FASE 6: CONFIGURAZIONE WIFI
   // =========================================================================
   
   WiFiManager wm;
   wm.setConfigPortalTimeout(180);
   wm.setTitle("ORAQUADRA2 WiFi Setup");
   
   bool portalOpened = false;
   
   wm.setAPCallback([&portalOpened](WiFiManager* myWiFiManager) {
       Serial.println("Portale di configurazione avviato");
       Serial.println("Connettiti all'AP: ORAQUADRA2_AP -> 192.168.4.1");
       portalOpened = true;
       ScrollText::show("ORAQUADRA2_AP 192.168.4.1", CRGB::Yellow, scrollTextSpeedMs);
   });
   
   fill_solid(leds, NUM_LEDS, CRGB::Blue);
   FastLED.show();
   
   Serial.println("Tentativo connessione WiFi...");
   
   bool res = wm.autoConnect("ORAQUADRA2_AP");
   
   if (!res) {
       Serial.println("Impossibile collegarsi alla rete");
       ScrollText::show("IMPOSSIBILE COLLEGARSI ALLA RETE", CRGB::White, 50);
       delay(3000);
       ESP.restart();
   }

   Serial.println("WiFi Connesso!");
   Serial.print("Indirizzo IP: ");
   Serial.println(WiFi.localIP());
   
   String ipMessage = "IP " + WiFi.localIP().toString();
   ScrollText::show(ipMessage, CRGB::White, 50);
   
   FastLED.clear();
   FastLED.show();
   
   WiFi.setSleep(false);
   WiFi.setAutoReconnect(true);
}

void Network::resetWiFi() {
    Serial.println("Impossibile collegarsi alla rete");
    ScrollText::show("RESET WIFI", CRGB::Red, 50);
    WiFiManager wm;
    wm.resetSettings();
    ESP.restart();
}

void Network::setupOTA() {
   ArduinoOTA.setHostname("ORAQUADRA2");
   
   ArduinoOTA.onStart([]() {
       FastLED.clear();
       FastLED.show();
   });
   
   ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
       uint8_t percentComplete = (progress / (total / 100));
       fill_solid(leds, map(percentComplete, 0, 100, 0, NUM_LEDS), CRGB::Blue);
       FastLED.show();
   });
   
   ArduinoOTA.begin();
}
