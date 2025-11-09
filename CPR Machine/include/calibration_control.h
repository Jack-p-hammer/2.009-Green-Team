#pragma once
#include <cstdint>

// Declare variables that would be useful in other files

// Controller period in ms
extern const uint8_t calibration_controller_period;

// Declare function templates to be used for motor control

// Initialize Calibration
void initializeCalibration();

// Update motor controller with next callibration control frame
void updateCalibration();

// Returns torque setpoint for current controller period
double computeCalibrationSetpoint();

// Helper functions, unique as to not define them multiple times
void sendCalibrationCommands(double controlOutput);
void printCalibrationStatus();