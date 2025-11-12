#include "calibration_control.h"
#include <math.h>
#include <Arduino.h>
#include <ACAN2517FD.h>
#include <Moteus.h>
#include "control_scheme.h"
#include "sensors.h"

void initializeCalibration() {
    // Initialize outer loop
    nextSendMillis = millis();
    loopCount = 0;

    // Uses same controller as compression mode, with linear decreasing setpoint
}

void updateCalibration() {
    // We intend to send control frames every controller_period ms.
    const auto time = millis();

    // Only continue if it's time, may cause issues w/HMI integration when on one MCU
    if (nextSendMillis >= time) { return; } 

    nextSendMillis += controller_period;
    loopCount++;

    // Update sensor variables and error check
    readSensors();

    // Send control command
    sendCommands(updateController(computeCalibrationSetpoint()));

    // Only print status every 25th cycle.
    if (loopCount % 10 == 0) {
        printStatus(nextSendMillis);
    }
}

double computeCalibrationSetpoint() {
    // TODO: Refine compression setpoint
    // For now, same as Simulink for validation
    return 0.0;
}