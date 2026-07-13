#pragma once

// Aggiornamento firmware dal server FreeClock (comando "ota_update" dall'app).
// Scarica da {config.quoteUrl}/api/fw/dl?device=oraquadra se la versione
// pubblicata differisce da FIRMWARE_CODE. Riavvia da solo se aggiorna.
void checkAndUpdateFirmware();
