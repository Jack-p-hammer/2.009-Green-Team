#include <SD.h>
#include <SPI.h>
// Audio includes - commented out since we're using PROGMEM and don't need audio
// #include <play_sd_aac.h>
// #include <Audio.h>
#include <JPEGDEC.h>

// Modified function signature - removed audio parameter since we don't use it
int playVid(File mjpeg,JPEGDEC jpeg,int JPEGDraw,uint32_t vidstart=0);
