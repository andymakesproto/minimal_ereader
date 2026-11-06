/*
Description: Defines the shared base class for every app screen in the reader.
Inputs: SystemContext* ctx and the app lifecycle methods from each screen.
Process: Gives each app a common startup, loop, button, exit, and redraw interface.
Output: A consistent app structure so AppManager.cpp can switch between screens safely.
*/

#pragma once
#include "SystemContext.h"

class BaseApp {
protected:
    SystemContext* ctx;

public:
    BaseApp(SystemContext* context) : ctx(context) {}
    virtual ~BaseApp() = default;

    // The required lifecycle methods
    virtual void onStart() = 0;
    virtual void onLoop() = 0;
    virtual void onButtonEvent(int btnId, int eventType) = 0; 
    virtual void onExit() = 0;
    virtual void prepareForSleep() {} 
    virtual void forceRedraw() {}
    virtual void setCustomMessage(String msg) {}
};