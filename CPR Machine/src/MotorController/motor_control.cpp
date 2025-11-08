#include "motor_control.h"
#include <math.h>
#include <Arduino.h>
#include <ACAN2517FD.h>
#include <Moteus.h>

static const byte MCP2517_SCK = 9 ; // SCK input of MCP2517
static const byte MCP2517_SDI =  10 ; // SDI input of MCP2517
static const byte MCP2517_SDO =  11 ; // SDO output of MCP2517

static const byte MCP2517_CS  = 17 ; // CS input of MCP2517
static const byte MCP2517_INT = 7 ; // INT output of MCP2517

const byte controller_period = 20; // Period between controller frames

// Driver objects
ACAN2517FD can(MCP2517_CS, SPI, MCP2517_INT);

// Moteus motors
Moteus moteus(can, []() {
  Moteus::Options options;
  options.id = 1;
  return options;
}());;

// Timing variables
uint32_t nextSendMillis = 0;
uint16_t loopCount = 0;

// Internal helpers
void sendCommands();
void printStatus();

void initializeMotor() {
    pinMode(LED_BUILTIN, OUTPUT);

    Serial.begin(115200);
    while (!Serial) {}
    Serial.println(F("MotorControl: initializeMotor()"));

    SPI.begin();

    // CAN-FD configuration
    ACAN2517FDSettings settings(
        ACAN2517FDSettings::OSC_40MHz, 1000ll * 1000ll, DataBitRateFactor::x1);

    settings.mArbitrationSJW = 2;
    settings.mDriverTransmitFIFOSize = 1;
    settings.mDriverReceiveFIFOSize = 2;

    // This line begins CAN communication
    const uint32_t errorCode = can.begin(settings, [] {can.isr();});

    if (errorCode != 0) {
        Serial.print(F("CAN error 0x"));
        Serial.println(errorCode, HEX);
        while (true) { delay(1000); }
    }

    // To clear any faults the controllers may have, we start by sending
    // a stop command to each.
    moteus.SetStop();
    Serial.println(F("Motor stopped"));
}

void updateMotor() {
    // We intend to send control frames every 10ms.
    const auto time = millis();
    if (nextSendMillis >= time) { return; }

    nextSendMillis += controller_period;
    loopCount++;

    sendCommands();

    // Only print our status every 5th cycle, so every 1s.
    if (loopCount % 5 == 0) {
        printStatus();
    }
}

void sendCommands() {
    // This needs to be changed into torque control!!!
    Moteus::PositionMode::Command cmd;
    cmd.position = NAN;
    cmd.velocity = 0.2 * ::sin(millis() / 1000.0);

    moteus.SetPosition(cmd);
}

void printStatus() {
    Serial.print(F("time "));
    Serial.print(nextSendMillis);

    auto print_moteus = [](const Moteus::Query::Result& q) {
        Serial.print(static_cast<int>(q.mode));
        Serial.print(F(" pos "));
        Serial.print(q.position);
        Serial.print(F(" vel "));
        Serial.print(q.velocity);
    };

    print_moteus(moteus.last_result().values);
    Serial.print(F(" / "));

}
