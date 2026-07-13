#include "scroll_text.h"
#include "state.h"
#include "config.h"
#include "led_canvas.h"

// Funzione per visualizzare testo scorrevole con dimensione font dinamica / Function to display scrolling text with dynamic font size
void ScrollText::show(String text, CRGB color, uint16_t scrollSpeed) {
  text.toUpperCase(); // Converti tutto in maiuscolo

  // Aggiungi spazi all'inizio e alla fine per l'effetto di entrata/uscita / Add spaces at start and end for entry/exit effect
  text = "    " + text + "    ";

  int textLength = text.length();

  // Parametri in base alla dimensione del font / Parameters based on font size
  int charWidth, charHeight, charSpacing, vertOffset;

  int scale = 1;
  switch (scrollFontSize) {
    case 1: // Piccolo 3x5
      charWidth = 3;
      charHeight = 5;
      charSpacing = 4; // 3 + 1
      vertOffset = 5;  // Centra verticalmente
      break;
    case 3: // Grande (font 5x7 scalato 2x)
      charWidth = 5;
      charHeight = 7;
      charSpacing = 6;
      vertOffset = 1;  // Centra verticalmente (16-14)/2 = 1
      scale = 2;
      break;
    default: // Medio 5x7
      charWidth = 5;
      charHeight = 7;
      charSpacing = 6; // 5 + 1
      vertOffset = 4;  // Centra verticalmente
      break;
  }

  int effectiveCharSpacing = charSpacing * scale;
  int totalWidth = textLength * effectiveCharSpacing;

  // Scorri il testo da destra a sinistra / Scroll text from right to left
  for (int offset = MATRIX_WIDTH; offset > -totalWidth; offset--) {
    FastLED.clear();

    // Disegna ogni carattere / Draw each character
    for (int charIndex = 0; charIndex < textLength; charIndex++) {
      char c = text.charAt(charIndex);
      int charX = offset + (charIndex * effectiveCharSpacing);

      if (scrollFontSize == 1) {
        // Font piccolo 3x5 / Small 3x5 font
        uint8_t charBitmap[3];
        Canvas::getCharBitmapSmall(c, charBitmap);

        if (charX + charWidth >= 0 && charX < MATRIX_WIDTH) {
          for (int col = 0; col < charWidth; col++) {
            int screenX = charX + col;
            if (screenX >= 0 && screenX < MATRIX_WIDTH) {
              for (int row = 0; row < charHeight; row++) {
                if (charBitmap[col] & (1 << row)) {
                  int screenY = row + vertOffset;
                  if (screenY < MATRIX_HEIGHT) {
                    uint16_t ledPos = Canvas::xyToLED(screenX, screenY);
                    leds[ledPos] = color;
                  }
                }
              }
            }
          }
        }
      } else if (scrollFontSize == 3) {
        // Font grande (5x7 scalato 2x = 10x14)
        uint8_t charBitmap[5];
        Canvas::getCharBitmap(c, charBitmap);
        int effectiveCharWidth = charWidth * scale;

        if (charX + effectiveCharWidth >= 0 && charX < MATRIX_WIDTH) {
          for (int col = 0; col < charWidth; col++) {
            for (int sx = 0; sx < scale; sx++) {
              int screenX = charX + (col * scale) + sx;
              if (screenX >= 0 && screenX < MATRIX_WIDTH) {
                for (int row = 0; row < charHeight; row++) {
                  if (charBitmap[col] & (1 << row)) {
                    for (int sy = 0; sy < scale; sy++) {
                      int screenY = (row * scale) + sy + vertOffset;
                      if (screenY >= 0 && screenY < MATRIX_HEIGHT) {
                        uint16_t ledPos = Canvas::xyToLED(screenX, screenY);
                        leds[ledPos] = color;
                      }
                    }
                  }
                }
              }
            }
          }
        }
      } else {
        // Font medio 5x7 / Medium 5x7 font
        uint8_t charBitmap[5];
        Canvas::getCharBitmap(c, charBitmap);

        if (charX + charWidth >= 0 && charX < MATRIX_WIDTH) {
          for (int col = 0; col < charWidth; col++) {
            int screenX = charX + col;
            if (screenX >= 0 && screenX < MATRIX_WIDTH) {
              for (int row = 0; row < charHeight; row++) {
                if (charBitmap[col] & (1 << row)) {
                  int screenY = row + vertOffset;
                  if (screenY < MATRIX_HEIGHT) {
                    uint16_t ledPos = Canvas::xyToLED(screenX, screenY);
                    leds[ledPos] = color;
                  }
                }
              }
            }
          }
        }
      }
    }

    FastLED.show();
    delay(scrollSpeed);
    yield(); // Permette al WiFi di gestire le sue operazioni
  }
}

// Converte il testo in maiuscolo e sostituisce le lettere accentate UTF-8 con
// l'equivalente ASCII; scarta tutto cio' che il font non sa disegnare.
// (Spostata qui dall'ex handler web handleSetScrollText.)
String ScrollText::sanitize(String raw) {
    raw.toUpperCase();

    String out = "";
    for (unsigned int i = 0; i < raw.length(); i++) {
        char c = raw.charAt(i);
        // Sostituisci lettere accentate comuni (UTF-8)
        if (c == (char)0xC3) { // Primo byte UTF-8 per lettere accentate
            i++;
            if (i < raw.length()) {
                char c2 = raw.charAt(i);
                if (c2 == (char)0x80 || c2 == (char)0xA0) c = 'A';      // A' a'
                else if (c2 == (char)0x88 || c2 == (char)0xA8) c = 'E'; // E' e'
                else if (c2 == (char)0x89 || c2 == (char)0xA9) c = 'E'; // E' e'
                else if (c2 == (char)0x8C || c2 == (char)0xAC) c = 'I'; // I' i'
                else if (c2 == (char)0x92 || c2 == (char)0xB2) c = 'O'; // O' o'
                else if (c2 == (char)0x99 || c2 == (char)0xB9) c = 'U'; // U' u'
                else c = '?';
                out += c;
            }
        } else if (c >= 32 && c <= 126) {
            if (c >= 'a' && c <= 'z') c = c - 32;
            out += c;
        }
        // Ignora altri caratteri non ASCII
    }
    return out;
}

// Nomi dei giorni della settimana in italiano / Italian weekday names
const char* giorniSettimana[] = {"DOMENICA", "LUNEDI'", "MARTEDI'", "MERCOLEDI'", "GIOVEDI'", "VENERDI'", "SABATO"};
// Nomi dei mesi in italiano / Italian month names
const char* mesiAnno[] = {"GENNAIO", "FEBBRAIO", "MARZO", "APRILE", "MAGGIO", "GIUGNO",
                          "LUGLIO", "AGOSTO", "SETTEMBRE", "OTTOBRE", "NOVEMBRE", "DICEMBRE"};

// Funzione helper per costruire una stringa testuale dell'ora / Helper function to build textual time string
String ScrollText::buildTextualTime() {
    String result = "SONO LE ORE ";
    
    const char* hourNames[] = {
        "ZERO", "UNA", "DUE", "TRE", "QUATTRO", "CINQUE", "SEI", "SETTE",
        "OTTO", "NOVE", "DIECI", "UNDICI", "DODICI", "TREDICI", "QUATTORDICI",
        "QUINDICI", "SEDICI", "DICIASSETTE", "DICIOTTO", "DICIANNOVE", "VENTI",
        "VENTUNO", "VENTIDUE", "VENTITRE'"
    };
    
    // USA currentHour invece di hour()
    result += hourNames[currentHour];
    
    // USA currentMinute invece di minute()
    int m = currentMinute;
    if (m > 0) {
        result += " E ";
        
        if (m <= 19) {
            const char* minuteNames[] = {
                "", "UNO", "DUE", "TRE", "QUATTRO", "CINQUE", "SEI", "SETTE",
                "OTTO", "NOVE", "DIECI", "UNDICI", "DODICI", "TREDICI", "QUATTORDICI",
                "QUINDICI", "SEDICI", "DICIASSETTE", "DICIOTTO", "DICIANNOVE"
            };
            result += minuteNames[m];
        } else {
            uint8_t tens = m / 10;
            uint8_t ones = m % 10;
            
            const char* tensNames[] = {"", "", "VENTI", "TRENTA", "QUARANTA", "CINQUANTA"};
            
            if (ones == 1 || ones == 8) {
                const char* truncTensNames[] = {"", "", "VENT", "TRENT", "QUARANT", "CINQUANT"};
                result += truncTensNames[tens];
            } else {
                result += tensNames[tens];
            }
            
            if (ones > 0) {
                const char* onesNames[] = {
                    "", "UNO", "DUE", "TRE", "QUATTRO", "CINQUE", "SEI", "SETTE", "OTTO", "NOVE"
                };
                if (ones == 1 && m >= 21) {
                    result += "UN";
                } else {
                    result += onesNames[ones];
                }
            }
        }
        
        result += " MINUTI";
    }
    
    return result;
}

String ScrollText::buildTextualDate() {
    // USA myTZ.weekday(), myTZ.day(), myTZ.month(), myTZ.year()
    // invece di weekday(), day(), month(), year()
    int giornoSett = myTZ.weekday() - 1;
    if (giornoSett < 0) giornoSett = 0;
    if (giornoSett > 6) giornoSett = 6;

    int meseIdx = myTZ.month() - 1;
    if (meseIdx < 0) meseIdx = 0;
    if (meseIdx > 11) meseIdx = 11;

    String result = giorniSettimana[giornoSett];
    result += " ";
    
    int d = myTZ.day();
    const char* dayNames[] = {
        "", "UNO", "DUE", "TRE", "QUATTRO", "CINQUE", "SEI", "SETTE", "OTTO", "NOVE", "DIECI",
        "UNDICI", "DODICI", "TREDICI", "QUATTORDICI", "QUINDICI", "SEDICI", "DICIASSETTE",
        "DICIOTTO", "DICIANNOVE", "VENTI", "VENTUNO", "VENTIDUE", "VENTITRE", "VENTIQUATTRO",
        "VENTICINQUE", "VENTISEI", "VENTISETTE", "VENTOTTO", "VENTINOVE", "TRENTA", "TRENTUNO"
    };
    
    result += dayNames[d];
    result += " ";
    result += mesiAnno[meseIdx];
    result += " ";
    result += String(myTZ.year());
    
    return result;
}

// Funzione helper per costruire il testo completo con data/ora / Helper function to build complete text with date/time
String ScrollText::buildDisplayText() {
    String parts[3];
    int numParts = 0;

    // Aggiungi data se abilitata / Add date if enabled
    if (scrollShowDate) {
        if (scrollTextualDateTime) {
            parts[numParts++] = ScrollText::buildTextualDate();
        } else {
            // Formato numerico / Numeric format
            int giornoSett = myTZ.weekday() - 1;
            if (giornoSett < 0) giornoSett = 0;
            if (giornoSett > 6) giornoSett = 6;
            
            String dateStr = giorniSettimana[giornoSett];
            dateStr += " ";
            if (myTZ.day() < 10) dateStr += "0";
            dateStr += String(myTZ.day());
            dateStr += "/";
            if (myTZ.month() < 10) dateStr += "0";
            dateStr += String(myTZ.month());
            dateStr += "/";
            dateStr += String(myTZ.year());
            parts[numParts++] = dateStr;
        }
    }

    // Aggiungi ora se abilitata / Add time if enabled
    if (scrollShowTime) {
        if (scrollTextualDateTime) {
            parts[numParts++] = ScrollText::buildTextualTime();
        } else {
            // Formato numerico / Numeric format
            char timeStr[6];
            snprintf(timeStr, sizeof(timeStr), "%02d:%02d", currentHour, currentMinute);
            parts[numParts++] = String(timeStr);
        }
    }

    // Aggiungi testo personalizzato / Add custom text
    if (scrollShowText && customScrollText.length() > 0) {
        parts[numParts++] = customScrollText;
    }

    // Costruisci il risultato unendo le parti con / Build result joining parts with
    String result = "";
    for (int i = 0; i < numParts; i++) {
        if (i > 0) result += " - ";
        result += parts[i];
    }

    if (result.length() == 0) result = " ";

    return result;
}

// Versione NON bloccante del testo scorrevole / NON-blocking version of scrolling text
void ScrollText::init() {
  // Usa ScrollText::buildDisplayText per includere data/ora se attivi / Use ScrollText::buildDisplayText to include date/time if active
  String t = ScrollText::buildDisplayText();
  if (t.length() == 0) t = " ";
  t.toUpperCase();
  scrollTextTotalWidth = t.length() * 6;
  scrollTextOffset = MATRIX_WIDTH;
  scrollTextInitialized = true;
}

void ScrollText::reset() { /* reset interno testo scorrevole, se necessario */ }

void ScrollText::update() {
    if (displayOff) {
        FastLED.clear();
        FastLED.show();
        return;
    }
   
    if (!scrollEnabled) {
        if (scrollTextInitialized) {
            scrollTextInitialized = false;
            scrollTextOffset = MATRIX_WIDTH;
        }
        return;
    }
    
    if (!scrollTextInitialized) {
        ScrollText::init();
    }
    
    // Se siamo in pausa tra i passaggi / If we are in pause between passes
    if (scrollPauseActive) {
        if (millis() - scrollPauseStart > (uint32_t)scrollPauseSeconds * 1000) {
            // Pausa finita / Pause finished
            scrollPauseActive = false;
            scrollTextOffset = MATRIX_WIDTH;
            scrollTextInitialized = false;
            lastPresetDuringPause = 255;
            pauseDelayActive = false;
        } else if (!pauseDelayActive) {
            // secondi / seconds
            if (millis() - scrollPauseStart < 2000) {
                FastLED.clear();
                FastLED.show();
                return;
            } else {
                pauseDelayActive = true;  // Dopo 2 sec, mostra preset
            }
        }
    return;
}

    if (millis() - lastScrollTextStep < scrollTextSpeedMs) return;
    lastScrollTextStep = millis();

    // Costruisci il testo completo con data/ora se abilitati / Build complete text with date/time if enabled
    String t = ScrollText::buildDisplayText();
    t.toUpperCase();

    // Parametri in base alla dimensione del font / Parameters based on font size
    int charWidth, charHeight, charSpacing, vertOffset;
    int scale = 1;

    switch (scrollFontSize) {
        case 1: // Piccolo 3x5
            charWidth = 3;
            charHeight = 5;
            charSpacing = 4;
            vertOffset = 5;
            break;
        case 3: // Grande
            charWidth = 5;
            charHeight = 7;
            charSpacing = 6;
            vertOffset = 1;
            scale = 2;
            break;
        default: // Medio 5x7
            charWidth = 5;
            charHeight = 7;
            charSpacing = 6;
            vertOffset = 4;
            break;
    }

    int effectiveCharSpacing = charSpacing * scale;
    scrollTextTotalWidth = t.length() * effectiveCharSpacing;

    FastLED.clear();
    int textLength = t.length();



    for (int charIndex = 0; charIndex < textLength; charIndex++) {
        char c = t.charAt(charIndex);
        int charX = scrollTextOffset + (charIndex * effectiveCharSpacing);

        CRGB charColor;
        if (scrollTextRainbow) {  // 
            uint8_t hue = (charIndex * 25) % 256;
            charColor = CHSV(hue, 255, 255);
        } else {
            charColor = scrollTextColor;  //  Usa colore specifico testo
        }

        if (scrollFontSize == 1) {
            uint8_t charBitmap[3];
            Canvas::getCharBitmapSmall(c, charBitmap);

            if (charX + charWidth >= 0 && charX < MATRIX_WIDTH) {
                for (int col = 0; col < charWidth; col++) {
                    int screenX = charX + col;
                    if (screenX >= 0 && screenX < MATRIX_WIDTH) {
                        for (int row = 0; row < charHeight; row++) {
                            if (charBitmap[col] & (1 << row)) {
                                int screenY = row + vertOffset;
                                if (screenY >= 0 && screenY < MATRIX_HEIGHT) {
                                    uint16_t ledPos = Canvas::xyToLED(screenX, screenY);
                                    if (ledPos < NUM_LEDS) leds[ledPos] = charColor;
                                }
                            }
                        }
                    }
                }
            }
        } else if (scrollFontSize == 3) {
            uint8_t charBitmap[5];
            Canvas::getCharBitmap(c, charBitmap);
            int effectiveCharWidth = charWidth * scale;

            if (charX + effectiveCharWidth >= 0 && charX < MATRIX_WIDTH) {
                for (int col = 0; col < charWidth; col++) {
                    for (int sx = 0; sx < scale; sx++) {
                        int screenX = charX + (col * scale) + sx;
                        if (screenX >= 0 && screenX < MATRIX_WIDTH) {
                            for (int row = 0; row < charHeight; row++) {
                                if (charBitmap[col] & (1 << row)) {
                                    for (int sy = 0; sy < scale; sy++) {
                                        int screenY = (row * scale) + sy + vertOffset;
                                        if (screenY >= 0 && screenY < MATRIX_HEIGHT) {
                                            uint16_t ledPos = Canvas::xyToLED(screenX, screenY);
                                            if (ledPos < NUM_LEDS) leds[ledPos] = charColor;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        } else {
            uint8_t charBitmap[5];
            Canvas::getCharBitmap(c, charBitmap);

            if (charX + charWidth >= 0 && charX < MATRIX_WIDTH) {
                for (int col = 0; col < charWidth; col++) {
                    int screenX = charX + col;
                    if (screenX >= 0 && screenX < MATRIX_WIDTH) {
                        for (int row = 0; row < charHeight; row++) {
                            if (charBitmap[col] & (1 << row)) {
                                int screenY = row + vertOffset;
                                if (screenY >= 0 && screenY < MATRIX_HEIGHT) {
                                    uint16_t ledPos = Canvas::xyToLED(screenX, screenY);
                                    if (ledPos < NUM_LEDS) leds[ledPos] = charColor;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    FastLED.show();
    scrollTextOffset--;
    
    if (scrollTextOffset < -scrollTextTotalWidth) {
        if (scrollPauseSeconds > 0) {
            scrollPauseActive = true;
            scrollPauseStart = millis();
        } else {
            scrollTextOffset = MATRIX_WIDTH;
            scrollTextInitialized = false;
        }
    }
}
