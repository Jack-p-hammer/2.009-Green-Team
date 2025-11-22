#include "start_up.h"
#include <Arduino.h>
#include "sd_to_display.h"

bool verifyBatteryPercentage() {
    // TODO: Implement battery percentage verification
    return true;
}

void displaySetupInstructions() { 
    // TODO: Send HMI instructions for setup display
    attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), NextFrame , FALLING);
}

bool checkUserStartConfirmation() {  
    // TODO: Implement HMI user confirmation check
    return true;
}