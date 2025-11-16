#include "control_scheme.h"
#include "sensors.h"
#include <Moteus.h>
#include <ACAN2517FD.h>
#include <Arduino.h>
#include <assert.h>

// State machine variables
#ifndef COMPRESSION_TEST
cprState currentState = START_UP;
cprState prevState = START_UP;
#else
cprState currentState = COMPRESSIONS;
cprState prevState = COMPRESSIONS;
#endif

// Control Loop Timing variables
uint32_t nextSendMillis = 0;
uint16_t loopCount = 0;

// Lag controller variables
double prevCommand = 0;
double prevError = 0;

// Lag controller gains
double errorGain = -173.3746;//-117.9813; // This is the constant term of the TF numerator, with sign
double prevErrorGain = 257.6539; //244.9501; // This is the z term of the TF numerator


// All Zeroing controller vars
double prev_prev_Command = 0;
double prev_prev_Error = 0;

double current_error_gain = 68.58;

double prev_command_gain = 1.002;
double prev_error_gain = -113.9;

double prev_prev_command_gain = -.00201;
double prev_prev_error_gain = 50.2;




// Driver object
ACAN2517FD can(MCP2517_CS, SPI, MCP2517_INT);

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
        DPRINT(F("CAN error 0x"));
        DPRINTLN(errorCode, HEX);
        while (true) { delay(1000); }
    }

    // To clear any faults the controllers may have, we start by sending
    // a stop command to each.
    moteus.SetStop();
    DPRINTLN(F("Motor stopped"));
}

double updateController(double setpoint_m) {
  readSensors();

  // Take setpoint relative to linear zero
  //double error = (setpoint_m - linearZeroPos) - linearPos;
  double error = setpoint_m - rotaryPos*(2*PI)*pinionRadius;

  // See MATLAB file SimulinkSetup.mlx for controller in discrete TF form
  // THIS REQUIRES 10 ms CONTROLLER UPDATE PERIOD
  assert(controller_period == 10);
  double torqueOutput = prevCommand - errorGain*error - prevErrorGain*prevError;

  // Saturate the control effort
  torqueOutput = constrain(torqueOutput, -5, 5);

  prevCommand = torqueOutput;
  prevError = error;
 
  DPRINT(">");
  DPRINT("Setpoint:"); DPRINT(setpoint_m);
  DPRINT(",");  
  DPRINT("ERROR:"); DPRINT(error);
  DPRINT(",");
  DPRINT("ROTARY POS:"); DPRINT(rotaryPos);
  DPRINT(",");
  DPRINT("TORQUE:"); DPRINTLN(torqueOutput);

  return torqueOutput;
}


double updateControllerZeroing(double setpoint_m) {
  readSensors();

  // Take setpoint relative to linear zero
  //double error = (setpoint_m - linearZeroPos) - linearPos;
  double error = setpoint_m - rotaryPos*(2*PI)*pinionRadius;

  // See MATLAB file SimulinkSetup.mlx for controller in discrete TF form
  // THIS REQUIRES 10 ms CONTROLLER UPDATE PERIOD
  assert(controller_period == 10);

  // Different controller setup --> no chest in the way
  double torqueOutput = current_error_gain*error
                      + prev_command_gain*prevCommand + prev_error_gain*prevError 
                      + prev_prev_command_gain*prev_prev_Command + prev_prev_error_gain*prev_prev_Error; 
                      

  // Saturate the control effort
  torqueOutput = constrain(torqueOutput, -5, 5);

  prev_prev_Command = prevCommand;
  prev_prev_Error = prevError;

  prevCommand = torqueOutput;
  prevError = error;

  return torqueOutput;
}

void sendCommands(double controlOutput_Nm) {
    // TODO: This needs to be changed into torque control!!!
    Moteus::PositionMode::Command cmd;

    // Send sine wave position command at 120 bpm with 2 inch amplitude
    // Positive rotation is down
    //cmd.position = ((0.0254*2)/2 * ::cos(2*PI*(2/1000.0) * millis()) + (0.0254*2)/2)/pinionRadius;

    // Velocity feedforward, derivative of position
    // Supposed to help on high freqs, see if it matters for us
    //cmd.velocity = (-2*PI*(2/1000.0) * (0.0254*2)/2 * ::sin(2*PI*(2/1000.0) * millis()))/pinionRadius;
    
    //DPRINTLN(read_rotary_encoder());
    //printStatus(millis());
    // No feedforward velocity, uncomment if above is commented
    // cmd.velocity = NAN;
    // ------------------------- old torque control code ------------------------- (doesnt work)
    cmd.position = std::numeric_limits<double>::quiet_NaN();
    cmd.velocity = 0.0;
    cmd.kp_scale = 0.0;
    cmd.kd_scale = 0.0;

    cmd.feedforward_torque = controlOutput_Nm;

    moteus.SetPosition(cmd, &positionModeOptions);
    // New attempt
}

void printStatus(uint32_t currentTime){
    DPRINT(F("time "));
    DPRINT(currentTime);

    auto print_moteus = [](const Moteus::Query::Result& q) {
        DPRINT(static_cast<int>(q.mode));
        DPRINT(F(" pos "));
        DPRINT(q.position);
        DPRINT(F(" vel "));
        DPRINT(q.velocity);
    };

    print_moteus(moteus.last_result().values);
    DPRINT(F(" / "));
}