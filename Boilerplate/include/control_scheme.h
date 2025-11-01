#ifndef CONTROL_SCHEME_H
#define CONTROL_SCHEME_H

void control_init();
float compute_control(float force, float linearPos, float rotation);

#endif
