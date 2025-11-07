#ifndef SENSORS_H
#define SENSORS_H

void sensors_init();
float read_force_sensor();
float read_linear_encoder();
float read_rotational_encoder();

#endif
