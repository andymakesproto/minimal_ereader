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