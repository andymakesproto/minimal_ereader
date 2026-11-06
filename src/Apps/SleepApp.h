/*
Description: Declares the sleep/idle screen shown when the device is resting or waking up.
Inputs: customMsg, lastWakeAttempt, and SystemContext* ctx from the app manager.
Process: Stores a wake message and provides the screen lifecycle methods used by SleepApp.cpp.
Output: A sleep app that shows a calm screen and returns the user to the library.
*/

#pragma once
#include "BaseApp.h"
#include "config.h"

class SleepApp : public BaseApp {
private:
    uint32_t lastWakeAttempt;
    String customMsg = "";

public:
    SleepApp(SystemContext* context);

    void setCustomMessage(String msg) override;
    void onStart() override;
    void onLoop() override;
    void onButtonEvent(int btnId, int eventType) override;
    void onExit() override;
    void forceRedraw() override {
        onStart();
    }
};