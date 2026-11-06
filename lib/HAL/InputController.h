/*
Description: Declares the button input layer that translates hardware presses into app events.
Inputs: BTN_1 through BTN_4 from config.h and the event callback from AppManager.cpp.
Process: Wraps the OneButton library and forwards click and long-press actions as logical app events.
Output: triggerEvent() calls with button IDs and event types like EVENT_CLICK or EVENT_LONG_PRESS.
*/

#pragma once
#include "config.h"
#include <OneButton.h>
#include <functional>

class InputController {
private:
    OneButton btn1;
    OneButton btn2;
    OneButton btn3;
    OneButton btn4;
    
    // The function pointer that will route events back to main/AppManager
    std::function<void(int, int)> eventCallback = nullptr;

    // Static wrapper functions required by the OneButton library
    static void handleClick(void* btnId);
    static void handleLongPress(void* btnId);

public:
    InputController();
    void begin();
    void update();
    
    // Hook to attach the AppManager callback
    void setEventCallback(std::function<void(int, int)> cb);
    
    // Internal router to fire the callback
    void triggerEvent(int btnId, int eventType);
};