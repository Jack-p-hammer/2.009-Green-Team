#include "sensors.h"
#include "control_scheme.h"
#include <Arduino.h>

// EXAMPLE PINS (FIX LATER!!!)
const int FORCE_PIN = A0;
const int LINEAR_ENCODER_A = 2;
const int LINEAR_ENCODER_B = 3;
const int ROTARY_ENCODER_A = 4;
const int ROTARY_ENCODER_B = 5;

const int forceCalibRate = 1187.7440; // Calibration constants for force sensor
const int forceCalibOffset = -548.7972;

// Assume 0 initial conditions
double linearPos = 0; double linearZeroPos = 0;
double rotaryPos = 0; double rotaryZeroPos = 0;
double forceVal = 0;

// Declare variables for sensor validation
const float pinionRadius = 0.01; // Meters

void initializeSensors() {
  // Set pins
  pinMode(FORCE_PIN, INPUT);
  pinMode(LINEAR_ENCODER_A, INPUT);
  pinMode(LINEAR_ENCODER_B, INPUT);
  pinMode(ROTARY_ENCODER_A, INPUT);
  pinMode(ROTARY_ENCODER_B, INPUT);

  // Record zero positions
  rotaryZeroPos = read_rotary_encoder();
  linearZeroPos = read_linear_encoder();
  // Force sensor is pre-calibrated
}

double read_force_sensor() {
  // Ensure that Voltage at the non-inverting terminal is less about 0.5V (voltage divider or sum shite)
  // If using a different reference voltage, recalibration is required
  int raw = analogRead(FORCE_PIN);
  float voltage = raw * (5.0 / 1023.0);
  float force = forceCalibRate * voltage + forceCalibOffset;
  return force; // should be value in newtons
}

double read_linear_encoder() {
  // TODO: Implement encoder reading logic
  // The following is dummy code
  double reading = analogRead(LINEAR_ENCODER_A);
  return reading - linearZeroPos; // always return zeroed value
}

void zeroLinearEncoder() {
  double reading = read_linear_encoder();
  linearZeroPos += reading; // Adjust zero position so that current value reads zero
}

double read_rotary_encoder() {
  // Read position from Moteus controller (rotary encoder feedback)
  // The moteus library continuously updates q_current.position (in revolutions)
  rotaryPos = moteus.last_result().values.position - rotaryZeroPos;
  return rotaryPos;
}
void zeroRotaryEncoder() {
  // Note that moteus encoder is semi-absolute: on power cycle, it will read an absolute position
  // to the nearest rotation (i.e. from -0.5 to 0.5). The moteus will also have its own position
  // limits based on rotaryPos, not the zeroed output we use internally. This, for all intents and
  // purposes, should be fine and adds another layer of safety
  double reading = read_rotary_encoder();
  linearZeroPos += reading; // Adjust zero position so that current value reads zero
}

void readSensors() {
    forceVal = read_force_sensor();
    linearPos = read_linear_encoder();
    rotaryPos = read_rotary_encoder(); // in revolutions

    // TODO: Encoder Validation
    double rotaryPosFromLinear = linearPos/pinionRadius; // in radians

    // Change tolerance based on linear encoder precision
    // must convert rotaryPos to radians
    if(abs(2*PI*rotaryPos - rotaryPosFromLinear) > 1e-3) {
      // TODO: Change from a print to throwing an exception
      Serial.println("ALERT: LINEAR - ROTARY MISMATCH");

    // Change limit from 550 to whatever we decide is a good worst-case limit
    } else if (forceVal > 550) {
      // TODO: Change from a print to a crash/set torque to 0 then crash
      Serial.println("ALERT: OVER FORCE");
    }
}