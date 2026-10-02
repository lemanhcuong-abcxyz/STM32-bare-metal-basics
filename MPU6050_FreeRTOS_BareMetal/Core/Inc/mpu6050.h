#ifndef MPU6050_H
#define MPU6050_H

#include <stdint.h>

typedef struct
{
    int16_t ax;
    int16_t ay;
    int16_t az;
    int16_t gx;
    int16_t gy;
    int16_t gz;
} MPU6050_Data;

uint8_t MPU6050_Init(void);
uint8_t MPU6050_ReadAll(MPU6050_Data *data);

#endif
