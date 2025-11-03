#include "mono_file.h"
#include "i2s.h"

/*

  createMonoWAVFile() - Creates an empty mono WAV file on SD card at given location.

  fs::FS &fs - File system. Since we are using on-board SD card using SD_MMC.h, it should always be SD_MMC.
  const char * path - Name of created file. Root directory MUST be included, i.e. "/test.wav" and not "test.wav".
  uint32_t num_samples - Total number of samples in file. If file size is not known on creation, this can be changed in editMonoWAVHeader().
  uint32_t sample_rate - Sample rate of file. We are using CD quality audio, so this should always be 44100.
  uint16_t bits_per_sample - Number of bits per sample. We are using 16 bit PCM WAV, so this should always be 16.

  return - This function does not return on success. Instead, file will simply appear on SD card. Can be checked using listDir() in created file directory.

*/

void createMonoWAVFile(fs::FS &fs, const char *path, uint32_t num_samples, uint32_t sample_rate, uint16_t bits_per_sample) {

  struct MonoWAVHeader header;

  int num_channels = 1;

  memcpy(header.riff, "RIFF", 4);
  header.chunk_size = 36 + (num_samples * num_channels * (bits_per_sample / 8));
  memcpy(header.wave, "WAVE", 4);
  memcpy(header.fmt, "fmt ", 4);
  header.subchunk1_size = 16;
  header.audio_format = 1;
  header.num_channels = num_channels;
  header.sample_rate = sample_rate;
  header.byte_rate = sample_rate * num_channels * (bits_per_sample / 8);
  header.block_align = num_channels * (bits_per_sample / 8);
  header.bits_per_sample = bits_per_sample;
  memcpy(header.data, "data", 4);
  header.subchunk2_size = num_samples * num_channels * (bits_per_sample / 8);

  File file = fs.open(path, FILE_WRITE);

  if (!file) {

    Serial.println("File could not be created.");

    return;
  }

  size_t headerSize = sizeof(header);
  size_t written = file.write((uint8_t *)&header, headerSize);

  if (written == headerSize) {

    Serial.print((uint16_t)written);
    Serial.println(" bytes written successfullyy.");

  }

  else {

    Serial.println("Data could not be written to file.");

    return;
  }

  file.close();
}

/*

  editMonoWAVHeader() - Edits header information after creation. Used when file information is not static, or not known at creation, and needs to be updated.

  fs::FS &fs - File system. Since we are using on-board SD card using SD_MMC.h, it should always be SD_MMC.
  const char * path - Name of created file. Root directory MUST be included, i.e. "/test.wav" and not "test.wav".
  uint32_t num_samples - Total number of samples in file. If file size is not known on creation, this can be changed in editMonoWAVHeader().
  uint32_t sample_rate - Sample rate of file. We are using CD quality audio, so this should always be 44100.
  uint16_t bits_per_sample - Number of bits per sample. We are using 16 bit PCM WAV, so this should always be 16.

  return - This function does not return on success. Updated file info can be checked using printMonoWAVData().

*/

void editMonoWAVHeader(fs::FS &fs, const char * path, uint32_t num_samples, uint32_t sample_rate, uint16_t bits_per_sample){

  File file = fs.open(path, "r+");

  if(!file){

    return;

  }

  uint32_t subChunk2Size = num_samples * (bits_per_sample / 8);
  uint32_t chunkSize = 36 + subChunk2Size;

  file.seek(4);
  file.write((uint8_t *)&chunkSize, 4);

  file.seek(40);
  file.write((uint8_t *)&subChunk2Size, 4);

  file.close();

}

/*

  writeSineWave() - Writes a sine wave of given frequency and duration to a mono WAV file.

  fs::FS &fs - File system. Since we are using on-board SD card using SD_MMC.h, it should always be SD_MMC.
  const char * path - Name of created file. Root directory MUST be included, i.e. "/test.wav" and not "test.wav".
  float freq - Frequency of generated wave.
  float duration - Duration of file, given in seconds. 

  return - This function does not return. listDir() can be used to check if file was created.

*/

void writeSineWave(fs::FS &fs, const char * path, float freq, float duration){

  createMonoWAVFile(fs, path, 44100, 44100, 16);

  File file = fs.open(path, FILE_APPEND);

  int sum = 0;
  int expectedSum = 0;

  int bufferSize = 256;

  uint16_t buffer[bufferSize];

  int j = 0;

  int i;

  int f = 1;

  Serial.println(freq);

  int s = 44100;

  for (i = 0; i < (s * duration); i++) {

    buffer[j++] = (uint16_t)((sin(M_TWO_PI * freq * ((double)i / 44100)) * 32767) * 0.5f);

    if (j == bufferSize) {

      j = 0;

      size_t written = file.write((uint8_t*)&buffer, bufferSize * sizeof(uint16_t));

    }

  }

  if (j > 0) {

    file.write((uint8_t*)&buffer, j * sizeof(uint16_t));
  }

  file.close();

  editMonoWAVHeader(fs, path, (int) (s * duration), 44100, 16);

}

/*

  printMonoWAVData() - Prints file information to serial monitor.

  fs::FS &fs - File system. Since we are using on-board SD card using SD_MMC.h, it should always be SD_MMC.
  const char * path - Name of created file. Root directory MUST be included, i.e. "/test.wav" and not "test.wav".

  return - This function does not return. Check serial monitor for output.

*/

void printMonoWAVData(fs::FS &fs, const char * path){

  File file = fs.open(path, FILE_READ);

  if(!file){

    Serial.printf("%s could not be opened.\n", path);

    return;

  }

  uint16_t numChannels;
  uint32_t sampleRate;
  uint32_t fileSize;


  file.seek(22);
  file.read((uint8_t *)&numChannels, 2);

  file.seek(24);
  file.read((uint8_t *)&sampleRate, 4);

  file.seek(40);
  file.read((uint8_t *)&fileSize, 4);

  Serial.printf("\nFILE INFO:\n\nNUM CHANNELS: %d\nSAMPLE RATE: %d\nDuration: %.2fs\n\n", numChannels, sampleRate, (double) (fileSize / sampleRate / 2));

  file.close();

}

/*

  playMonoWAVFile() - Plays a given WAV file by sending sample data to I2S audio converter. Uses i2s.h i2s_write() to send buffer data.

  fs::FS &fs - File system. Since we are using on-board SD card using SD_MMC.h, it should always be SD_MMC.
  const char * path - Name of created file. Root directory MUST be included, i.e. "/test.wav" and not "test.wav".

  return - This function does not return. 

*/

void playMonoWAVFile(fs::FS &fs, const char * path){

  File file = fs.open(path, "r");

  size_t bytes_read;
  size_t bytes_written;
  uint16_t buffer[256];
  int16_t * samples;
  size_t sampleCount;

  if(!file){

    Serial.printf("%s could not be opened.\n", path);

    return;

  }

  Serial.printf("Opened %s\n", path);

  file.seek(44);

  while(file.available() && (bytes_read = file.read((uint8_t *)buffer, sizeof(buffer))) > 0){

    sampleCount = bytes_read / 2;

    samples = (int16_t*)buffer;

    i2s_write(I2S_NUM_1, buffer, bytes_read, &bytes_written, portMAX_DELAY);

  }

  file.close();

}

/*

  normalizeMonoWAVFile() - Function to normalize a given mono WAV file using the root mean square of all samples.

  Temporary file is created with original file header, and updated samples of original are written to it.
  After writing, original file is deleted, and temporary file is renamed. I did it this way because reading and writing
  over same file significantly increased amount of time needed. 

  This is used primarily to normalize all files on the SD card for listening purposes. Should be called when new 
  files are added to ensure that all files are at approximately the same level of "loudness".

  fs::FS &fs - File system. Since we are using on-board SD card using SD_MMC.h, it should always be SD_MMC.
  const char * path - Name of created file. Root directory MUST be included, i.e. "/test.wav" and not "test.wav".
  double normalization - Normalization level, given on a scale of 0.0 - 1.0. For this purpose, I have it set at
  a predetermined level relative to how loud files sound on external earbuds.

  return - This function does not return. rootMeanSquare() can be used to check if normalization worked or not.

*/

void normalizeMonoWAVFile(fs::FS &fs, const char * path, double normalization){

  int16_t buffer[512];
  int16_t * samples;
  size_t bytes_read;
  size_t sampleCount;
  size_t totalSamples = 0;

  double rms = rootMeanSquare(fs, path);
  double normalizationRatio = normalization / rms;
  double normalizedSample;

  File file = fs.open(path, "r+");

  if(!file){

    Serial.println("File could not be found.");

    return;

  }

  uint32_t fileSize;
  file.seek(40);
  file.read((uint8_t *)&fileSize, 4);

  double numSamples = (fileSize / 2);

  createMonoWAVFile(fs, "/temp.wav", numSamples, 44100, 16);
  File temp = fs.open("/temp.wav", "r+");
  temp.seek(44);

  file.seek(44);

  Serial.println("Normalizing.");

  while(file.available() && (bytes_read = file.read((uint8_t *)buffer, sizeof(buffer))) > 0){

    sampleCount = bytes_read / 2;

    samples = (int16_t*)buffer;

    for(size_t i = 0; i < sampleCount; i++){

      normalizedSample = normalizationRatio * samples[i];

      if(normalizedSample > 32767) normalizedSample = 32767;
      if(normalizedSample < -32768) normalizedSample = -32768;

      samples[i] = normalizedSample;

    }

    //file.seek(file.position() - bytes_read);

    temp.write((uint8_t*)&buffer, bytes_read);

    totalSamples += sampleCount;

    Serial.printf("%d / %.2f samples written.\n", totalSamples, numSamples);

  }

  file.close();
  temp.close();

  Serial.printf("PATH: %s\n", path);

  deleteFile(fs, path);
  renameFile(fs, "/temp.wav", path);

  //Serial.println("Normalization complete.");

  // rms = rootMeanSquare(fs, path);
  // printMonoWAVData(fs, path);

}

/*

  rootMeanSquare() Used to find root mean square of given file, which gives an approximate average "loudness".

  Used in normalizeMonoWavFile() as basis for normalization ratio.

  fs::FS &fs - File system. Since we are using on-board SD card using SD_MMC.h, it should always be SD_MMC.
  const char * path - Name of created file. Root directory MUST be included, i.e. "/test.wav" and not "test.wav".

  return - This function should return a double between 0.0 and 1.0, representing the approximate average loudness of the file.

*/

double rootMeanSquare(fs::FS &fs, const char * path){

  int buffer[512];
  double rms = 0.0;
  size_t bytes_read;
  size_t totalSamples = 0;
  size_t sampleCount;
  int16_t * samples;

  double normalizedSample;

  File file = fs.open(path, "r+");

  if(!file){

    Serial.println("File could not be opened.");

    return -1.0;

  }

  file.seek(44);

  while(file.available() && (bytes_read = file.read((uint8_t*)&buffer, sizeof(buffer))) > 0){

    sampleCount = bytes_read / 2;

    totalSamples += sampleCount;

    samples = (int16_t*)buffer;

    for(int i=0;i<sampleCount;i++){

      normalizedSample = (double)samples[i] / 32768.0f;

      rms += (normalizedSample * normalizedSample);

    }

  }

  rms = sqrt(rms/totalSamples);

  Serial.printf("%.2f\t%d\n", rms, totalSamples);

  file.close();

  return rms;

}

void record(fs::FS &fs, const char * path, double duration){

  i2s_set_adc_mode(ADC_UNIT_1, ADC1_CHANNEL_4); // for example, GPIO32 = ADC1_CH4
  i2s_adc_enable(I2S_NUM_0);

  size_t bytesTotal = 0;

  createMonoWAVFile(fs, path, 44100 * duration, 44100, 16);

  File file = SD_MMC.open(path, FILE_WRITE);

  file.seek(44);

  const int BUF_LEN = 256;
  uint16_t buffer[BUF_LEN];
  int16_t buffer16[BUF_LEN];

  size_t bytesRead;

  while(bytesTotal < 44100 * 2 * duration){

    i2s_read(I2S_NUM_0, (void*)buffer, sizeof(buffer), &bytesRead, portMAX_DELAY);

    int numSamples = bytesRead / 2;

    for (int i = 0; i < numSamples; i++) {

      int16_t sample12 = buffer[i] & 0x0FFF;  
      int16_t sample16 = ((int32_t)sample12 - 2048) << 4;

      int32_t amp = sample16 * 2.00;

      if (amp > 32767) amp = 32767;
      if (amp < -32768) amp = -32768;
      buffer16[i] = amp; 

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