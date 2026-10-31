/**
 * Serves as the blueprint for all UI states (e.g., ReaderApp, LibraryMenuApp).
 * Enforces standardized execution callbacks (onStart, onLoop, onButtonEvent, onExit)
 * and grants derived apps access to shared hardware peripherals via SystemContext.
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
};