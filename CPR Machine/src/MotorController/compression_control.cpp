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
    // Elapsed time in seconds since compression started
    double time_sec = (millis() - compression_start_time) / 1000.0;
    double cycleTime = fmod(time_sec, 0.56);  // One complete cycle = 0.56 s
    double outputPos_cm = 0.0;                 // Compression position in centimeters

    // Piecewise trapezoidal profile (periodic)
    // Positive rotation of motor is down on rack, so down is positive up is negative
    if (cycleTime < 0.12) {
        // Bottom hold
        outputPos_cm = 0.0;
    } 
    else if (cycleTime < 0.24) {
        // Downstroke (compression phase)
        outputPos_cm = 41.6666667 * (cycleTime - .12);
    } 
    else if (cycleTime < 0.323) {
        // Top hold
        outputPos_cm = 5.0;
    } 
    else if (cycleTime < 0.56) {
        // Return stroke (release)
        outputPos_cm = -21.0965609 * (cycleTime - 0.323) - 5.0;
    }

    // Convert to meters if needed by the rest of your control code
    // SI UNITS!!!!!!!!!!!!!!
    double outputPos_m = outputPos_cm / 100.0;
   
    return outputPos_m;
}

bool checkPauseCommand() {
    // TODO: Implement pause command check
    return false;
}

void returnToZero() {
    // For whatever reason, we need to go to our calibrated zero
    // If pre-calibration, this defaults to position on startup

        // We intend to send control frames every controller_period ms.
    const auto time = millis();
    if (nextSendMillis >= time) { return; }

    nextSendMillis += controller_period;
    loopCount++;

    readSensors();
    sendCommands(updateController(0));

    // Only print our status every 25th cycle.
    if (loopCount % 25 == 0) {
        printStatus(nextSendMillis);
    }

}