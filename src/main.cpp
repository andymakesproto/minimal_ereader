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
// #include "PowerController.h" // Uncomment when implemented

// --- Application Layer ---
#include "Apps/LibraryMenuApp.h"
#include "Apps/ReaderApp.h"
#include "Apps/SleepApp.h"
#include "Apps/TransferApp.h"

// ==========================================
// 1. GLOBAL INSTANCES (Hardware & Core)
// ==========================================
DisplayController displayCtrl;
StorageManager storageCtrl;
InputController inputCtrl;
BookParser bookParser;
AppManager appManager;

// PowerController powerCtrl;

SystemContext systemCtx;

// ==========================================
// 2. EXTERN VARIABLES (For App Routing)
// ==========================================
String globalTargetFile = "";
LibraryMenuApp* libraryApp = nullptr;
ReaderApp* readerApp = nullptr;
SleepApp* sleepApp = nullptr;
TransferApp* transferApp = nullptr;
// ==========================================
// 3. MAIN SETUP
// ==========================================
void setup() {
    Serial.begin(115200);
    delay(1000); // Brief pause for serial monitor connection

    // --- Initialize Storage ---
    if (!storageCtrl.begin()) {
        Serial.println("CRITICAL ERROR: StorageManager failed to initialize.");
        // We do not halt execution here, but files won't load.
    }

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

    // --- Bind Hardware Events to Application State ---
    // This lambda cleanly routes OneButton ticks to the active UI state
    inputCtrl.setEventCallback([](int btnId, int eventType) {
        appManager.handleButtonEvent(btnId, eventType);
    });

    // --- Allocate Applications ---
    libraryApp = new LibraryMenuApp(&systemCtx);
    readerApp  = new ReaderApp(&systemCtx);
    sleepApp   = new SleepApp(&systemCtx);
    transferApp = new TransferApp(&systemCtx);
    
    // --- Boot the System ---
    appManager.switchApp(sleepApp);

    // --- Bind Hardware Events to Application State ---
    inputCtrl.setEventCallback([](int btnId, int eventType) {
        // ADD THIS LINE TO DEBUG:
        Serial.printf("Hardware fired! Button: %d, EventType: %d\n", btnId, eventType);
        
        appManager.handleButtonEvent(btnId, eventType);
    });
}

// ==========================================
// 4. MAIN LOOP
// ==========================================
void loop() {
    // 1. Tick the hardware button states
    inputCtrl.update();

    // 2. Tick the active application logic (if any)
    appManager.updateLoop();

    systemCtx.appManager->onLoop();

    // Small yield to feed the ESP32 watchdog timer
    delay(10);
}