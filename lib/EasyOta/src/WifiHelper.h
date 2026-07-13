//
// Created by hector on 08/03/18.
//

#ifndef NODE_WIFIHELPER_H
#define NODE_WIFIHELPER_H

#include <Arduino.h>
#if defined(ESP32)
    #include <Wifi.h>
#else
    #include <ESP8266WiFi.h>
#endif



class WifiHelper {
public:
    /***
     * Start the wifi connection
     */
    static void startWIFI(const char *ssid, const char *password, bool wait4Connect = false, bool debugOTA = false) {
        WiFi.mode(WIFI_STA);
        WiFi.begin(ssid, password);
        while (wait4Connect and WiFi.waitForConnectResult() != WL_CONNECTED) {
            if (debugOTA) {
                Serial.println("Connection Failed! Rebooting...");
            }
            ESP.restart();
        }
    }
};


#endif //NODE_WIFIHELPER_H
