#pragma once
#include <Arduino.h>

// Hardware Pins
#define BTN_1      D0
#define BTN_2      D1  
#define BTN_3      D2  
#define BTN_4      D3  
#define EPD_CS     D4  
#define BAT_ADC    D5  
#define EPD_DC     D6  
#define EPD_BUSY   D7  
#define EPD_SCK    D8  
#define EPD_RST    D9  
#define EPD_MOSI   D10 

// Display Config
#define DISPLAY_WIDTH 280
#define DISPLAY_HEIGHT 416
#define FONT_CHAR_WIDTH 12
#define FONT_LINE_HEIGHT 20
#define MARGIN_X 8
#define MARGIN_Y 8

// Input Config
#define EVENT_CLICK 0
#define EVENT_LONG_PRESS 1

// Power Config
#define ENABLE_BATTERY_MANAGEMENT 0