#include <Arduino.h>

#include <Adafruit_RA8875.h>
#include <SPI.h>
#include <Wire.h>
#include <SD.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_STMPE610.h>
#include "sd_to_display.h"

const int SD_CHIP_SELECT = BUILTIN_SDCARD;  // No external CS pin needed

// Library only supports hardware SPI at this time
// Connect SCLK to UNO Digital #13 (Hardware SPI clock)
// Connect MISO to UNO Digital #12 (Hardware SPI MISO)
// Connect MOSI to UNO Digital #11 (Hardware SPI MOSI)
#define RA8875_INT 3
#define RA8875_CS 37//10
#define RA8875_RESET 9


// ---- Button pin (5-pin LED button, switch part) ----
const int BUTTON_PIN = 4;    // CHANGE if you wired to a different pin
//const int LED_PIN    = 1;   // LED part of the button


Adafruit_RA8875 tft = Adafruit_RA8875(RA8875_CS, RA8875_RESET);


// -------- Animation settings --------
const int TARGET_FPS          = 2;               // how fast to flip between the 2 images
const uint16_t FRAME_INTERVAL = 1000 / TARGET_FPS;


// 3 groups, 2 BMPs each, in bmp1 / bmp2 / bmp3
// Note: SD card paths must start with "/" (root of SD card)
const char *frameGroups[3][2] = {
  { "/bmp1/ezgif-frame-001.bmp", "/bmp1/ezgif-frame-002.bmp" },   // group 0
  { "/bmp2/ezgif-frame-001.bmp", "/bmp2/ezgif-frame-002.bmp" },   // group 1
  { "/bmp3/ezgif-frame-001.bmp", "/bmp3/ezgif-frame-002.bmp" }    // group 2
};

uint8_t currentGroup = 0;    // 0..2 (which bmpN)
uint8_t currentIndex = 0;    // 0 or 1 (which of the two bmps in that group)

unsigned long lastFrameTime  = 0;

// ---- Button debounce state ----
bool lastButtonReading = HIGH;       // raw last reading
bool buttonState       = HIGH;       // debounced state
bool lastButtonState   = HIGH;       // previous debounced state (for edge detection)
unsigned long lastDebounceTime = 0;
const unsigned long DEBOUNCE_DELAY = 50;   // ms



void showCurrentFrame() {
  const char *filename = frameGroups[currentGroup][currentIndex];

  Serial.print("Showing group ");
  Serial.print(currentGroup);
  Serial.print(", frame ");
  Serial.print(currentIndex);
  Serial.print(" -> ");
  Serial.println(filename);
// 
  if (SD.exists(filename)) {
    bmpDraw(&tft, filename, 0, 0);
  } else {
    Serial.print("ERROR: File not found: ");
    Serial.println(filename);
    tft.fillScreen(RA8875_RED);  // Show red screen if file missing
  }
}

void HMI_setup() {

  // ---- Button setup ----
  pinMode(BUTTON_PIN, INPUT_PULLUP);   // button to GND, so LOW = pressed
  lastButtonReading = digitalRead(BUTTON_PIN);  // Initialize button state
  buttonState = lastButtonReading;
  lastButtonState = lastButtonReading;
  
  Serial.print("Button initialized on pin ");
  Serial.print(BUTTON_PIN);
  Serial.print(", initial state: ");
  Serial.println(lastButtonReading == HIGH ? "HIGH (not pressed)" : "LOW (pressed)");

  if (!SD.begin(SD_CHIP_SELECT)) {
    Serial.println("SD initialization failed!");
    return;
  }

  Serial.println("SD initialization done.");
  Serial.println("RA8875 start");

  if (!tft.begin(RA8875_800x480)) {
    Serial.println("RA8875 Not Found!");
    while (1);
  }

  Serial.println("Found RA8875");

  tft.displayOn(true);
  tft.GPIOX(true);      // Enable TFT - display enable tied to GPIOX
  tft.PWM1config(true, RA8875_PWM_CLK_DIV1024); // PWM output for backlight
  tft.PWM1out(255);

  Serial.print("(");
  Serial.print(tft.width());
  Serial.print(", ");
  Serial.print(tft.height());
  Serial.println(")");
  tft.graphicsMode();                 // go back to graphics mode
  tft.fillScreen(RA8875_BLACK);
  tft.graphicsMode();

  // Draw the very first frame of the first group
  currentGroup = 0;
  currentIndex = 0;
  showCurrentFrame();

  unsigned long now = millis();
  lastFrameTime = now;

}

void HMI_loop() {
     unsigned long now = millis();

  // ========== 1) Handle button with debounce ==========
  bool rawReading = digitalRead(BUTTON_PIN);

  if (rawReading != lastButtonReading) {
    // button state changed, reset debounce timer
    lastDebounceTime = now;
  }

  if ((now - lastDebounceTime) > DEBOUNCE_DELAY) {
    // if the reading has been stable longer than debounce delay
    if (rawReading != buttonState) {
      buttonState = rawReading;
    }
  }

  // Detect button press on edge (HIGH -> LOW transition)
  // This ensures we only trigger once per press, not continuously while held
  if (lastButtonState == HIGH && buttonState == LOW) {
    // Button was just pressed (edge detected)
    // Advance to next group
    currentGroup++;
    if (currentGroup >= 3) {
      currentGroup = 0;
    }

    // Start each group on its first frame
    currentIndex = 0;

    Serial.print("Button press -> switched to group ");
    Serial.println(currentGroup);

    showCurrentFrame();
  }

  // Update for next iteration
  lastButtonReading = rawReading;
  lastButtonState = buttonState;


  // ========== 2) Flip between the 2 frames in the current group ==========
  if (now - lastFrameTime >= FRAME_INTERVAL) {
    lastFrameTime = now;

    // toggle between 0 and 1
    currentIndex ^= 1;

    showCurrentFrame();
  }
}