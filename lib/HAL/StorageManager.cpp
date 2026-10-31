#include "StorageManager.h"

bool StorageManager::begin() {
    if (!LittleFS.begin(true)) { 
        // true forces formatting on the very first boot if the partition is unformatted
        Serial.println("Error: LittleFS Mount Failed!");
        return false;
    }
    Serial.println("LittleFS Mounted Successfully");
    return true;
}

std::vector<String> StorageManager::listTextFiles() {
    std::vector<String> files;
    
    File root = LittleFS.open("/");
    if (!root || !root.isDirectory()) {
        return files;
    }

    File file = root.openNextFile();
    while (file) {
        if (!file.isDirectory()) {
            String name = file.name();
            
            if (name.endsWith(".txt")) {
                // Ensure a leading slash is always present for consistent file paths
                if (!name.startsWith("/")) {
                    name = "/" + name;
                }
                files.push_back(name);
            }
        }
        file = root.openNextFile();
    }
    return files;
}

String StorageManager::generateNvsKey(const String& filename) {
    // Clean the string to save characters
    String cleanName = filename;
    cleanName.replace("/", "");
    cleanName.replace(".txt", "");
    
    // If it's already under the 15-char NVS limit, return it directly
    if (cleanName.length() <= 15) {
        return cleanName;
    }
    
    // If it's too long, safely extract the start and end (e.g., "LordOfThe_Rings")
    return cleanName.substring(0, 7) + "_" + cleanName.substring(cleanName.length() - 7);
}

void StorageManager::saveBookmark(const String& filename, uint32_t byteOffset) {
    // false = Read/Write mode
    preferences.begin("reader_state", false);
    preferences.putUInt(generateNvsKey(filename).c_str(), byteOffset);
    preferences.end();
}

uint32_t StorageManager::loadBookmark(const String& filename) {
    // true = Read-only mode (safer and slightly faster)
    preferences.begin("reader_state", true);
    uint32_t offset = preferences.getUInt(generateNvsKey(filename).c_str(), 0); 
    preferences.end();
    
    return offset;
}

String StorageManager::getStorageStats() {
    // LittleFS must be initialized first. 
    // If you are calling this after begin(), it's safe.
    
    size_t total = LittleFS.totalBytes();
    size_t used = LittleFS.usedBytes();
    size_t free = total - used;
    
    // Convert to Kilobytes for readability
    float totalKB = total / 1024.0;
    float usedKB = used / 1024.0;
    float freeKB = free / 1024.0;
    
    // Format into a clean string: "Used: 12.5 / 1500.0 KB"
    char buffer[64];
    snprintf(buffer, sizeof(buffer), "Free: %.1f/%.1f KB", freeKB, totalKB);
    
    return String(buffer);
}