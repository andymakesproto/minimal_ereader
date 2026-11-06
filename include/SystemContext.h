/*
Description: Holds the shared app state and references to the main device systems.
Inputs: Display, storage, input, parser, power, and app manager objects.
Process: Tracks sleep timing, auto-save settings, and activity so the app can decide when to warn or sleep.
Output: One shared context used by the rest of the app.
*/

#pragma once
#include <Arduino.h>
#include <esp_timer.h>
#include "config.h"

// Forward declarations of HAL engines and App routing
class DisplayController;
class StorageManager;
class InputController;
class BookParser;
class PowerController;
class AppManager; 

struct SystemContext {
    DisplayController* display;
    StorageManager* storage;
    InputController* input;
    BookParser* parser;
    PowerController* power;
    AppManager* appManager; 

    uint32_t inactivitySleepTimeoutMs = INACTIVITY_SLEEP_MS;
    uint32_t readerAutoSavePageThreshold = READER_AUTO_SAVE_PAGE_THRESHOLD;
    uint32_t lastActivityTime = 0;

    uint32_t inactivityWarningTimeoutMs() const {
        uint32_t warningLeadMs = inactivitySleepTimeoutMs < INACTIVITY_WARNING_LEAD_MS
            ? inactivitySleepTimeoutMs / 2
            : INACTIVITY_WARNING_LEAD_MS;
        return inactivitySleepTimeoutMs - warningLeadMs;
    }

    void resetActivityTimer() {
        lastActivityTime = (uint32_t)(esp_timer_get_time() / 1000ULL);
    }
};