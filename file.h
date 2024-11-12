#pragma once

#include <Arduino.h>

#define FORMAT_LITTLEFS_IF_FAILED true

int initFileSystem();
void formatFileSystem();
void checkAvailableFlashSpace();
void syncFilesystem();
size_t getAvalibleDiskSpace();
void listDir(const char * dirname, uint8_t levels);
size_t writeFile(const char * path, const char * message);
size_t appendFile(const char * path, const char * message);
void readFile(const char * path);
