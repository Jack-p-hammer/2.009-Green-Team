#pragma once
#include <cstdint>
#include <Arduino.h>
#include <ACAN2517FD.h>
#include <Moteus.h>

// Declare variables that would be useful in other files

// State enumeration for state machine
enum cprState {
    STANDBY,
    HOMING,
    COMPRESSIONS
};

static const uint8_t MCP2517_SCK = 9 ; // SCK input of MCP2517
static const uint8_t MCP2517_SDI =  10 ; // SDI input of MCP2517
static const uint8_t MCP2517_SDO =  11 ; // SDO output of MCP2517

static const uint8_t MCP2517_CS  = 17 ; // CS input of MCP2517
static const uint8_t MCP2517_INT = 7 ; // INT output of MCP2517

extern ACAN2517FD can;

extern Moteus moteus;

// Initializer for motor driver, CAN bus, and sensors
void initializeMotor();