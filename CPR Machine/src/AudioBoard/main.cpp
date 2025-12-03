#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <SD.h>
#include <Adafruit_STMPE610.h>
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

bool audioWasPlaying;

// NEW: path to pause image
const char *startUpWavFile = "/mainwav/startUpWav.wav";
const char *cutClothingWavFile = "/mainwav/cutClothingWav.wav";
const char *unfoldWavFile = "/mainwav/unfoldWav.wav";
const char *alignmentWavFile = "/mainwav/alignmentWav.wav";
const char *zeroingPrepWavFile = "/mainwav/zeroingPrepWav.wav";
const char *zeroingWavFile = "/mainwav/zeroingWav.wav";
const char *compressionPrepWavFile = "/mainwav/compressionPrepWav.wav";
const char *compressionsWavFile = "/mainwav/compressionsWav.wav";
const char *pausedWavFile = "/mainwav/pausedWav.wav";
const char *kneelFailureWavFile = "/mainwav/kneelFailureWav.wav";
const char *abortWavFile = "/mainwav/abortWav.wav";

enum cprState {
    START_UP,
    BATTERY_CHECK,
    CUT_CLOTHING,
    UNFOLD,
    ALIGNMENT,
    ZEROING_PREP,
    ZEROING,
    COMPRESSION_PREP,
    COMPRESSIONS,
    PAUSED,
    KNEEL_FAILURE,
    ABORT,
    NO_READING,
    UNKNOWN_READING
};

// Define a command for each wav file
// Arranged such that cprState can be used to index into this array
const char *audioCommands[] = {
    startUpWavFile,
    nullptr, // No audio for BATTERY_CHECK
    cutClothingWavFile,
    unfoldWavFile,
    alignmentWavFile,
    zeroingPrepWavFile,
    zeroingWavFile,
    compressionPrepWavFile,
    compressionsWavFile,
    pausedWavFile,
    kneelFailureWavFile,
    abortWavFile,
    nullptr, // No audio for NO_READING
    nullptr  // No audio for UNKNOWN_READING
};

const int SD_CHIP_SELECT = BUILTIN_SDCARD;

// NEW: audio gain / pause state
float audioGainDefault = 0.7f;    // your chosen normal gain
bool audioPaused = false;

cprState currentState = START_UP;

void setup() {
    // Use Serial port 1 for communication with MotorController/Screens
    Serial1.begin(115200);

    if (!SD.begin(SD_CHIP_SELECT)) {
        // SD init failed
        return;
    }
    
    AudioMemory(30);

    if (!SD.begin(BUILTIN_SDCARD)) {
        // SD init failed
        while (1) {
            if(millis() % 1000 < 500){
                digitalWrite(LED_BUILTIN, HIGH);
            } else {
                digitalWrite(LED_BUILTIN, LOW);
            }
        }
    }

    amp1.gain(audioGainDefault);

    // Hitch here until we read confirmation from Motor Controller
    while(true) {
        if (Serial1.available() > 0) {
            String input = Serial1.readStringUntil('\n');
            input.trim();
            if (input == "MC_READY") {
                currentState = START_UP;
                break;  // Exit the while loop
            }
        }
    }
}

void loop() {
    // Start each loop by checking Serial1
    currentState = parseUART();

    // Play audio corresponding to current state
    if (currentState != NO_READING && currentState != UNKNOWN_READING) {
        const char *wavFile = audioCommands[currentState];
        if (wavFile != nullptr) {
            playAudio(wavFile);
        }
    } else if (currentState == UNKNOWN_READING) {
        // FUCKED OUT
    }    
}

void playAudio(const char *wavFileName) {
  // Stop any currently playing WAV
  //if (playWav1.isPlaying()) {
   // playWav1.stop();
   // delay(50);  // Brief delay to ensure stop completes
  //}

    //   Serial.print("Playing WAV ");
    //   Serial.println(wavFileName);
  
  if (SD.exists(wavFileName)) {
    playWav1.play(wavFileName);
  } else {
    // FUCKED OUT
    //    Serial.print("ERROR: WAV file not found: ");
    //    Serial.println(wavFileName);
  }
}

cprState parseUART() {
    // Check to see if something new is on Serial1
    if (Serial1.available() > 0) {
        // Read the incoming string
        String input = Serial1.readStringUntil('\n');
        // Trim any trailing whitespace
        input.trim();

        // Match input string to known states
        if(input == "START_UP") {
            return START_UP;
        } else if(input == "BATTERY_CHECK") {
            return BATTERY_CHECK;
        } else if(input == "CUT_CLOTHING") {
            return CUT_CLOTHING;
        } else if(input == "UNFOLD") {
            return UNFOLD;
        } else if(input == "ALIGNMENT") {
            return ALIGNMENT;
        } else if(input == "ZEROING_PREP") {
            return ZEROING_PREP;
        } else if(input == "ZEROING") {
            return ZEROING;
        } else if(input == "COMPRESSION_PREP") {
            return COMPRESSION_PREP;
        } else if(input == "COMPRESSIONS") {
            return COMPRESSIONS;
        } else if(input == "PAUSED") {
            return PAUSED;
        } else if(input == "KNEEL_FAILURE") {
            return KNEEL_FAILURE;
        } else if(input == "ABORT") {
            return ABORT;
        } else {
            return UNKNOWN_READING;
        }
    }
    return NO_READING;
}