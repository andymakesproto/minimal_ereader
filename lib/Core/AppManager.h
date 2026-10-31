#pragma once
#include "BaseApp.h"

class AppManager {
private:
    BaseApp* currentApp = nullptr;

public:
    void switchApp(BaseApp* newApp);
    void updateLoop();
    void handleButtonEvent(int btnId, int eventType);
    void onLoop(); // New method to call the current app's onLoop
};