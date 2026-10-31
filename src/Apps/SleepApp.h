#pragma once
#include "BaseApp.h"
#include "config.h"

class SleepApp : public BaseApp {
private:
    uint32_t lastWakeAttempt;

public:
    SleepApp(SystemContext* context);
    
    void onStart() override;
    void onLoop() override;
    void onButtonEvent(int btnId, int eventType) override;
    void onExit() override;
};