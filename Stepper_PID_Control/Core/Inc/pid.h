#ifndef PID_H
#define PID_H

typedef struct
{
    float Kp;
    float Ki;
    float Kd;

    float setpoint;
    float integral;
    float previous_error;

    float output_min;
    float output_max;

    float sample_time;
} PID_TypeDef;

void PID_Init(PID_TypeDef *pid,
              float kp,
              float ki,
              float kd,
              float sample_time);

float PID_Compute(PID_TypeDef *pid, float feedback);

void PID_Reset(PID_TypeDef *pid);

#endif
