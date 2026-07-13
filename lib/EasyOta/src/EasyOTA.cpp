//
// Created by hector on 08/03/18.
// Modified by lrossi on 03/09/24.
//


#include "EasyOTA.h"
#include "OTADebug.h"

#include <ArduinoOTA.h>

#if defined(ESP32)

    #include <HTTPClient.h>
    #include <HTTPUpdate.h>
    #include <WiFiClientSecure.h>
    #include <Update.h>
#else
    #include <ESP8266WiFi.h>
    #include <ESP8266HTTPClient.h>
    #include <ESP8266httpUpdate.h>
#endif
#include "OTAStructs.h"

EasyOTA::EasyOTA(OTAConfig config) : config(std::move(config)) {
    if (this->config.debugOTA){
        OTADebug::debugOtaConfig(this->config);
    }
    this->firmwareURL = this->config.serverUrl + this->config.firmwareEndpoint;
    this->versionURL = this->config.serverUrl + this->config.versionEndpoint;
}

void EasyOTA::runUpdateRoutine(WiFiClientSecure *client) {
    UpdateCheckResult result = checkForUpdates(client);

    OTADebug::debugCheckUpdateResult(result, config.debugOTA);

    if (result.result == DIFFERENT_VERSION) {
        HTTPUpdateResult r = updateFirmware(client);
        OTADebug::debugCheckUpdateResult(r, config.debugOTA);
    }
}

UpdateCheckResult EasyOTA::checkForUpdates(WiFiClientSecure *client) {
    UpdateCheckResult result;
    httpClient.begin(*client, this->versionURL);
    int httpCode = httpClient.GET();
    Serial.print("CODE: "+httpCode);
    if (httpCode == 200) {
        String serverFWVersion = httpClient.getString();
        if (serverFWVersion != this->config.fwVersion) {
            result = UpdateCheckResult(DIFFERENT_VERSION, serverFWVersion);
        } else {
            result = UpdateCheckResult(SAME_VERSION, serverFWVersion);
        }
    } else {
        if (httpCode == 404) {
            result = UpdateCheckResult(NOT_FW_IN_SERVER, String(""));
        } else {
            Serial.println("ATTENZIONE: ERRORE: " + (String)httpCode);
            result = UpdateCheckResult(CHECK_FAILED, String(""));
        }
    }
    httpClient.end();
    return result;
}

HTTPUpdateResult EasyOTA::updateFirmware(WiFiClientSecure *client) {
    #if defined(ESP32)
        HTTPClient http;
        http.begin(*client, this->firmwareURL);
        http.setTimeout(30000);
        http.addHeader("Accept-Encoding", "identity");
        int httpCode = http.GET();
        if (httpCode != 200) {
            Serial.println("OTA: HTTP error " + String(httpCode));
            http.end();
            return HTTP_UPDATE_FAILED;
        }

        int contentLength = http.getSize();
        Serial.println("OTA: Content-Length = " + String(contentLength));

        if (!Update.begin(contentLength > 0 ? contentLength : UPDATE_SIZE_UNKNOWN)) {
            Serial.println("OTA: Update.begin failed, error: " + String(Update.getError()));
            http.end();
            return HTTP_UPDATE_FAILED;
        }

        WiFiClient *stream = http.getStreamPtr();
        size_t written = 0;
        bool error = false;
        uint8_t buff[1024];

        // Legge una riga con timeout (per leggere chunk size hex)
        auto readLine = [&](uint32_t timeoutMs) -> String {
            String line = "";
            uint32_t start = millis();
            while (millis() - start < timeoutMs) {
                if (stream->available()) {
                    char c = (char)stream->read();
                    if (c == '\n') return line;
                    if (c != '\r') line += c;
                    start = millis();
                } else { delay(1); }
            }
            return line;
        };

        // Legge esattamente len byte con timeout
        auto readBytes = [&](uint8_t *buf, size_t len, uint32_t timeoutMs) -> size_t {
            size_t total = 0;
            uint32_t start = millis();
            while (total < len && millis() - start < timeoutMs) {
                if (stream->available()) {
                    total += stream->readBytes(buf + total, len - total);
                    start = millis();
                } else { delay(1); }
            }
            return total;
        };

        // Decoder chunked transfer encoding
        while (!error) {
            String sizeLine = readLine(10000);
            if (sizeLine.length() == 0) {
                Serial.println("OTA: Timeout chunk size");
                error = true; break;
            }
            size_t chunkSize = strtol(sizeLine.c_str(), nullptr, 16);
            if (chunkSize == 0) break; // chunk finale

            size_t remaining = chunkSize;
            while (remaining > 0 && !error) {
                size_t toRead = min(remaining, sizeof(buff));
                size_t got = readBytes(buff, toRead, 10000);
                if (got != toRead) {
                    Serial.println("OTA: Timeout chunk data at " + String(written) + " bytes");
                    error = true; break;
                }
                if (Update.write(buff, got) != got) {
                    Serial.println("OTA: Write error: " + String(Update.getError()));
                    error = true; break;
                }
                written += got;
                remaining -= got;
            }
            readBytes(buff, 2, 1000); // consuma \r\n finale del chunk
        }

        Serial.println("OTA: Written " + String(written) + " bytes");

        if (error) {
            Update.abort();
            http.end();
            return HTTP_UPDATE_FAILED;
        }

        if (!Update.end(true)) {
            Serial.println("OTA: Update.end failed, error: " + String(Update.getError()));
            http.end();
            return HTTP_UPDATE_FAILED;
        }

        http.end();
        Serial.println("OTA: Update successful, rebooting...");
        delay(500);
        ESP.restart();
        return HTTP_UPDATE_OK; // mai raggiunto
    #else
        return ESPhttpUpdate.update(client, this->firmwareURL);
    #endif
}
