#include "time_utils.h"
#include "state.h"

// Funzione per verificare se siamo nel periodo di modalità serale / Function to check if we are in evening mode period
bool TimeUtils::isNightTime() {
    uint32_t currentTimeMinutes = currentHour * 60 + currentMinute;
    uint32_t dayStartMinutes = dayStartHour * 60 + dayStartMinute;
    uint32_t nightStartMinutes = nightStartHour * 60 + nightStartMinute;
    
    if (dayStartMinutes > nightStartMinutes) {
        return (currentTimeMinutes >= nightStartMinutes) && (currentTimeMinutes < dayStartMinutes);
    } else {
        return (currentTimeMinutes >= nightStartMinutes) || (currentTimeMinutes < dayStartMinutes);
    }
}

// Funzione per verificare se siamo nel periodo di spegnimento notturno / Function to check if we are in night shutdown period
bool TimeUtils::isSleepTime() {
    if (!sleepModeEnabled) return false;
    
    uint32_t currentTimeMinutes = currentHour * 60 + currentMinute;
    uint32_t sleepStartMinutes = sleepStartHour * 60 + sleepStartMinute;
    uint32_t dayStartMinutes = dayStartHour * 60 + dayStartMinute;
    
    if (dayStartMinutes > sleepStartMinutes) {
        return (currentTimeMinutes >= sleepStartMinutes) && (currentTimeMinutes < dayStartMinutes);
    } else {
        return (currentTimeMinutes >= sleepStartMinutes) || (currentTimeMinutes < dayStartMinutes);
    }
}

void TimeUtils::updateCurrentTimeFromTZ() {
    // Assicurati che myTZ sia già sincronizzato con NTP in setup()
    currentHour   = myTZ.hour();    // ora locale con fuso e ora legale
    currentMinute = myTZ.minute();
    currentSecond = myTZ.second();
}
