#pragma once
#include "config.h"
#include <GxEPD2_BW.h>

class DisplayController {
private:
    // We will instantiate the physical display object in the .cpp file
    void configureSPI();

public:
    DisplayController();
    
    void begin();
    void clear();
    void printText(const String& text);
    void printTextFast(const String& text);
    
    // Future power management hook
    void drawBatteryIcon(int percentage); 

    uint32_t printPageDynamic(const String& text, bool fastRefresh);

    // --- Generic Drawing API for Apps ---
    void startFullUpdate();
    void startPartialUpdate();
    bool nextPage();
    void fillWhite();
    void setMenuFont();
    void drawString(int x, int y, const String& text);

    void drawBitmap(int x, int y, const uint8_t *bitmap, int w, int h);
    int getWidth();
    int getHeight();
};