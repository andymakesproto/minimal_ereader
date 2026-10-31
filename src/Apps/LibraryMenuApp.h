#pragma once
#include "AppManager.h" // Contains BaseApp
#include <vector>
#include <Arduino.h>

class LibraryMenuApp : public BaseApp {
private:
    std::vector<String> fileList;
    int selectedIndex;
    int scrollOffset;
    const int maxVisibleItems = 8; // Tweak based on your font size
    uint32_t lastDrawTime;
    void scanFiles();
    void drawMenu();

public:
    LibraryMenuApp(SystemContext* context);
    
    void onStart() override;
    void onLoop() override;
    void onButtonEvent(int btnId, int eventType) override;
    void onExit() override;
};