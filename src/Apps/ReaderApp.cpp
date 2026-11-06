/*
Description: Runs the reading screen, loads progress, and renders book content page by page.
Inputs: globalTargetFile, ctx->parser, currentOffset, pageHistory, totalReadingSeconds, and button events from the reader screen.
Process: Opens the text file, saves/loads progress, handles page turns, and turns on light sleep during idle time.
Output: The current book page is displayed and reading progress is kept in storage.
*/

#include "config.h"
#include "Apps/ReaderApp.h"
#include "DisplayController.h"
#include "BookParser.h"
#include "Apps/LibraryMenuApp.h" 
#include <esp_sleep.h>
#include <driver/gpio.h>
#include <esp_timer.h>

extern LibraryMenuApp* libraryApp; 
extern String globalTargetFile; 

ReaderApp::ReaderApp(SystemContext* context) : BaseApp(context) {
    currentOffset = 0;
    nextOffset = 0;
    lastDrawTime = 0;
    pagesTurnedSinceSave = 0;
}

void ReaderApp::saveProgress() {
    uint32_t now = (uint32_t)(esp_timer_get_time() / 1000000ULL);
    if (sessionStartTime != 0) {
        totalReadingSeconds += now - sessionStartTime;
        sessionStartTime = now;
    }

    String metaPath = globalTargetFile + ".meta";
    File f = LittleFS.open(metaPath, FILE_WRITE);
    if (!f) {
        Serial.println("Error: Could not open .meta file for writing");
        return;
    }

    // Line 1: Current Offset
    f.println(currentOffset);
    // Line 2: Size of the history stack
    f.println(pageHistory.size());
    
    // Line 3+: The stack values
    for (uint32_t offset : pageHistory) {
        f.println(offset);
    }
    // Final line: cumulative reading time in seconds
    f.println(totalReadingSeconds);
    f.close();
    
    pagesTurnedSinceSave = 0; // Reset counter after a successful save
    Serial.printf("Progress saved: %s at offset %d\n", metaPath.c_str(), currentOffset);
}

void ReaderApp::loadProgress() {
    pageHistory.clear();
    currentOffset = 0;
    pagesTurnedSinceSave = 0;
    totalReadingSeconds = 0;

    String metaPath = globalTargetFile + ".meta";
    if (LittleFS.exists(metaPath)) {
        File f = LittleFS.open(metaPath, FILE_READ);
        if (f) {
            // Read current offset
            String line = f.readStringUntil('\n');
            currentOffset = line.toInt();
            // Read stack size
            line = f.readStringUntil('\n');
            int stackSize = line.toInt();
            
            // Rebuild the history stack
            for (int i = 0; i < stackSize; i++) {
                line = f.readStringUntil('\n');
                pageHistory.push_back(line.toInt());
            }
            if (f.available()) {
                line = f.readStringUntil('\n');
                totalReadingSeconds = line.toInt();
            }
            f.close();
            Serial.printf("Progress loaded: %s at offset %d\n", metaPath.c_str(), currentOffset);
        }
    } else {
        Serial.println("No .meta file found. Starting book from beginning.");
    }
}

void ReaderApp::onStart() {
    // 1. Restore exact offset and history stack from Flash
    loadProgress();

    // 2. Open the file in the parser
    ctx->parser->openBook(globalTargetFile, currentOffset);

    // 3. Draw the first page (false = deep clean refresh)
    renderPage(false); 

    // 4. Record the session start time for tracking total reading duration
    sessionStartTime = (uint32_t)(esp_timer_get_time() / 1000000ULL);
}

void ReaderApp::onLoop() {
    // --- 1. Hardware-Aware Awake Window ---
    // DO NOT go to sleep if the E-Ink panel is currently updating!
    // (Adjust "isBusy()" to whatever your display library uses)
    if (ctx->display->isBusy()) {
        return; 
    }

    // Give the OneButton library a tiny window to register the physical release
    // so we don't freeze the buttons.
    static uint64_t buttonReleaseWindow = 0;
    if (digitalRead(BTN_1) == LOW || digitalRead(BTN_2) == LOW || 
        digitalRead(BTN_3) == LOW || digitalRead(BTN_4) == LOW) {
        buttonReleaseWindow = esp_timer_get_time() + READER_BUTTON_RELEASE_WINDOW_US;
    }
    if (esp_timer_get_time() < buttonReleaseWindow) {
        return;
    }
    
    // --- 2. Calculate Idle Time (Using RTC Clock) ---
    uint32_t idleTime = (uint32_t)(esp_timer_get_time() / 1000ULL) - ctx->lastActivityTime;
    
    // --- 3. Release Control for Overlays ---
    if (idleTime >= ctx->inactivityWarningTimeoutMs()) {
        return; 
    }

    // --- 4. Light Sleep Execution ---
    uint32_t timeUntilWake = ctx->inactivityWarningTimeoutMs() - idleTime;

    gpio_wakeup_enable((gpio_num_t)BTN_1, GPIO_INTR_LOW_LEVEL);
    gpio_wakeup_enable((gpio_num_t)BTN_2, GPIO_INTR_LOW_LEVEL);
    gpio_wakeup_enable((gpio_num_t)BTN_3, GPIO_INTR_LOW_LEVEL);
    gpio_wakeup_enable((gpio_num_t)BTN_4, GPIO_INTR_LOW_LEVEL);
    esp_sleep_enable_gpio_wakeup();

    esp_sleep_enable_timer_wakeup((uint64_t)timeUntilWake * 1000ULL);
    
    Serial.flush();
    esp_light_sleep_start();
    
    delay(READER_WAKE_DEBOUNCE_MS); // Hardware debounce after waking up
}

void ReaderApp::onExit() {
    // Save progress directly to Flash upon exiting back to library
    saveProgress();
}

void ReaderApp::prepareForSleep() {
    // Save progress before entering deep sleep
    saveProgress();
}

void ReaderApp::renderPage(bool fastRefresh) {
    ctx->parser->setCurrentOffset(currentOffset);
    String textChunk = ctx->parser->getRawChunk();
    uint32_t bytesDrawn = ctx->display->printPageDynamic(textChunk, fastRefresh);
    nextOffset = currentOffset + bytesDrawn; 
}

void ReaderApp::drawOptionsMenu() {
    // 1. Calculate Time
    uint32_t currentSession = (uint32_t)(esp_timer_get_time() / 1000000ULL) - sessionStartTime;
    uint32_t totalSecs = totalReadingSeconds + currentSession;
    int hours = totalSecs / 3600;
    int mins = (totalSecs % 3600) / 60;
    
    // 2. Calculate Progress Percentage
    uint32_t fileSize = ctx->parser->getFileSize();
    float progressPct = 0.0f;
    if (fileSize > 0) {
        progressPct = ((float)currentOffset / (float)fileSize) * 100.0f;
        if (progressPct > 100.0f) progressPct = 100.0f;
    }
    
    String timeStr = "Time: " + String(hours) + "h " + String(mins) + "m";
    String progStr = "Progress: " + String(progressPct, 1) + "%";
    
    // --- FIX: Add the E-Ink physical update loop ---
    ctx->display->startFullUpdate();
    do {
        ctx->display->fillWhite(); 
        ctx->display->setMenuFont(); 
        
        ctx->display->drawString(8, 40, "Reading Stats:");
        ctx->display->drawString(8, 80, timeStr);
        ctx->display->drawString(8, 110, progStr);
        
        ctx->display->drawString(8, 170, "[1] Back to Book");
        ctx->display->drawString(8, 200, "[4] Reset Progress");
    } while (ctx->display->nextPage());
}

void ReaderApp::onButtonEvent(int btnId, int eventType) {
    // Long press on Button 4 opens the options menu
    if (eventType == EVENT_LONG_PRESS) {
        if (btnId == 4) { 
            if (!inOptionsMenu && !inConfirmReset) {
                inOptionsMenu = true;
                drawOptionsMenu();
            }
        }
        return; 
    }

    if (eventType != EVENT_CLICK) return;

    // Cofirm reset state takes precedence over all other states
    if (inConfirmReset) {
        if (btnId == 1) { // Cancel
            inConfirmReset = false;
            drawOptionsMenu();
        } else if (btnId == 4) { // Confirm
            currentOffset = 0;
            pageHistory.clear();
            pagesTurnedSinceSave = 0;
            totalReadingSeconds = 0; 
            sessionStartTime = (uint32_t)(esp_timer_get_time() / 1000000ULL);
            
            inConfirmReset = false;
            inOptionsMenu = false;
            
            saveProgress(); // Overwrite the file immediately
            renderPage(false); // Hard refresh back to page 1
        }
        return;
    }

    // Options menu state takes precedence over normal reading
    if (inOptionsMenu) {
        if (btnId == 1) { // Exit Menu
            inOptionsMenu = false;
            renderPage(false); 
        } else if (btnId == 4) { // Trigger Reset Confirm
            inConfirmReset = true;

            ctx->display->startFullUpdate();
            do {
                ctx->display->fillWhite();
                ctx->display->setMenuFont();
                ctx->display->drawString(8, 80, "Reset all progress?");
                ctx->display->drawString(8, 140, "[1] Cancel");
                ctx->display->drawString(8, 170, "[4] Confirm");
            } while (ctx->display->nextPage());
        }
        return;
    }

    // Normal reading state button handling
    if (btnId == 1) { 
        ctx->appManager->switchApp(libraryApp);
    } 
    else if (btnId == 2) { 
        if (!pageHistory.empty()) {
            currentOffset = pageHistory.back();
            pageHistory.pop_back();
            renderPage(true);       
            pagesTurnedSinceSave++;
        }
    } 
    else if (btnId == 3) { 
        pageHistory.push_back(currentOffset);
        currentOffset = nextOffset;
        renderPage(true); 
        pagesTurnedSinceSave++;
    } 
    else if (btnId == 4) { 
        // Short click scrubs the ghosting manually
        renderPage(false);
    }

    // Auto-Save Trigger
    if (pagesTurnedSinceSave >= ctx->readerAutoSavePageThreshold) {
        saveProgress();
        Serial.printf("Auto-saved progress after %d pages turned.\n", pagesTurnedSinceSave);
    }
}

// Clear the warning overlay
void ReaderApp::forceRedraw() {
    if (inConfirmReset) {
        // Abort the reset safely and return to the book
        inConfirmReset = false;
        inOptionsMenu = false;
        
        ctx->display->clear();
        renderPage(true);
    } 
    else if (inOptionsMenu) {
        // Redraw the main stats menu
        drawOptionsMenu(); 
    } 
    else {
        // Normal reading mode redraw
        ctx->display->clear();
        renderPage(true); 
    }
}