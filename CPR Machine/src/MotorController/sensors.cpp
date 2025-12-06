#include "sensors.h"
#include "control_scheme.h"
#include <Arduino.h>
#include "Adafruit_VL53L0X.h"
#include <Adafruit_BNO08x.h>

// FIXME: EXAMPLE PINS
const int FORCE_PIN = A8;
// const int LINEAR_ENCODER_A = 2;
// const int LINEAR_ENCODER_B = 3;

const double forceCalibRate = 1187.7440; // Calibration constants for force sensor
const double forceCalibOffset = -4295;
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

// IMU Setup
Adafruit_BNO08x IMU = Adafruit_BNO08x();
sh2_SensorValue_t imuValue;

float imu_ax = 0;   // m/s^2
float imu_ay = 0;
float imu_az = 0;

// Rack & pinion dimensions
double rackLength = 12.925*0.0254; // double check this (inches converted to meters)
double plungerLength = 3.0*0.0254; // double check this (inches converted to meters)
double bushingLength = 4.355*0.0254; // double check this (inches converted to meters)


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

  // ----- Initialize BNO085 IMU -----
  if (!IMU.begin_I2C()) {
    DPRINTLN(F("Failed to initialize BNO085 IMU"));
    return true; //changed from false
  }

  // Enable linear acceleration reports (gravity removed) at ~100 Hz
  if (!IMU.enableReport(SH2_LINEAR_ACCELERATION, 10000)) { 
    // 10,000 µs = 10 ms = 100 Hz update rate
    DPRINTLN(F("Could not enable linear acceleration report"));
    return true; //changed from false
  }

  DPRINTLN(F("BNO085 IMU initialized"));

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
  VL53L0X_RangingMeasurementData_t measure;
    
  ToFSensor.rangingTest(&measure, false); // pass in 'true' to get debug data printout!

  if (measure.RangeStatus != 4) {  // phase failures have incorrect data
    double reading = measure.RangeMilliMeter/1000.0; // convert millimeters to meters
    //Serial.print("Distance (mm): "); Serial.println(measure.RangeMilliMeter);

    // always return zeroed value 
    // return reading - linearZeroPos; 
    // TODO: DOnt return rotaryPos again
    return 2*PI*rotaryPos*pinionRadius -  absLinearZero;
  }


  // If no reading is available, return 0
  // This is fine because sensor noise will prevent a real zero reading
  return 2*PI*rotaryPos*pinionRadius - absLinearZero;
}

bool read_imu() {
  sh2_SensorValue_t sensorValue;

  // Try to get a new event (non-blocking)
  if (!IMU.getSensorEvent(&sensorValue)) {
    return false;  // No new IMU data yet
  }

  // We only care about linear acceleration
  if (sensorValue.sensorId == SH2_LINEAR_ACCELERATION) {
    imu_ax = sensorValue.un.linearAcceleration.x;
    imu_ay = sensorValue.un.linearAcceleration.y;
    imu_az = sensorValue.un.linearAcceleration.z;
    return true;
  }

  return false;
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
    read_imu();

    // read_linear_encoder returns zero if no new reading is available
    if(tempPos != 0) {
      linearPos = tempPos;
    }

    // TODO: Encoder Validation
    double rotaryPosFromLinear = linearPos/pinionRadius; // in radians

    // Change tolerance based on linear encoder precision
    // must convert rotaryPos to radians
    if(abs(2*PI*rotaryPos - rotaryPosFromLinear) > 0.1) {
      // TODO: update state machine
      //currentState = ABORT; uncomment later!!!
      DPRINTLN("ALERT: LINEAR - ROTARY MISMATCH");
      return true; // change back to false; later
    }

    // Change limit from 550 to whatever we decide is a good worst-case limit
    if (forceVal > 550) {
      // TODO: update state machine
      //currentState = ABORT; uncomment later!!!
      DPRINTLN("ALERT: OVER FORCE");
      return true; //change back to false later
    }
    return true;
}