#ifndef PID_H
#define PID_H

#include <stdint.h>

typedef struct
{
    float Kp;
    float Ki;
    float Kd;

    float setpoint;
    float integral;
    float previous_measurement;
    float output;

    float output_limit;
    float integral_limit;

    uint8_t initialized;
} PID_Controller;

void PID_Init(PID_Controller *pid,
              float Kp,
              float Ki,
              float Kd,
              float output_limit,
              float integral_limit);

float PID_Update(PID_Controller *pid,
                float measurement,
                float dt);

void PID_Reset(PID_Controller *pid);

#endif
