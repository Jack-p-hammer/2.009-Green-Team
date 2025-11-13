#pragma once
#include <cstdint>
#include <ACAN2517FD.h>
#include <Moteus.h>

// Macro to disable print statements when DEBUG_PRINT is not defined
#ifdef DEBUG_PRINT
  // Variadic macros allow forwarding optional format/base args like: DPRINT(val, HEX)
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

// State Machine functions
// TODO: Spin each state into its own file and header?

// Start Up
bool verifyBatteryPercentage();
void displaySetupInstructions();
bool checkUserStartConfirmation(); // True if user begins device operation

// Zeroing
// See zeroing_control.h

// Zeroing failure
void displayAlignmentConfirmation();
bool checkUserAlignmentConfirmation();
// TODO: Handle repeated entries to state having different behavior?

// Waiting for compression confirmation
void displayCompressionConfirmation();
bool checkUserCompressionConfirmation();

// Compressions
// See compression_control.h

// Paused
void displayPauseMessage();
bool isPaused(); // Returns true if the system is currently paused

// Kneeling failure
void displayKneelFailureMessage();
bool isKneelingFailure(); // Returns true if the system is currently in kneeling failure state

// Abort
void displayAbortMessage();
void updateAbort(); // Return motor to zero position ASAP

// General functions

// Retrieve new compression controller output from sensor data
// Requires setpoint input to allow for use in both zero/comp modes
double updateController(double setpoint);

// Send controller command to motor
void sendCommands(double controlOutput);

// Print current motor status
void printStatus(uint32_t currentTime);
