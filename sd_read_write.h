#ifndef __SD_READ_WRITE_H
#define __SD_READ_WRITE_H

#include "Arduino.h"
#include "FS.h"
#include "SD_MMC.h"
#include <vector>

#define SD_MMC_CMD 15  
#define SD_MMC_CLK 14  
#define SD_MMC_D0 2  

#define CHUNK_SIZE 1024


// General IO functions.

int SDInit();
void SDInfo();
void listDir(fs::FS &fs, const char * dirname, uint8_t levels);
std::vector<String> getDirFilePaths(fs::FS &fs, const char * dirname);
void createDir(fs::FS &fs, const char * path);
void removeDir(fs::FS &fs, const char * path);
void readFile(fs::FS &fs, const char * path);
void writeFile(fs::FS &fs, const char * path, const char * message);
void writeFileFormatString(fs::FS &fs, const char * path, const char * message, ...);
void appendFile(fs::FS &fs, const char * path, const char * message);
void renameFile(fs::FS &fs, const char * path1, const char * path2);
void deleteFile(fs::FS &fs, const char * path);
void testFileIO(fs::FS &fs, const char * path);

// writeFile specific helper functions

char * intToString(unsigned int num, int base);

#endif
