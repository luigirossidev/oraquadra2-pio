# Oraquadra2 — PlatformIO Port

Questo repository contine la conversione del progetto Oraquadra 2 convertito per PlatformIO e con l'utilizzo di una ESP32 DevKit standard.

Per schema, stampa3d e riferimenti: [SurvivalHacking/Oraquadra2](https://github.com/SurvivalHacking/Oraquadra2) — versione v1.2.8

Su questo branch (`feature/freeclock-oraquadra`) il progetto è integrato nell'ecosistema **FreeClock**: la configurazione avviene esclusivamente tramite l'app mobile FreeClock via Bluetooth LE (niente più web server né captive portal), con in più jukebox su buzzer, frase e indovinello del giorno dal server FreeClock.

---

## Hardware

| Componente | Dettaglio |
|---|---|
| MCU | ESP32 (board `esp32dev`) |
| Matrice LED | 16×16 WS2812B, 256 LED totali |
| Pin dati LED | GPIO 5 |
| Pulsante MODE | GPIO 18 |
| Pulsante SEC | GPIO 19 |
| Buzzer passivo | GPIO 25 (jukebox FreeClock) |

### Collegamenti

```
                 ┌────────────────────┐
Alimentatore 5V  │                    │
  (+) ───────────┼──► rosso centrale  │
  (–) ───┬───────┼──► nero centrale   │   Matrice 16×16 WS2812B
         │       │                    │
         │       │    connettore IN:  │
ESP32    │       │                    │
  GND ───┴───────┼──► GND             │
  GPIO 5 ──[330Ω]┼──► DIN             │
                 └────────────────────┘

ESP32 GPIO 18 ──► pulsante MODE ──► GND
ESP32 GPIO 19 ──► pulsante SEC  ──► GND
ESP32 GPIO 25 ──► buzzer passivo (+), (–) ──► GND
```

| ESP32 | Collegato a | Note |
|---|---|---|
| GPIO 5 | **DIN** della matrice (connettore IN) | Resistenza da 330 Ω in serie, vicino alla matrice |
| GND | GND matrice / alimentatore | **Massa comune obbligatoria** tra ESP32, matrice e alimentatore |
| VIN (5V) | +5V alimentatore | Facoltativo: alimenta l'ESP32 dallo stesso alimentatore (mai sul pin 3V3) |
| GPIO 18 | Pulsante MODE → GND | Pull-up interno, nessuna resistenza esterna (con `BUTTON_LOGIC_INVERTED 1`) |
| GPIO 19 | Pulsante SEC → GND | Come sopra |
| GPIO 25 | Buzzer passivo (+) | Il polo (–) va a GND. Serve un buzzer **passivo**: uno attivo suonerebbe una sola nota |

**I fili della matrice.** La matrice ha tre gruppi di fili:
- **Connettore IN** (3 pin: +5V, DIN, GND): è l'ingresso e va collegato all'ESP32. Lo riconosci dalla scritta **DIN** o dalle frecce stampate sul retro del circuito.
- **Coppia centrale rosso/nero:** è l'alimentazione e va all'alimentatore 5V.
- **Connettore OUT** (DOUT): serve solo per collegare un'altra matrice in cascata. Va lasciato libero.

**Alimentazione.**
- **Con alimentatore dedicato** (consigliato per l'uso definitivo): usa un alimentatore 5V da almeno 4 A e metti un condensatore da 1000 µF (≥ 6,3 V) tra +5V e GND, vicino ai fili centrali. Poi alza `LED_MAX_POWER_MA` in `include/config.h` (es. 3000 con un alimentatore da 4 A).
- **Solo da USB** (va bene per sviluppo e prove): collega il +5V della matrice al pin **VIN** dell'ESP32. Il firmware limita i LED a `LED_MAX_POWER_MA` (500 mA di default): oltre quel valore FastLED abbassa da sola la luminosità, così l'ESP32 non va in brownout. Usa un caricatore USB da 2 A invece della porta del PC.

**Livelli logici.** L'ESP32 manda i dati a 3,3 V e di solito la matrice li legge senza problemi. Se vedi sfarfallii o colori casuali, aggiungi un level shifter (es. 74AHCT125) sul filo dati.

### Pulsanti

- **Pulsante MODE (breve):** cambia preset/effetto
- **Pulsante MODE (6 s):** attiva/disattiva il quadrante digitale alternato
- **Pulsante SEC (breve):** mostra la frase del giorno; una seconda pressione mostra l'indovinello (il lampeggio dei secondi si regola dall'app)
- **Pulsante SEC (6 s):** fa scorrere il PIN BLE per il pairing con l'app
- **Entrambi tenuti 5 s:** factory reset (cancella la configurazione e riavvia)

Impostare `BUTTON_LOGIC_INVERTED` in `include/config.h`:
- `1` — pulsanti normali NA verso GND (pull-up interno)
- `0` — pulsanti touch (pull-down)

## Configurazione tramite app FreeClock

Il dispositivo si annuncia via BLE come `FreeClock-OQ-XXXX` (stesso service UUID di FreeClock: l'app lo trova con la normale scansione e lo riconosce come tipo `oraquadra`).

1. Al primo avvio la matrice fa scorrere il **PIN BLE** (riproposto ogni 60 s finché non configurato, o su pressione lunga di SEC)
2. Dall'app FreeClock: **Aggiungi dispositivo** → scansione → inserimento PIN → configurazione WiFi → fuso orario
3. Dal dettaglio dispositivo si regolano: preset/effetti e colori, luminosità e fasce giorno/sera/notte, testo scorrevole, quadrante digitale, frase/indovinello del giorno, jukebox, WiFi, fuso orario e aggiornamento firmware

Le funzioni meteo, news e brand di FreeClock non esistono su questo dispositivo e l'app non le propone.

Le impostazioni sono salvate su SPIFFS in `/settings.json` (stile FreeClock); la EEPROM non è più usata.

## Dipendenze

Gestite automaticamente da PlatformIO tramite `platformio.ini`:

| Libreria | Versione | Uso |
|---|---|---|
| [FastLED](https://github.com/FastLED/FastLED) | ^3.10.3 | Controllo LED WS2812B |
| [ezTime](https://github.com/ropg/ezTime) | ^0.8.3 | Sincronizzazione NTP e timezone (stringa POSIX dall'app) |
| [ArduinoJson](https://github.com/bblanchon/ArduinoJson) | ^7.2.2 | Payload JSON delle characteristic BLE e settings su SPIFFS |
| [NimBLE-Arduino](https://github.com/h2zero/NimBLE-Arduino) | ^2.1.2 | Server BLE di configurazione (app FreeClock) |
| EasyOTA | in `lib/` | Aggiornamento firmware dal server FreeClock |

Le librerie `WiFi`, `ESPmDNS`, `ArduinoOTA`, `HTTPClient` e `SPIFFS` sono incluse nel framework ESP32 Arduino. Le partizioni usate sono `min_spiffs.csv` (necessarie per BLE+WiFi+OTA).

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

L'aggiornamento firmware è disponibile anche dalla pagina **Aggiornamento** dell'app, se il server FreeClock pubblica un firmware per `device=oraquadra`.

## Branch

| Branch | Contenuto |
|---|---|
| `main` | Porting PlatformIO del codice originale: tutto il codice in un unico `src/main.cpp` (~6700 righe), fedele allo sketch Arduino di partenza |
| `refactor/split-main` | Refactor strutturale: lo stesso codice suddiviso in ~58 file per area funzionale |
| `feature/freeclock-oraquadra` | **(questo branch)** Integrazione ecosistema FreeClock: configurazione via app mobile (BLE), jukebox, frase/indovinello del giorno; rimossi web server, WiFiManager ed EEPROM |

### Struttura dei moduli (questo branch)

```
include/                       src/
  config.h        pin (LED, pulsanti, buzzer), matrice, ID modalità, tuning effetti
  fc_config.h     configurazione persistente SPIFFS (WiFi, PIN BLE,   fc_config.cpp
                  timezone POSIX, frase/quiz, colori per preset)
  ble_config.h    server BLE compatibile app FreeClock                 ble_config.cpp
  melody_player.h jukebox: task melodie su buzzer (28 brani)           melody_player.cpp
  melody_assets/  archivio melodie (PROGMEM, da FreeClock)
  sound_effects.h effetti sonori buzzer (da FreeClock)
  daily_content.h frase e indovinello del giorno                       daily_content.cpp
  ota_helper.h    aggiornamento firmware dal server FreeClock          ota_helper.cpp
  state.h         stato globale condiviso (extern)     state.cpp
  words_it.h      tabelle parole italiane (PROGMEM)    words_it.cpp
  fonts.h         font 5x7 e 3x5 (PROGMEM)             fonts.cpp
  clock_colors.h  gestione colori (ClockColors)        clock_colors.cpp
  led_canvas.h    Canvas: mapping serpentina, caratteri led_canvas.cpp
  time_utils.h    TimeUtils: fasce giorno/notte, NTP   time_utils.cpp
  settings.h      Settings: persistenza (su fc_config) settings.cpp
  clock_display.h ClockDisplay: orologio a parole      clock_display.cpp
  scroll_text.h   ScrollText: testo scorrevole         scroll_text.cpp
  network.h       Network: WiFi da config, OTA locale  network.cpp
  buttons.h       Buttons: pulsanti fisici             buttons.cpp
  presets.h       Presets: applicazione preset         presets.cpp
  app.h           App: corpo di setup() e loop()       app.cpp
  effects/        un file per effetto (fade, slow, matrix, matrix2,
                  tron, drop, rainbow, moto, galaga, pacman, digitale,
                  overlay digitale) + Effects (dispatch e reset al minuto)
lib/EasyOta       libreria OTA da server (da FreeClock)
src/main.cpp      ~90 righe: changelog + setup()/loop() che delegano ad App
docs/PLAN-freeclock-oraquadra.md   piano dell'integrazione FreeClock
```

---

## 🧾 Licenza

Questo progetto segue la licenza del progetto originale, ovvero è distribuito con licenza Creative Commons – Attribuzione – Non Commerciale 4.0 Internazionale (CC BY-NC 4.0).

Puoi condividerlo e modificarlo liberamente, citando l'autore (Davide Gatti / survivalhacking) e senza scopi commerciali.

🔗 https://creativecommons.org/licenses/by-nc/4.0/
