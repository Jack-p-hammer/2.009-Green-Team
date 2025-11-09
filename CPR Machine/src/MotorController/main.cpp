#include <Arduino.h>
#include "motor_control.h"
#include "sensors.h"
#include "control_scheme.h"
#include <Moteus.h>
#include <ACAN2517FD.h>


// Sampling period (ms), might have to adjust this
// const unsigned long LOOP_INTERVAL = 10; 
// unsigned long lastLoopTime = 0;

void setup() {

  initializeMotor();

  // sensors_init();
  // control_init();

}

void loop() {
  
  updateMotor();

}
