#include "web_ui.h"
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <EEPROM.h>
#include <FastLED.h>
#include "webpage.h"
#include "state.h"
#include "config.h"
#include "eeprom_map.h"
#include "clock_colors.h"
#include "clock_display.h"
#include "time_utils.h"
#include "presets.h"
#include "effects/effect_slow.h"
#include "effects/effect_fade.h"

// Server web per interfaccia
WebServer server(80);

// ===== Web handlers: forward declarations =====
void handleRoot();
void handleStatus();
void handleSetMode();
void handleSetBlink();
void handleSetColor();
void handleSetRainbow();
void handleSetBrightness();
void handleSetDayNightSettings();
void handleSetDigitalOverlay();
void handleSetDigitalTiming();
void handleSetScrollText();
void handleSetScrollPause();
void handleSetScrollFontSize();
void handleSetScrollOptions();
void handleSetScrollShowPreset();
void handleSetScrollTextualDateTime();
void handleSetScrollSpeed();
void handleSetScrollEnabled();
void handleSetScrollTextColor();
void handleSetScrollTextRainbow();
void handlePower();
void handleReset();
void handleNotFound();

void WebUI::begin() {
    server.on("/", HTTP_GET, handleRoot);
    server.on("/status", HTTP_GET, handleStatus);
    server.on("/setMode", HTTP_POST, handleSetMode);
    server.on("/setBlink", HTTP_POST, handleSetBlink);
    server.on("/setColor", HTTP_POST, handleSetColor);
    server.on("/setRainbow", HTTP_POST, handleSetRainbow);
    server.on("/setBrightness", HTTP_POST, handleSetBrightness);
    server.on("/setDayNightSettings", HTTP_POST, handleSetDayNightSettings);
    server.on("/setDigitalOverlay", HTTP_POST, handleSetDigitalOverlay);
    server.on("/setDigitalTiming", HTTP_POST, handleSetDigitalTiming);
    server.on("/setScrollText", HTTP_POST, handleSetScrollText);
    server.on("/setScrollPause", HTTP_POST, handleSetScrollPause);
    server.on("/setScrollFontSize", HTTP_POST, handleSetScrollFontSize);
    server.on("/setScrollOptions", HTTP_POST, handleSetScrollOptions);
    server.on("/setScrollShowPreset", HTTP_POST, handleSetScrollShowPreset);
    server.on("/setScrollTextualDateTime", HTTP_POST, handleSetScrollTextualDateTime);
    server.on("/setScrollSpeed", HTTP_POST, handleSetScrollSpeed); // Nuovo endpoint per velocità
    server.on("/setScrollEnabled", HTTP_GET, handleSetScrollEnabled);
    server.on("/setScrollTextColor", HTTP_POST, handleSetScrollTextColor);
    server.on("/setScrollTextRainbow", HTTP_POST, handleSetScrollTextRainbow);
    server.on("/power", HTTP_POST, handlePower);
    server.on("/reset", HTTP_POST, handleReset);
    server.onNotFound(handleNotFound);
    
    server.begin();
    webInitialized = true;
    Serial.println("Server web avviato su http://" + WiFi.localIP().toString());
   // preset / preset
   lastEffectChangeTime = millis();
   digitalOverlayActive = false;
   scrollTextInitialized = false;
}

void WebUI::handle() {
    if(webInitialized) {
        server.handleClient();
    }
}

void handleRoot() {
    server.send_P(200, "text/html", webpage);
}


//    server.send(200, "text/html", html);
//}

void handleStatus() {
    StaticJsonDocument<1024> doc;
    
    // Nome della modalità corrente / Current mode name
    String modeName = "Sconosciuta";
    switch(currentPreset) {
        case 0: modeName = "Lento Casuale"; break;
        case 1: modeName = "Sfumato Blu"; break;
        case 2: modeName = "Matrix Verde"; break;
        case 3: modeName = "Lento Arancione"; break;
        case 4: modeName = "Matrix Blu"; break;
        case 5: modeName = "Matrix Giallo"; break;
        case 6: modeName = "Sfumato Verde"; break;
        case 7: modeName = "Lento Blu"; break;
        case 8: modeName = "Matrix Ciano"; break;
        case 9: modeName = "Sfumato Rosa"; break;
        case 10: modeName = "Veloce Giallo"; break;
        case 11: modeName = "Veloce Bianco"; break;
        case 12: modeName = "Veloce Ciano"; break;
        case 13: modeName = "Effetto TRON"; break;
        case 14: modeName = "Effetto Goccia"; break;
        case 15: modeName = "Effetto Moto"; break;
        case 16: modeName = "Effetto Galaga"; break;
        case 17: modeName = "Effetto Pacman"; break;
        case 18: modeName = "Digitale"; break;
        case 19: modeName = "Scroll Text"; break;
        default: modeName = "Sconosciuta"; break;
    }
    
    // Nome del colore corrente / Current color name
    String colorName = "Personalizzato";
    CRGB current = clockColors.current;
    if (current == CRGB::White) colorName = "Bianco";
    else if (current == CRGB::Blue) colorName = "Blu";
    else if (current == CRGB::Red) colorName = "Rosso";
    else if (current == CRGB::Green) colorName = "Verde";
    else if (current == CRGB::Yellow) colorName = "Giallo";
    else if (current == CRGB(255,165,0)) colorName = "Arancione";
    
    // Formatta gli orari / Format times
    char dayStart[6], nightStart[6], sleepStart[6];
    snprintf(dayStart, sizeof(dayStart), "%02d:%02d", dayStartHour, dayStartMinute);
    snprintf(nightStart, sizeof(nightStart), "%02d:%02d", nightStartHour, nightStartMinute);
    snprintf(sleepStart, sizeof(sleepStart), "%02d:%02d", sleepStartHour, sleepStartMinute);
  
    Serial.println("=== INVIO STATUS ===");
    Serial.print("dayStartTime: ");
    Serial.println(dayStart);
    Serial.print("nightStartTime: ");
    Serial.println(nightStart);
    Serial.print("dayBrightness: ");
    Serial.println(map(dayBrightness, 0, 255, 0, 100));
    Serial.print("nightBrightness: ");
    Serial.println(map(nightBrightness, 0, 255, 0, 100));
    Serial.print("sleepStartTime: ");
    Serial.println(sleepStart);
    Serial.print("sleepModeEnabled: ");
    Serial.println(sleepModeEnabled);

    doc["mode"] = currentPreset;
    doc["modeName"] = modeName;
    doc["colorName"] = colorName;
    doc["blink"] = (currentBlink == 1);
    doc["brightness"] = map(intBrightness, 0, 255, 0, 100);
    doc["power"] = !displayOff;
    doc["dayStartTime"] = dayStart;
    doc["nightStartTime"] = nightStart;
    doc["sleepStartTime"] = sleepStart;
    doc["sleepModeEnabled"] = sleepModeEnabled;
    doc["dayBrightness"] = map(dayBrightness, 0, 255, 0, 100);
    doc["nightBrightness"] = map(nightBrightness, 0, 255, 0, 100);
    doc["digitalOverlayEnabled"] = digitalOverlayEnabled;
    doc["digitalInterval"] = digitaleInterval;
    doc["digitalDuration"] = digitaleDuration;
    doc["scrollPause"] = scrollPauseSeconds;
    doc["scrollManualSpeed"] = scrollTextSpeedMs;  // Velocità manuale del testo scorrevole
    doc["scrollFontSize"] = scrollFontSize;
    doc["scrollShowText"] = scrollShowText;
    doc["scrollShowDate"] = scrollShowDate;
    doc["scrollShowTime"] = scrollShowTime;
    doc["scrollShowPreset"] = scrollShowPresetDuringPause;
    doc["scrollTextualDateTime"] = scrollTextualDateTime;
    doc["scrollEnabled"] = scrollEnabled ? 1 : 0;
    doc["rainbowMode"] = textRainbowMode;
    // Colore corrente in formato hex / Current color in hex format
    char colorHex[7];
    sprintf(colorHex, "%02X%02X%02X", clockColors.current.r, clockColors.current.g, clockColors.current.b);
    doc["currentColor"] = colorHex;
    doc["hour"] = currentHour;
    doc["minute"] = currentMinute;
    doc["second"] = currentSecond;
    
    String response;
    serializeJson(doc, response);
    
    Serial.print("JSON inviato: ");
    Serial.println(response);
    Serial.println("===================");
    
    server.send(200, "application/json", response);
}


void handleSetMode() {
    if (server.hasArg("mode")) {
        uint8_t newMode = server.arg("mode").toInt();
        if (newMode < 20) {
            currentPreset = newMode;
            
            Presets::apply(currentPreset);
            
            EEPROM.write(EEPROM_PRESET_ADDR, currentPreset);
            EEPROM.commit();
            
            server.send(200, "text/plain", "OK");
            return;
        }
    }
    server.send(400, "text/plain", "Invalid mode");
}

void handleSetBlink() {
    if (server.hasArg("blink")) {
        currentBlink = server.arg("blink").toInt();
        EEPROM.write(EEPROM_BLINK_ADDR, currentBlink);
        EEPROM.commit();
        ClockDisplay::updateDisplay(); // Aggiorna il display per applicare il cambio del lampeggio
        server.send(200, "text/plain", "OK");
        return;
    }
    server.send(400, "text/plain", "Invalid blink value");
}

void handleSetColor() {
    if (server.hasArg("color")) {
        String colorHex = server.arg("color");
        if (colorHex.length() == 6) {
            long colorValue = strtol(colorHex.c_str(), NULL, 16);
            CRGB newColor;
            newColor.r = (colorValue >> 16) & 0xFF;
            newColor.g = (colorValue >> 8) & 0xFF;
            newColor.b = colorValue & 0xFF;

            clockColors.current = newColor;
            textRainbowMode = false;  // Disattiva rainbow quando si sceglie un colore

            // Reset effetti per applicare subito il nuovo colore
            slowState.active = false;
            slowState.completed = false;
            fadeState.active = false;
            fadeState.completed = false;

            // NON salvare in EEPROM - cambio temporaneo
            // Il colore originale verrà ripristinato quando si preme di nuovo il pulsante effetto

            // preset / preset
            uint8_t presetIndex = 255; // Valore per "colore personalizzato"
            if (newColor == CRGB::White) presetIndex = 0;
            else if (newColor == CRGB::Blue) presetIndex = 1;
            else if (newColor == CRGB::Red) presetIndex = 2;
            else if (newColor == CRGB::Green) presetIndex = 3;
            else if (newColor == CRGB::Yellow) presetIndex = 4;
            else if (newColor == CRGB(255,165,0)) presetIndex = 5; // Arancione

            clockColors.presetIndex = presetIndex;

            ClockDisplay::updateDisplay();
            server.send(200, "text/plain", "OK");
            return;
        }
    }
    server.send(400, "text/plain", "Invalid color");
}

void handleSetRainbow() {
    // Attiva colore Rainbow per il testo (ogni lettera un colore diverso)
    textRainbowMode = true;
    // NON salvare in EEPROM - cambio temporaneo
    server.send(200, "text/plain", "OK");
}

void handleSetBrightness() {
    if (server.hasArg("brightness")) {
        int brightnessPercent = server.arg("brightness").toInt();
        intBrightness = map(brightnessPercent, 0, 100, 0, 255);
        
        // NON salvare in EEPROM - questo è solo un aggiornamento temporaneo
        // L'EEPROM verrà aggiornato solo quando si clicca "Salva Impostazioni Giorno/sera/Notte"
        
        FastLED.setBrightness(intBrightness);
        FastLED.show();
        
        Serial.print("Luminosità temporanea impostata a: ");
        Serial.print(brightnessPercent);
        Serial.println("%");
        
        server.send(200, "text/plain", "OK");
        return;
    }
    server.send(400, "text/plain", "Invalid brightness");
}

void handleSetDayNightSettings() {
    if (server.hasArg("dayStartTime") && server.hasArg("nightStartTime") && 
        server.hasArg("dayBrightness") && server.hasArg("nightBrightness") &&
        server.hasArg("sleepStartTime") && server.hasArg("sleepModeEnabled")) {
        
        String dayStart = server.arg("dayStartTime");
        String nightStart = server.arg("nightStartTime");
        String sleepStart = server.arg("sleepStartTime");
        int dayBright = server.arg("dayBrightness").toInt();
        int nightBright = server.arg("nightBrightness").toInt();
        bool sleepEnabled = (server.arg("sleepModeEnabled") == "true" || server.arg("sleepModeEnabled") == "1");
        
        // Parse orari
        dayStartHour = dayStart.substring(0, 2).toInt();
        dayStartMinute = dayStart.substring(3, 5).toInt();
        nightStartHour = nightStart.substring(0, 2).toInt();
        nightStartMinute = nightStart.substring(3, 5).toInt();
        sleepStartHour = sleepStart.substring(0, 2).toInt();
        sleepStartMinute = sleepStart.substring(3, 5).toInt();
        sleepModeEnabled = sleepEnabled;
        
        // Converti luminosità
        dayBrightness = map(dayBright, 0, 100, 0, 255);
        nightBrightness = map(nightBright, 0, 100, 0, 255);
        
        // Salva in EEPROM / Save to EEPROM
        EEPROM.write(EEPROM_DAY_START_HOUR_ADDR, dayStartHour);
        EEPROM.write(EEPROM_DAY_START_MINUTE_ADDR, dayStartMinute);
        EEPROM.write(EEPROM_NIGHT_START_HOUR_ADDR, nightStartHour);
        EEPROM.write(EEPROM_NIGHT_START_MINUTE_ADDR, nightStartMinute);
        EEPROM.write(EEPROM_SLEEP_START_HOUR_ADDR, sleepStartHour);
        EEPROM.write(EEPROM_SLEEP_START_MINUTE_ADDR, sleepStartMinute);
        EEPROM.write(EEPROM_SLEEP_ENABLED_ADDR, sleepEnabled ? 1 : 0);
        EEPROM.write(EEPROM_DAY_BRIGHTNESS_ADDR, dayBrightness);
        EEPROM.write(EEPROM_NIGHT_BRIGHTNESS_ADDR, nightBrightness);
        
        EEPROM.commit();
        
        // Applica immediatamente
        if (TimeUtils::isSleepTime()) {
            displayOff = true;
            FastLED.clear();
            FastLED.show();
        } else {
            displayOff = false;
            intBrightness = TimeUtils::isNightTime() ? nightBrightness : dayBrightness;
            FastLED.setBrightness(intBrightness);
            ClockDisplay::updateDisplay();
        }
        
        server.send(200, "text/plain", "OK");
        return;
    }
    server.send(400, "text/plain", "Invalid parameters");
}

void handleSetDigitalOverlay() {
    if (server.hasArg("enable")) {
        String val = server.arg("enable");
        // Consideriamo "1" o "true" (case-insensitive) come ON
        val.toLowerCase();
        digitalOverlayEnabled = (val == "1" || val == "true");
        EEPROM.write(EEPROM_DIGITAL_OVERLAY_ADDR, digitalOverlayEnabled ? 1 : 0);
        EEPROM.commit();
        server.send(200, "text/plain", "OK");
        return;
    }
    server.send(400, "text/plain", "Invalid parameters");
}

void handleSetDigitalTiming() {
    if (server.hasArg("interval") && server.hasArg("duration")) {
        uint8_t interval = server.arg("interval").toInt();
        uint8_t duration = server.arg("duration").toInt();

        // Validazione / Validation
        if (interval < 5) interval = 5;
        if (interval > 120) interval = 120;
        if (duration < 10) duration = 10;
        if (duration > 59) duration = 59;

        digitaleInterval = interval;
        digitaleDuration = duration;

        // Salva in EEPROM / Save to EEPROM
        EEPROM.write(EEPROM_DIGITAL_START_ADDR, digitaleInterval);
        EEPROM.write(EEPROM_DIGITAL_DURATION_ADDR, digitaleDuration);
        EEPROM.commit();

        server.send(200, "text/plain", "OK");
        return;
    }
    server.send(400, "text/plain", "Invalid parameters");
}

void handleSetScrollPause() {
    if (server.hasArg("pause")) {
        uint8_t pause = server.arg("pause").toInt();
        if (pause >= 5 && pause <= 600) pause = 5;

        scrollPauseSeconds = pause;

        // Salva in EEPROM / Save to EEPROM
        EEPROM.write(EEPROM_SCROLL_PAUSE_ADDR, scrollPauseSeconds);
        EEPROM.commit();

        server.send(200, "text/plain", "OK");
        return;
    }
    server.send(400, "text/plain", "Invalid parameters");
}

// Nuova funzione per gestire la velocità del testo scorrevole
void handleSetScrollSpeed() {
    if (server.hasArg("speed")) {
        uint16_t speed = server.arg("speed").toInt();
        
        // Validazione / Validation
        if (speed < 10) speed = 10;
        if (speed > 500) speed = 500;

        scrollTextSpeedMs = speed;

        // Salva in EEPROM / Save to EEPROM
        EEPROM.write(EEPROM_SCROLL_SPEED_ADDR, speed & 0xFF); // Byte basso
        EEPROM.write(EEPROM_SCROLL_SPEED_ADDR_H, (speed >> 8) & 0xFF); // Byte alto
        EEPROM.commit();

        Serial.print("Velocità testo scorrevole impostata a: ");
        Serial.print(speed);
        Serial.println(" ms");

        server.send(200, "text/plain", "OK");
        return;
    }
    server.send(400, "text/plain", "Invalid parameters");
}

void handleSetScrollEnabled() {
    if (server.hasArg("state")) {
        String s = server.arg("state");
        bool newState = (s == "1" || s.equalsIgnoreCase("true"));

        // Aggiorna runtime e EEPROM
        scrollEnabled = newState ? true : false;
        EEPROM.write(EEPROM_SCROLL_ENABLED_ADDR, scrollEnabled ? 1 : 0);
        EEPROM.commit();

        // Se disabilitato, ferma immediatamente lo scroll e mostra orologio
        if (!scrollEnabled) {
            // Forza terminazione stato scroll
            scrollPauseActive = false;
            scrollTextInitialized = false;
            scrollTextOffset = MATRIX_WIDTH;
            // preset / preset
            FastLED.clear();
            ClockDisplay::showCurrentTime(); // usa la tua funzione esistente per mostrare l'orologio
            FastLED.show();
        }

        // Risposta JSON
        StaticJsonDocument<128> doc;
        doc["ok"] = true;
        doc["scrollEnabled"] = scrollEnabled;
        String out;
        serializeJson(doc, out);
        server.send(200, "application/json", out);
        return;
    }
    server.send(400, "text/plain", "missing state");
}

void handleSetScrollFontSize() {
    if (server.hasArg("fontSize")) {
        uint8_t fontSize = server.arg("fontSize").toInt();
        if (fontSize < 1) fontSize = 1;
        if (fontSize > 3) fontSize = 3;

        scrollFontSize = fontSize;

        // Salva in EEPROM / Save to EEPROM
        EEPROM.write(EEPROM_SCROLL_FONT_SIZE_ADDR, scrollFontSize);
        EEPROM.commit();

        // Reinizializza il testo scorrevole per applicare il nuovo font
        scrollTextInitialized = false;

        server.send(200, "text/plain", "OK");
        return;
    }
    server.send(400, "text/plain", "Invalid parameters");
}

void handleSetScrollOptions() {
   // ✅ DEBUG: stampa TUTTI gli argomenti ricevuti
    Serial.println("=== handleSetScrollOptions chiamata ===");
    Serial.print("Numero argomenti: ");
    Serial.println(server.args());
    
    for (int i = 0; i < server.args(); i++) {
        Serial.print("Argomento ");
        Serial.print(i);
        Serial.print(": ");
        Serial.print(server.argName(i));
        Serial.print(" = ");
        Serial.println(server.arg(i));
    }
    
    Serial.print("hasArg('pause'): ");
    Serial.println(server.hasArg("pause") ? "SI" : "NO");
    // ✅ FINE DEBUG
    if (server.hasArg("showText") && server.hasArg("showDate") && server.hasArg("showTime")) {
        scrollShowText = server.arg("showText").toInt() == 1;
        scrollShowDate = server.arg("showDate").toInt() == 1;
        scrollShowTime = server.arg("showTime").toInt() == 1;
        Serial.println("✅ Opzioni scroll salvati: ");  

        // ✅ Salva anche la pausa se presente
        if (server.hasArg("pause")) {
            uint8_t pause = server.arg("pause").toInt();
            if (pause >= 1) {
                scrollPauseSeconds = pause;
                EEPROM.write(EEPROM_SCROLL_PAUSE_ADDR, scrollPauseSeconds);
                Serial.print("✅ Pausa salvata: ");  
                Serial.println(scrollPauseSeconds);  
            }
        }
        
        scrollTextInitialized = false;
        
        EEPROM.write(EEPROM_SCROLL_SHOW_TEXT_ADDR, scrollShowText ? 1 : 0);
        EEPROM.write(EEPROM_SCROLL_SHOW_DATE_ADDR, scrollShowDate ? 1 : 0);
        EEPROM.write(EEPROM_SCROLL_SHOW_TIME_ADDR, scrollShowTime ? 1 : 0);
        EEPROM.commit();
        
        server.send(200, "text/plain", "OK");
        return;
    }
    server.send(400, "text/plain", "Invalid parameters");
}

void handleSetScrollShowPreset() {
    if (server.hasArg("showPreset")) {
        scrollShowPresetDuringPause = server.arg("showPreset").toInt() == 1;

        EEPROM.write(EEPROM_SCROLL_SHOW_PRESET_ADDR, scrollShowPresetDuringPause ? 1 : 0);
        EEPROM.commit();

        server.send(200, "text/plain", "OK");
        return;
    }
    server.send(400, "text/plain", "Invalid parameters");
}

void handleSetScrollTextualDateTime() {
    if (server.hasArg("textual")) {
        scrollTextualDateTime = server.arg("textual").toInt() == 1;

        scrollTextInitialized = false;  // Reinizializza per applicare il cambiamento

        EEPROM.write(EEPROM_SCROLL_TEXTUAL_DATETIME_ADDR, scrollTextualDateTime ? 1 : 0);
        EEPROM.commit();

        server.send(200, "text/plain", "OK");
        return;
    }
    server.send(400, "text/plain", "Invalid parameters");
}

void handleSetScrollText() {
    if (server.hasArg("text")) {
        String rawText = server.arg("text");
        Serial.print("Raw text ricevuto: [");
        Serial.print(rawText);
        Serial.print("] len=");
        Serial.println(rawText.length());

        // Decodifica URL encoding
        rawText.replace("+", " ");
        rawText.replace("%20", " ");
        rawText.replace("%2C", ",");
        rawText.replace("%2E", ".");
        rawText.replace("%21", "!");
        rawText.replace("%3F", "?");
        rawText.replace("%27", "'");
        rawText.replace("%2D", "-");
        rawText.toUpperCase();

        Serial.print("Dopo decode/upper: [");
        Serial.print(rawText);
        Serial.println("]");

        // Filtra solo caratteri ASCII stampabili (32-126) e sostituisci accentate
        customScrollText = "";
        for (unsigned int i = 0; i < rawText.length(); i++) {
            char c = rawText.charAt(i);
            // Sostituisci lettere accentate comuni (UTF-8)
            if (c == (char)0xC3) { // Primo byte UTF-8 per lettere accentate
                i++;
                if (i < rawText.length()) {
                    char c2 = rawText.charAt(i);
                    if (c2 == (char)0x80 || c2 == (char)0xA0) c = 'A'; // À à
                    else if (c2 == (char)0x88 || c2 == (char)0xA8) c = 'E'; // È è
                    else if (c2 == (char)0x89 || c2 == (char)0xA9) c = 'E'; // É é
                    else if (c2 == (char)0x8C || c2 == (char)0xAC) c = 'I'; // Ì ì
                    else if (c2 == (char)0x92 || c2 == (char)0xB2) c = 'O'; // Ò ò
                    else if (c2 == (char)0x99 || c2 == (char)0xB9) c = 'U'; // Ù ù
                    else c = '?';
                    customScrollText += c;
                }
            } else if (c >= 32 && c <= 126) {
                // Converti minuscole in maiuscole
                if (c >= 'a' && c <= 'z') c = c - 32;
                customScrollText += c;
            }
            // Ignora altri caratteri non ASCII
        }

        scrollTextInitialized = false;
        digitalOverlayActive = false; // disattiva overlay se eravamo in digitale
        lastEffectChangeTime = millis();

        // Salva il testo in EEPROM
        uint8_t textLen = customScrollText.length();
        if (textLen > EEPROM_SCROLL_TEXT_MAX_LEN) textLen = EEPROM_SCROLL_TEXT_MAX_LEN;
        EEPROM.write(EEPROM_SCROLL_TEXT_LEN_ADDR, textLen);
        for (uint8_t i = 0; i < textLen; i++) {
            EEPROM.write(EEPROM_SCROLL_TEXT_START_ADDR + i, customScrollText.charAt(i));
        }
        // preset / preset
        EEPROM.write(EEPROM_PRESET_ADDR, currentPreset);
        EEPROM.commit();

        Serial.print("Testo ricevuto: ");
        Serial.println(customScrollText);
        Serial.print("Lunghezza: ");
        Serial.println(textLen);

        server.send(200, "text/plain", "OK");
        return;
    }
    server.send(400, "text/plain", "Missing text");
}

void handleSetScrollTextColor() {
    if (server.hasArg("color")) {
        String colorHex = server.arg("color");
        if (colorHex.length() == 6) {
            long colorValue = strtol(colorHex.c_str(), NULL, 16);
            scrollTextColor.r = (colorValue >> 16) & 0xFF;
            scrollTextColor.g = (colorValue >> 8) & 0xFF;
            scrollTextColor.b = colorValue & 0xFF;
            scrollTextRainbow = false;
            
            // Salva in EEPROM / Save to EEPROM
            EEPROM.write(EEPROM_SCROLLTEXT_R_ADDR, scrollTextColor.r);
            EEPROM.write(EEPROM_SCROLLTEXT_G_ADDR, scrollTextColor.g);
            EEPROM.write(EEPROM_SCROLLTEXT_B_ADDR, scrollTextColor.b);
            EEPROM.write(EEPROM_SCROLLTEXT_RAINBOW_ADDR, 0);
            EEPROM.commit();
            
            server.send(200, "text/plain", "OK");
            return;
        }
    }
    server.send(400, "text/plain", "Invalid color");
}

void handleSetScrollTextRainbow() {
    scrollTextRainbow = true;
    
    // Salva in EEPROM / Save to EEPROM
    EEPROM.write(EEPROM_SCROLLTEXT_RAINBOW_ADDR, 1);
    EEPROM.commit();
    
    server.send(200, "text/plain", "OK");
}


void handlePower() {
    if (server.hasArg("power")) {
        displayOff = !server.arg("power").toInt(); // Inverti perché displayOff = true significa spento
        if (displayOff) {
            FastLED.clear();
            FastLED.show();
        } else {
            ClockDisplay::updateDisplay();
        }
        server.send(200, "text/plain", "OK");
        return;
    }
    server.send(400, "text/plain", "Invalid power value");
}

void handleReset() {
    server.send(200, "text/plain", "OK");
    delay(1000);
    ESP.restart();
}

void handleNotFound() {
    server.send(404, "text/plain", "Pagina non trovata");
}
