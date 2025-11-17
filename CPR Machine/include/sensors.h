#pragma once

#include "Adafruit_VL53L0X.h"

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
extern const double forceCalibRate;
extern const double forceCalibOffset;

// ToF Sensor
extern Adafruit_VL53L0X ToFSensor;

// Declare variables for sensor validation
extern const float pinionRadius; // Meters

// Sensor function definitions

// Setup sensor pins and zero sensors in position
bool initializeSensors();

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
// This also does error checking, so bool
bool readSensors();

// IMU Sensor
// Return IMU reading (T/F), roll, pitch, yaw in degrees
bool read_imu();
extern float imu_ax, imu_ay, imu_az;