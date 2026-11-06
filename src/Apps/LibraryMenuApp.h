/*
Description: Declares the library screen that lists books and special menu items.
Inputs: SystemContext* ctx, fileList, selectedIndex, scrollOffset, and LIBRARY_MENU_MAX_VISIBLE_ITEMS.
Process: Stores the menu state and exposes the lifecycle methods used to draw, scroll, and select items.
Output: A menu app that opens books, settings, or Wi-Fi transfer.
*/

#pragma once
#include "AppManager.h" // Contains BaseApp
#include "config.h"
#include <vector>
#include <Arduino.h>

class LibraryMenuApp : public BaseApp {
private:
    std::vector<String> fileList;
    int selectedIndex;
    int scrollOffset;
    const int maxVisibleItems = LIBRARY_MENU_MAX_VISIBLE_ITEMS;
    uint32_t lastDrawTime;
    void scanFiles();
    void drawMenu();

public:
    LibraryMenuApp(SystemContext* context);
    
    void onStart() override;
    void onLoop() override;
    void onButtonEvent(int btnId, int eventType) override;
    void onExit() override;
    void forceRedraw() override{
        onStart();
    }
};