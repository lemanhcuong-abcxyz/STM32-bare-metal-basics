#include "mixer.h"

static float Limit(float value,
                   float minimum,
                   float maximum)
{
    if (value > maximum) return maximum;
    if (value < minimum) return minimum;
    return value;
}

void Mixer_Calculate(Mixer_Output *mixer)
{
    float T = mixer->throttle;
    float R = mixer->roll;
    float P = mixer->pitch;
    float Y = mixer->yaw;

    mixer->motor1 = T - R + P - Y;
    mixer->motor2 = T + R + P + Y;
    mixer->motor3 = T + R - P - Y;
    mixer->motor4 = T - R - P + Y;

    mixer->motor1 = Limit(mixer->motor1, 0.0f, 1000.0f);
    mixer->motor2 = Limit(mixer->motor2, 0.0f, 1000.0f);
    mixer->motor3 = Limit(mixer->motor3, 0.0f, 1000.0f);
    mixer->motor4 = Limit(mixer->motor4, 0.0f, 1000.0f);
}
