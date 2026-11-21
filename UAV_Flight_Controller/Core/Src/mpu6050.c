#include "mpu6050.h"
#include <math.h>

#define MPU_ADDR            (0x68 << 1)

#define REG_SMPLRT_DIV      0x19
#define REG_CONFIG          0x1A
#define REG_GYRO_CONFIG    0x1B
#define REG_ACCEL_CONFIG   0x1C
#define REG_PWR_MGMT_1      0x6B
#define REG_WHO_AM_I        0x75
#define REG_ACCEL_XOUT_H    0x3B

#define GYRO_SCALE          131.0f
#define FILTER_ALPHA        0.98f

#define CALIBRATION_SAMPLES 500

static I2C_HandleTypeDef *mpu_i2c;

static float gyro_bias_x = 0.0f;
static float gyro_bias_y = 0.0f;
static float gyro_bias_z = 0.0f;

static uint8_t angle_initialized = 0;

static HAL_StatusTypeDef MPU_Write(uint8_t reg, uint8_t value)
{
    return HAL_I2C_Mem_Write(
        mpu_i2c,
        MPU_ADDR,
        reg,
        I2C_MEMADD_SIZE_8BIT,
        &value,
        1,
        100
    );
}

static HAL_StatusTypeDef MPU_ReadBytes(
    uint8_t reg,
    uint8_t *data,
    uint16_t length
)
{
    return HAL_I2C_Mem_Read(
        mpu_i2c,
        MPU_ADDR,
        reg,
        I2C_MEMADD_SIZE_8BIT,
        data,
        length,
        100
    );
}

HAL_StatusTypeDef MPU6050_Init(I2C_HandleTypeDef *hi2c)
{
    uint8_t who_am_i = 0;

    mpu_i2c = hi2c;

    if (MPU_ReadBytes(REG_WHO_AM_I, &who_am_i, 1) != HAL_OK)
        return HAL_ERROR;

    if (who_am_i != 0x68)
        return HAL_ERROR;

    if (MPU_Write(REG_PWR_MGMT_1, 0x00) != HAL_OK)
        return HAL_ERROR;

    HAL_Delay(100);

    if (MPU_Write(REG_SMPLRT_DIV, 0x09) != HAL_OK)
        return HAL_ERROR;

    if (MPU_Write(REG_CONFIG, 0x03) != HAL_OK)
        return HAL_ERROR;

    if (MPU_Write(REG_GYRO_CONFIG, 0x00) != HAL_OK)
        return HAL_ERROR;

    if (MPU_Write(REG_ACCEL_CONFIG, 0x00) != HAL_OK)
        return HAL_ERROR;

    angle_initialized = 0;

    gyro_bias_x = 0.0f;
    gyro_bias_y = 0.0f;
    gyro_bias_z = 0.0f;

    return HAL_OK;
}

HAL_StatusTypeDef MPU6050_Read(MPU6050_Data *data)
{
    uint8_t buffer[14];

    if (MPU_ReadBytes(REG_ACCEL_XOUT_H, buffer, 14) != HAL_OK)
        return HAL_ERROR;

    data->ax = (int16_t)((buffer[0] << 8) | buffer[1]);
    data->ay = (int16_t)((buffer[2] << 8) | buffer[3]);
    data->az = (int16_t)((buffer[4] << 8) | buffer[5]);

    data->gx = (int16_t)((buffer[8] << 8) | buffer[9]);
    data->gy = (int16_t)((buffer[10] << 8) | buffer[11]);
    data->gz = (int16_t)((buffer[12] << 8) | buffer[13]);

    data->gyro_roll =
        ((float)data->gx - gyro_bias_x) / GYRO_SCALE;

    data->gyro_pitch =
        ((float)data->gy - gyro_bias_y) / GYRO_SCALE;

    data->gyro_yaw =
        ((float)data->gz - gyro_bias_z) / GYRO_SCALE;

    return HAL_OK;
}

HAL_StatusTypeDef MPU6050_CalibrateGyro(void)
{
    MPU6050_Data temp;

    int32_t sum_x = 0;
    int32_t sum_y = 0;
    int32_t sum_z = 0;

    for (uint16_t i = 0; i < CALIBRATION_SAMPLES; i++)
    {
        if (MPU6050_Read(&temp) != HAL_OK)
            return HAL_ERROR;

        sum_x += temp.gx;
        sum_y += temp.gy;
        sum_z += temp.gz;

        HAL_Delay(2);
    }

    gyro_bias_x = (float)sum_x / CALIBRATION_SAMPLES;
    gyro_bias_y = (float)sum_y / CALIBRATION_SAMPLES;
    gyro_bias_z = (float)sum_z / CALIBRATION_SAMPLES;

    angle_initialized = 0;

    return HAL_OK;
}

void MPU6050_CalculateAngles(MPU6050_Data *data, float dt)
{
    float roll_acc;
    float pitch_acc;

    roll_acc = atan2f(
        (float)data->ay,
        (float)data->az
    ) * 57.2957795f;

    pitch_acc = atan2f(
        -(float)data->ax,
        sqrtf(
            (float)data->ay * data->ay +
            (float)data->az * data->az
        )
    ) * 57.2957795f;

    if (angle_initialized == 0)
    {
        data->roll = roll_acc;
        data->pitch = pitch_acc;

        angle_initialized = 1;
    }
    else
    {
        data->roll =
            FILTER_ALPHA *
            (data->roll + data->gyro_roll * dt) +
            (1.0f - FILTER_ALPHA) * roll_acc;

        data->pitch =
            FILTER_ALPHA *
            (data->pitch + data->gyro_pitch * dt) +
            (1.0f - FILTER_ALPHA) * pitch_acc;
    }
}
