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
};