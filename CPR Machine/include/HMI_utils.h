#pragma once
#include <Audio.h>

// Utility functions for CPR Machine operation
void HMI_util_setup();
bool verifyBatteryPercentage();
void displayStartUpInstructions();
void displayCutClothingInstructions();
void displayUnfoldInstructions();
bool checkUserConfirmation(); // True if user begins device operation
void displayAlignmentInstructions();
void displayZeroingInstructions();
bool displayCompressionConfirmation();
void showScreen(const char *file);
void playAudio(const char *wavFileName);
bool nextButtonLoop();
bool pauseButtonLoop();

extern unsigned long GreenNow;
extern unsigned long PauseNow;

bool audioWasPlaying;

extern AudioPlaySdWav playWav1;

extern const char *startBmpFile;
extern const char *startWavFile; 