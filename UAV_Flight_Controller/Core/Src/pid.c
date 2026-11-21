#include "pid.h"
#include <math.h>

static float Limit(float value, float min, float max)
{
    if (value > max) return max;
    if (value < min) return min;
    return value;
}

void PID_Init(PID_Controller *pid,
              float Kp,
              float Ki,
              float Kd,
              float output_limit,
              float integral_limit)
{
    pid->Kp = Kp;
    pid->Ki = Ki;
    pid->Kd = Kd;

    pid->setpoint = 0.0f;
    pid->integral = 0.0f;
    pid->previous_measurement = 0.0f;
    pid->output = 0.0f;

    pid->output_limit = output_limit;
    pid->integral_limit = integral_limit;

    pid->initialized = 0U;
}

float PID_Update(PID_Controller *pid,
                float measurement,
                float dt)
{
    if (dt <= 0.0f || dt > 0.1f)
    {
        PID_Reset(pid);
        return 0.0f;
    }

    if (!isfinite(measurement))
    {
        PID_Reset(pid);
        return 0.0f;
    }

    float error = pid->setpoint - measurement;

    pid->integral += error * dt;

    pid->integral = Limit(pid->integral,
                          -pid->integral_limit,
                           pid->integral_limit);

    float derivative = 0.0f;

    if (pid->initialized)
    {
        derivative = -(measurement -
                       pid->previous_measurement) / dt;
    }

    pid->previous_measurement = measurement;
    pid->initialized = 1U;

    float output =
        pid->Kp * error +
        pid->Ki * pid->integral +
        pid->Kd * derivative;

    output = Limit(output,
                   -pid->output_limit,
                    pid->output_limit);

    pid->output = output;

    return output;
}

void PID_Reset(PID_Controller *pid)
{
    pid->integral = 0.0f;
    pid->previous_measurement = 0.0f;
    pid->output = 0.0f;
    pid->initialized = 0U;
}
