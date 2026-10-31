#include "BookParser.h"

BookParser::BookParser() {}

bool BookParser::openBook(const String& filename, uint32_t startOffset) {
    File f = LittleFS.open(filename, "r");
    if (!f) return false;
    
    currentFile = filename;
    fileSize = f.size();
    currentOffset = startOffset;
    f.close();
    
    return true;
}

String BookParser::getRawChunk() {
    File f = LittleFS.open(currentFile, "r");
    if (!f) return "";

    f.seek(currentOffset);
    
    char buffer[CHUNK_SIZE + 1] = {0};
    f.readBytes(buffer, CHUNK_SIZE);
    f.close();

    return String(buffer);
}

uint32_t BookParser::getFileSize() { return fileSize; }
uint32_t BookParser::getCurrentOffset() { return currentOffset; }

void BookParser::setCurrentOffset(uint32_t offset) {
    if (offset > fileSize) offset = fileSize;
    currentOffset = offset;
}