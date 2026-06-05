# Oraquadra2 — PlatformIO Port

Questo repository contine la conversione del progetto Oraquadra 2 convertito per PlatformIO e con l'utilizzo di una ESP32 DevKit standard.

Per schema, stampa3d e riferimenti: [SurvivalHacking/Oraquadra2](https://github.com/SurvivalHacking/Oraquadra2) — versione v1.2.8

---

## Hardware

| Componente | Dettaglio |
|---|---|
| MCU | ESP32 (board `esp32dev`) |
| Matrice LED | 16×16 WS2812B, 256 LED totali |
| Pin dati LED | GPIO 5 |
| Pulsante MODE | GPIO 18 |
| Pulsante SEC | GPIO 19 |

### Pulsanti

- **Pulsante 1 (SEC):** attiva/disattiva il lampeggio dei secondi sulla lettera E
- **Pulsante 2 (MODE):** cambia preset/effetto
- **Entrambi tenuti 5 s:** reset configurazione WiFi
- **Uno tenuto 6 s:** attiva/disattiva il quadrante digitale alternato

Impostare `BUTTON_LOGIC_INVERTED` in `main.cpp`:
- `1` — pulsanti normali NA verso GND (pull-up interno)
- `0` — pulsanti touch (pull-down)


## Dipendenze

Gestite automaticamente da PlatformIO tramite `platformio.ini`:

| Libreria | Versione | Uso |
|---|---|---|
| [FastLED](https://github.com/FastLED/FastLED) | ^3.10.3 | Controllo LED WS2812B |
| [ezTime](https://github.com/ropg/ezTime) | ^0.8.3 | Sincronizzazione NTP e timezone |
| [WiFiManager](https://github.com/tzapu/WiFiManager) | ^2.0.17 | Captive portal configurazione WiFi |
| [ArduinoJson](https://github.com/bblanchon/ArduinoJson) | ^7.2.2 | API REST interfaccia web |

Le librerie `WiFi`, `WebServer`, `ESPmDNS`, `ArduinoOTA` ed `EEPROM` sono incluse nel framework ESP32 Arduino e non richiedono dichiarazione esplicita.

---

## Build e upload

su visual studio code, dopo aver caricato il progetto, andare su esp32dev -> General -> Upload and Monitor. Solita procedura, oppure a linea di comando:

```bash
# Compilazione
pio run

# Upload via USB
pio run --target upload

# Upload via OTA (dopo il primo flash)
pio run --target upload --upload-port <ip-esp32>

# Monitor seriale
pio device monitor --baud 115200
```

---

## Adattamento da Arduino IDE a PlatformIO

Il codice originale è un singolo file `.ino` pensato per Arduino IDE. Di seguito le modifiche necessarie per compilarlo con PlatformIO.

### 1. Rimozione di `jwrw/ESP_EEPROM`

Il codice originale dichiara `#include <EEPROM.h>` con un commento che rimanda alla libreria `jwrw/ESP_EEPROM`. Questa libreria è progettata per **ESP8266** e include internamente `flash_hal.h`, un header non presente nel toolchain ESP32, causando un errore di build:

```
fatal error: flash_hal.h: No such file or directory
```

Il framework **ESP32 Arduino** include già una propria implementazione di `EEPROM.h` con la stessa identica API (`begin`, `read`, `write`, `commit`). È sufficiente non dichiarare `jwrw/ESP_EEPROM` in `lib_deps` e lasciare l'`#include <EEPROM.h>` invariato.

### 2. Correzione macro `NUM_MODES` duplicata

Nel file originale la macro `NUM_MODES` veniva definita due volte:

```cpp
// Riga 128 — conteggio effetti display
#define NUM_MODES   11

// Riga 176 — numero preset EEPROM
#define NUM_MODES 20
```

Arduino IDE non blocca le ridefinizioni di macro; il compilatore GCC usato da PlatformIO emette un warning. La prima definizione (`11`) è stata rimossa: la seconda (`20`) è quella effettivamente usata da tutto il codice per la gestione dei preset in EEPROM.

### 3. Forward declarations mancanti

Arduino IDE pre-processa automaticamente i file `.ino` generando le forward declarations per tutte le funzioni prima della compilazione. Il compilatore C++ standard (usato da PlatformIO) non fa questo: ogni funzione deve essere dichiarata prima del punto in cui viene chiamata.

Il codice originale aveva già una sezione di forward declarations, ma mancavano alcune funzioni usate prima della loro definizione. **Aggiunte:**

```cpp
void updateDisplay();
void updateRainbowEffect();
void setupOTA();
void updateCurrentTimeFromTZ();
bool isNightTime();
bool isSleepTime();
void handleSetScrollTextColor();
void handleSetScrollTextRainbow();
```

## 🧾 Licenza

Questo progetto segue la licenza del progetto originale, ovvero è distribuito con licenza Creative Commons – Attribuzione – Non Commerciale 4.0 Internazionale (CC BY-NC 4.0).

Puoi condividerlo e modificarlo liberamente, citando l’autore (Davide Gatti / survivalhacking) e senza scopi commerciali.

🔗 https://creativecommons.org/licenses/by-nc/4.0/