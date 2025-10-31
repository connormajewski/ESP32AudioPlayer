#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#include "mono_file.h"
#include "sd_read_write.h"
#include "i2s.h"
#include "button.h"

int voltage = 0;

std::vector<String> filepaths;

LiquidCrystal_I2C lcd(0x27, 16, 2);

int filepathsIndex;

void clear(LiquidCrystal_I2C &lcd) {
    lcd.setCursor(0, 0);      
    lcd.print("                ");
    lcd.setCursor(0, 1);      
    lcd.print("                ");          
    lcd.setCursor(0, 0);      
}

void playTest(const char* path) {

  //Serial.printf("Playing %s\n", path);

  char temp[64];

  snprintf(temp, sizeof(temp), "/%s", path);

  playMonoWAVFile(SD_MMC, temp);
  
}

void normalizeTest(const char * path, double n){

  char temp[64];

  snprintf(temp, sizeof(temp), "/%s", path);

  normalizeMonoWAVFile(SD_MMC, temp, n); 

}

void monoWAVTest(){

  Serial.println("TESTING mono_file SD functionality.");

  listDir(SD_MMC, "/", 0);

  Serial.println("Writing 5.wav.");

  writeSineWave(SD_MMC, "/5.wav", 261.63, 5.00);

  printMonoWAVData(SD_MMC, "/5.wav");

  Serial.println("Writing 10.wav.");

  writeSineWave(SD_MMC, "/10.wav", 392.00, 10.00);

  printMonoWAVData(SD_MMC, "/10.wav");

  Serial.println("Writing 7.wav.");

  writeSineWave(SD_MMC, "/7.wav", 329.23, 7.00);

  printMonoWAVData(SD_MMC, "/7.wav");

  listDir(SD_MMC, "/", 0);

  Serial.println("Testing sine wave generation...");

  for(int i=3;i>0;i--){

    Serial.println(i);

  }

  generateSineWave(261.63, 1.00, 1.00);
  generateSineWave(329.628, 1.00, 0.8);
  generateSineWave(392, 1.00, 0.6);
  generateSineWave(329.628, 1.00, 0.4);
  generateSineWave(261.63, 1.00, 0.2);

  Serial.println("Testing playing from file.");

  int i;

  for(i=3;i>0;i--){

    Serial.println(i);

    delay(1000);

  }

  Serial.println("Playing 5.wav");

  playMonoWAVFile(SD_MMC, "/5.wav");

  Serial.println("Playing 10.wav");

  playMonoWAVFile(SD_MMC, "/10.wav");

  Serial.println("Playing 7.wav");

  playMonoWAVFile(SD_MMC, "/7.wav");

  Serial.println("Playing song.");

  for(int i=3;i>0;i--) {
    
    Serial.println(i);

    delay(1000);

  }

  playTest("/anima.wav");

}

void record(const char * path, double duration){

  i2s_set_adc_mode(ADC_UNIT_1, ADC1_CHANNEL_4); // for example, GPIO32 = ADC1_CH4
  i2s_adc_enable(I2S_NUM_0);

  size_t bytesTotal = 0;

  createMonoWAVFile(SD_MMC, path, 44100 * duration, 44100, 16);

  File file = SD_MMC.open(path, FILE_WRITE);

  file.seek(44);

  const int BUF_LEN = 256;
  uint16_t buffer[BUF_LEN];
  int16_t buffer16[BUF_LEN];

  size_t bytesRead;

  while(bytesTotal < 44100 * 2 * duration){

    //printf("%d/%.2f\n",bytesTotal, 44100 * 2 * duration);

    // Read raw ADC samples via I2S
    i2s_read(I2S_NUM_0, (void*)buffer, sizeof(buffer), &bytesRead, portMAX_DELAY);

    int numSamples = bytesRead / 2;

    for (int i = 0; i < numSamples; i++) {
      int16_t sample12 = buffer[i] & 0x0FFF;  
      int16_t sample16 = ((int32_t)sample12 - 2048) << 4;

      int32_t amp = sample16 * 2.00;

      if (amp > 32767) amp = 32767;
      if (amp < -32768) amp = -32768;
      buffer16[i] = amp; 
      //printf("%d/%d\n", i, numSamples);
    }

    static float prev = 0;
    float alpha = 0.995;
    for (int i = 0; i < numSamples; i++) {
      float s = buffer16[i];
      float filtered = s - prev;
      prev = alpha * s + (1.0 - alpha) * prev;

      if (filtered > 32767) filtered = 32767;
      if (filtered < -32768) filtered = -32768;

      buffer16[i] = (int16_t)filtered;
    }

    // Write directly to SD
    file.write((uint8_t*)buffer16, numSamples * 2);

    bytesTotal += numSamples * 2;


  }

  file.close();

  editMonoWAVHeader(SD_MMC, path, 44100 * duration, 44100, 16);

  Serial.println("DONE.");

  return;

}

void setup(){

  Serial.begin(115200);

  int isSDInit = SDInit();

  if (!isSDInit) {

    Serial.println("Error initializing SD card.");

  }

  else {

    SDInfo();

    Serial.println("Initializing I2S.");

    I2SInit();

    listDir(SD_MMC, "/", 0);

    filepaths = getDirFilePaths(SD_MMC, "/"); 

    Wire.begin(21, 22);  // SDA=21, SCL=22 (can be changed)
    lcd.init();
    lcd.backlight();

    // for(const auto &filepath: filepaths){

    //   clear(lcd);

    //   lcd.setCursor(0,0);
    //   lcd.print("Normalizing...");
    //   lcd.setCursor(0,1);
    //   lcd.print(filepath.c_str());

    //   const char * path = filepath.c_str();

    //   char temp[64];

    //   snprintf(temp, sizeof(temp), "/%s", path);

    //   normalizeMonoWAVFile(SD_MMC, temp, 0.05);

    // }

    filepathsIndex = 0;

    lcd.setCursor(0,0);
    lcd.print("> ");
    lcd.print(filepaths[filepathsIndex].c_str());

    if(filepathsIndex < filepaths.size() - 1){

      lcd.setCursor(0,1);
      lcd.print(filepaths[filepathsIndex + 1].c_str());

    }


  }

  // printMonoWAVData(SD_MMC, "/dracula.wav");

  // double rms = rootMeanSquare(SD_MMC, "/dracula.wav");

  // Serial.printf("RMS: %.2f\n", rms);

}


void loop(){

  buttonReturn button = getButtonEvent();

  if(button.event == SINGLE_PRESS){

    switch(button.type){

      case BUTTON_1:
        playTest(filepaths[filepathsIndex].c_str());
        //normalizeTest(filepaths[filepathsIndex].c_str(), 0.02);
        Serial.println("BUTTON 1"); 
        break;
      case BUTTON_3: 
        if(filepathsIndex < filepaths.size() - 1) filepathsIndex++;

        clear(lcd);

        break;
      case BUTTON_2:
        if(filepathsIndex > 0) filepathsIndex--; 

        clear(lcd);

        break;
      default: break;

    }

    lcd.setCursor(0,0);
    lcd.print("> ");
    lcd.print(filepaths[filepathsIndex].c_str());

    if(filepathsIndex < filepaths.size() - 1){

      lcd.setCursor(0,1);
      lcd.print(filepaths[filepathsIndex + 1].c_str());

    }


  }


}