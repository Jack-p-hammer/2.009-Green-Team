#pragma once
#include <cstdint>
#include <ACAN2517FD.h>
#include <Moteus.h>

// Macro to disable print statements when DEBUG_PRINT is not defined in PIO settings
#ifdef DEBUG_PRINT
  // If you use these, then we can disable all prints at once (helpful)
  #define DPRINT(...) Serial.print(__VA_ARGS__)
  #define DPRINTLN(...) Serial.println(__VA_ARGS__)
#else
  #define DPRINT(...) ((void)0)
  #define DPRINTLN(...) ((void)0)
#endif

// State enumeration for state machine
enum cprState {
    START_UP,
    ZEROING,
    ZERO_FAILED,
    WAIT_FOR_COMPRESSION_CONFIRMATION,
    COMPRESSIONS,
    PAUSED,
    KNEEL_FAILURE,
    ABORT
};

// State variables (declared here, defined in a single .cpp)
extern cprState currentState;

// Track previous state to handle state transitions
extern cprState prevState;

// Control Loop Timing variables
// Declared here, defined in a single .cpp
extern uint32_t nextSendMillis;
extern uint16_t loopCount;

// Controller variables
extern double prevCommand;
extern double prevPrevCommand;
extern double prevError;
extern double prevPrevError;

// Control Loop Timing variables
extern uint32_t nextSendMillis;
extern uint16_t loopCount;
const uint8_t controller_period = 10;

static const uint8_t MCP2517_SCK = 13;//27;//13 ; // SCK input of MCP2517
static const uint8_t MCP2517_SDI =  11;//26; //11 ; // SDI input of MCP2517
static const uint8_t MCP2517_SDO =  12;//39; //12 ; // SDO output of MCP2517

static const uint8_t MCP2517_CS  = 10;//38; //10 ; // CS input of MCP2517
static const uint8_t MCP2517_INT = 9;//28; //9 ; // INT output of MCP2517

extern ACAN2517FD can;

extern Moteus moteus;

// Initializer for motor driver, CAN bus, and sensors
void initializeMotor();

// General functions

// Send controller command to motor
void sendCommands(double controlOutput);

// Print current motor status
void printStatus(uint32_t currentTime);
