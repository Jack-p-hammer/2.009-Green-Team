#pragma once

#include <Arduino.h>
#include <ACAN2517FD.h>
#include <Moteus.h>

class MotorControl {
public:
    MotorControl();

    void begin();          // Initialize CAN, Moteus, etc.
    void update();         // Called repeatedly in loop

private:

    static const byte MCP2517_SCK = 9 ; // SCK input of MCP2517
    static const byte MCP2517_SDI =  10 ; // SDI input of MCP2517
    static const byte MCP2517_SDO =  11 ; // SDO output of MCP2517
    
    static const byte MCP2517_CS  = 17 ; // CS input of MCP2517
    static const byte MCP2517_INT = 7 ; // INT output of MCP2517



    // Driver objects
    ACAN2517FD can;

    // Moteus motors
    Moteus moteus;

    // Timing variables
    uint32_t nextSendMillis = 0;
    uint16_t loopCount = 0;

    // Internal helpers
    void sendCommands();
    void printStatus();
};
