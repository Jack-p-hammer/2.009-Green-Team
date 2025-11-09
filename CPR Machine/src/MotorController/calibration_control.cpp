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
    if (nextSendMillis >= time) { return; }

    nextSendMillis += controller_period;
    loopCount++;

    readSensors();
    sendCommands(updateController(computeCalibrationSetpoint(), linearPos, rotaryPos));

    // Only print our status every 25th cycle.
    if (loopCount % 25 == 0) {
        printStatus(nextSendMillis);
    }
}

double computeCalibrationSetpoint() {
    // TODO: Define compression setpoint
    return 0.0;
}