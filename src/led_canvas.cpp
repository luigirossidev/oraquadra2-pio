#include "led_canvas.h"
#include "fonts.h"
#include "state.h"

// Funzione per ottenere un carattere dal font piccolo 3x5 / Function to get a character from small 3x5 font
void Canvas::getCharBitmapSmall(char c, uint8_t* bitmap) {
  if (c >= 32 && c <= 90) {
    for (int i = 0; i < 3; i++) {
      bitmap[i] = pgm_read_byte(&font3x5[c - 32][i]);
    }
  } else {
    for (int i = 0; i < 3; i++) {
      bitmap[i] = 0x00;
    }
  }
}

// Funzione per ottenere un carattere dal font medio 5x7 / Function to get a character from medium 5x7 font
void Canvas::getCharBitmap(char c, uint8_t* bitmap) {
  if (c >= 32 && c <= 90) {
    for (int i = 0; i < 5; i++) {
      bitmap[i] = pgm_read_byte(&font5x7[c - 32][i]);
    }
  } else {
    // Carattere non supportato - spazio vuoto / Unsupported character - empty space
    for (int i = 0; i < 5; i++) {
      bitmap[i] = 0x00;
    }
  }
}

// Funzione per disegnare un carattere sulla matrice in una posizione specifica / Function to draw a character on matrix at specific position
void Canvas::drawChar5x7(uint8_t x, uint8_t y, char c, CRGB color) {
  uint8_t charBitmap[5];
  Canvas::getCharBitmap(c, charBitmap);
  
  for (int col = 0; col < 5; col++) {
    int screenX = x + col;
    if (screenX >= 0 && screenX < MATRIX_WIDTH) {
      for (int row = 0; row < 7; row++) {
        if (charBitmap[col] & (1 << row)) {
          int screenY = y + row;
          if (screenY >= 0 && screenY < MATRIX_HEIGHT) {
            uint16_t ledPos = Canvas::xyToLED(screenX, screenY);
            if (ledPos < NUM_LEDS) {
              leds[ledPos] = color;
            }
          }
        }
      }
    }
  }
}

// Funzione per disegnare una cifra centrata in una area / Function to draw a centered digit in an area
void Canvas::drawDigitCentered(uint8_t x, uint8_t y, uint8_t digit, CRGB color) {
  char digitChar = '0' + digit;
  Canvas::drawChar5x7(x, y, digitChar, color);
}

// Funzione per disegnare i due punti tra ore e minuti / Function to draw colon between hours and minutes
void Canvas::drawColon(uint8_t x, uint8_t y, CRGB color, bool blink) {
  // I due punti sono composti da 2 pixel / Colon is composed of 2 pixels
  if (!blink || (millis() / 500) % 2 == 0) {
    uint16_t topPos = Canvas::xyToLED(x, y + 2);
    uint16_t bottomPos = Canvas::xyToLED(x, y + 4);
    if (topPos < NUM_LEDS) leds[topPos] = color;
    if (bottomPos < NUM_LEDS) leds[bottomPos] = color;
  }
}

// Funzione per convertire un pixel / Function to convert a pixel
uint16_t Canvas::xyToLED(uint8_t x, uint8_t y) {
  // La matrice è mappata in serpentina / Matrix is mapped in serpentine
  if (y % 2 == 0) {
    // Riga pari: da destra a sinistra / Even row: right to left
    return y * MATRIX_WIDTH + (MATRIX_WIDTH - 1 - x);
  } else {
    // Riga dispari: da sinistra a destra / Odd row: left to right
    return y * MATRIX_WIDTH + x;
  }
}
