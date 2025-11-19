#include <Arduino.h>
#include <Audio.h>
#include <SPI.h>
#include <SD.h>

#include "sd_to_speaker.h"

// ===============================
// Teensy Audio Library objects
// ===============================

// Play WAV from SD card
static AudioPlaySdWav     playWav1;

// I2S output -> MAX98357A amp
static AudioOutputI2S     i2s1;

// Connect WAV player to I2S left/right
static AudioConnection    patchCord1(playWav1, 0, i2s1, 0);   // left
static AudioConnection    patchCord2(playWav1, 1, i2s1, 1);   // right

// Allocate audio memory in setup via speaker_init()


// ===============================
// Public API
// ===============================

void speaker_init() {
  // Initialize Audio system
  AudioMemory(16);  // adjust up if you play multiple things at once

  // Initialize SD using the Teensy 4.1 built-in SD slot
  if (!SD.begin(BUILTIN_SDCARD)) {
    Serial.println("ERROR: SD card init failed in speaker_init()");
    // You can decide whether to halt or continue
    // while (1) { delay(100); }
  } else {
    Serial.println("SD card OK (speaker_init)");
  }

  // Nothing else is needed for MAX98357A:
  // It just takes I2S (BCLK/LRCLK/DATA) and makes audio.
}

void speaker_play(const char *filename) {
  if (!filename) return;

  Serial.print("Playing audio file: ");
  Serial.println(filename);

  // Stop any currently playing audio
  if (playWav1.isPlaying()) {
    playWav1.stop();
    delay(5);
  }

  // Start playing from SD; filename is case-sensitive on some systems
  playWav1.play(filename);

  // Give the library a short moment to start reading the file
  delay(10);

  if (!playWav1.isPlaying()) {
    Serial.println("WARN: speaker_play() - file did not start playing");
  }
}

bool speaker_isPlaying() {
  return playWav1.isPlaying();
}
