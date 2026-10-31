#include "ReaderApp.h"
#include "DisplayController.h"
#include "BookParser.h"
#include "LibraryMenuApp.h" // Needed so we can switch back to it

// Assume you instantiate this globally in main.cpp
extern LibraryMenuApp* libraryApp; 
extern String globalTargetFile; 

std::map<String, uint32_t> ReaderApp::ramBookmarks;

ReaderApp::ReaderApp(SystemContext* context) : BaseApp(context) {
    currentOffset = 0;
    nextOffset = 0;
    lastDrawTime = 0;
}

void ReaderApp::onStart() {
    pageHistory.clear();
    
    // 1. Look for a saved bookmark in RAM, otherwise start at 0
    if (ramBookmarks.find(globalTargetFile) != ramBookmarks.end()) {
        currentOffset = ramBookmarks[globalTargetFile];
    } else {
        currentOffset = 0;
    }

    // 2. Open the file in the parser using your new API
    ctx->parser->openBook(globalTargetFile, currentOffset);

    // 3. Draw the first page (false = deep clean refresh)
    renderPage(false); 
}

void ReaderApp::onLoop() {
    // Keep empty. UI logic is strictly event-driven by buttons.
}

void ReaderApp::onExit() {
    // Save progress to RAM dictionary upon exiting
    ramBookmarks[globalTargetFile] = currentOffset;
}

void ReaderApp::renderPage(bool fastRefresh) {
    // 1. Sync the parser to our current reading position
    ctx->parser->setCurrentOffset(currentOffset);
    
    // 2. Fetch the text chunk using your simplified API
    String textChunk = ctx->parser->getRawChunk();

    // 3. Feed it to the layout engine
    uint32_t bytesDrawn = ctx->display->printPageDynamic(textChunk, fastRefresh);

    // 4. Store where the NEXT page should start (without overwriting currentOffset!)
    nextOffset = currentOffset + bytesDrawn; 
}

void ReaderApp::onButtonEvent(int btnId, int eventType) {
    if (eventType != 0) return; // Only trigger on press

    if (btnId == 1) { 
        // BACK TO MENU
        ctx->appManager->switchApp(libraryApp);
    } 
    else if (btnId == 2) { 
        // PREVIOUS PAGE
        if (!pageHistory.empty()) {
            currentOffset = pageHistory.back();
            pageHistory.pop_back(); // Remove it from history since we are moving back to it
            renderPage(true);       // Fast refresh
        }
    } 
    else if (btnId == 3) { 
        // NEXT PAGE
        // Push the current offset to the history stack so we can go backward to it later
        pageHistory.push_back(currentOffset);
        
        // Update the offset to exactly where the layout engine stopped drawing
        currentOffset = nextOffset;
        
        renderPage(true); // Fast refresh
    } 
    else if (btnId == 4) { 
        // FULL REFRESH
        // Redraws the exact same text, but the `false` parameter forces a deep clean blink
        renderPage(false);
    }
}