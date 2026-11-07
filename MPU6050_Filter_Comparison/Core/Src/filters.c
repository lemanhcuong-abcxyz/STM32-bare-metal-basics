#include "filters.h"

#define KALMAN_Q_ANGLE  0.001f
#define KALMAN_Q_BIAS   0.003f
#define KALMAN_R_MEASURE 0.03f

void Kalman1D_Init(Kalman1D *k, float initial_angle)
{
    k->angle = initial_angle;
    k->bias = 0.0f;
    k->p00 = 0.0f;
    k->p01 = 0.0f;
    k->p10 = 0.0f;
    k->p11 = 0.0f;
}

float Kalman1D_Update(Kalman1D *k, float measured_angle,
                      float gyro_rate, float dt)
{
    float rate = gyro_rate - k->bias;
    float s, k0, k1, y;
    float p00_temp, p01_temp;

    k->angle += dt * rate;

    k->p00 += dt * (dt * k->p11 - k->p01 - k->p10 + KALMAN_Q_ANGLE);
    k->p01 -= dt * k->p11;
    k->p10 -= dt * k->p11;
    k->p11 += KALMAN_Q_BIAS * dt;

    s = k->p00 + KALMAN_R_MEASURE;
    k0 = k->p00 / s;
    k1 = k->p10 / s;

    y = measured_angle - k->angle;
    k->angle += k0 * y;
    k->bias += k1 * y;

    p00_temp = k->p00;
    p01_temp = k->p01;

    k->p00 -= k0 * p00_temp;
    k->p01 -= k0 * p01_temp;
    k->p10 -= k1 * p00_temp;
    k->p11 -= k1 * p01_temp;

    return k->angle;
}

float Complementary_Update(float previous_angle, float gyro_rate,
                           float accel_angle, float dt, float alpha)
{
    return alpha * (previous_angle + gyro_rate * dt)
           + (1.0f - alpha) * accel_angle;
}
