#include "sensors.h"
#include "control_scheme.h"
#include <Arduino.h>
#include "Adafruit_VL53L0X.h"

// FIXME: EXAMPLE PINS
const int FORCE_PIN = A8;
// const int LINEAR_ENCODER_A = 2;
// const int LINEAR_ENCODER_B = 3;

const double forceCalibRate = 1187.7440; // Calibration constants for force sensor
const double forceCalibOffset = -2045;
const int samplesToAverage = 30;

float absRotaryZero = 0.0; // Absolute zero position of rotary encoder on power up
float absLinearZero = 0.0; // Absolute zero position of rotary encoder on power up


// Assume 0 initial conditions
double linearPos = 0; double linearZeroPos = 0;
double rotaryPos = 0; double rotaryZeroPos = 0;
double forceVal = 0;
float rawForceVal = 0;

// Declare ToF Sensor
Adafruit_VL53L0X ToFSensor = Adafruit_VL53L0X();

// Declare variables for sensor validation
const float pinionRadius = 0.01; // Meters

bool initializeSensors() {
  // Set pins
  pinMode(FORCE_PIN, INPUT);

  // TODO: Ensure that plunger is in full retract position before we take initial zeros
  // Record zero positions
  rotaryZeroPos = read_rotary_encoder();

    // Initialize ToF Sensor
  if (!ToFSensor.begin()) {
    DPRINTLN(F("Failed to boot Time of Flight sensor"));
    return false;
  }

  // TODO: Do we want continuous readings?
  ToFSensor.startRangeContinuous();
  
  linearZeroPos = read_linear_encoder();
  // Force sensor is pre-calibrated

  // Initialization successful
  return true;
}

double read_force_sensor() {
  // Ensure that Voltage at the non-inverting terminal is less about 0.5V (voltage divider or sum shite)
  // If using a different reference voltage, recalibration is required
  // Read analog value and average over several samples

  //average samples to return force value
  float total = 0;
  for(int i = 0; i < samplesToAverage; i++) {
    total += analogRead(FORCE_PIN);
  }
  rawForceVal = total / samplesToAverage;
  // Convert to force in Newtons
  double voltage = (rawForceVal / 1023.0) * 5.0; // Assuming 10-bit ADC and 5V reference
  double force = forceCalibRate * voltage + forceCalibOffset;


  // DPRINT(">");
  // DPRINT("Force:"); DPRINT(force);
  // DPRINT(",");
  // DPRINT("State:"); DPRINTLN(currentState);

  return force; // should be value in newtons
}

double read_linear_encoder() {
  // Record reading from ToF Sensor
  if (ToFSensor.isRangeComplete()) {
    double reading = ToFSensor.readRange();

    // Sensor is in millimeters, want meters
    reading /= 1000.0;

    // always return zeroed value 
    // return reading - linearZeroPos; 
    // TODO: DOnt return rotaryPos again
    return 2*PI*rotaryPos*pinionRadius -  absLinearZero;
  }
  // If no reading is available, return 0
  // This is fine because sensor noise will prevent a real zero reading
  return 2*PI*rotaryPos*pinionRadius - absLinearZero;
}

void zeroLinearEncoder() {
  // Go through existing read function to get zero pos
  double reading = read_linear_encoder();
  linearZeroPos += reading; // Adjust zero position so that current value reads zero
}

double read_rotary_encoder() {
  // Read position from Moteus controller (rotary encoder feedback)
  // The moteus library continuously updates q_current.position (in revolutions)
  rotaryPos = moteus.last_result().values.position - absRotaryZero;
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

bool readSensors() {
    forceVal = read_force_sensor();
    // forceVal = 5; // TEMP FOR TESTING
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
      DPRINTLN("ALERT: LINEAR - ROTARY MISMATCH");
      return false;
    }

    // Change limit from 550 to whatever we decide is a good worst-case limit
    if (forceVal > 550) {
      // TODO: Change from a print to a state switch
      DPRINTLN("ALERT: OVER FORCE");
      return false;
    }
    return true;
}