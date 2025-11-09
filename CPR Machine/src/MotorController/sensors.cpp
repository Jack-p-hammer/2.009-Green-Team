#include "sensors.h"
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
double linearPos = 0;
double rotaryPos = 0;
double forceVal = 0;

void sensors_init() {
  pinMode(FORCE_PIN, INPUT);
  pinMode(LINEAR_ENCODER_A, INPUT);
  pinMode(LINEAR_ENCODER_B, INPUT);
  pinMode(ROTARY_ENCODER_A, INPUT);
  pinMode(ROTARY_ENCODER_B, INPUT);
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
  

  return 0.0;
}

double read_rotational_encoder() {
  // TODO: Implement encoder reading logic
  return 0.0;
}

void readSensors() {
    // Read sensor values
    // TODO: Change return type from void based on how we implement
    forceVal = read_force_sensor();
    linearPos = read_linear_encoder();
    rotaryPos = read_rotational_encoder();
}