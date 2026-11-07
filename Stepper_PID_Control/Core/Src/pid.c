#include "pid.h"

void PID_Init(PID_TypeDef *pid,
              float kp,
              float ki,
              float kd,
              float sample_time)
{
    pid->Kp = kp;
    pid->Ki = ki;
    pid->Kd = kd;

    pid->setpoint = 0.0f;
    pid->integral = 0.0f;
    pid->previous_error = 0.0f;

    pid->output_min = 0.0f;
    pid->output_max = 10.0f;

    pid->sample_time = sample_time;
}

float PID_Compute(PID_TypeDef *pid, float feedback)
{
    float error = pid->setpoint - feedback;

    pid->integral += error * pid->sample_time;

    float derivative =
        (error - pid->previous_error) / pid->sample_time;

    float output =
        pid->Kp * error +
        pid->Ki * pid->integral +
        pid->Kd * derivative;

    if (output > pid->output_max)
    {
        output = pid->output_max;
    }

    if (output < pid->output_min)
    {
        output = pid->output_min;
    }

    pid->previous_error = error;

    return output;
}

void PID_Reset(PID_TypeDef *pid)
{
    pid->integral = 0.0f;
    pid->previous_error = 0.0f;
}
