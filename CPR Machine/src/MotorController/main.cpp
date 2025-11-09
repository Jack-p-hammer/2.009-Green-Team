#include <Arduino.h>
#include "compression_control.h"
#include "calibration_control.h"
#include "sensors.h"
#include "control_scheme.h"
#include <Moteus.h>
#include <ACAN2517FD.h>

// State enumeration for state machine
enum cprState {
    STANDBY,
    HOMING,
    COMPRESSIONS
};

// Control Loop Timing variables
uint32_t nextSendMillis = 0;
uint16_t loopCount = 0;

void setup() {
  // Do everything that needs to occur on power up
  initializeMotor();

  // sensors_init();
  // control_init();

}

void loop() {
  // TODO: State Machine
  updateCompressions();
  updateCalibration();
}
