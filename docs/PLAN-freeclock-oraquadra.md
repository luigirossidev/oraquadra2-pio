# Oraquadra → ecosistema FreeClock ("FreeClock Oraquadra")

## Context

Oraquadra2 (orologio a parole su matrice 16×16 WS2812B) oggi si configura via WiFiManager (captive portal) + web server HTTP con ~20 endpoint. L'obiettivo è integrarlo nell'ecosistema FreeClock: associazione WiFi e **tutta** la configurazione tramite l'app mobile FreeClock (BLE), eliminando il web server. In più si aggiungono tre funzionalità di FreeClock: frase del giorno, indovinello del giorno (mostrati con lo scroll testo già esistente) e jukebox (richiede l'aggiunta di un buzzer passivo). News, meteo e brand di FreeClock **non** vengono portati.

L'app deve distinguere alla connessione se il dispositivo è un FreeClock normale o un "FreeClock Oraquadra" (campo `deviceType` già presente nella characteristic STATUS) e mostrare solo le impostazioni pertinenti.

Decisioni prese con l'utente:
- **Buzzer su GPIO 25** (GPIO 5 è occupato dai LED, 18/19 dai pulsanti).
- **Frase/indovinello mostrati premendo il pulsante SEC** (ex "blink"): il toggle blink si sposta nell'app.
- **OTA da server**: si porta anche `OTAHelper` con `DEVICE_TYPE="oraquadra"` (la pagina Aggiornamento dell'app funziona così com'è; serve che il server pubblichi il firmware per questo device — dipendenza esterna).
- **Persistenza: tutto su SPIFFS JSON** (stile `FileMemorySettings.h` di FreeClock); EEPROM ed `eeprom_map.h` vengono eliminati. Nessuna migrazione dai vecchi dati EEPROM: alla prima accensione si riconfigura dall'app.

Repo coinvolti:
- **oraquadra2-pio** (questo repo): nuovo branch `feature/freeclock-oraquadra` da `refactor/split-main`
- **freeclock-mobile** (`/Users/luigirossi/iot-dev/freeclock-mobile`): branch `feature/oraquadra-support`
- **freeclock-esp32**: solo lettura (sorgente da cui portare i moduli), nessuna modifica

---

## Parte 1 — Firmware oraquadra2-pio

### 1.1 platformio.ini
- Aggiungere `h2zero/NimBLE-Arduino@^2.1.2`, rimuovere `tzapu/WiFiManager`
- `board_build.partitions = min_spiffs.csv` (BLE+WiFi+OTA non stanno nella partizione app di default; FreeClock usa lo stesso schema)
- `build_flags = -DDEVICE_TYPE=\"oraquadra\"`

### 1.2 Nuova persistenza SPIFFS (sostituisce EEPROM)
Nuovo `include/fc_config.h` + `src/fc_config.cpp` modellati su `freeclock-esp32/src/workers/FileMemorySettings.h` (pattern: struct `Config` globale, `loadConfigurations()` con default via `|`, `writeConfigurations()` con file temp + rename). Campi:
- **Connettività**: `WIFI_SSID`, `WIFI_PASSWORD`, `blePinCode` (generato al primo avvio come in `bleConfigInit()`), `TIMEZONE` (stringa POSIX, default Europe/Rome)
- **Quote/quiz**: `quoteUrl`, `quizUrl` (default `https://freeclock.matesemakers.it`), `quizHour` (default 18), `isQuote`, `isQuiz`
- **Orologio** (oggi in EEPROM, vedi `include/eeprom_map.h`): `currentPreset`, `currentBlink`, `dayBrightness`, `nightBrightness`, orari day/night/sleep, `sleepModeEnabled`, `digitalOverlayEnabled`, `digitaleInterval`, `digitaleDuration`
- **Scroll** : `scrollEnabled`, testo custom (max 200), colore RGB, rainbow, `scrollTextSpeedMs`, `scrollFontSize`, `scrollPauseSeconds`, showText/Date/Time, `scrollTextualDateTime`, `scrollShowPresetDuringPause`
- **Preset colori**: array di 20 voci `{saved, r, g, b, rainbow}` (sostituisce i 20×5 byte EEPROM di `Settings::save/loadModeSettings`)

Conseguenze:
- `src/settings.cpp` riscritto: `Settings::begin()` monta SPIFFS e carica config nelle variabili globali di `state.h`; `save/loadModeSettings` operano sull'array preset
- Eliminare `include/eeprom_map.h`; in `src/buttons.cpp` sostituire le ~6 coppie `EEPROM.write/commit` con aggiornamento campo + `writeConfigurations()`

### 1.3 BLE config server — nuovo `include/ble_config.h` + `src/ble_config.cpp`
Porting di `freeclock-esp32/src/workers/BLEConfig.h`, **stesso service UUID** `FC000001-...` così l'app lo trova in scansione. Nome device: `FreeClock-OQ-XXXX` (mantiene il prefisso `FreeClock-` filtrato da `ble.service.ts:43`, ma riconoscibile a vista).

Characteristic riusate identiche (stessi UUID, stessa semantica JSON):
- `AUTH` (PIN app-level), `WIFI`, `WIFI_SCAN` (notify), `STATUS` (notify, **senza auth**: `fw`, `fwCode`, `serverUrl`, `deviceType:"oraquadra"`, `wifi`, `ip`, `heap`, `ssid`), `COMMAND` (`save|reboot|factory_reset|wifi_reconnect|wifi_scan|ota_update`; factory_reset = `SPIFFS.remove("/settings.json")` + restart), `MELODY` (lista + trigger)
- `TIMEWEATHER`: implementa **solo** il campo `tz` (il wizard timezone dell'app scrive `{tz}` qui — `timezone.page.ts:61`); gli altri campi ignorati in write e omessi in read
- `QUOTEQUIZ`: `quoteUrl`, `quizUrl`, `quizHour` come FreeClock + campi extra `quote`/`quiz` (enable, che su FreeClock stanno in GENERAL)

Characteristic nuove (pattern UUID FC0000xx):
- `CHAR_OQ_CLOCK` `FC0000D0-...`: `preset` (0–19), `blink`, `color` (hex), `rainbow`, `power`, `dayBright`, `nightBright` (0–100), `dayStart`/`nightStart`/`sleepStart` (HH:MM), `sleepEnabled` — replica di `handleSetMode/Blink/Color/Rainbow/Brightness/DayNightSettings/Power` di `src/web_ui.cpp`
- `CHAR_OQ_SCROLL` `FC0000E0-...`: `enabled`, `text`, `color`, `rainbow`, `speed`, `fontSize`, `pause`, `showText`, `showDate`, `showTime`, `textual`, `showPreset` — replica degli handler `handleSetScroll*`
- `CHAR_OQ_DIGITAL` `FC0000F0-...`: `enabled`, `interval` (5–120), `duration` (10–59)

Come in FreeClock, le write settano flag (`_bleNeedsReconnect`, `_bleNeedsTimezoneUpdate`, `_bleNeedsClockApply`, `_bleNeedsScrollApply`, `_bleNeedsMelodyPlay`, `_bleNeedsOTAUpdate`, …) applicati nel loop principale; la sanificazione del testo scroll (maiuscole + accenti → ASCII) si sposta da `handleSetScrollText` (web_ui.cpp:565) a un helper in `ScrollText`.

### 1.4 Rete: via WiFiManager, dentro connessione da config
Riscrivere `src/network.cpp`:
- `connectToWiFi()` come `freeclock-esp32/src/main.cpp:116`: `WiFi.begin(config.WIFI_SSID, ...)`, ~20 tentativi, niente captive portal. Senza SSID configurato il dispositivo resta in attesa con BLE attivo e **fa scorrere il PIN BLE sulla matrice** periodicamente (serve per il pairing, come FreeClock mostra il PIN sul display)
- NTP: mantenere ezTime ma `myTZ.setPosix(config.TIMEZONE)` al posto di `setLocation(F("Europe/Rome"))` hardcoded (`src/app.cpp:75`); su cambio tz da BLE riapplicare
- Mantenere ArduinoOTA (sviluppo locale)
- **OTA da server**: portare `freeclock-esp32/src/workers/OTAHelper.h` → `src/ota_helper.cpp`; su comando `ota_update` scarica da `quoteUrl + /api/fw/dl?device=oraquadra`; progresso mostrato riempiendo la matrice (riusare il pattern di `Network::setupOTA` onProgress)

### 1.5 Buzzer + Jukebox
- `config.h`: `#define BUZZER_PIN 25`, `ENABLE_SOUND`/`ENABLE_MELODY`
- Portare da freeclock-esp32: `assets/effects/SoundEffects.h` (buzzerInit/buzzerPlay via LEDC), `assets/MelodyList.h`, `assets/notes_int.h`, `assets/melodies/*` (28 melodie), `workers/MelodyPlayer.h` → `src/melody_player.cpp`
- Adattamento MelodyPlayer: al posto di `PDisplay.displayMode = 20`, il messaggio della richiesta viene mostrato con lo scroll di oraquadra (interruzione temporanea del ciclo normale, poi ritorno all'orologio); abort con pressione di un pulsante fisico

### 1.6 Frase e indovinello del giorno
- Portare `services/DailyQuoteApiClient.{h,cpp}` e `services/DailyQuizApiClient.{h,cpp}` (fetch HTTPS `GET {url}/api/daily/quote|quiz`, cache per data, skip se NTP non sincronizzato) in `src/services/`
- Fetch: una volta al giorno nel loop (quando WiFi connesso e `isQuote`/`isQuiz` attivi)
- **Visualizzazione col pulsante SEC** (ripurposato, vedi 1.7): 1ª pressione → scroll "FRASE: …"; 2ª pressione → scroll "INDOVINELLO: …" (con soluzione accodata se `hour() >= quizHour`, come `DailyQuiz` di FreeClock); poi ritorno all'orologio. Testo passato per la sanificazione accenti prima dello scroll

### 1.7 Pulsanti (`src/buttons.cpp`)
- MODE breve: invariato (preset successivo, salvataggio su config invece che EEPROM)
- MODE lungo 6 s: invariato (toggle overlay digitale)
- **SEC breve: mostra frase → indovinello** (non più toggle blink; il blink si comanda da app via `CHAR_OQ_CLOCK`)
- **SEC lungo 6 s: scroll del PIN BLE** sulla matrice (equivalente del pulsante fisico FreeClock)
- Entrambi 5 s: **factory reset** (cancella `settings.json` + riavvio) al posto del solo reset WiFi

### 1.8 Rimozioni e collante
- Eliminare `src/web_ui.cpp`, `include/web_ui.h`, `src/webpage.cpp`, `include/webpage.h` (~2650 righe)
- `src/app.cpp`: setup → Settings (SPIFFS) → FastLED → `bleConfigInit()` + `melodyPlayerInit()` → WiFi → NTP/posix → preset; loop → logica display esistente + gestione flag BLE (riconnessione, scan, tz, OTA, melody, apply clock/scroll) + `bleUpdateStatus()` ogni 10 s + fetch giornaliero quote/quiz. Rimuovere le chiamate `WebUI::begin/handle`
- Aggiornare README (sezione branch + nuovo hardware buzzer + pairing via app)

## Parte 2 — App freeclock-mobile

### 2.1 Rilevamento tipo dispositivo
- `paired-device.model.ts`: aggiungere `deviceType?: string`
- Wizard `pin.page.ts` (dopo auth riuscita, prima di `addPairedDevice` a riga ~84): leggere `CHAR_STATUS_UUID` e salvare `deviceType`
- `devices-list.page.ts:connectDevice`: dopo `authenticate`, leggere STATUS, aggiornare il device salvato e passare il tipo a `loadAllFromDevice`
- `DeviceStoreService.loadAllFromDevice(deviceType)`: per `oraquadra` leggere WIFI, QUOTEQUIZ, OQ_CLOCK, OQ_SCROLL, OQ_DIGITAL (niente GENERAL/NEWS/BRAND/TIMEWEATHER/DISPLAY che sul device non esistono); per gli altri, invariato

### 2.2 Costanti e modelli
- `ble-uuids.ts`: `CHAR_OQ_CLOCK/SCROLL/DIGITAL`
- `device-config.model.ts`: `OqClockConfig`, `OqScrollConfig`, `OqDigitalConfig`; `QuoteQuizConfig` estesa con `quote?`/`quiz?` (enable, solo oraquadra)
- `device-store.service.ts`: subject + read/write per le tre nuove characteristic

### 2.3 Pagine
- `device-detail.page.ts`: menu condizionato da `deviceType`. Per oraquadra: **Orologio ed effetti** (nuova), **Testo scorrevole** (nuova), **Quadrante digitale** (nuova), **Fuso orario** (nuova, riusa `timezone-selector` e scrive `{tz}` su TIMEWEATHER), WiFi (riuso), Frase & Indovinello (riuso + toggle enable visibili solo per oraquadra), Jukebox (riuso invariato), Aggiornamento (riuso invariato: usa già `status.deviceType`), Info dispositivo (riuso). Per FreeClock normale: invariato
- Nuove pagine in `main/devices/settings/`: `oq-clock/` (select preset con i 20 nomi da `web_ui.cpp:100-119`, color picker, toggle rainbow/blink/power, slider luminosità giorno/notte, time picker fasce orarie, toggle sleep), `oq-scroll/`, `oq-digital/` — stesso pattern draft + Salva delle pagine esistenti (es. `general.page.ts`)
- Wizard: **nessuna modifica al flusso** (scan filtra per service UUID + prefisso nome, già compatibile; wifi-setup e timezone usano characteristic che oraquadra implementa)
- i18n: nuove chiavi in `src/assets/i18n/it.json` e `en.json`
- `mock-ble.service.ts`: variante mock oraquadra per sviluppo browser (facoltativo ma utile)

---

## Verifica

**Firmware** (build + hardware):
1. `pio run` compila; controllare dimensione firmware nella partizione min_spiffs
2. Flash su ESP32: al primo avvio (senza config) la matrice fa scorrere il PIN BLE; advertise come `FreeClock-OQ-XXXX`
3. Wizard app completo: scan → PIN → WiFi (scan reti + credenziali) → timezone → done; l'orologio si sincronizza e mostra l'ora
4. Da app: cambio preset/colore/luminosità/fasce/scroll/overlay applicati live e persistenti al riavvio (riavvio da app con comando `reboot`)
5. Jukebox: melodia sul buzzer GPIO 25 + messaggio a scorrimento; pulsante fisico interrompe
6. Pulsante SEC: frase → indovinello (soluzione solo dopo `quizHour`); MODE: preset; entrambi 5 s: factory reset
7. Monitor seriale 115200 per i log di ogni fase

**App** (in freeclock-mobile):
1. `npm run build` senza errori; test con mock in browser (`ionic serve`)
2. Su dispositivo reale: connessione a oraquadra → menu ridotto corretto; connessione a un FreeClock normale → **nessuna regressione** (menu e pagine invariati)

**Dipendenza esterna** (fuori scope, da segnalare): il server `freeclock.matesemakers.it` deve pubblicare il firmware per `device=oraquadra` perché la pagina Aggiornamento funzioni.

## Ordine di lavoro suggerito
1. Branch + platformio.ini + fc_config (SPIFFS) + rimozione web server/WiFiManager → build verde
2. BLE server con characteristic base (AUTH/WIFI/SCAN/STATUS/COMMAND/TIMEWEATHER) → wizard app funzionante end-to-end
3. Characteristic OQ_* + rework pulsanti
4. Buzzer/MelodyPlayer + jukebox
5. Quote/quiz client + pulsante SEC + OTA helper
6. App mobile: rilevamento tipo, nuove pagine, menu condizionale
7. README e test finali su hardware
