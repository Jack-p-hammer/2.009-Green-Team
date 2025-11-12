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
    WAIT_FOR_CONFIRMATION,
    COMPRESSIONS
};

// Control Loop Timing variables
// Defined here to put in most global of scopes, not to
// be used directly in loop() or setup()
uint32_t nextSendMillis = 0;
uint16_t loopCount = 0;

void setup() {
  // Do everything that needs to occur on power up
  initializeMotor();

  // sensors_init();
  // control_init();

}

void loop() {
  // Refresh moteus controller readings
  // This is unneeded, as updateCalibration() and updateCompressions
  // will always call readSensors(), which will cause a moteus read
  // moteus.read();

  // TODO: State Machine
  updateCompressions();
  updateCalibration();
}
