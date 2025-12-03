#include "HMI_utils.h"
#include <Arduino.h>
#include "sensors.h"
#include "control_scheme.h"
#include <Arduino.h>
#include "Adafruit_VL53L0X.h"
#include <Adafruit_RA8875.h>

#include <SPI.h>
#include <Wire.h>
#include <SD.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_STMPE610.h>

#include "sd_to_display.h"
#include <Audio.h>

// AUDIO 
AudioPlaySdWav playWav1;
AudioAmplifier amp1;
AudioOutputI2S i2s1;


// playWav1 -> amp1 -> i2s1 (L+R)
AudioConnection  patchCord1(playWav1, 0, amp1, 0);
AudioConnection  patchCord2(amp1, 0, i2s1, 0);  // left
AudioConnection  patchCord3(amp1, 0, i2s1, 1);  // right

// NEW: audio gain / pause state
float audioGainDefault = 0.7f;    // your chosen normal gain

// NEW: path to pause image
const char *startUpWavFile = "startUpWav.wav";
const char *cutClothingWavFile = "cutClothingWav.wav";
const char *unfoldWavFile = "unfoldWav.wav";
const char *alignmentWavFile = "alignmentWav.wav";
const char *zeroingPrepWavFile = "zeroingPrepWav.wav";
const char *zeroingWavFile = "zeroingWav.wav";
const char *compressionPrepWavFile = "compressionPrepWav.wav";
const char *compressionsWavFile = "compressionsWav.wav";
const char *pausedWavFile = "pausedWav.wav";
const char *kneelFailureWavFile = "kneelFailureWav.wav";
const char *abortWavFile = "abortWav.wav";

// BUTTONS
// Pin definitions
// const int SD_CHIP_SELECT = BUILTIN_SDCARD;
// const int NEXT_BUTTON_PIN = 28; //green "next/resume"
// const int PAUSE_BUTTON_PIN = 26; //pause button 


// const int BUTTON_LED_PIN = 29;
// const int PAUSE_LED_PIN = 27;
// int button_light_count = 0;

// const int RA8875_CS = 16;
// const int RA8875_RESET = 15;

// Pin definitions
const int SD_CHIP_SELECT = BUILTIN_SDCARD;
const int NEXT_BUTTON_PIN = 28; //blue "next/resume"
const int NEXT_LED_PIN = 29; // green


const int PAUSE_BUTTON_PIN = 16; //orange //pause button 
const int PAUSE_LED_PIN = 14; //yellow

int button_light_count = 0;

const int RA8875_CS = 17;
const int RA8875_RESET = 15;

// Button state variables
bool nextButtonState = HIGH;
bool lastNextButtonReading = HIGH;
unsigned long lastDebounceTime = 0;

// NEW: Pause button debounce
bool pauseButtonState = HIGH;
bool lastPauseButtonReading = HIGH;
unsigned long lastPauseDebounceTime = 0;

const unsigned long DEBOUNCE_DELAY = 5;  // ms - reduced for instant button response

unsigned long GreenNow;
unsigned long PauseNow;

// SCREEN
// Display object
Adafruit_RA8875 tft = Adafruit_RA8875(RA8875_CS, RA8875_RESET);
// Track if the screen is “on” (backlight & display enabled)
bool screenOn = false;


// NEW: path to pause image
const char *startUpBmpFile = "startUpBmp.bmp";
const char *cutClothingBmpFile = "cutClothingBmp.bmp";
const char *unfoldBmpFile = "unfoldBmp.bmp";
const char *alignmentBmpFile = "alignmentBmp.bmp";
const char *zeroingPrepBmpFile = "zeroingPrepBmp.bmp";
const char *zeroingBmpFile = "zeroingBmp.bmp";
const char *compressionPrepBmpFile = "compressionPrepBmp.bmp";
const char *compressionsBmpFile = "compressionsBmp.bmp";
const char *pausedBmpFile = "pausedBmp.bmp";
const char *kneelFailureBmpFile = "kneelFailureBmp.bmp";
const char *abortBmpFile = "abortBmp.bmp";


void HMI_util_setup() {

  SPI.setMOSI(11);
  SPI.setMISO(12); 
  SPI.setSCK(13);

  SPI.begin();

  // ---- Button setup ----
  pinMode(NEXT_BUTTON_PIN, INPUT_PULLUP);   // button to GND, so LOW = pressed
  pinMode(PAUSE_BUTTON_PIN, INPUT_PULLUP);   // NEW: pause button


  lastNextButtonReading = digitalRead(NEXT_BUTTON_PIN);  // Initialize button state
  //lastPauseButtonReading = digitalRead(PAUSE_BUTTON_PIN);  // NEW

  
  Serial.print("Button initialized on pin ");
  Serial.print(NEXT_BUTTON_PIN);
  Serial.print(", initial state: ");
  Serial.println(lastNextButtonReading == HIGH ? "HIGH (not pressed)" : "LOW (pressed)");


//   Serial.print("Pause button on pin ");
//   Serial.print(PAUSE_BUTTON_PIN);
//   Serial.print(", initial state: ");
//   Serial.println(lastPauseButtonReading == HIGH ? "HIGH (not pressed)" : "LOW (pressed)");

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
  screenOn = true;       

  AudioMemory(30);

  if (!SD.begin(BUILTIN_SDCARD)) {
    Serial.println("SD init failed!");
    while (1) delay(500);
  }

  amp1.gain(audioGainDefault);

 

}


void showScreen(const char *file) {  // NEW

  Serial.println("Showing screen...");
  if (SD.exists(file)) {
    bmpDraw(&tft, file, 0, 0);
  } else {
    Serial.print("ERROR: BMP file not found: ");
    Serial.println(file);
    tft.fillScreen(RA8875_BLUE);
  }
}


void playAudio(const char *wavFileName) {
  // Stop any currently playing WAV
  //if (playWav1.isPlaying()) {
   // playWav1.stop();
   // delay(50);  // Brief delay to ensure stop completes
  //}

  Serial.print("Playing WAV ");
  Serial.println(wavFileName);
  
  if (SD.exists(wavFileName)) {
    playWav1.play(wavFileName);
  } else {
    Serial.print("ERROR: WAV file not found: ");
    Serial.println(wavFileName);
  }
}

bool nextButtonLoop() {
  
  // ====== Handle GREEN button (pin 4) with debounce ======
  digitalWrite(NEXT_LED_PIN, HIGH); 
  digitalWrite(PAUSE_LED_PIN, LOW);

  bool rawReading = digitalRead(NEXT_BUTTON_PIN);  // LOW = pressed (INPUT_PULLUP)

  // If the reading changed from last time, reset the debounce timer
  if (rawReading != lastNextButtonReading) {
    lastDebounceTime = GreenNow;
  }

    lastNextButtonReading = rawReading;
  // Has the reading been stable for long enough to be considered valid?
  if ((GreenNow - lastDebounceTime) > DEBOUNCE_DELAY) {
    // If the stable reading is different from the current debounced state
    if (rawReading != nextButtonState) {
      nextButtonState = rawReading;

      // We just got a clean transition
      // Detect a press on HIGH -> LOW (button down)
     // Transition: HIGH -> LOW = button pressed
      if (nextButtonState == LOW) {
        // Only do anything if screen is ON
        return true;
        }
    }
  }

  return false;

}


bool pauseButtonLoop() {
     // ====== Handle PAUSE button with debounce ======
  digitalWrite(NEXT_LED_PIN, LOW); 
  digitalWrite(PAUSE_LED_PIN, HIGH);

  bool rawPauseReading = digitalRead(PAUSE_BUTTON_PIN);  // LOW = pressed

  if (rawPauseReading != lastPauseButtonReading) {
    lastPauseDebounceTime = PauseNow;
  }

  lastPauseButtonReading = rawPauseReading;

  if ((PauseNow - lastPauseDebounceTime) > DEBOUNCE_DELAY) {
    if (rawPauseReading != pauseButtonState) {
      pauseButtonState = rawPauseReading;

      if (pauseButtonState == LOW) {
          return true;     // show pause.bmp
        } 
      }
    }

    return false;
}


bool verifyBatteryPercentage() {
    float voltage = moteus.last_result().values.voltage;
    float cell = voltage / 6.0f;

    if (cell < 3.60f) {
        DPRINTLN(F("Battery too low!"));
        return false;
    }
    DPRINTLN(F("Battery check passed!!!!"));
    return true;
}


void displayCutClothingInstructions() { 
    // TODO: Send HMI instructions for setup display
    DPRINTLN("Welcome to HeartBridge, your automated CPR device assistant. Call 911. Press NEXT when device is with patient.");
}

void displayUnfoldInstructions() { 
    // TODO: Send HMI instructions for setup display
    DPRINTLN("Welcome to HeartBridge, your automated CPR device assistant. Call 911. Press NEXT when device is with patient.");
}


void displayAlignmentInstructions() { 
    // TODO: Send HMI instructions for setup display
    DPRINTLN("Align device over bare chest and center it between the nipples. Press NEXT when done");
}


void displayZeroingInstructions(){
    DPRINTLN("Kneel on knee pads and and lean forward on handles. Use force to resist strong up and down movement. Press NEXT button to lower plunger to patient’s chest.");
}

bool displayCompressionConfirmation() {
    // TODO: Implement HMI display compression confirmation
    DPRINTLN("Performing compressions, continue holding frame");
    return true; // Placeholder return, wait till audio is finished
}