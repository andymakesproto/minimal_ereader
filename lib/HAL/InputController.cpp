/*
Description: Watches the four physical buttons and reports button presses to the app layer.
Inputs: BTN_1 to BTN_4, OneButton callbacks, and the event callback set by AppManager.cpp.
Process: Calls btn1.tick() through btn4.tick(), then forwards click and long-press events as logical app actions.
Output: AppManager.cpp receives button events like btnId 1-4 and eventType EVENT_CLICK or EVENT_LONG_PRESS.
*/

#include "InputController.h"

// Store a global instance pointer so the static OneButton callbacks can reach this class
static InputController* instance = nullptr;

InputController::InputController() 
    // Format: (PIN, activeLow, enableInternalPullUp)
    : btn1(BTN_1, true, true), 
      btn2(BTN_2, true, true),
      btn3(BTN_3, true, true),
      btn4(BTN_4, true, true) {
    instance = this;
}

void InputController::begin() {

    // Attach standard clicks and pass the logical button ID (1, 2, 3, or 4)
    btn1.attachClick(handleClick, (void*)1);
    btn2.attachClick(handleClick, (void*)2);
    btn3.attachClick(handleClick, (void*)3);
    btn4.attachClick(handleClick, (void*)4);

    // Attach long press events and pass the logical button ID (1, 2, 3, or 4)
    btn1.attachLongPressStart(handleLongPress, (void*)1);
    btn2.attachLongPressStart(handleLongPress, (void*)2);
    btn3.attachLongPressStart(handleLongPress, (void*)3);
    btn4.attachLongPressStart(handleLongPress, (void*)4);
}

void InputController::update() {
    // This must be called continuously in the main loop
    btn1.tick();
    btn2.tick();
    btn3.tick();
    btn4.tick();
}

void InputController::setEventCallback(std::function<void(int, int)> cb) {
    eventCallback = cb;
}

void InputController::triggerEvent(int btnId, int eventType) {
    if (eventCallback) {
        eventCallback(btnId, eventType); // Fire the event to the AppManager
    }
}

void InputController::handleClick(void* btnId) {
    if (instance) {
        instance->triggerEvent((int)(uintptr_t)btnId, EVENT_CLICK);
    }
}

void InputController::handleLongPress(void* btnId) {
    if (instance) {
        instance->triggerEvent((int)(uintptr_t)btnId, EVENT_LONG_PRESS); 
    }
}