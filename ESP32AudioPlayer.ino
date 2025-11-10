#include <Wire.h>
//#include <LiquidCrystal_I2C.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_HEIGHT 32
#define SCREEN_WIDTH 128
#define OLED_RESET -1

#include "mono_file.h"
#include "sd_read_write.h"
#include "i2s.h"
#include "button.h"

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// This vector is used to traverse files in given directory. Used for selecting audio.

std::vector<String> filepaths;
int filepathsIndex;

int isPaused = 1;

// Seperate from filepathsIndex. When a file is played, this is set, then compared against filepathsIndex to check if selection has changed.

int currentFileIndex = -1;

File currentFile;

int isFileSelection = 0;

// audioTask specific variables. Copy of variables in playMonoWAVFile() in mono_file.cpp.

size_t bytes_read;
size_t bytes_written;
uint16_t buffer[512];
int16_t *samples;
size_t sampleCount;

double amplitude = 1.0;

// I2C screen-specific variables.

int16_t textX;
int16_t textWidth;
unsigned long lastFrame = 0;
const int frameDelay = 30;

// Attach audio playback to seperate core to eliminate audio loss when reading button events.

void audioTask(void *parameters) {
  while (true) {

    if (!isPaused && currentFile && currentFile.available()) {

      bytes_read = currentFile.read((uint8_t *)buffer, sizeof(buffer));

      if (bytes_read > 0) {

        samples = (int16_t*)buffer;

        sampleCount = bytes_read / 2;

        for(int i=0;i<sampleCount;i++){

          samples[i] *= (0.4 * amplitude);

        }

        i2s_write(I2S_NUM_1, buffer, bytes_read, &bytes_written, portMAX_DELAY);

      } 

    }

    vTaskDelay(1);
  }

}

// This is a helper function to normalize audio files to some level, i.e. 0.05. EXTREMELY SLOW.

void normalizeAllFiles(std::vector<String> filepaths, double normalization) {

  for (const auto &filepath : filepaths) {

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

    Wire.begin(21, 22);

    

    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    
      Serial.println("SSD1306 allocation failed");
      
    }

    else{

      delay(2000);
      display.clearDisplay();
      display.setTextWrap(false);
      display.setTextSize(1);
      display.setTextColor(WHITE);
      
      textWidth = strlen(filepaths[filepathsIndex].c_str()) * 6;
      textX = SCREEN_WIDTH;

      display.print("Now Playing");
      display.setCursor(0,16);
      display.print(filepaths[filepathsIndex].c_str());

    }

  }
}

// loop() contains the main program flow. This is basically the "menu" for the audio player.

void loop() {

  // Potentiometer on GPIO34. Divide by 400 to get values 0.0 - 10.0 for volume.

  amplitude = (analogRead(35) / 400);

  //Serial.println(amplitude);

  buttonReturn button = getButtonEvent();

  if (button.event == SINGLE_PRESS) {

    if(button.type == BUTTON_1){

      isPaused = !isPaused;

    }

    if(button.type == BUTTON_2 || button.type == BUTTON_3){

      if(button.type == BUTTON_2){

        if (filepathsIndex > 0) filepathsIndex--;

      }

      if(button.type == BUTTON_3){

        if (filepathsIndex < filepaths.size() - 1) filepathsIndex++;

      }

      char temp[64];

      snprintf(temp, sizeof(temp), "/%s", filepaths[filepathsIndex].c_str());

      currentFile = SD_MMC.open(temp, "r");
      currentFileIndex = filepathsIndex;

      isPaused = 0;

    }

  }

  if(!isPaused){

    unsigned long now = millis();
    if (now - lastFrame >= frameDelay) {
      lastFrame = now;

      display.clearDisplay();

      display.setCursor(0,0);
      display.print("Now Playing");

      display.setCursor(textX, 16);
      display.print(filepaths[filepathsIndex].c_str());
      display.display();

      textX--;
      if (textX < -textWidth) {
        textX = SCREEN_WIDTH - 10;
      }
    }
  }

}

