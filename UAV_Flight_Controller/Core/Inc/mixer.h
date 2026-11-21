#ifndef MIXER_H
#define MIXER_H

typedef struct
{
    float throttle;
    float roll;
    float pitch;
    float yaw;

    float motor1;
    float motor2;
    float motor3;
    float motor4;

} Mixer_Output;

void Mixer_Calculate(Mixer_Output *mixer);

#endif
