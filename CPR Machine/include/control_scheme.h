#pragma once

// Control Loop Timing variables
extern uint32_t nextSendMillis = 0;
extern uint16_t loopCount = 0;

// Initialize Compression Controller
void compressionControllerInit();

// Initialize Calibration Controller
void calibrationControllerInit();

// Retrieve new compression controller output from sensor data
double updateCompressionController(double setpoint, double linearPos, double rotation);

// Retrieve new calibration controller output from sensor data
double updateCalibrationController(double setpoint, double linearPos, double rotation);