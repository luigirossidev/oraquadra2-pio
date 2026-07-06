#pragma once

#include <Arduino.h>

#include "effects/effect_fade.h"
#include "effects/effect_slow.h"
#include "effects/effect_matrix.h"
#include "effects/effect_matrix2.h"
#include "effects/effect_tron.h"
#include "effects/effect_drop.h"
#include "effects/effect_rainbow.h"
#include "effects/effect_moto.h"
#include "effects/effect_galaga.h"
#include "effects/effect_pacman.h"
#include "effects/effect_digitale.h"

// Coordinamento effetti: dispatch della modalità corrente e reset al cambio minuto
class Effects {
public:
    static void update(uint32_t currentMillis);
    static void onMinuteChange();
};
