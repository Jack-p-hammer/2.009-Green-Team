#pragma once
#include <cstdint>
#include <ACAN2517FD.h>
#include <Moteus.h>



// Declare variables that would be useful in other files
extern const double extensionStrokeLimit;

// Declare function templates to be used for zeroing control

// Initialize Zeroing
void initializeZeroing();

// Update motor controller with next zeroing control frame
bool updateZeroing();

// Retrieve new zeroing controller output from sensor data
// Requires setpoint input to allow for use in both zero/comp modes
double updateZeroingController(double setpoint);

// Return to zero position if paused or recoverable error
void returnToPreZeroingZero(); // No need to

// Returns torque setpoint for current controller period
double computeZeroingSetpoint();
