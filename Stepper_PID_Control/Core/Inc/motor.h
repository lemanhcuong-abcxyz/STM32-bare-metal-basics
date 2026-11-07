#ifndef MOTOR_H
#define MOTOR_H

#include "main.h"
#include <stdint.h>

void Motor_Init(void);
void Motor_SetSpeed(float rpm);
void Motor_Stop(void);
void Motor_Update(void);

#endif
