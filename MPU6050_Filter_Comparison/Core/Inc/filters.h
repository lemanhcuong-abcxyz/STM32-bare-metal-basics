#ifndef FILTERS_H
#define FILTERS_H

typedef struct
{
    float angle;
    float bias;
    float p00;
    float p01;
    float p10;
    float p11;
} Kalman1D;

void Kalman1D_Init(Kalman1D *k, float initial_angle);
float Kalman1D_Update(Kalman1D *k, float measured_angle,
                      float gyro_rate, float dt);

float Complementary_Update(float previous_angle, float gyro_rate,
                           float accel_angle, float dt, float alpha);

#endif
