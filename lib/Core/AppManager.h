/*
Description: Declares the app manager that switches screens and controls idle-time behavior.
Inputs: BaseApp pointers like currentApp and sleepApp, plus SystemContext* ctx and button events from AppManager.cpp.
Process: Provides the interface used by app code to swap screens, check idle states, and register the sleep screen.
Output: A central app controller that drives the active screen and sleep logic.
*/

#pragma once
#include "BaseApp.h"

class AppManager {
private:
    BaseApp* currentApp = nullptr;
    SystemContext* ctx = nullptr;
    BaseApp* sleepApp = nullptr;

    bool warningShown = false; // Track if the inactivity warning has been displayed

public:
    void setContext(SystemContext* context);
    void switchApp(BaseApp* newApp);
    void updateLoop();
    void handleButtonEvent(int btnId, int eventType);
    void onLoop();
    void setSleepApp(BaseApp* app) { sleepApp = app; }
};