#include "zeroing_control.h"
#include <math.h>
#include <Arduino.h>
#include <ACAN2517FD.h>
#include <Moteus.h>
#include "control_scheme.h"
#include "sensors.h"

const double extensionStrokeLimit = 0.0254*10; // 10 inches, in meters

void initializeZeroing() {
    // Initialize outer loop
    nextSendMillis = millis();
    loopCount = 0;

    // Uses same controller as compression mode, with linear decreasing setpoint
}

bool updateZeroing() {
    // We intend to send control frames every controller_period ms.
    const auto time = millis();

    // Only continue if it's time, may cause issues w/HMI integration when on one MCU
    if (nextSendMillis >= time) { return false; } // Return false because not zeroed yet

    nextSendMillis += controller_period;
    loopCount++;

    // Update sensor variables and error check
    // readSensors() updates global variables, so call inside if statement works 
    // as if outside the if
    if(!readSensors()) {
        prevState = currentState;
        currentState = ABORT;
        return false;
    }

    // Check for zeroing failure conditions
    if(linearPos > extensionStrokeLimit) {
        prevState = currentState;
        currentState = ZERO_FAILED;
        return false;
    }

    // Check for successful zeroing
    // TODO: Refine zeroing setpoint
    if(forceVal >= 10) {
        // Handle state change in main state machine, just return true for now
        return true;
    }

    // Send control command
    sendCommands(updateController(computeZeroingSetpoint()));

    // Only print status every 25th cycle.
    if (loopCount % 10 == 0) {
        printStatus(nextSendMillis);
    }

    // No errors, return false because setpoint not found
    return false;
}

double computeZeroingSetpoint() {
    // TODO: Refine zeroing setpoint
    // For now, nothin'
    return 0.0;
}