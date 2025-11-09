#include "compression_control.h"
#include <math.h>
#include <Arduino.h>
#include <ACAN2517FD.h>
#include <Moteus.h>
#include "control_scheme.h"
#include "sensors.h"

void initializeCompressions() {
    // Initialize outer loop
    nextSendMillis = 0;
    loopCount = 0;

    // Initialize controller (may not be used)
    compressionControllerInit();
}

void updateCompressions() {
    // We intend to send control frames every controller_period ms.
    const auto time = millis();
    if (nextSendMillis >= time) { return; }

    nextSendMillis += controller_period;
    loopCount++;

    readSensors();
    sendCommands(updateController(computeCompressionSetpoint(), linearPos, rotaryPos));

    // Only print our status every 25th cycle.
    if (loopCount % 25 == 0) {
        printStatus(nextSendMillis);
    }
}

double computeCompressionSetpoint() {
    // TODO: Define compression setpoint
    return 0.0;
}