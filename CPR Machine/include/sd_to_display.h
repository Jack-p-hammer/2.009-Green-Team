#include "Adafruit_RA8875.h"
#include <Adafruit_STMPE610.h>
#include <Adafruit_GFX.h>

void bmpDraw(Adafruit_RA8875 *tft, const char *filename, int x, int y, volatile bool *abortFlag = nullptr);
uint16_t read16(File f);
uint32_t read32(File f);
uint16_t color565(uint8_t r, uint8_t g, uint8_t b);
byte decToBcd(byte val);

// Pin definitions
extern const int SD_CHIP_SELECT;
extern const int BUTTON_PIN;
extern const int RA8875_CS;
extern const int RA8875_RESET;

// Display object
extern Adafruit_RA8875 tft;

// Frame groups (1D array - just the first BMP from each group)
extern const char *frameGroups[11];

// Current group state (0, 1, or 2)
extern uint8_t currentGroup;

// Button state variables
extern bool buttonState;
extern bool lastButtonReading;
extern unsigned long lastDebounceTime;
extern const unsigned long DEBOUNCE_DELAY;

// Functions
void showCurrentFrame();