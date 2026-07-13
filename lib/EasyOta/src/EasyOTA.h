//
// Created by hector on 04/03/18.
// Modified by lrossi on 03/09/24.
//

#ifndef TEST_2_OTA_H
#define TEST_2_OTA_H

#include <Arduino.h>

#if defined(ESP32)
    #include <HTTPUpdate.h>
    #include <WiFiClientSecure.h>
#else
    #include <ESP8266httpUpdate.h>
#endif

#include "OTAStructs.h"


class EasyOTA {
public:
    explicit EasyOTA(OTAConfig config);

    /**
     *
     */
    void runUpdateRoutine(WiFiClientSecure *client);

    /**
     * Asks the server about the current firmware version.
     * For it uses the config.versionEndpoint.
     * @return The version ot the current firmware
     */
    UpdateCheckResult checkForUpdates(WiFiClientSecure *client);

    /**
     * Update the firmware ussing the config.firmwareEndpoint
     * @return The result
     */
    HTTPUpdateResult updateFirmware(WiFiClientSecure *client);


private:
    HTTPClient httpClient;
    OTAConfig config;
    String firmwareURL;
    String versionURL;
};


#endif //TEST_2_OTA_H
