#ifndef __SD_READ_WRITE_H
#define __SD_READ_WRITE_H

#include "Arduino.h"
#include "FS.h"
#include "SD_MMC.h"
#include <vector>

/*

  These are general IO functions to use the on-board SD card slot for the ESP32-WROVER board.

  These functions are used primarily in mono_file.cpp. 

*/

// SD card slot pins. These are hardwired on board. When using SD_MMC library, I was unable to use an ST7735 board using the TFT_eSPI library.

#define SD_MMC_CMD 15  
#define SD_MMC_CLK 14  
#define SD_MMC_D0 2  

#define CHUNK_SIZE 1024

int SDInit();
void SDInfo();
void listDir(fs::FS &fs, const char * dirname, uint8_t levels);
std::vector<String> getDirFilePaths(fs::FS &fs, const char * dirname);
void createDir(fs::FS &fs, const char * path);
void removeDir(fs::FS &fs, const char * path);
void readFile(fs::FS &fs, const char * path);
void writeFile(fs::FS &fs, const char * path, const char * message);
void appendFile(fs::FS &fs, const char * path, const char * message);
void renameFile(fs::FS &fs, const char * path1, const char * path2);
void deleteFile(fs::FS &fs, const char * path);

#endif
