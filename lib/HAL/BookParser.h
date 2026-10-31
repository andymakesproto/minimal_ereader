#pragma once
#include "config.h"
#include <LittleFS.h>

class BookParser {
private:
    String currentFile = "";
    uint32_t currentOffset = 0;
    uint32_t fileSize = 0;
    
    // Increased massively to guarantee we always have enough text to fill the screen
    const int CHUNK_SIZE = 1500; 

public:
    BookParser();
    
    bool openBook(const String& filename, uint32_t startOffset = 0);
    String getRawChunk(); // Renamed and simplified
    
    uint32_t getFileSize();
    uint32_t getCurrentOffset();
    void setCurrentOffset(uint32_t offset);
};