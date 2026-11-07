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

void I2C1_Init(void);
int MPU6050_Init(void);
int MPU6050_ReadData(MPU6050_Data *data);
uint8_t MPU6050_ReadWhoAmI(void);

#endif
