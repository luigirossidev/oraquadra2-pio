#pragma once

// Funzioni di gestione orario e fasce giorno/notte
class TimeUtils {
public:
    static bool isNightTime();
    static bool isSleepTime();
    static void updateCurrentTimeFromTZ();
};
