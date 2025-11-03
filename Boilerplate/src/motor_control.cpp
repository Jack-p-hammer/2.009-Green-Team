#include "motor_control.h"
#include <Arduino.h>

// For now I assumed we are using an ODrive and just coded for speed control with PWM
const int MOTOR_PWM_PIN = 9;

void motor_init() {
  pinMode(MOTOR_PWM_PIN, OUTPUT);
  analogWrite(MOTOR_PWM_PIN, 0);
}

void motor_set_speed(float speed) {
  // Clamp speed [0, 255] for analogWrite
  int pwmValue = constrain((int) speed, 0, 255);
  analogWrite(MOTOR_PWM_PIN, pwmValue);
}
