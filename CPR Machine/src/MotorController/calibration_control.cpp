#include "calibration_control.h"
#include <math.h>
#include <Arduino.h>
#include <ACAN2517FD.h>
#include <Moteus.h>
#include "control_scheme.h"
#include "sensors.h"

void initializeCalibration() {
    // Initialize outer loop
    nextSendMillis = 0;
    loopCount = 0;

    // Initialize controller (may not be used)
    calibrationControllerInit();
}

void updateCalibration() {
    // We intend to send control frames every controller_period ms.
    const auto time = millis();
    if (nextSendMillis >= time) { return; } // Only continue if it's time, may cause issues w/HMI integration on one MCU

    nextSendMillis += controller_period;
    loopCount++;

    readSensors();
    sendCommands(updateController(computeCalibrationSetpoint()));

    // Only print our status every 25th cycle.
    if (loopCount % 10 == 0) {
        printStatus(nextSendMillis);
    }
}

double computeCalibrationSetpoint() {
    // TODO: Refine compression setpoint
    // For now, same as Simulink for validation
    return 0.0;
}