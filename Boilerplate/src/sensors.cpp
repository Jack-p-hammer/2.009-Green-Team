#include "sensors.h"
#include <Arduino.h>

// EXAMPLE PINS (FIX LATER!!!)
const int FORCE_PIN = A0;
const int LINEAR_ENCODER_A = 2;
const int LINEAR_ENCODER_B = 3;
const int ROTARY_ENCODER_A = 4;
const int ROTARY_ENCODER_B = 5;

const int a = 1187.7440; // Calibration constants for force sensor
const int b = -548.7972;

void sensors_init() {
  pinMode(FORCE_PIN, INPUT);
  pinMode(LINEAR_ENCODER_A, INPUT);
  pinMode(LINEAR_ENCODER_B, INPUT);
  pinMode(ROTARY_ENCODER_A, INPUT);
  pinMode(ROTARY_ENCODER_B, INPUT);
}

float read_force_sensor() {
  // Ensure that Voltage at the non-inverting terminal is less about 0.5V (voltage divider or sum shite)
  // If using a different reference voltage, recalibration is required
  int raw = analogRead(FORCE_PIN);
  float voltage = raw * (5.0 / 1023.0);
  float force = a * voltage + b;
  return force; // should be value in newtons
}

float read_linear_encoder() {
  // TODO: Implement encoder reading logic
  

  return 0.0;
}

float read_rotational_encoder() {
  // TODO: Implement encoder reading logic
  return 0.0;
}
