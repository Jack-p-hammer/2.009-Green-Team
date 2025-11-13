#include "sensors.h"
#include "control_scheme.h"
#include <Arduino.h>
#include "Adafruit_VL53L0X.h"

// TODO: EXAMPLE PINS (FIX LATER!!!)
const int FORCE_PIN = A0;
const int LINEAR_ENCODER_A = 2;
const int LINEAR_ENCODER_B = 3;

const double forceCalibRate = 1187.7440; // Calibration constants for force sensor
const double forceCalibOffset = -548.7972;

// Assume 0 initial conditions
double linearPos = 0; double linearZeroPos = 0;
double rotaryPos = 0; double rotaryZeroPos = 0;
double forceVal = 0;

// Declare ToF Sensor
Adafruit_VL53L0X ToFSensor = Adafruit_VL53L0X();

// Declare variables for sensor validation
const float pinionRadius = 0.01; // Meters

void initializeSensors() {
  // Set pins
  pinMode(FORCE_PIN, INPUT);

  // Initialize ToF Sensor
  if (!ToFSensor.begin()) {
    Serial.println(F("Failed to boot Time of Flight sensor"));
    while(1);
  }

  // TODO: Do we want continuous readings?
  ToFSensor.startRangeContinuous();

  // Record zero positions
  rotaryZeroPos = read_rotary_encoder();
  linearZeroPos = read_linear_encoder();
  // Force sensor is pre-calibrated
}

double read_force_sensor() {
  // Ensure that Voltage at the non-inverting terminal is less about 0.5V (voltage divider or sum shite)
  // If using a different reference voltage, recalibration is required
  int raw = analogRead(FORCE_PIN);
  double voltage = raw * (5.0 / 1023.0);
  double force = forceCalibRate * voltage + forceCalibOffset;
  return force; // should be value in newtons
}

double read_linear_encoder() {
  // Record reading from ToF Sensor
  if (ToFSensor.isRangeComplete()) {
    double reading = ToFSensor.readRange();

    // Sensor is in millimeters, want meters
    reading /= 1000.0;

    // always return zeroed value 
    return reading - linearZeroPos; 
  }
  // If no reading is available, return 0
  // This is fine because sensor noise will prevent a real zero reading
  return 0;
}

void zeroLinearEncoder() {
  // Go through existing read function to get zero pos
  double reading = read_linear_encoder();
  linearZeroPos += reading; // Adjust zero position so that current value reads zero
}

double read_rotary_encoder() {
  // Read position from Moteus controller (rotary encoder feedback)
  // The moteus library continuously updates q_current.position (in revolutions)
  rotaryPos = moteus.last_result().values.position - rotaryZeroPos;
  return rotaryPos;
}
void zeroRotaryEncoder() {
  // Note that moteus encoder is semi-absolute: on power cycle, it will read an absolute position
  // to the nearest rotation (i.e. from -0.5 to 0.5). The moteus will also have its own position
  // limits based on rotaryPos, not the zeroed output we use internally. This, for all intents and
  // purposes, should be fine and adds another layer of safety
  double reading = read_rotary_encoder();
  rotaryZeroPos += reading; // Adjust zero position so that current value reads zero
}

void readSensors() {
    forceVal = read_force_sensor();
    rotaryPos = read_rotary_encoder(); // in revolutions
    // ToF sensor only updates at 30ish ms max, if nothing read keep prev. encoder value
    double tempPos = read_linear_encoder();
    // read_linear_encoder returns zero if no new reading is available
    if(tempPos != 0) {
      linearPos = tempPos;
    }

    // TODO: Encoder Validation
    double rotaryPosFromLinear = linearPos/pinionRadius; // in radians

    // Change tolerance based on linear encoder precision
    // must convert rotaryPos to radians
    if(abs(2*PI*rotaryPos - rotaryPosFromLinear) > 0.1) {
      // TODO: Change from a print to state switch
      Serial.println("ALERT: LINEAR - ROTARY MISMATCH");
    }

    // Change limit from 550 to whatever we decide is a good worst-case limit
    if (forceVal > 550) {
      // TODO: Change from a print to a state switch
      Serial.println("ALERT: OVER FORCE");
    }
}