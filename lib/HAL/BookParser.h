/*
Description: Declares the book-reading helper used to open a text file and track its position.
Inputs: filename, startOffset, currentFile, currentOffset, fileSize, and READER_CHUNK_SIZE_BYTES.
Process: Stores the selected file and its reading position so callers can load chunks safely.
Output: Methods to open, read, and move through the file while keeping file state available.
*/

#pragma once
#include "config.h"
#include <LittleFS.h>

class BookParser {
private:
    String currentFile = "";
    uint32_t currentOffset = 0;
    uint32_t fileSize = 0;
    
    const int CHUNK_SIZE = READER_CHUNK_SIZE_BYTES;

public:
    BookParser();
    
    bool openBook(const String& filename, uint32_t startOffset = 0);
    String getRawChunk(); // Renamed and simplified
    
    uint32_t getFileSize();
    uint32_t getCurrentOffset();
    void setCurrentOffset(uint32_t offset);
};