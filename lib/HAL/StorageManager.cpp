/*
Description: Manages the device's file storage and saved settings on LittleFS and NVS.
Inputs: Book file names, Preferences keys like sleep_timeout, and values such as INACTIVITY_SLEEP_MS and TRANSFER_WIFI_PASSWORD.
Process: Lists .txt files, tracks bookmarks, validates settings, and returns storage usage data.
Output: File lists, saved progress, and the current storage status for the app.
*/

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

uint32_t StorageManager::loadInactivitySleepTimeout(uint32_t defaultValue) {
    if (!preferences.begin("user_settings", true)) {
        Serial.println("Error: Could not open user settings.");
        return defaultValue;
    }

    uint32_t timeoutMs = preferences.getUInt("sleep_timeout", defaultValue);
    preferences.end();

    if (timeoutMs != SLEEP_TIMEOUT_OPTION_1_MS &&
        timeoutMs != SLEEP_TIMEOUT_OPTION_2_MS &&
        timeoutMs != SLEEP_TIMEOUT_OPTION_3_MS &&
        timeoutMs != SLEEP_TIMEOUT_OPTION_4_MS) {
        Serial.println("Warning: Invalid saved sleep timeout; using default.");
        return defaultValue;
    }

    return timeoutMs;
}

bool StorageManager::saveInactivitySleepTimeout(uint32_t timeoutMs) {
    if (!preferences.begin("user_settings", false)) {
        Serial.println("Error: Could not open user settings for writing.");
        return false;
    }

    size_t bytesWritten = preferences.putUInt("sleep_timeout", timeoutMs);
    preferences.end();
    if (bytesWritten != sizeof(timeoutMs)) {
        Serial.println("Error: Could not save sleep timeout.");
        return false;
    }

    return true;
}

uint32_t StorageManager::loadReaderAutoSavePageThreshold(uint32_t defaultValue) {
    if (!preferences.begin("user_settings", true)) {
        Serial.println("Error: Could not open user settings.");
        return defaultValue;
    }

    uint32_t pageThreshold = preferences.getUInt("autosave_pages", defaultValue);
    preferences.end();

    if (pageThreshold != READER_AUTO_SAVE_OPTION_1_PAGES &&
        pageThreshold != READER_AUTO_SAVE_OPTION_2_PAGES &&
        pageThreshold != READER_AUTO_SAVE_OPTION_3_PAGES &&
        pageThreshold != READER_AUTO_SAVE_OPTION_4_PAGES) {
        Serial.println("Warning: Invalid saved autosave threshold; using default.");
        return defaultValue;
    }

    return pageThreshold;
}

bool StorageManager::saveReaderAutoSavePageThreshold(uint32_t pageThreshold) {
    if (pageThreshold != READER_AUTO_SAVE_OPTION_1_PAGES &&
        pageThreshold != READER_AUTO_SAVE_OPTION_2_PAGES &&
        pageThreshold != READER_AUTO_SAVE_OPTION_3_PAGES &&
        pageThreshold != READER_AUTO_SAVE_OPTION_4_PAGES) {
        Serial.println("Error: Refusing to save invalid autosave threshold.");
        return false;
    }

    if (!preferences.begin("user_settings", false)) {
        Serial.println("Error: Could not open user settings for writing.");
        return false;
    }

    size_t bytesWritten = preferences.putUInt("autosave_pages", pageThreshold);
    preferences.end();
    if (bytesWritten != sizeof(pageThreshold)) {
        Serial.println("Error: Could not save autosave threshold.");
        return false;
    }

    return true;
}

String StorageManager::loadTransferWifiPassword(const String& defaultValue) {
    if (!preferences.begin("user_settings", true)) {
        Serial.println("Error: Could not open user settings.");
        return defaultValue;
    }

    String password = preferences.getString("wifi_password", defaultValue);
    preferences.end();

    if (password.length() < 8 || password.length() > 63) {
        Serial.println("Warning: Invalid saved Wi-Fi password; using default.");
        return defaultValue;
    }

    return password;
}

bool StorageManager::saveTransferWifiPassword(const String& password) {
    if (password.length() < 8 || password.length() > 63) {
        Serial.println("Error: Wi-Fi password must be 8 to 63 characters.");
        return false;
    }

    if (!preferences.begin("user_settings", false)) {
        Serial.println("Error: Could not open user settings for writing.");
        return false;
    }

    size_t bytesWritten = preferences.putString("wifi_password", password);
    preferences.end();
    if (bytesWritten != password.length() + 1) {
        Serial.println("Error: Could not save Wi-Fi password.");
        return false;
    }

    return true;
}

bool StorageManager::resetUserSettingsToDefaults() {
    if (!preferences.begin("user_settings", false)) {
        Serial.println("Error: Could not open user settings for reset.");
        return false;
    }

    size_t sleepBytes = preferences.putUInt("sleep_timeout", INACTIVITY_SLEEP_MS);
    size_t autosaveBytes = preferences.putUInt(
        "autosave_pages", READER_AUTO_SAVE_PAGE_THRESHOLD);
    size_t passwordBytes = preferences.putString("wifi_password", TRANSFER_WIFI_PASSWORD);
    preferences.end();

    if (sleepBytes != sizeof(uint32_t) || autosaveBytes != sizeof(uint32_t) ||
        passwordBytes != sizeof(TRANSFER_WIFI_PASSWORD)) {
        Serial.println("Error: Could not reset all user settings.");
        return false;
    }

    return true;
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