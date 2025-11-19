#include "compression_control.h"
#include <math.h>
#include <Arduino.h>
#include <ACAN2517FD.h>
#include <Moteus.h>
#include "control_scheme.h"
#include "sensors.h"
#include <assert.h>

long compression_start_time = 0;

// Lag controller gains
double errorGain = 257.6539;//-117.9813; // This is the constant term of the TF numerator, with sign
double prevErrorGain = -173.3746; //244.9501; // This is the z term of the TF numerator

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
    sendCommands(updateCompressionController(computeCompressionSetpoint()));

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
        outputPos_cm = -21.0965609 * (cycleTime - 0.323) + 5.0;
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

void returnToCompressionZero() {
    // For whatever reason, we need to go to our calibrated zero
    // If pre-calibration, this defaults to position on startup

    // We intend to send control frames every controller_period ms.
    const auto time = millis();
    if (nextSendMillis >= time) { return; }

    nextSendMillis += controller_period;
    loopCount++;

    readSensors();
    sendCommands(updateCompressionController(0));

    // Only print our status every 25th cycle.
    if (loopCount % 25 == 0) {
        printStatus(nextSendMillis);
    }

}

double updateCompressionController(double setpoint_m) {
  readSensors();

  // Take setpoint relative to linear zero
  double error = (setpoint_m - linearZeroPos) - linearPos;
  //double error = setpoint_m - rotaryPos*(2*PI)*pinionRadius;

  // See MATLAB file SimulinkSetup.mlx for controller in discrete TF form
  // THIS REQUIRES 10 ms CONTROLLER UPDATE PERIOD
  assert(controller_period == 10);
  double torqueOutput = errorGain*error + prevErrorGain*prevError + prevCommand;

  // Saturate the control effort
  torqueOutput = constrain(torqueOutput, -5, 5);

  prevCommand = torqueOutput;
  prevError = error;
 
  DPRINT(">");
  DPRINT("Setpoint:"); DPRINT(setpoint_m);
  DPRINT(",");  
  DPRINT("ERROR:"); DPRINT(error);
  DPRINT(",");
  DPRINT("ROTARY POS:"); DPRINT(rotaryPos);
  DPRINT(",");
  DPRINT("TORQUE:"); DPRINT(torqueOutput);
  DPRINT(" | STATE: "); DPRINTLN(currentState);

  return torqueOutput;
}