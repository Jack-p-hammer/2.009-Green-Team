// sd_to_speaker.h
#pragma once

void speaker_init();                       // call once in setup()
void speaker_play(const char *filename);   // play a WAV file from SD
bool speaker_isPlaying();                  // query play state
