/*
Description: Manages app switching and the global idle-time warning/sleep flow.
Inputs: AppManager.h state, SystemContext values like lastActivityTime and inactivitySleepTimeoutMs, button events, and the current/sleep app pointers.
Process: Switches currentApp, checks idle time, shows the warning screen, and calls esp_deep_sleep_start() when the timeout is reached.
Output: The active screen changes, the warning UI is shown, or the device sleeps safely.
*/

#include "config.h"
#include "AppManager.h"
#include "DisplayController.h"
#include <esp_sleep.h>
#include <driver/rtc_io.h> //For sleep pull-ups
#include <esp_timer.h>     

void AppManager::setContext(SystemContext* context) {
    ctx = context;
}

void AppManager::switchApp(BaseApp* newApp) {
    if (currentApp != nullptr) {
        currentApp->onExit();
    }
    
    currentApp = newApp;
    
    if (currentApp != nullptr) {
        currentApp->onStart();
    }
}

void AppManager::updateLoop() {
    if (ctx != nullptr) {
        uint32_t idleTime = (uint32_t)(esp_timer_get_time() / 1000ULL) - ctx->lastActivityTime;
        uint32_t warningTime = ctx->inactivityWarningTimeoutMs();

        // 1. Trigger Deep Sleep
        if (idleTime >= ctx->inactivitySleepTimeoutMs) {
            Serial.println("Global Timeout: Entering Deep Sleep.");
            warningShown = false; 

            if (currentApp != nullptr) {
                currentApp->prepareForSleep();
            }

            ctx->display->clear();

            if (sleepApp != nullptr) {
                sleepApp->setCustomMessage("Going to Sleep...");
                if (currentApp != sleepApp) {
                    switchApp(sleepApp);
                }
                sleepApp->forceRedraw(); 
            } else {
                Serial.println("WARNING: sleepApp pointer is null! Falling back to blank screen.");
            }

            rtc_gpio_pullup_en((gpio_num_t)BTN_4); 
            esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);
            esp_sleep_enable_ext0_wakeup((gpio_num_t)BTN_4, 0); 
            
            Serial.flush();
            esp_deep_sleep_start(); 
        }
        
        // 2. Warning Screen
        else if (idleTime >= warningTime && !warningShown) {
            Serial.println("Global Timeout: Warning.");
            warningShown = true;
            
            int w = ctx->display->getWidth();
            int h = ctx->display->getHeight();
            int charW = FONT_CHAR_WIDTH;
            uint32_t warningLeadMs = ctx->inactivitySleepTimeoutMs - warningTime;
            String sleepMessage = warningLeadMs < 60000
                ? "Sleep in " + String((warningLeadMs + 999) / 1000) + " sec"
                : "Sleep in " + String((warningLeadMs + 59999) / 60000) + " min";
            
            ctx->display->startFullUpdate(); 
            do {
                ctx->display->fillWhite();
                ctx->display->setMenuFont();
 
                ctx->display->drawString((w - (sleepMessage.length() * charW)) / 2, (h / 2) - 30, sleepMessage);
                ctx->display->drawString((w - (13 * charW)) / 2, (h / 2),      "Press any key");
                ctx->display->drawString((w - (9 * charW)) / 2,  (h / 2) + 30, "to cancel");
            } while (ctx->display->nextPage());
        }
    }

    if (currentApp != nullptr) {
        currentApp->onLoop();
    }
}

void AppManager::handleButtonEvent(int btnId, int eventType) {
    if (ctx != nullptr) {
        ctx->resetActivityTimer(); // Reset the clock
        
        if (warningShown) {
            warningShown = false;
            ctx->display->clear(); // Physically scrub the E-Ink buffer
            
            if (currentApp != nullptr) {
                currentApp->forceRedraw(); // Redraw the UI
            }
            return; 
        }
    }

    // Only process normal clicks if the warning was NOT on screen
    if (currentApp != nullptr) {
        currentApp->onButtonEvent(btnId, eventType);
    }
}

void AppManager::onLoop() {
    if (currentApp != nullptr) {
        currentApp->onLoop();
    }
}