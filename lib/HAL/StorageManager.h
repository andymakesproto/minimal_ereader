/*
Description: Declares the storage layer for LittleFS files and saved reader settings.
Inputs: Preferences keys, file names such as "/book.txt", and settings values like sleep_timeout and autosave_pages.
Process: Lists text books, reads and writes bookmarks, and saves user settings to NVS.
Output: File lists, bookmark offsets, and validated settings values for the app.
*/

#pragma once
#include "config.h"
#include <LittleFS.h>
#include <Preferences.h>
#include <vector>

class StorageManager {
private:
    Preferences preferences;
    
    // Internal helper to ensure bookmark keys never exceed the ESP32's 15-character limit
    String generateNvsKey(const String& filename);

public:
    bool begin();
    
    // Scans the flash drive and returns a list of all .txt files
    std::vector<String> listTextFiles();

    // Returns a string summarizing storage usage and available space
    String getStorageStats();
    
    // NVS wrappers to save reading progress safely
    void saveBookmark(const String& filename, uint32_t byteOffset);
    uint32_t loadBookmark(const String& filename);

    uint32_t loadInactivitySleepTimeout(uint32_t defaultValue);
    bool saveInactivitySleepTimeout(uint32_t timeoutMs);
    uint32_t loadReaderAutoSavePageThreshold(uint32_t defaultValue);
    bool saveReaderAutoSavePageThreshold(uint32_t pageThreshold);
    String loadTransferWifiPassword(const String& defaultValue);
    bool saveTransferWifiPassword(const String& password);
    bool resetUserSettingsToDefaults();
};