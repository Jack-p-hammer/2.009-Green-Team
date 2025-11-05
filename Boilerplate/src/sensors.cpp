#include "sensors.h"
#include <Arduino.h>

// EXAMPLE PINS (FIX LATER!!!)
const int FORCE_PIN = A0;
const int LINEAR_ENCODER_A = 2;
const int LINEAR_ENCODER_B = 3;
const int ROTARY_ENCODER_A = 4;
const int ROTARY_ENCODER_B = 5;

void sensors_init() {
  pinMode(FORCE_PIN, INPUT);
  pinMode(LINEAR_ENCODER_A, INPUT);
  pinMode(LINEAR_ENCODER_B, INPUT);
  pinMode(ROTARY_ENCODER_A, INPUT);
  pinMode(ROTARY_ENCODER_B, INPUT);
}

float read_force_sensor() {
  int raw = analogRead(FORCE_PIN);
  float voltage = raw * (5.0 / 1023.0); // https://docs.arduino.cc/built-in-examples/basics/ReadAnalogVoltage/
  return voltage; // Convert to Newtons later!
}

float read_linear_encoder() {
  // TODO: Implement encoder reading logic
  

  return 0.0;
}

float read_rotational_encoder() {
  // TODO: Implement encoder reading logic
  return 0.0;
}
