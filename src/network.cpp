#include "network.h"
#include <WiFi.h>
#include <ESPmDNS.h>
#include <ArduinoOTA.h>
#include <FastLED.h>
#include <ezTime.h>
#include "state.h"
#include "config.h"
#include "fc_config.h"
#include "scroll_text.h"

bool Network::connect() {
   if (strlen(config.WIFI_SSID) == 0) {
       Serial.println("Nessun SSID configurato - in attesa di configurazione via BLE (app FreeClock)");
       return false;
   }

   fill_solid(leds, NUM_LEDS, CRGB::Blue);
   FastLED.show();

   Serial.println("Connessione WiFi a: " + String(config.WIFI_SSID));
   WiFi.mode(WIFI_STA);
   WiFi.begin(config.WIFI_SSID, config.WIFI_PASSWORD);

   int attempts = 0;
   while (WiFi.status() != WL_CONNECTED && attempts < 20) {
       delay(500);
       Serial.print(".");
       attempts++;
   }
   Serial.println();

   if (WiFi.status() != WL_CONNECTED) {
       Serial.println("Connessione WiFi fallita - configurabile via BLE");
       WiFi.disconnect(false);  // ferma i retry del driver, mantiene le credenziali
       FastLED.clear();
       FastLED.show();
       return false;
   }

   Serial.println("WiFi Connesso!");
   Serial.print("Indirizzo IP: ");
   Serial.println(WiFi.localIP());

   String ipMessage = "IP " + WiFi.localIP().toString();
   ScrollText::show(ipMessage, CRGB::White, 50);

   FastLED.clear();
   FastLED.show();

   // Niente WiFi.setSleep(false): con il BLE sempre attivo il driver richiede
   // il modem sleep per la coesistenza radio, altrimenti va in abort() e riavvia.
   WiFi.setSleep(true);
   WiFi.setAutoReconnect(true);
   return true;
}

void Network::applyTimezone() {
   // L'app invia direttamente la stringa POSIX (es. "CET-1CEST,M3.5.0,M10.5.0/3")
   if (!myTZ.setPosix(config.TIMEZONE)) {
       Serial.println("Timezone POSIX non valida, fallback Europe/Rome");
       myTZ.setPosix(F("CET-1CEST,M3.5.0,M10.5.0/3"));
   }
   Serial.println("Timezone applicata: " + config.TIMEZONE);
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
