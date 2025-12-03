#include "control_scheme.h"
#include "sensors.h"
#include <Moteus.h>
#include <ACAN2517FD.h>
#include <Arduino.h>
#include <assert.h>
#include <zeroing_control.h>

// State machine variables
#if defined(COMPRESSION_TEST)
cprState currentState = COMPRESSIONS;
cprState prevState = COMPRESSIONS;
#elif defined(ZEROING_TEST)
cprState currentState = ZEROING;
cprState prevState = START_UP;
#else
cprState currentState = BATTERY_CHECK;
cprState prevState = BATTERY_CHECK;
#endif

// Control Loop Timing variables
uint32_t nextSendMillis = 0;
uint16_t loopCount = 0;

// Controller variables
double prevCommand = 0;
double prevError = 0;
double prevPrevCommand = 0;
double prevPrevError = 0;

// Driver object
ACAN2517FD can(MCP2517_CS, SPI1, MCP2517_INT);

// Moteus motor
Moteus moteus(can, []() {
  Moteus::Options options;
  options.id = 1;
  return options;
}());

// Initialize PositionMode::Format at global scope using a lambda so the
// assignment is performed during initialization rather than as a standalone
// statement (which is invalid at file scope).
const Moteus::PositionMode::Format positionModeOptions = []() {
  Moteus::PositionMode::Format options;
  options.feedforward_torque = Moteus::kFloat;
  options.kd_scale = Moteus::kFloat;
  options.kp_scale = Moteus::kFloat;
  return options;
}();

void initializeMotor() {
    pinMode(LED_BUILTIN, OUTPUT);

    Serial.begin(115200);
    // TODO: Remove this delay for M6
    while (!Serial) {}
    DPRINTLN(F("control_scheme: initializeMotor()"));

    SPI1.setMOSI(26); 
    SPI1.setMISO(1);
    SPI1.setSCK(27);

    SPI1.begin();

    // CAN-FD configuration
    ACAN2517FDSettings settings(
        ACAN2517FDSettings::OSC_40MHz, 1000ll * 1000ll, DataBitRateFactor::x1);

    settings.mArbitrationSJW = 2;
    settings.mDriverTransmitFIFOSize = 1;
    settings.mDriverReceiveFIFOSize = 2;

    // This line begins CAN communication
    const uint32_t errorCode = can.begin(settings, [] {can.isr();});

    if (errorCode != 0) {
        DPRINT(F("CAN error 0x"));
        DPRINTLN(errorCode, HEX);
        while (true) { delay(1000); }
    }

    // To clear any faults the controllers may have, we start by sending
    // a stop command to each.
    moteus.SetStop();
    DPRINTLN(F("Motor stopped"));
}


void sendCommands(double controlOutput, controlMode control_mode) {
    // TODO: This needs to be changed into torque control!!!
    // Moteus::PositionMode::Command cmd;
    Moteus::PositionMode::Command cmd;

    switch (control_mode) {
    case POSITION:
       cmd.position = ((controlOutput + linearZeroPos)/(2*PI*pinionRadius));

      break;

    case VELOCITY:  
        cmd.position = std::numeric_limits<double>::quiet_NaN();
        cmd.velocity = controlOutput; // in revolutions per second

      break;

    case TORQUE:
        cmd.position = std::numeric_limits<double>::quiet_NaN();
        cmd.velocity = 0.0;
        cmd.kp_scale = 0.0;
        cmd.kd_scale = 0.0;

        cmd.feedforward_torque = controlOutput/10; 

      break;
    }

    moteus.SetPosition(cmd, &positionModeOptions);
}

void printStatus(uint32_t currentTime){
    DPRINT(F("time "));
    DPRINT(currentTime);

    auto print_moteus = [](const Moteus::Query::Result& q) {
        DPRINT(static_cast<int>(q.mode));
        DPRINT(F(" pos "));
        DPRINT(q.position);
        DPRINT(F(" vel "));
        DPRINTLN(q.velocity);
    };

    print_moteus(moteus.last_result().values);
    DPRINT(F(" / "));
}