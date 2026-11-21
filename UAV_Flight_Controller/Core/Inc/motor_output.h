#ifndef MOTOR_OUTPUT_H
#define MOTOR_OUTPUT_H

#include "main.h"
#include <stdint.h>

typedef struct
{
    uint16_t motor1;
    uint16_t motor2;
    uint16_t motor3;
    uint16_t motor4;

} Motor_Output;

void Motor_Init(void);

void Motor_Set(uint16_t m1,
               uint16_t m2,
               uint16_t m3,
               uint16_t m4);

void Motor_Set_From_Mixer(float m1,
                          float m2,
                          float m3,
                          float m4);

void Motor_Stop(void);

#endif
