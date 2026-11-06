/*
Description: Declares the E-Ink display controller used for drawing text, menus, and book pages.
Inputs: DISPLAY_WIDTH, DISPLAY_HEIGHT, FONT_CHAR_WIDTH, MARGIN_X, MARGIN_Y, and the EPD pins from config.h.
Process: Exposes drawing and refresh methods so app screens can paint to the display safely.
Output: A display object with page updates, text drawing, bitmap handling, and size queries.
*/

#pragma once
#include "config.h"
#include <GxEPD2_BW.h>

class DisplayController {
private:
    void configureSPI();

public:
    DisplayController();
    
    void begin();
    void clear();
    void printText(const String& text);
    void printTextFast(const String& text);
    
    // Future power management hook
    // void drawBatteryIcon(int percentage); 

    uint32_t printPageDynamic(const String& text, bool fastRefresh);

    // Generic Drawing API for Apps
    void startFullUpdate();
    void startPartialUpdate();
    bool nextPage();
    void fillWhite();
    void setMenuFont();
    void drawString(int x, int y, const String& text);

    void drawBitmap(int x, int y, const uint8_t *bitmap, int w, int h);
    int getWidth();
    int getHeight();

    bool isBusy();
};