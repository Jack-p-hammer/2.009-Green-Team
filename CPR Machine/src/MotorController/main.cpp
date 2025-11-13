#include <Arduino.h>
#include "control_scheme.h"
#include "compression_control.h"
#include "zeroing_control.h"
#include "sensors.h"
#include <Moteus.h>
#include <ACAN2517FD.h>

void setup() {
  // Do everything that needs to occur on power up
  initializeMotor();
  initializeSensors();
  // TODO: Does anything else need to happen here?
}

void loop() {
  // Main State Machine
  switch (currentState) {
    // TODO: Implement state machine cases
    case START_UP:  
    case ZEROING:
    case CALIBRATION_FAILED:
    case COMPRESSIONS:
    case PAUSED:
    case KNEEL_FAILURE:
    case ABORT:
    break;
  }

  // TODO: State Machine
  updateCompressions();
  updateZeroing();
}
