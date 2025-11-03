#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#include "mono_file.h"
#include "sd_read_write.h"
#include "i2s.h"
#include "button.h"

// This vector is used to traverse files in given directory. Used for selecting audio.

std::vector<String> filepaths;
int filepathsIndex;

int isPaused = 1;

// Seperate from filepathsIndex. When a file is played, this is set, then compared against filepathsIndex to check if selection has changed.

int currentFileIndex = -1;

File currentFile;

LiquidCrystal_I2C lcd(0x27, 16, 2);

// audioTask specific variables. Copy of variables in playMonoWAVFile() in mono_file.cpp.

size_t bytes_read;
size_t bytes_written;
uint16_t buffer[512];
int16_t *samples;
size_t sampleCount;

// Attatch audio playback to seperate core to eliminate audio loss when reading button events.

void audioTask(void *parameters) {
  while (true) {

    if (!isPaused && currentFile && currentFile.available()) {

      bytes_read = currentFile.read((uint8_t *)buffer, sizeof(buffer));

      if (bytes_read > 0) {
        i2s_write(I2S_NUM_1, buffer, bytes_read, &bytes_written, portMAX_DELAY);
      } else {
        currentFile.close();
      }

    }

    vTaskDelay(1);
  }

}

// Quick function to "clear" lcd by overwriting with spaces.
// Used when moving through filepaths.

void clear(LiquidCrystal_I2C &lcd) {

  lcd.setCursor(0, 0);
  lcd.print("                ");
  lcd.setCursor(0, 1);
  lcd.print("                ");
  lcd.setCursor(0, 0);

}

// This is a helper function to normalize audio files to some level, i.e. 0.05. EXTREMELY SLOW.

void normalizeAllFiles(std::vector<String> filepaths, double normalization) {

  for (const auto &filepath : filepaths) {

    clear(lcd);

    lcd.setCursor(0, 0);
    lcd.print("Normalizing...");
    lcd.setCursor(0, 1);
    lcd.print(filepath.c_str());

    const char *path = filepath.c_str();

    char temp[64];

    snprintf(temp, sizeof(temp), "/%s", path);

    normalizeMonoWAVFile(SD_MMC, temp, normalization);
  }
}

void setup() {

  Serial.begin(115200);

  int isSDInit = SDInit();

  if (!isSDInit) {

    Serial.println("Error initializing SD card.");

  }

  else {

    xTaskCreatePinnedToCore(audioTask, "Audio", 4096, NULL, 1, NULL, 0);

    SDInfo();

    Serial.println("Initializing I2S.");

    I2SInit();

    listDir(SD_MMC, "/", 0);

    filepaths = getDirFilePaths(SD_MMC, "/");

    Wire.begin(21, 22);  // SDA=21, SCL=22 (can be changed)
    lcd.init();
    lcd.backlight();

    filepathsIndex = 0;

    lcd.setCursor(0, 0);
    lcd.print("> ");
    lcd.print(filepaths[filepathsIndex].c_str());

    if (filepathsIndex < filepaths.size() - 1) {

      lcd.setCursor(0, 1);
      lcd.print(filepaths[filepathsIndex + 1].c_str());
    }
  }
}

// loop() contains the main program flow. This is basically the "menu" for the audio player.

void loop() {

  buttonReturn button = getButtonEvent();

  if (button.event == SINGLE_PRESS) {

    switch (button.type) {

      case BUTTON_1:

        // On play button press, check that file isn't already selected or played.
        // If different, we need to create temp string to hold path, and update.

        if (filepathsIndex == currentFileIndex) {

          isPaused = !isPaused;

          Serial.printf("PAUSE TOGGLE. %d\n", isPaused);

        }

        else {

          // Possible fix.

          // i2s_zero_dma_buffer(I2S_NUM_1);
          // currentFile.close();

          char temp[64];

          snprintf(temp, sizeof(temp), "/%s", filepaths[filepathsIndex].c_str());

          currentFile = SD_MMC.open(temp, "r");
          currentFileIndex = filepathsIndex;

          isPaused = 0;

          Serial.printf("NEW FILE SELECTED: %s\t%d\t%d\n", temp, currentFileIndex, isPaused);
        }

        break;
      case BUTTON_3:
        if (filepathsIndex < filepaths.size() - 1) filepathsIndex++;

        clear(lcd);

        break;
      case BUTTON_2:
        if (filepathsIndex > 0) filepathsIndex--;

        clear(lcd);

        break;
      default: break;
    }

    lcd.setCursor(0, 0);
    lcd.print("> ");
    lcd.print(filepaths[filepathsIndex].c_str());

    if (filepathsIndex < filepaths.size() - 1) {

      lcd.setCursor(0, 1);
      lcd.print(filepaths[filepathsIndex + 1].c_str());
    }
  }
}