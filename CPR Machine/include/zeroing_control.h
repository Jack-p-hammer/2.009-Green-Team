#pragma once
#include <cstdint>

// Declare variables that would be useful in other files
extern const double extensionStrokeLimit;

// Declare function templates to be used for zeroing control

// Initialize Zeroing
void initializeZeroing();

// Update motor controller with next zeroing control frame
bool updateZeroing();

// Returns torque setpoint for current controller period
double computeZeroingSetpoint();