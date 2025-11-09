#include "control_scheme.h"
#include <Arduino.h>

// Example placeholder PID-esque logic
// Assume 0 initial conditions
double prevCommand = 0;
double prevError = 0;

double errorGain = 109.1;
double prevErrorGain = 243.6;

void comressionControllerInit() {
  // You can set targetForce, PID gains, etc. here
}

double updateCompressionController(double setpoint, double linearPos, double rotation) {
  double error = setpoint - linearPos;

  // See MATLAB file SimulinkSetup.mlx for controller in discrete TF form
  // THIS REQUIRES 10 ms CONTROLLER UPDATE PERIOD
  double output = errorGain*error - prevErrorGain*prevError + prevCommand;
  output = constrain(output, 0, 255);

  prevCommand = output;
  prevError = error;
  return output;
}
