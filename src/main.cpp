/*
Description: Starts the reader, initializes the hardware, and launches the main app flow.
Inputs: config.h values like SERIAL_BAUD_RATE, storageCtrl, displayCtrl, inputCtrl, bookParser, and the app objects in AppManager.h.
Process: Mounts storage, sets up the display and buttons, builds SystemContext, allocates app screens, and switches to the library menu.
Output: The device boots into the library UI and waits for user input.
*/

#include <Arduino.h>

// --- Core & Config ---
#include "config.h"
#include "SystemContext.h"
#include "AppManager.h"

// --- Hardware Abstraction Layer ---
#include "DisplayController.h"
#include "StorageManager.h"
#include "InputController.h"
#include "BookParser.h"
// #include "PowerController.h" // Hardware is not yet implemented, so this is commented out for now

// --- Application Layer ---
#include "Apps/LibraryMenuApp.h"
#include "Apps/ReaderApp.h"
#include "Apps/SleepApp.h"
#include "Apps/TransferApp.h"
#include "Apps/SettingsApp.h"

// Global system context and hardware instances
DisplayController displayCtrl;
StorageManager storageCtrl;
InputController inputCtrl;
BookParser bookParser;
AppManager appManager;

// PowerController powerCtrl;
SystemContext systemCtx;

// Extern
String globalTargetFile = "";
LibraryMenuApp* libraryApp = nullptr;
ReaderApp* readerApp = nullptr;
SleepApp* sleepApp = nullptr;
TransferApp* transferApp = nullptr;
SettingsApp* settingsApp = nullptr;

// Main setup
void setup() {
    Serial.begin(SERIAL_BAUD_RATE);

    if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_EXT0) {
        
        pinMode(BTN_4, INPUT_PULLUP);
        delay(WAKE_BUTTON_STABILIZE_MS); // Let voltage stabilize
        
        unsigned long wakeStart = millis();
        bool heldLongEnough = true;
        
        while (millis() - wakeStart < WAKE_BUTTON_HOLD_CONFIRM_MS) {
            if (digitalRead(BTN_4) == HIGH) { 
                heldLongEnough = false;
                break;
            }
            delay(BUTTON_POLL_INTERVAL_MS);
        }
        
        if (!heldLongEnough) {
            esp_sleep_enable_ext0_wakeup((gpio_num_t)BTN_4, 0);
            esp_deep_sleep_start();
        }
    }
    
    pinMode(BTN_1, INPUT_PULLUP);
    pinMode(BTN_2, INPUT_PULLUP);
    pinMode(BTN_3, INPUT_PULLUP);
    pinMode(BTN_4, INPUT_PULLUP);
    while(digitalRead(BTN_1) == LOW || digitalRead(BTN_2) == LOW || 
          digitalRead(BTN_3) == LOW || digitalRead(BTN_4) == LOW) {
        delay(BUTTON_POLL_INTERVAL_MS); // Trap execution here until hands are off
    }

    delay(STARTUP_SERIAL_MONITOR_DELAY_MS); // Brief pause for serial monitor connection

    // --- Initialize Storage ---
    if (!storageCtrl.begin()) {
        Serial.println("StorageManager failed to initialize.");
    }
    systemCtx.inactivitySleepTimeoutMs =
        storageCtrl.loadInactivitySleepTimeout(INACTIVITY_SLEEP_MS);
    systemCtx.readerAutoSavePageThreshold =
        storageCtrl.loadReaderAutoSavePageThreshold(READER_AUTO_SAVE_PAGE_THRESHOLD);

    // --- Initialize Hardware ---
    displayCtrl.begin();
    inputCtrl.begin();

    // --- Build System Context ---
    systemCtx.display = &displayCtrl;
    systemCtx.storage = &storageCtrl;
    systemCtx.input   = &inputCtrl;
    systemCtx.parser  = &bookParser;
    systemCtx.appManager = &appManager;
    systemCtx.power   = nullptr; // Set to &powerCtrl when implemented

    // --- Allocate Applications ---
    libraryApp = new LibraryMenuApp(&systemCtx);
    readerApp  = new ReaderApp(&systemCtx);
    sleepApp   = new SleepApp(&systemCtx);
    transferApp = new TransferApp(&systemCtx);
    settingsApp = new SettingsApp(&systemCtx);
    
    // --- Initialize App Manager ---
    appManager.setContext(&systemCtx);
    appManager.setSleepApp(sleepApp);
    systemCtx.resetActivityTimer();

    // --- Bind Hardware Events to Application State ---
    inputCtrl.setEventCallback([](int btnId, int eventType) {
        appManager.handleButtonEvent(btnId, eventType);
    });
    
    // --- Boot the System ---
    appManager.switchApp(libraryApp);
}

// Main loop
void loop() {
    // 1. Tick the hardware button states
    inputCtrl.update();

    // 2. Tick the active application logic (if any)
    appManager.updateLoop();

    // Small yield to feed the ESP32 watchdog timer
    delay(MAIN_LOOP_DELAY_MS);
}