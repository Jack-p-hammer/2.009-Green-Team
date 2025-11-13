#pragma once
#include <cstdint>

// Declare variables that would be useful in other files

// Declare function templates to be used for zeroing control

// Initialize Zeroing
void initializeZeroing();

// Update motor controller with next zeroing control frame
void updateZeroing();

// Returns torque setpoint for current controller period
double computeZeroingSetpoint();