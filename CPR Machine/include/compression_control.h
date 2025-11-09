#pragma once
#include <cstdint>

// Declare variables that would be useful in other files

// Controller period in ms
extern const uint8_t compression_controller_period;

// Declare function templates to be used for motor control

// Initialize Compressions
void initializeCompressions();

// Update motor controller with next compression control frame
void updateCompressions();

// Returns torque setpoint for current controller period
double computeCompressionSetpoint();

// class MotorControl {
// public:
//     MotorControl();

//     void begin();          // Initialize CAN, Moteus, etc.
//     void update();         // Called repeatedly in loop

// private:

//     static const byte MCP2517_SCK = 9 ; // SCK input of MCP2517
//     static const byte MCP2517_SDI =  10 ; // SDI input of MCP2517
//     static const byte MCP2517_SDO =  11 ; // SDO output of MCP2517
    
//     static const byte MCP2517_CS  = 17 ; // CS input of MCP2517
//     static const byte MCP2517_INT = 7 ; // INT output of MCP2517



//     // Driver objects
//     ACAN2517FD can;

//     // Moteus motors
//     Moteus moteus;

//     // Timing variables
//     uint32_t nextSendMillis = 0;
//     uint16_t loopCount = 0;

//     // Internal helpers
//     void sendCompressionCommands();
//     void printCompressionStatus();
// };
