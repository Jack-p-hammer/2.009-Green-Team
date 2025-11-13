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
    // // TODO: Refine compression time profile
    // // Simulink profile for now
    // long currentTime = millis() - compression_start_time;

    // // 2 Hz wave, starting at 0 and going down 2 inches
    // // SI UNITS!!!!!!!!!!!!!!
    // return 0.0508/2*::cos(2*PI*2*currentTime / 1000.0) - 0.0508/2;
    double computeCompressionSetpoint() {
    // Elapsed time in seconds since compression started
    double t = (millis() - compression_start_time) / 1000.0;
    double cycleTime = fmod(t, 0.56);  // One complete cycle = 0.56 s
    double y_cm = 0.0;                 // Compression position in centimeters

    // Piecewise trapezoidal profile (periodic)
    if (cycleTime < 0.12) {
        // Bottom hold
        y_cm = 0.0;
    } 
    else if (cycleTime < 0.24) {
        // Downstroke (compression phase)
        y_cm = -40.53 * cycleTime;
    } 
    else if (cycleTime < 0.323) {
        // Top hold
        y_cm = -5.0;
    } 
    else if (cycleTime < 0.56) {
        // Return stroke (release)
        y_cm = 21.15 * cycleTime;
    }

    // Convert to meters if needed by the rest of your control code
    // (comment this out if you want to stay in cm)
    double y_m = y_cm / 100.0;

    return y_m;
}

}