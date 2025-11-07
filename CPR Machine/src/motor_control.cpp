#include "motor_control.h"
#include <math.h>

// Constructor initialization list
MotorControl::MotorControl()
    : can(MCP2517_CS, SPI, MCP2517_INT),
      moteus(can, []() {
          Moteus::Options options;
          options.id = 1;
          return options;
      }()) {}

void MotorControl::begin() {
    pinMode(LED_BUILTIN, OUTPUT);

    Serial.begin(115200);
    while (!Serial) {}
    Serial.println(F("MotorControl: begin()"));

    SPI.begin();

    // CAN-FD configuration
    ACAN2517FDSettings settings(
        ACAN2517FDSettings::OSC_20MHz, 1000ll * 1000ll, DataBitRateFactor::x1);

    settings.mArbitrationSJW = 2;
    settings.mDriverTransmitFIFOSize = 1;
    settings.mDriverReceiveFIFOSize = 2;

    // Commented this out because it kept giving me issues, and it's really only to throw an error code
    // const uint32_t errorCode = can.begin(settings, [] {can.isr();});
    // if (errorCode != 0) {
    //     Serial.print(F("CAN error 0x"));
    //     Serial.println(errorCode, HEX);
    //     while (true) { delay(1000); }
    // }

    // To clear any faults the controllers may have, we start by sending
    // a stop command to each.
    moteus.SetStop();
    Serial.println(F("Motor stopped"));
}

void MotorControl::update() {
    // We intend to send control frames every 20ms.
    const auto time = millis();
    if (nextSendMillis >= time) { return; }

    nextSendMillis += 20;
    loopCount++;

    sendCommands();

    // Only print our status every 5th cycle, so every 1s.
    if (loopCount % 5 == 0) {
        printStatus();
    }
}

void MotorControl::sendCommands() {
    // This needs to be changed into torque control!!!
    Moteus::PositionMode::Command cmd;
    cmd.position = NAN;
    cmd.velocity = 0.2 * ::sin(millis() / 1000.0);

    moteus.SetPosition(cmd);
}

void MotorControl::printStatus() {
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
