#include "sd_read_write.h"

// Initialization function for SD card. This needs to be called before any other SD functions can be used. Should be caled in setup().

int SDInit(){

	SD_MMC.setPins(SD_MMC_CLK, SD_MMC_CMD, SD_MMC_D0);
	
	if(!SD_MMC.begin("/sdcard", true, true, SDMMC_FREQ_DEFAULT, 5)) {
		Serial.println("SD card could not be mounted.");
		return 0;
	}
	
	uint8_t cardType = SD_MMC.cardType();
	
	if(cardType == CARD_NONE){
	
		Serial.println("No SD_MMC card attatched.");
		return 0;
		
	}
	
	Serial.println("SD card successfully mounted.");
	
	return 1;
	
}

// Print general infor about SD card. 

void SDInfo(){
	
	uint8_t cardType = SD_MMC.cardType();

	Serial.printf("-------------------------\nSD_MMC card type: ");
	
	if(cardType == CARD_MMC) Serial.println("MMC.");
	else if(cardType == CARD_SD) Serial.println("SDSC.");
	else if(cardType == CARD_SDHC) Serial.println("SDHC.");
	else Serial.println("Unknown.");
	
	uint64_t cardSize = SD_MMC.cardSize() / (CHUNK_SIZE * CHUNK_SIZE);
	
	Serial.printf("Total space: %lluMB\r\n", SD_MMC.totalBytes() / (CHUNK_SIZE * CHUNK_SIZE));
	Serial.printf("Used space: %lluMB\r\n-------------------------\n", SD_MMC.usedBytes() / (CHUNK_SIZE * CHUNK_SIZE));
	
}

// List all files/dir in given directory.

void listDir(fs::FS &fs, const char *dirname, uint8_t levels) {
  Serial.printf("Listing directory: %s\n", dirname);

  File root = fs.open(dirname);
  if (!root) {
    Serial.println("Failed to open directory");
    return;
  }
  if (!root.isDirectory()) {
    Serial.println("Not a directory");
    return;
  }

  File file = root.openNextFile();
  while (file) {
    if (file.isDirectory()) {
      Serial.print("  DIR : ");
      Serial.println(file.name());
      if (levels) {
        listDir(fs, file.path(), levels - 1);
      }
    } else {
      Serial.print("  FILE: ");
      Serial.print(file.name());
      Serial.print("  SIZE: ");
      Serial.println(file.size());
    }
    file = root.openNextFile();
  }

}

/*

  Similar to listDir(), but returns a vector of filepaths.

  Returned filepaths DO NOT have root directory appended, i.e. "/test.wav"
  will return as "test.wav".

*/

std::vector<String> getDirFilePaths(fs::FS &fs, const char * dirname){

  std::vector<String> filepaths = {};

  File root = fs.open(dirname);

  if (!root) {

    Serial.println("Failed to open directory");
    return filepaths;

  }

  if (!root.isDirectory()) {

    Serial.println("Not a directory");
    return filepaths;

  }

  File file = root.openNextFile();

  while (file) {

    if (!file.isDirectory()) {

      filepaths.push_back(file.name());

    } 

    file = root.openNextFile();

  }

  return filepaths;

}

// General file/dir IO functions.

void createDir(fs::FS &fs, const char *path) {
  Serial.printf("Creating Dir: %s\n", path);
  if (fs.mkdir(path)) {
    Serial.println("Dir created");
  } else {
    Serial.println("mkdir failed");
  }
}

void removeDir(fs::FS &fs, const char *path) {
  Serial.printf("Removing Dir: %s\n", path);
  if (fs.rmdir(path)) {
    Serial.println("Dir removed");
  } else {
    Serial.println("rmdir failed");
  }
}

void readFile(fs::FS &fs, const char *path) {
  Serial.printf("Reading file: %s\n", path);

  File file = fs.open(path);
  if (!file) {
    Serial.println("Failed to open file for reading");
    return;
  }

  Serial.print("Read from file: ");
  while (file.available()) {
    Serial.write(file.read());
  }
}

void writeFile(fs::FS &fs, const char *path, const char *message) {
  Serial.printf("Writing file: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }
  if (file.print(message)) {
    Serial.println("File written");
  } else {
    Serial.println("Write failed");
  }
}

void appendFile(fs::FS &fs, const char *path, const char *message) {
  Serial.printf("Appending to file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if (!file) {
    Serial.println("Failed to open file for appending");
    return;
  }
  if (file.print(message)) {
    Serial.println("Message appended");
  } else {
    Serial.println("Append failed");
  }
}

void renameFile(fs::FS &fs, const char *path1, const char *path2) {
  Serial.printf("Renaming file %s to %s\n", path1, path2);
  if (fs.rename(path1, path2)) {
    Serial.println("File renamed");
  } else {
    Serial.println("Rename failed");
  }
}

void deleteFile(fs::FS &fs, const char *path) {
  Serial.printf("Deleting file: %s\n", path);
  if (fs.remove(path)) {
    Serial.println("File deleted");
  } else {
    Serial.println("Delete failed");
  }
}
