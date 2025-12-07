#pragma once
// #include <cstdint>
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
    START_UP_BATTERY,
    UNFOLD_EXPOSE,
    ALIGNMENT,
    ZEROING_PREP,
    ZEROING,
    COMPRESSION_PREP,
    COMPRESSIONS,
    PAUSED,
    KNEEL_FAILURE,
    ABORT
};

// State enumeration for state machine
enum controlMode {
    POSITION,
    VELOCITY,
    TORQUE,
    RETRACT_POSITION,
    ZEROING_POSITION
};
// State variables (declared here, defined in a single .cpp)
extern cprState currentState;

// Track previous state to handle state transitions
extern cprState prevState;

extern controlMode control_mode;

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

// static const uint8_t MCP2517_SCK = 13 ; // SCK input of MCP2517
// static const uint8_t MCP2517_SDI =  11 ; // SDI input of MCP2517
// static const uint8_t MCP2517_SDO =  12 ; // SDO output of MCP2517

// static const uint8_t MCP2517_CS  = 10 ; // CS input of MCP2517
// static const uint8_t MCP2517_INT = 9 ; // INT output of MCP2517

static const byte MCP2517_SCK = 27;//13 ; // SCK input of MCP2517
static const byte MCP2517_SDI =  26;//11 ; // MOSI SDI input of MCP2517
static const byte MCP2517_SDO =  1;//12 ; // MISO SDO output of MCP2517

static const byte MCP2517_CS  = 10 ; // CS input of MCP2517
static const byte MCP2517_INT = 9 ; // INT output of MCP2517
//static uint32_t gNextSendMillis = 0;

extern ACAN2517FD can;

extern Moteus moteus;

// Initializer for motor driver, CAN bus, and sensors
void initializeMotor();

// General functions

// Send controller command to motor
void sendCommands(double controlOutput, controlMode control_mode);

// Print current motor status
void printStatus(uint32_t currentTime);
