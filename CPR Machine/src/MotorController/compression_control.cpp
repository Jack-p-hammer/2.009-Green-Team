#include "compression_control.h"
#include <math.h>
#include <Arduino.h>
#include <ACAN2517FD.h>
#include <Moteus.h>
#include "control_scheme.h"
#include "sensors.h"

long compression_start_time = 0;

void initializeCompressions() {
    // Initialize outer loop
    nextSendMillis = millis();
    loopCount = 0;

    // Record time start of compressions
    compression_start_time = millis();
}

void updateCompressions() {
    // We intend to send control frames every controller_period ms.
    const auto time = millis();
    if (nextSendMillis >= time) { return; }

    nextSendMillis += controller_period;
    loopCount++;

    readSensors();
    sendCommands(updateController(computeCompressionSetpoint()));

    // Only print our status every 25th cycle.
    if (loopCount % 25 == 0) {
        printStatus(nextSendMillis);
    }
}

double computeCompressionSetpoint() {
    // TODO: Refine compression time profile
    // Simulink profile for now
    long currentTime = millis() - compression_start_time;

    // 2 Hz wave, starting at 0 and going down 2 inches
    // SI UNITS!!!!!!!!!!!!!!
    return 0.0508/2*::cos(2*PI*2*currentTime / 1000.0) - 0.0508/2;
}