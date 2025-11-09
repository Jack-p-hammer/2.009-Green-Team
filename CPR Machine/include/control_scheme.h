#pragma once
#include <cstdint>
#include <ACAN2517FD.h>
#include <Moteus.h>

// Control Loop Timing variables
extern uint32_t nextSendMillis;
extern uint16_t loopCount;
const uint8_t controller_period = 10;

static const uint8_t MCP2517_SCK = 9 ; // SCK input of MCP2517
static const uint8_t MCP2517_SDI =  10 ; // SDI input of MCP2517
static const uint8_t MCP2517_SDO =  11 ; // SDO output of MCP2517

static const uint8_t MCP2517_CS  = 17 ; // CS input of MCP2517
static const uint8_t MCP2517_INT = 7 ; // INT output of MCP2517

extern ACAN2517FD can;

extern Moteus moteus;

// Initializer for motor driver, CAN bus, and sensors
void initializeMotor();

// Initialize Compression Controller
void compressionControllerInit();

// Initialize Calibration Controller
void calibrationControllerInit();

// Retrieve new compression controller output from sensor data
// Requires setpoint input to allow for use in both calib/comp modes
double updateController(double setpoint);

// Send controller command to motor
void sendCommands(double controlOutput);

// Print current motor status
void printStatus(uint32_t currentTime);