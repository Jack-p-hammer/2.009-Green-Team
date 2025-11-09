#include "calibration_control.h"
#include "state_machine.h"
#include <math.h>
#include <Arduino.h>
#include <ACAN2517FD.h>
#include <Moteus.h>
#include "control_scheme.h"
#include "sensors.h"

// TODO: How should we control calibration phase?
const uint8_t calibration_controller_period = 20;

void initializeCalibration() {
    nextSendMillis = 0;
    loopCount = 0;
    calibrationControllerInit();
}

void updateCalibration() {
    // We intend to send control frames every controller_period ms.
    const auto time = millis();
    if (nextSendMillis >= time) { return; }

    nextSendMillis += calibration_controller_period;
    loopCount++;

    readSensors();
    sendCalibrationCommands(updateCompressionController(computeCalibrationSetpoint(), linearPos, rotaryPos));

    // Only print our status every 25th cycle.
    if (loopCount % 25 == 0) {
        printCalibrationStatus();
    }
}

void sendCalibrationCommands(double controlOutput) {
    // This needs to be changed into torque control!!!
    Moteus::PositionMode::Command cmd;
    cmd.position = NAN;
    cmd.velocity = 0.2 * ::sin(millis() / 1000.0);
    // cmd.feedforward_torque = controlOutput;

    moteus.SetPosition(cmd);
}

void printCalibrationStatus() {
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