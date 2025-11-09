#pragma once

// Sensor Pins

extern const int FORCE_PIN;
extern const int LINEAR_ENCODER_A;
extern const int LINEAR_ENCODER_B;
extern const int ROTARY_ENCODER_A;
extern const int ROTARY_ENCODER_B;

// Sensor outputs

extern double linearPos; extern double linearZeroPos;
extern double rotaryPos; extern double rotaryZeroPos;
extern double forceVal;

// Force Sensor Calibration Values

extern const int forceCalibRate;
extern const int forceCalibOffset;

// Sensor function definitions

// Setup sensor pins and zero sensors in position
void initializeSensors();

// Return force sensor reading
double read_force_sensor();

// Return linear encoder reading, relative to zero position
double read_linear_encoder();

// Set zero of linear encoder to current position
void zeroLinearEncoder();

// Return rotary encoder reading, relative to zero position
double read_rotary_encoder();

// Set zero of rotary encoder to current position
void zeroRotaryEncoder();

// Read all sensor values, zeroed, and store them in global variables
void readSensors();