#pragma once
#include <cstdint>

// Declare variables that would be useful in other files

// Controller period in ms
extern const uint8_t compression_controller_period;

// Start time of compressions
extern long compression_start_time;

// Declare function templates to be used for motor control

// Initialize Compressions
void initializeCompressions();

// Update motor controller with next compression control frame
void updateCompressions();

// Retrieve new compression controller output from sensor data
// Requires setpoint input to allow for use in both zero/comp modes
double updateCompressionController(double setpoint);

// Returns rack position setpoint for current controller period,
// relative to linear encoder zero position
double computeCompressionSetpoint();

// Check for pause command
bool checkPauseCommand();

// Return to zero position if paused or recoverable error
void returnToCompressionZero(); // No need to

// Check for user pause
bool checkPauseCommand();