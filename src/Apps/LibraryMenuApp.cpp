/*
Description: Builds the library menu and handles navigation between books, settings, and Wi-Fi transfer.
Inputs: ctx->storage->listTextFiles(), selectedIndex, scrollOffset, globalTargetFile, transferApp, settingsApp, and libraryApp.
Process: Scans .txt files, draws the menu, updates scrolling, and switches to the selected app when a button is pressed.
Output: A visible library list and the correct app launch for the selected item.
*/

#include "LibraryMenuApp.h"
#include "DisplayController.h" 
#include "StorageManager.h"    // Required to access the storage HAL
#include "AppManager.h"
#include "ReaderApp.h" 
#include "SleepApp.h"
#include "TransferApp.h"
#include "SettingsApp.h"

extern ReaderApp* readerApp; 
extern String globalTargetFile;
extern SleepApp* sleepApp;
extern TransferApp* transferApp;
extern SettingsApp* settingsApp;

static bool fullRefreshNextDraw = false;

LibraryMenuApp::LibraryMenuApp(SystemContext* context) : BaseApp(context) {
    selectedIndex = 0;
    scrollOffset = 0;
    lastDrawTime = 0;
}

void LibraryMenuApp::onStart() {
    fullRefreshNextDraw = true;
    scanFiles();
    drawMenu();
}

void LibraryMenuApp::onLoop() {
    if (ctx->display->isBusy()) {
        return; 
    }
}

void LibraryMenuApp::onExit() {
    fileList.clear(); 
}

void LibraryMenuApp::scanFiles() {
    fileList.clear();  
    // Fetch the actual files from the LittleFS partition
    fileList = ctx->storage->listTextFiles();
    
    // Safety check: Display a message if the flash drive is empty
    if (fileList.empty()) {
        fileList.push_back("No .txt files found");
    }
    fileList.insert(fileList.begin(), "[ Wi-Fi Transfer ]");
    fileList.insert(fileList.begin() + 1, "[ Settings ]");
}

void LibraryMenuApp::drawMenu() {
    if (fullRefreshNextDraw) {
        ctx->display->startFullUpdate();
        fullRefreshNextDraw = false;
    } else {
        ctx->display->startPartialUpdate();
    }
    
    // 1. Calculate how many characters fit on the screen
    int screenWidth = ctx->display->getWidth();
    int charWidth = FONT_CHAR_WIDTH;
    int maxLineChars = screenWidth / charWidth;
    int maxFileNameChars = maxLineChars - 2; // Reserve 2 spaces for the "> " cursor

    do {
        ctx->display->fillWhite();
        ctx->display->setMenuFont();
        ctx->display->drawString(0, 20, "______ LIBRARY _______");
        
        String stats = ctx->storage->getStorageStats();
        ctx->display->drawString(0, ctx->display->getHeight() - 20, stats); 
        
        int cursorY = 50;
        int itemHeight = LIBRARY_MENU_ITEM_HEIGHT;

        for (int i = scrollOffset; i < std::min((int)fileList.size(), scrollOffset + maxVisibleItems); i++) {
            String fileName = fileList[i];
            
            // 2. Truncate the filename if it exceeds the screen width
            if (fileName.length() > maxFileNameChars) {
                // Keep the beginning, chop the rest, and add "..."
                fileName = fileName.substring(0, maxFileNameChars - 3) + "...";
            }
            
            String lineText = (i == selectedIndex) ? "> " : "  ";
            lineText += fileName;
            
            ctx->display->drawString(0, cursorY, lineText);
            cursorY += itemHeight;
        }
    } while (ctx->display->nextPage());
}

void LibraryMenuApp::onButtonEvent(int btnId, int eventType) {
    if (eventType != 0) return; 

    if (btnId == 1) {
        // Back Button: Go to sleep screen
        ctx->appManager->switchApp(sleepApp);
    }

    if (btnId == 2) { 
        // UP
        if (selectedIndex > 0) {
            selectedIndex--;
            if (selectedIndex < scrollOffset) scrollOffset--;
            drawMenu(); 
        }
    } 
    else if (btnId == 3) { 
        // DOWN
        if (selectedIndex < fileList.size() - 1) {
            selectedIndex++;
            if (selectedIndex >= scrollOffset + maxVisibleItems) scrollOffset++;
            drawMenu();
        }
    } 
    else if (btnId == 4) { 
        // SELECT
        
        if (fileList.empty()) return;

        // 1. Handle Wi-Fi Transfer (Always forced to index 0 in scanFiles)
        if (selectedIndex == 0) {
            ctx->appManager->switchApp(transferApp);
        }
        else if (selectedIndex == 1) {
            ctx->appManager->switchApp(settingsApp);
        }
        else if (fileList[selectedIndex] == "No .txt files found") {
            // Do nothing, it's just a message
            return; 
        }
        // 3. It's a real book, send it to the reader
        else {
            globalTargetFile = fileList[selectedIndex];
            ctx->appManager->switchApp(readerApp); 
        }
    }
}