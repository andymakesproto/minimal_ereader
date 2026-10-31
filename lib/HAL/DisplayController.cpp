#include "DisplayController.h"
#include <SPI.h>
#include <vector>

// 1. Include BOTH the regular and oblique (italic) versions of the font
#include <Fonts/FreeMono9pt7b.h>
#include <Fonts/FreeMonoOblique9pt7b.h> 

GxEPD2_BW<GxEPD2_370_GDEY037T03, GxEPD2_370_GDEY037T03::HEIGHT> epd(
    GxEPD2_370_GDEY037T03(EPD_CS, EPD_DC, EPD_RST, EPD_BUSY)
);

DisplayController::DisplayController() {}

void DisplayController::configureSPI() {
    SPI.begin(EPD_SCK, -1, EPD_MOSI, EPD_CS);
}

void DisplayController::begin() {
    configureSPI();
    epd.init(115200, true, 2, false); 
    epd.setTextColor(GxEPD_BLACK);
    epd.setTextWrap(false);
}

// ==========================================
// Paging & Drawing Abstractions
// ==========================================

void DisplayController::startFullUpdate() {
    epd.setFullWindow();
    epd.firstPage();
}

void DisplayController::startPartialUpdate() {
    epd.setPartialWindow(0, 0, epd.width(), epd.height());
    epd.firstPage();
}

bool DisplayController::nextPage() {
    return epd.nextPage();
}

void DisplayController::fillWhite() {
    epd.fillScreen(GxEPD_WHITE);
}

void DisplayController::setMenuFont() {
    epd.setFont(&FreeMono9pt7b);
    epd.setTextSize(1);
}

void DisplayController::drawString(int x, int y, const String& text) {
    epd.setCursor(x, y);
    epd.print(text);
}

void DisplayController::drawBitmap(int x, int y, const uint8_t *bitmap, int w, int h) {
    epd.drawBitmap(x, y, bitmap, w, h, GxEPD_BLACK);
}

int DisplayController::getWidth() {
    return epd.width();
}

int DisplayController::getHeight() {
    return epd.height();
}
// ==========================================
// Consolidated Refresh Routines
// ==========================================

void DisplayController::clear() {
    startFullUpdate();
    do { 
        fillWhite(); 
    } while (nextPage());
}

void DisplayController::printText(const String& text) {
    epd.setFont();
    epd.setTextSize(1);
    startFullUpdate();
    do {
        fillWhite();
        drawString(MARGIN_X, MARGIN_Y, text);
    } while (nextPage());
}

void DisplayController::printTextFast(const String& text) {
    epd.setFont();
    epd.setTextSize(1);
    startPartialUpdate();
    do {
        fillWhite();
        drawString(MARGIN_X, MARGIN_Y, text);
    } while (nextPage());
}

// ==========================================
// Reader Layout Engine
// ==========================================

uint32_t DisplayController::printPageDynamic(const String& text, bool fastRefresh) {
    int maxWidth = epd.width() - (MARGIN_X * 2);
    int maxHeight = epd.height() - MARGIN_Y;
    
    epd.setFont(&FreeMono9pt7b);
    epd.setTextSize(1);
    
    int charWidth = 11; 
    int w_space = 6; 
    int lineHeight = 20; 
    int cursorY = MARGIN_Y + 14; 
    
    uint32_t bytesConsumed = 0;
    
    struct WordData { String text; bool italic; };
    struct DrawCmd { String word; int x; int y; bool italic; };
    std::vector<DrawCmd> drawList;
    
    bool inItalics = false; 
    
    int i = 0;
    while (i < text.length() && cursorY + 4 <= maxHeight) {
        std::vector<WordData> lineWords;
        int lineWidth = 0;
        bool forceNewline = false;
        int lineExtraNewlines = 0; 
        
        while (i < text.length()) {
            int nextSpace = text.indexOf(' ', i);
            int nextNewline = text.indexOf('\n', i);
            
            int breakIndex = -1;
            if (nextSpace == -1 && nextNewline == -1) breakIndex = text.length();
            else if (nextSpace == -1) breakIndex = nextNewline;
            else if (nextNewline == -1) breakIndex = nextSpace;
            else breakIndex = min(nextSpace, nextNewline);
            
            String rawWord = text.substring(i, breakIndex);
            bool isNewline = (breakIndex == nextNewline);
            
            String cleanWord = "";
            for (size_t c = 0; c < rawWord.length(); c++) {
                uint8_t byte = rawWord[c];
                if (byte == '\t') cleanWord += "  "; 
                else if (byte >= 32 && byte <= 126) cleanWord += (char)byte;
                else if (byte == 0xC2 && c + 1 < rawWord.length()) { 
                    uint8_t b2 = rawWord[c+1];
                    if (b2 == 0xA0) cleanWord += " ";
                    c++;
                }
                else if (byte == 0xE2 && c + 2 < rawWord.length()) {
                    uint8_t b2 = rawWord[c+1];
                    uint8_t b3 = rawWord[c+2];
                    if (b2 == 0x80) {
                        if (b3 == 0x98 || b3 == 0x99) cleanWord += "'";
                        else if (b3 == 0x9C || b3 == 0x9D) cleanWord += "\"";
                        else if (b3 == 0x93 || b3 == 0x94) cleanWord += "-";
                        else if (b3 == 0xA6) cleanWord += "...";
                    }
                    c += 2; 
                }
            }
            
            int extraNewlines = 0;
            if (isNewline) {
                int peek = breakIndex + 1;
                int newlineCount = 1;
                
                while (peek < text.length() && (text[peek] == '\n' || text[peek] == '\r')) {
                    if (text[peek] == '\n') newlineCount++;
                    peek++;
                }
                
                if (newlineCount > 1) {
                    isNewline = true;
                    extraNewlines = newlineCount - 1;
                    breakIndex = peek - 1; 
                } else {
                    char lastChar = cleanWord.length() > 0 ? cleanWord[cleanWord.length() - 1] : ' ';
                    
                    bool isTerminal = (lastChar == '.' || lastChar == '?' || lastChar == '!' || 
                                       lastChar == '"' || lastChar == '\'' || lastChar == ':' || lastChar == ';');
                    
                    char nextChar = ' ';
                    int peekNext = breakIndex + 1;
                    while (peekNext < text.length() && (text[peekNext] == ' ' || text[peekNext] == '\t' || text[peekNext] == '\r')) {
                        peekNext++;
                    }
                    if (peekNext < text.length()) nextChar = text[peekNext];
                    
                    bool explicitIndent = (breakIndex + 1 < text.length() && (text[breakIndex + 1] == ' ' || text[breakIndex + 1] == '\t'));
                    bool nextIsQuoteOrDash = (nextChar == '"' || nextChar == '-' || nextChar == '\'');
                    
                    if (isTerminal || explicitIndent || nextIsQuoteOrDash || cleanWord.length() == 0) {
                        isNewline = true; 
                    } else {
                        isNewline = false; 
                    }
                }
            }
            
            bool isItalicWord = inItalics;
            int uCount = 0;
            for (char c : cleanWord) { if (c == '_') uCount++; }
            
            if (uCount >= 2) {
                isItalicWord = true;
            } else if (uCount == 1) {
                if (!inItalics) {
                    inItalics = true;
                    isItalicWord = true;
                } else {
                    isItalicWord = true;
                    inItalics = false;
                }
            }
            cleanWord.replace("_", ""); 
            
            if (cleanWord.length() > 0) {
                int tw = cleanWord.length() * charWidth;
                
                if (lineWords.empty()) {
                    lineWords.push_back({cleanWord, isItalicWord});
                    lineWidth += tw;
                } else {
                    if (lineWidth + w_space + tw > maxWidth) {
                        break; 
                    } else {
                        lineWords.push_back({cleanWord, isItalicWord});
                        lineWidth += tw;
                    }
                }
            }
            
            i = breakIndex + 1;
            if (isNewline) {
                forceNewline = true;
                lineExtraNewlines = extraNewlines; 
                break; 
            }
        }
        
        int drawX = MARGIN_X;
        int totalSpace = maxWidth - lineWidth;
        int baseSpace = 0;
        
        if (lineWords.size() > 1) {
            baseSpace = totalSpace / (lineWords.size() - 1);
        }
        
        if (lineWords.size() <= 1 || forceNewline || i >= text.length() || baseSpace > 25) {
            for (WordData w_data : lineWords) {
                drawList.push_back({w_data.text, drawX, cursorY, w_data.italic});
                drawX += (w_data.text.length() * charWidth) + (w_data.text == " " ? 0 : w_space);
            }
        } else {
            int extraPixels = totalSpace % (lineWords.size() - 1);
            for (size_t j = 0; j < lineWords.size(); j++) {
                drawList.push_back({lineWords[j].text, drawX, cursorY, lineWords[j].italic});
                if (j < lineWords.size() - 1) {
                    drawX += (lineWords[j].text.length() * charWidth) + baseSpace + (j < extraPixels ? 1 : 0);
                }
            }
        }
        
        cursorY += lineHeight;
        cursorY += (lineExtraNewlines * lineHeight); 
        bytesConsumed = i; 
    }
    
    // --- REFACTORED RENDERING PHASE ---
    
    if (!fastRefresh) {
        clear(); // Reuses your clean full-refresh abstraction
    }

    startPartialUpdate(); // Uses the new partial update wrapper
    
    do {
        fillWhite();
        for (auto cmd : drawList) {
            if (cmd.italic) epd.setFont(&FreeMonoOblique9pt7b);
            else epd.setFont(&FreeMono9pt7b);
            
            drawString(cmd.x, cmd.y, cmd.word); // Reuses your drawString abstraction
        }
    } while (nextPage());
    
    epd.setFont(); 
    return bytesConsumed;
}