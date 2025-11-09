#include "control_scheme.h"
#include <Moteus.h>
#include <ACAN2517FD.h>
#include <Arduino.h>

// Example placeholder PID-esque logic
// Assume 0 initial conditions
double prevCommand = 0;
double prevError = 0;

double errorGain = 109.1;
double prevErrorGain = 243.6;

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

void compressionControllerInit() {
  // You can set targetForce, PID gains, etc. here
}

void calibrationControllerInit() {
  // You can set targetForce, PID gains, etc. here
}

double updateController(double setpoint, double linearPos, double rotation) {
  double error = setpoint - linearPos;

  // See MATLAB file SimulinkSetup.mlx for controller in discrete TF form
  // THIS REQUIRES 10 ms CONTROLLER UPDATE PERIOD
  double output = errorGain*error - prevErrorGain*prevError + prevCommand;
  output = constrain(output, 0, 255);

  prevCommand = output;
  prevError = error;
  return output;
}

void sendCommands(double controlOutput) {
    // TODO: This needs to be changed into torque control!!!
    Moteus::PositionMode::Command cmd;
    cmd.position = NAN;
    cmd.velocity = 0.2 * ::sin(millis() / 1000.0);
    // cmd.feedforward_torque = controlOutput;

    moteus.SetPosition(cmd);
}

void printStatus(uint32_t currentTime){
    Serial.print(F("time "));
    Serial.print(currentTime);

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