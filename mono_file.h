#ifndef _MONO_FILE_H
#define _MONO_FILE_H

/*
  WAV FILE HEADER REFERENCE

  1-4     RIFF
  5-8     FILE_SIZE
  9-12    WAVE
  13-16   FMT_CHUNK_MARKER
  17-20   FMT_LEN
  21-22   FMT_TYPE
  23-24   NUM_CHANNELS
  25-28   SAMPLE_RATE
  29-32   (SAMPLE_RATE * BITS_PER_SAMPLE * NUM_CHANNELS) / 8
  33-34   (BITS_PER_SAMPLE * NUM_CHANNELS) / 8
  35-36   BITS_PER_SAMPLE
  37-40   DATA_CHUNK_MARKER
  41-44   DATA_SECTION_SIZE
*/

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>

#include "sd_read_write.h"

#define M_PI (3.141592654)

#ifndef M_TWO_PI
#define M_TWO_PI (2.00 * M_PI)
#endif

struct __attribute__((packed)) MonoWAVHeader {

  char riff[4];
  uint32_t chunk_size;
  char wave[4];
  char fmt[4];
  uint32_t subchunk1_size;
  uint16_t audio_format;
  uint16_t num_channels = 1;
  uint32_t sample_rate;
  uint32_t byte_rate;
  uint16_t block_align;
  uint16_t bits_per_sample;
  char data[4];
  uint32_t subchunk2_size;

};

// WAV specific functions.

void createMonoWAVFile(fs::FS &fs, const char * path, uint32_t num_samples, uint32_t sample_rate, uint16_t bits_per_sample);
void writeSineWave(fs::FS &fs, const char * path, float freq, float duration);
void editMonoWAVHeader(fs::FS &fs, const char * path, uint32_t num_samples, uint32_t sample_rate, uint16_t bits_per_sample);
void normalizeMonoWAVFile(fs::FS &fs, const char * path, double normalization);
double rootMeanSquare(fs::FS &fs, const char * path);
//Need to add

void printMonoWAVData(fs::FS &fs, const char * path);
void playMonoWAVFile(fs::FS &fs, const char * path);
#endif