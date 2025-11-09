#ifndef SENSORS_H
#define SENSORS_H

// Sensor Pins

extern const int FORCE_PIN;
extern const int LINEAR_ENCODER_A;
extern const int LINEAR_ENCODER_B;
extern const int ROTARY_ENCODER_A;
extern const int ROTARY_ENCODER_B;

// Sensor outputs

extern double linearPos;
extern double rotaryPos;
extern double forceVal;

// Force Sensor Calibration Values

extern const int forceCalibRate;
extern const int forceCalibOffset;

// Sensor function definitions

void sensors_init();
double read_force_sensor();
double read_linear_encoder();
double read_rotational_encoder();
void readSensors();

#endif
