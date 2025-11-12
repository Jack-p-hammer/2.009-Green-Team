#pragma once
#include <cstdint>

// Declare variables that would be useful in other files

// Declare function templates to be used for motor control

// Initialize Calibration
void initializeCalibration();

// Update motor controller with next callibration control frame
void updateCalibration();

// Returns torque setpoint for current controller period
double computeCalibrationSetpoint();