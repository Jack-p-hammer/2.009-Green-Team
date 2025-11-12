#pragma once
#include <cstdint>
#include <ACAN2517FD.h>
#include <Moteus.h>

// State enumeration for state machine
enum cprState {
    STANDBY,
    HOMING,
    WAIT_FOR_CONFIRMATION,
    COMPRESSIONS
};

// State variables (declared here, defined in a single .cpp)
extern cprState currentState;
extern cprState prevState;

// Control Loop Timing variables
// Declared here, defined in a single .cpp
extern uint32_t nextSendMillis;
extern uint16_t loopCount;


// Control Loop Timing variables
extern uint32_t nextSendMillis;
extern uint16_t loopCount;
const uint8_t controller_period = 10;

static const uint8_t MCP2517_SCK = 13 ; // SCK input of MCP2517
static const uint8_t MCP2517_SDI =  11 ; // SDI input of MCP2517
static const uint8_t MCP2517_SDO =  12 ; // SDO output of MCP2517

static const uint8_t MCP2517_CS  = 10 ; // CS input of MCP2517
static const uint8_t MCP2517_INT = 9 ; // INT output of MCP2517

extern ACAN2517FD can;

extern Moteus moteus;

// Initializer for motor driver, CAN bus, and sensors
void initializeMotor();

// Retrieve new compression controller output from sensor data
// Requires setpoint input to allow for use in both calib/comp modes
double updateController(double setpoint);

// Send controller command to motor
void sendCommands(double controlOutput);

// Print current motor status
void printStatus(uint32_t currentTime);