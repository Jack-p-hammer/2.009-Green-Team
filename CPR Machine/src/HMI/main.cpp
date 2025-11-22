#include <Arduino.h>

#include <Adafruit_RA8875.h>

#include <SPI.h>
#include <Wire.h>
#include <SD.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include "Adafruit_RA8875.h"
#include <Adafruit_STMPE610.h>

#include "sd_to_display.h"

// Pin definitions
const int SD_CHIP_SELECT = BUILTIN_SDCARD;
const int BUTTON_PIN = 4;
const int POWER_BUTTON_PIN = 5;
const int RA8875_CS = 37;
const int RA8875_RESET = 9;

// Display object
Adafruit_RA8875 tft = Adafruit_RA8875(RA8875_CS, RA8875_RESET);

const char *frameGroups[] = {
  "/bmp01/bare_chest.bmp",
  "/bmp02/ezgif-frame-002.bmp",
  "/bmp03/hold_down.bmp",
  "/bmp04/start_img.bmp",
};


//const uint8_t NUM_GROUPS = sizeof(frameGroups) / sizeof(frameGroups[0]);


// Current group state (0, 1, or 2)
uint8_t currentGroup = 0;

// Track if the screen is “on” (backlight & display enabled)
bool screenOn = false;

// Button state variables
bool buttonState = HIGH;
bool lastButtonReading = HIGH;
unsigned long lastDebounceTime = 0;

// Button state variables (power button)
bool powerButtonState = HIGH;
bool lastPowerButtonReading = HIGH;
unsigned long lastPowerDebounceTime = 0;


const unsigned long DEBOUNCE_DELAY = 40;  // ms



void showCurrentFrame() {
  const char *filename = frameGroups[currentGroup];

  Serial.print("Showing group ");
  Serial.print(currentGroup);
  Serial.print(" -> ");
  Serial.println(filename);

  if (SD.exists(filename)) {
    bmpDraw(&tft, filename, 0, 0);
  } else {
    Serial.print("ERROR: File not found: ");
    Serial.println(filename);
    tft.fillScreen(RA8875_RED);
  }
}

void toggleScreen() {
  if (screenOn) {
    // ----- TURN SCREEN OFF -----
    Serial.println("Turning screen OFF");
    screenOn = false;

    tft.PWM1out(0);        // backlight off
    tft.displayOn(false);  // disable display output
    // Optional: clear it so it’s black if it ever flickers back on
    tft.graphicsMode();
    tft.fillScreen(RA8875_BLACK);
  } else {
    // ----- TURN SCREEN ON -----
    Serial.println("Turning screen ON");
    currentGroup = 0;
    screenOn = true;

    tft.displayOn(true);   // enable display
    tft.PWM1out(255);      // backlight full

    // either keep currentGroup, or reset to 0 if you always want to
    // restart from the first image when turning on:
    // currentGroup = 0;

    showCurrentFrame();
  }
}



void setup() {
  Serial.begin(9600);

  // ---- Button setup ----
  pinMode(BUTTON_PIN, INPUT_PULLUP);   // button to GND, so LOW = pressed
  pinMode(POWER_BUTTON_PIN, INPUT_PULLUP);  // power button

  lastButtonReading = digitalRead(BUTTON_PIN);  // Initialize button state
  lastPowerButtonReading = digitalRead(POWER_BUTTON_PIN);

  
  Serial.print("Button initialized on pin ");
  Serial.print(BUTTON_PIN);
  Serial.print(", initial state: ");
  Serial.println(lastButtonReading == HIGH ? "HIGH (not pressed)" : "LOW (pressed)");

  Serial.print("Power button on pin ");
  Serial.print(POWER_BUTTON_PIN);
  Serial.print(", initial state: ");
  Serial.println(lastPowerButtonReading == HIGH ? "HIGH (not pressed)" : "LOW (pressed)");



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

  tft.displayOn(false);
  tft.GPIOX(true);      // Enable TFT - display enable tied to GPIOX
  tft.PWM1config(true, RA8875_PWM_CLK_DIV1024); // PWM output for backlight
  tft.PWM1out(0);   // <<< backlight off (0 = dark)

  Serial.print("(");
  Serial.print(tft.width());
  Serial.print(", ");
  Serial.print(tft.height());
  Serial.println(")");
  tft.graphicsMode();                 // go back to graphics mode
  tft.fillScreen(RA8875_BLACK);
  tft.graphicsMode();

  // Draw the first frame of the first group
  currentGroup = 0;
  screenOn = false;       // <<< start with screen off
}

void loop() {
  // Handle button with debounce
  unsigned long PowerNow = millis();

  // ====== 1) Handle POWER button (pin 5) with debounce ======
  bool rawPowerReading = digitalRead(POWER_BUTTON_PIN);  // LOW = pressed
  if (rawPowerReading != lastPowerButtonReading) {
    lastPowerDebounceTime = PowerNow;
  }

  if ((PowerNow - lastPowerDebounceTime) > DEBOUNCE_DELAY) {
    if (rawPowerReading != powerButtonState) {
      powerButtonState = rawPowerReading;

      if (powerButtonState == LOW) {
        toggleScreen();
        }

      }
    }
  
  lastPowerButtonReading = rawPowerReading;
  bool rawReading = digitalRead(BUTTON_PIN);  // LOW = pressed (INPUT_PULLUP)
  unsigned long now = millis();

  // If the reading changed from last time, reset the debounce timer
  if (rawReading != lastButtonReading) {
    lastDebounceTime = now;
  }

  // Has the reading been stable for long enough to be considered valid?
  if ((now - lastDebounceTime) > DEBOUNCE_DELAY) {
    // If the stable reading is different from the current debounced state
    if (rawReading != buttonState) {
      buttonState = rawReading;

      // We just got a clean transition
      // Detect a press on HIGH -> LOW (button down)
     // Transition: HIGH -> LOW = button pressed
      if (buttonState == LOW) {
        // Only do anything if screen is ON
        if (screenOn) {
          currentGroup++;
          if (currentGroup >= 4) {
            currentGroup = 0;
          }

        Serial.print("Button press -> switched to group ");
        Serial.println(currentGroup);

        // Immediately show the new group's first frame
        showCurrentFrame();
      } else {
        Serial.println("Group button pressed but screen is OFF; ignoring.");
      }
      }
    }
  }

  // Save raw reading for next pass
  lastButtonReading = rawReading;
}
