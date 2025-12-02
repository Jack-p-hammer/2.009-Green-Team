#include <Arduino.h>

#include <Adafruit_RA8875.h>

#include <SPI.h>
#include <Wire.h>
#include <SD.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include "Adafruit_RA8875.h"
#include <Adafruit_STMPE610.h>

#include "sd_to_display.h"
#include <Audio.h>

AudioPlaySdWav playWav1;
AudioAmplifier   amp1;
AudioOutputI2S i2s1;
// AudioAmplifier   amp2;
// AudioOutputI2S i2s2;


// playWav1 -> amp1 -> i2s1 (L+R)
AudioConnection  patchCord1(playWav1, 0, amp1, 0);
AudioConnection  patchCord2(amp1, 0, i2s1, 0);  // left
AudioConnection  patchCord3(amp1, 0, i2s1, 1);  // right

// AudioConnection  patchCord4(playWav1, 0, amp2, 0);
// AudioConnection  patchCord5(amp2, 0, i2s2, 0);  // left
// AudioConnection  patchCord6(amp2, 0, i2s2, 1);  // right





// Pin definitions
const int SD_CHIP_SELECT = BUILTIN_SDCARD;
const int BUTTON_PIN = 28; //green "next/resume"
const int PAUSE_BUTTON_PIN = 26; //pause button 
const int POWER_BUTTON_PIN = 25; //start button 


const int BUTTON_LED_PIN = 29;
const int PAUSE_LED_PIN = 27;
int button_light_count = 0;

const int RA8875_CS = 16;
const int RA8875_RESET = 15;



// Display object
Adafruit_RA8875 tft = Adafruit_RA8875(RA8875_CS, RA8875_RESET);

const char *frameGroups[] = {
  "/bmp01/bare_chest.bmp",
  "/bmp02/ezgif-frame-002.bmp",
  "/bmp03/hold_down.bmp",
  "/bmp04/start_img.bmp",
};

// WAV files corresponding to each frame group
const char *wavFiles[] = {
  "1ExposeChest.WAV",   // Group 0
  "2AlignDevice.WAV",   // Group 1
  "3Kneel.WAV",   // Group 2
  "VoiceFile1.WAV",   // Group 3
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

// NEW: Pause button debounce
bool pauseButtonState = HIGH;
bool lastPauseButtonReading = HIGH;
unsigned long lastPauseDebounceTime = 0;


const unsigned long DEBOUNCE_DELAY = 5;  // ms - reduced for instant button response

// NEW: audio gain / pause state
float audioGainDefault = 0.7f;    // your chosen normal gain
bool audioPaused = false;

// NEW: path to pause image
const char *pauseBmpFile = "/bmp04/start_img.bmp";

void showPauseScreen() {  // NEW
  Serial.println("Showing pause screen...");
  if (SD.exists(pauseBmpFile)) {
    bmpDraw(&tft, pauseBmpFile, 0, 0);
  } else {
    Serial.print("ERROR: pause BMP not found: ");
    Serial.println(pauseBmpFile);
    tft.fillScreen(RA8875_BLUE);
  }
}



void playCurrentWav() {
  // Only play if not paused
  if (audioPaused) {
    return;  // Don't play audio if we're paused
  }

  // Stop any currently playing WAV
  if (playWav1.isPlaying()) {
    playWav1.stop();
    delay(50);  // Brief delay to ensure stop completes
  }
  
  const char *wavFilename = wavFiles[currentGroup];
  
  Serial.print("Playing WAV for group ");
  Serial.print(currentGroup);
  Serial.print(" -> ");
  Serial.println(wavFilename);
  
  if (SD.exists(wavFilename)) {
    amp1.gain(audioGainDefault);  // ensure gain is restored
    playWav1.play(wavFilename);
  } else {
    Serial.print("ERROR: WAV file not found: ");
    Serial.println(wavFilename);
  }
}

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

void showCurrentFrameAndAudio() {
  showCurrentFrame();
  playCurrentWav();
}

void toggleScreen() {
  if (screenOn) {
    // ----- TURN SCREEN OFF -----
    Serial.println("Turning screen OFF");
    screenOn = false;
    audioPaused = false;   //clear paused state


    // Stop any playing audio
    if (playWav1.isPlaying()) {
      playWav1.stop();
    }

    tft.PWM1out(0);        // backlight off
    tft.displayOn(false);  // disable display output
    // Optional: clear it so it's black if it ever flickers back on
    tft.graphicsMode();
    tft.fillScreen(RA8875_BLACK);
  } else {
    // ----- TURN SCREEN ON -----
    Serial.println("Turning screen ON");
    currentGroup = 0;  // Reset to first group when turning on
    screenOn = true;

    tft.displayOn(true);   // enable display
    tft.PWM1out(255);      // backlight full

    // Show first frame and start first WAV file
    showCurrentFrameAndAudio();
  }

}



void setup() {
  Serial.begin(9600);
  while (!Serial && millis() < 2000) {}

  // ---- Button setup ----
  pinMode(BUTTON_PIN, INPUT_PULLUP);   // button to GND, so LOW = pressed
  pinMode(POWER_BUTTON_PIN, INPUT_PULLUP);  // power button
  pinMode(PAUSE_BUTTON_PIN, INPUT_PULLUP);   // NEW: pause button


  lastButtonReading = digitalRead(BUTTON_PIN);  // Initialize button state
  lastPowerButtonReading = digitalRead(POWER_BUTTON_PIN);
  lastPauseButtonReading = digitalRead(PAUSE_BUTTON_PIN);  // NEW

  
  Serial.print("Button initialized on pin ");
  Serial.print(BUTTON_PIN);
  Serial.print(", initial state: ");
  Serial.println(lastButtonReading == HIGH ? "HIGH (not pressed)" : "LOW (pressed)");

  Serial.print("Power button on pin ");
  Serial.print(POWER_BUTTON_PIN);
  Serial.print(", initial state: ");
  Serial.println(lastPowerButtonReading == HIGH ? "HIGH (not pressed)" : "LOW (pressed)");

  Serial.print("Pause button on pin ");
  Serial.print(PAUSE_BUTTON_PIN);
  Serial.print(", initial state: ");
  Serial.println(lastPauseButtonReading == HIGH ? "HIGH (not pressed)" : "LOW (pressed)");



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

  tft.GPIOX(true);      // Enable TFT - display enable tied to GPIOX
  tft.PWM1config(true, RA8875_PWM_CLK_DIV1024); // PWM output for backlight
  
  Serial.print("(");
  Serial.print(tft.width());
  Serial.print(", ");
  Serial.print(tft.height());
  Serial.println(")");
  tft.graphicsMode();                 // go back to graphics mode
  tft.fillScreen(RA8875_BLACK);
  tft.graphicsMode();

  // Turn on the screen automatically on startup
  currentGroup = 0;
  screenOn = true;
  tft.displayOn(true);   // enable display
  tft.PWM1out(255);      // backlight full


  AudioMemory(30);

  if (!SD.begin(BUILTIN_SDCARD)) {
    Serial.println("SD init failed!");
    while (1) delay(500);
  }

  amp1.gain(audioGainDefault);
  audioPaused = false;

  // Show first frame and start first WAV file automatically on startup
  showCurrentFrameAndAudio();

}

void buttonBlink (int ButtonPin){
    // each time period of blink is 0.8s
    digitalWrite(ButtonPin, HIGH);
    delay(400);
    digitalWrite(ButtonPin, LOW);
    delay(400);
}

void loop() {
  // Get current time once for all buttons
  unsigned long GreenNow = millis();
  unsigned long PowerNow = millis();
  unsigned long PauseNow = millis();

  digitalWrite(BUTTON_LED_PIN, HIGH);

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
  
  // ====== 2) Handle GREEN button (pin 4) with debounce ======
  bool rawReading = digitalRead(BUTTON_PIN);  // LOW = pressed (INPUT_PULLUP)

  // If the reading changed from last time, reset the debounce timer
  if (rawReading != lastButtonReading) {
    lastDebounceTime = GreenNow;
  }

  // Has the reading been stable for long enough to be considered valid?
  if ((GreenNow - lastDebounceTime) > DEBOUNCE_DELAY) {
    // If the stable reading is different from the current debounced state
    if (rawReading != buttonState) {
      buttonState = rawReading;

      // We just got a clean transition
      // Detect a press on HIGH -> LOW (button down)
     // Transition: HIGH -> LOW = button pressed
      if (buttonState == LOW) {
        // Only do anything if screen is ON
        if (screenOn) {
            if (audioPaused) {
                // RESUME behavior
                Serial.println("Green button: RESUME from pause");
                audioPaused = false;
                showCurrentFrameAndAudio(); // redraw group frame and restart audio
            }else {
            currentGroup++;
            if (currentGroup >= 4) {
                currentGroup = 0;
            }

            Serial.print("Button press -> switched to group ");
            Serial.println(currentGroup);

          // Show the new group's frame and play corresponding WAV
          showCurrentFrameAndAudio();
        }
        } else {
          Serial.println("Group button pressed but screen is OFF; ignoring.");
        }
      }
    }
  }
  lastButtonReading = rawReading;

  // ====== 3) Handle PAUSE button with debounce ======
  bool rawPauseReading = digitalRead(PAUSE_BUTTON_PIN);  // LOW = pressed
  if (rawPauseReading != lastPauseButtonReading) {
    lastPauseDebounceTime = PauseNow;
  }

  if ((GreenNow - lastPauseDebounceTime) > DEBOUNCE_DELAY) {
    if (rawPauseReading != pauseButtonState) {
      pauseButtonState = rawPauseReading;

      if (pauseButtonState == LOW) {
        if (screenOn && !audioPaused) {
          // PAUSE: enter pause state (only if not already paused)
          Serial.println("Pause button: entering PAUSE state");
          audioPaused = true;

          if (playWav1.isPlaying()) {
            playWav1.stop();     // stop audio
          }
          amp1.gain(0.0f);       // ensure muted
          showPauseScreen();     // show pause.bmp
        } else if (!screenOn) {
          Serial.println("Pause button pressed but screen is OFF; ignoring.");
        } else if (audioPaused) {
          Serial.println("Pause button pressed but already paused. Use green button to resume.");
        }
      }
    }
  }

  // Save raw reading for next pass
  lastPauseButtonReading = rawPauseReading;


  
}
