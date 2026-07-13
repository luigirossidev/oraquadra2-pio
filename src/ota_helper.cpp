#include "ota_helper.h"
#include <Arduino.h>
#include <EasyOTA.h>
#include <WiFiClientSecure.h>
#include <FastLED.h>
#include "fc_config.h"
#include "state.h"
#include "config.h"

#ifndef DEVICE_TYPE
#define DEVICE_TYPE "oraquadra"
#endif

void checkAndUpdateFirmware() {
    // Segnala l'aggiornamento riempiendo la matrice di blu
    fill_solid(leds, NUM_LEDS, CRGB::Blue);
    FastLED.show();

    String versionEndpoint  = "/api/fw/ver?device=" + String(DEVICE_TYPE);
    String firmwareEndpoint = "/api/fw/dl?device="  + String(DEVICE_TYPE);

    OTAConfig configOTA(
        String(FIRMWARE_CODE),
        config.quoteUrl,
        versionEndpoint,
        firmwareEndpoint,
        true
    );

    WiFiClientSecure *clientOTA = new WiFiClientSecure;
    if (clientOTA) {
        clientOTA->setInsecure();
        EasyOTA ota(configOTA);
        ota.runUpdateRoutine(clientOTA);
        delete clientOTA;
    }

    // Se siamo ancora qui l'aggiornamento non e' partito: ripulisci la matrice
    FastLED.clear();
    FastLED.show();
}
