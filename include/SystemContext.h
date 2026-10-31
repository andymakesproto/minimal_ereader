#pragma once

// Forward declarations of HAL engines and App routing
class DisplayController;
class StorageManager;
class InputController;
class BookParser;
class PowerController;
class AppManager; // NEW: Added AppManager forward declaration

struct SystemContext {
    DisplayController* display;
    StorageManager* storage;
    InputController* input;
    BookParser* parser;
    PowerController* power;
    AppManager* appManager; // NEW: Pointer to allow apps to switch states
};