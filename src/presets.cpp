#include "presets.h"
#include <EEPROM.h>
#include <FastLED.h>
#include "eeprom_map.h"
#include "config.h"
#include "state.h"
#include "clock_colors.h"
#include "settings.h"
#include "clock_display.h"
#include "effects/effect_fade.h"
#include "effects/effect_slow.h"
#include "effects/effect_matrix2.h"
#include "effects/effect_tron.h"
#include "effects/effect_drop.h"
#include "effects/effect_moto.h"
#include "effects/effect_galaga.h"
#include "effects/effect_pacman.h"

void Presets::apply(uint8_t preset) {
   // preset / preset
   digitalOverlayActive = false;
   waitingForDigital = false;
   lastEffectChangeTime = millis();

   // Reset stato effetti per ripartire da capo
   slowState.active = false;
   slowState.completed = false;
   fadeState.active = false;
   fadeState.completed = false;

   // Di default, l'overlay digitale automatico può funzionare
   // Verrà impostato a true solo per DIGITALE e PACMAN
   manualModeSelected = false;

   switch(preset) {
       case 0:
           currentMode = MODE_SLOW;
           randomColor = true;
           clockColors.current = CRGB(random8(), random8(), random8());
           slowState.active = false;
           slowState.completed = false;  // Parte subito
           break;

       case 1:
           currentMode = MODE_FADE;
           clockColors.current = CRGB::Blue;
           fadeState.active = false;
           fadeState.completed = false;  // Parte subito
           break;

       case 2:
           currentMode = MODE_MATRIX2;
           matrixWordColor = CRGB::Green;
           matrix2State.needsReset = true;
           matrix2State.completed = false;  // Parte subito
           break;

       case 3:
           currentMode = MODE_SLOW;
           randomColor = false;
           clockColors.current = CRGB(255,165,0); // slow arancione
           slowState.active = false;
           slowState.completed = false;  // Parte subito
           break;

       case 4:
           currentMode = MODE_MATRIX2;
           matrixWordColor = CRGB::Blue;
           matrix2State.needsReset = true;
           matrix2State.completed = false;  // Parte subito
           break;
           
       case 5:
           currentMode = MODE_MATRIX;
           matrixWordColor = CRGB::Yellow;
           break;
           
       case 6:
           currentMode = MODE_FADE;
           clockColors.current = CRGB::Green;
           fadeState.active = false;
           fadeState.completed = false;  // Parte subito
           break;

       case 7:
           currentMode = MODE_SLOW;
           randomColor = false;
           clockColors.current = CRGB::Blue; // slow blu
           slowState.active = false;
           slowState.completed = false;  // Parte subito
           break;
           
       case 8:
           currentMode = MODE_MATRIX;
           matrixWordColor = CRGB::Cyan;
           break;
           
       case 9:
           currentMode = MODE_FADE;
           clockColors.current = CRGB(255,20,147);
           fadeState.active = false;
           fadeState.completed = false;  // Parte subito
           break;
           
       case 10:
           currentMode = MODE_FAST;
           clockColors.current = CRGB::Yellow;
           break;
           
       case 11:
           currentMode = MODE_FAST;
           clockColors.current = CRGB::White;
           break;
           
       case 12:
           currentMode = MODE_FAST;
           clockColors.current = CRGB(0,255,255);
           break;
           
       case 13:
           currentMode = MODE_TRON;
           tronNeedsInit = true;  // Parte subito
           tronEffectActive = true;
           tronCycleStart = 0;
           memset(tronIntensity, 0, sizeof(tronIntensity));
           break;

       case 14:
           currentMode = MODE_DROP;
           dropState.active = false;
           dropState.completed = false;  // Parte subito
           break;

       case 15:
           currentMode = MODE_MOTO;
           MotoEffect::init();  // Parte subito
           break;

       case 16:
           currentMode = MODE_GALAGA;
           galagaTextHue = random8();
           GalagaEffect::init();  // Parte subito
           break;

       case 17:
           currentMode = MODE_PACMAN;
           manualModeSelected = false;  // Permetti overlay digitale
           PacmanEffect::init();
           break;

       case 18:
           currentMode = MODE_DIGITALE;
           manualModeSelected = true;  // Non interrompere con overlay
           lastDigitaleUpdate = 0;
           break;

       default:
           currentMode = MODE_SLOW;
           randomColor = true;
           clockColors.current = CRGB(random8(), random8(), random8());
           slowState.active = false;
           slowState.completed = false;  // Parte subito
           break;
   }
   
   FastLED.setBrightness(intBrightness);  // Mantiene la luminosità corretta
   ClockDisplay::updateDisplay();
   
   uint16_t baseAddr = EEPROM_MODE_SETTINGS_START + (preset * EEPROM_MODE_SETTINGS_SIZE);
   uint8_t marker = EEPROM.read(baseAddr);
    
   if (marker != EEPROM_MODE_MARKER) {
       // preset / preset
       Settings::saveModeSettings(preset);
       Serial.print("Impostazioni auto-salvate per preset: ");
       Serial.println(preset);
   }

}
