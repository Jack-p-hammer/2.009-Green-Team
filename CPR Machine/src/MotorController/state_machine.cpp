#include <math.h>
#include <Arduino.h>
#include <ACAN2517FD.h>
#include <Moteus.h>
#include "state_machine.h"

// Driver object
ACAN2517FD can(MCP2517_CS, SPI, MCP2517_INT);

// Moteus motor
Moteus moteus(can, []() {
  Moteus::Options options;
  options.id = 1;
  return options;
}());;

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