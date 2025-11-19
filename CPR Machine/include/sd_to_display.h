#include "Adafruit_RA8875.h"
#include <Adafruit_STMPE610.h>
#include <Adafruit_GFX.h>

void bmpDraw(Adafruit_RA8875 *tft, const char *filename, int x, int y, volatile bool *abortFlag = nullptr);
uint16_t read16(File f);
uint32_t read32(File f);
uint16_t color565(uint8_t r, uint8_t g, uint8_t b);
byte decToBcd(byte val);