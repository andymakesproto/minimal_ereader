#include "AppManager.h"

void AppManager::switchApp(BaseApp* newApp) {
    if (currentApp != nullptr) {
        currentApp->onExit(); // Cleanly shut down the old app
    }
    
    currentApp = newApp;
    
    if (currentApp != nullptr) {
        currentApp->onStart(); // Boot up the new app
    }
}

void AppManager::updateLoop() {
    if (currentApp != nullptr) {
        currentApp->onLoop();
    }
}

void AppManager::handleButtonEvent(int btnId, int eventType) {
    if (currentApp != nullptr) {
        currentApp->onButtonEvent(btnId, eventType);
    }
}

void AppManager::onLoop() {
    if (currentApp != nullptr) {
        currentApp->onLoop();
    }
}