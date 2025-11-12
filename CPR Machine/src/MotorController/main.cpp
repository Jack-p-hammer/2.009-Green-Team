#include <Arduino.h>
#include "control_scheme.h"
#include "compression_control.h"
#include "calibration_control.h"
#include "sensors.h"
#include <Moteus.h>
#include <ACAN2517FD.h>

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
