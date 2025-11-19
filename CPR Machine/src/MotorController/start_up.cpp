#include "start_up.h"
#include <Arduino.h>

// include for HMI
// from lines 5-12 of Ashley's HMI main.cpp
// TODO: add these header files into the repo if not there
#include <Adafruit_RA8875.h>

#include <SPI.h>
#include <Wire.h>
#include <SD.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include "Adafruit_RA8875.h"
#include <Adafruit_STMPE610.h>
#include "sd_to_display.h"

bool verifyBatteryPercentage() {
    Moteus::PositionMode::Command cmd;
    Moteus::MoteusController::Status status;
    auto maybe_status = controller.SetCommand(cmd);
    if (maybe_status) {
        status = *maybe_status;
        // Access input voltage
        double input_voltage = status.voltage;
        // voltage range from 19.2V to 22.2 V
        float battery_percent = map(input_voltage, 19.2, 22.2, 0, 100);
        
        // TODO: Display battery percentage on display (if applicable)
        
        if (battery_percent >= 50){
            return true;
    return false;
}

void displaySetupInstructions() {
    // combined variable definitions and traditional setup() function into this function
    // not sure if the combo is a bad idea

    // from lines 14-77 of initialization from Ashley's HMI main.cpp

    const int SD_CHIP_SELECT = BUILTIN_SDCARD;  // No external CS pin needed

    // Library only supports hardware SPI at this time
    // Connect SCLK to UNO Digital #13 (Hardware SPI clock)
    // Connect MISO to UNO Digital #12 (Hardware SPI MISO)
    // Connect MOSI to UNO Digital #11 (Hardware SPI MOSI)
    #define RA8875_INT 3
    #define RA8875_CS 10
    #define RA8875_RESET 9


    // ---- Button pin (5-pin LED button, switch part) ----
    // Button for going to next animation/instruction step
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

    if (SD.exists(filename)) {
        bmpDraw(&tft, filename, 0, 0);
    } else {
        Serial.print("ERROR: File not found: ");
        Serial.println(filename);
        tft.fillScreen(RA8875_RED);  // Show red screen if file missing
    }
    }


    // from setup function of Ashley's HMI main.cpp

    Serial.begin(9600);

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

// function from Ashley's main.cpp
void showCurrentFrame() {
  const char *filename = frameGroups[currentGroup][currentIndex];

  Serial.print("Showing group ");
  Serial.print(currentGroup);
  Serial.print(", frame ");
  Serial.print(currentIndex);
  Serial.print(" -> ");
  Serial.println(filename);

  if (SD.exists(filename)) {
    bmpDraw(&tft, filename, 0, 0);
  } else {
    Serial.print("ERROR: File not found: ");
    Serial.println(filename);
    tft.fillScreen(RA8875_RED);  // Show red screen if file missing
  }
}

bool checkUserStartConfirmation() {  
    // TODO: Implement HMI user confirmation check
    return true;
}