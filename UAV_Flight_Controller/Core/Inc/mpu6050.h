#ifndef MPU6050_H
#define MPU6050_H

#include "stm32f4xx_hal.h"

typedef struct
{
    int16_t ax, ay, az;
    int16_t gx, gy, gz;

    float gyro_roll;
    float gyro_pitch;
    float gyro_yaw;

    float roll;
    float pitch;

} MPU6050_Data;

HAL_StatusTypeDef MPU6050_Init(I2C_HandleTypeDef *hi2c);

HAL_StatusTypeDef MPU6050_Read(MPU6050_Data *data);

HAL_StatusTypeDef MPU6050_CalibrateGyro(void);

void MPU6050_CalculateAngles(MPU6050_Data *data, float dt);

#endif
