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

  // unsigned long now = millis();
  // if (now - lastLoopTime >= LOOP_INTERVAL) {
  //   lastLoopTime = now;

    // Read sensors
  
    // float force = read_force_sensor();
    // float linearPos = read_linear_encoder();
    // float rotation = read_rotational_encoder();

    // Print sensor readings
    // Serial.println(force);
    // Serial.println(linearPos);
    // Serial.println(rotation);

    // Compute control output
    //float controlOutput = compute_control(force, linearPos, rotation);

    // Send command to motor driver
    //motor_set_speed(controlOutput);
  //}
}
