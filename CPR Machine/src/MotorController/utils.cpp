#include "utils.h"
#include <Arduino.h>
#include "sensors.h"
#include "control_scheme.h"
#include <Arduino.h>
#include "Adafruit_VL53L0X.h"

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

bool checkUserConfirmation() {  
    // TODO: Implement HMI user confirmation check
    DPRINTLN("Waiting for user start confirmation...");
    return true;
}

void displaySetupInstructions() { 
    // TODO: Send HMI instructions for setup display
    DPRINTLN("Expose bare chest and remove bra. Press next when done");
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