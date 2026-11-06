/*
Description: Declares the reader screen used for viewing a book and tracking reading progress.
Inputs: currentOffset, nextOffset, pageHistory, totalReadingSeconds, and SystemContext* ctx from the app manager.
Process: Tracks reading state and exposes the methods to load, render, and navigate pages.
Output: A reading app that persists progress and redraws the current page.
*/

#pragma once
#include "BaseApp.h"
#include <vector>
#include <map>
#include <LittleFS.h>

class ReaderApp : public BaseApp {
private:
    uint32_t currentOffset;
    uint32_t nextOffset;
    uint32_t lastDrawTime;
    
    // The history stack for backward scrolling
    std::vector<uint32_t> pageHistory;
    
    // Track the number of pages turned since the last save
    int pagesTurnedSinceSave = 0;
    
    // Helper function to render the text
    void renderPage(bool fastRefresh);

    // Helper functions for progress persistence
    void saveProgress();
    void loadProgress();

    bool inOptionsMenu = false;
    bool inConfirmReset = false;

    // Time tracking
    uint32_t sessionStartTime = 0;
    uint32_t totalReadingSeconds = 0; 

    void drawOptionsMenu();

public:
    ReaderApp(SystemContext* context);
    
    void onStart() override;
    void onLoop() override;
    void onButtonEvent(int btnId, int eventType) override;
    void onExit() override;
    void prepareForSleep() override;
    void forceRedraw() override;
};