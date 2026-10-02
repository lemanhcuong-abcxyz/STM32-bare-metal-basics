/*
 * mpu6050.c
 *
 *  Created on: Oct 2, 2026
 *      Author: lmc
 */


#include "mpu6050.h"
#include "stm32f4xx.h"

#define MPU6050_ADDR       0x68
#define MPU6050_WHO_AM_I   0x75
#define MPU6050_PWR_MGMT_1 0x6B
#define MPU6050_ACCEL_XOUT 0x3B

static uint8_t I2C1_WaitFlag(volatile uint32_t *reg, uint32_t flag)
{
    uint32_t timeout = 100000;

    while (((*reg) & flag) == 0)
    {
        if (--timeout == 0)
            return 0;
    }

    return 1;
}

static void I2C1_Init(void)
{
    RCC->AHB1ENR |= (1U << 1);
    RCC->APB1ENR |= (1U << 21);

    GPIOB->MODER &= ~((3U << 16) | (3U << 18));
    GPIOB->MODER |=  ((2U << 16) | (2U << 18));

    GPIOB->OTYPER |= (1U << 8) | (1U << 9);

    GPIOB->PUPDR &= ~((3U << 16) | (3U << 18));
    GPIOB->PUPDR |=  ((1U << 16) | (1U << 18));

    GPIOB->AFR[1] &= ~((0xFU << 0) | (0xFU << 4));
    GPIOB->AFR[1] |=  ((4U << 0) | (4U << 4));

    I2C1->CR1 = 0;
    I2C1->CR2 = 42;
    I2C1->CCR = 210;
    I2C1->TRISE = 43;
    I2C1->CR1 |= (1U << 0);
}

static uint8_t I2C1_WriteReg(uint8_t reg, uint8_t data)
{
    uint32_t timeout = 100000;

    I2C1->CR1 |= (1U << 8);

    if (!I2C1_WaitFlag(&I2C1->SR1, (1U << 0)))
        return 0;

    I2C1->DR = MPU6050_ADDR << 1;

    if (!I2C1_WaitFlag(&I2C1->SR1, (1U << 1)))
        return 0;

    (void)I2C1->SR1;
    (void)I2C1->SR2;

    I2C1->DR = reg;

    if (!I2C1_WaitFlag(&I2C1->SR1, (1U << 7)))
        return 0;

    I2C1->DR = data;

    if (!I2C1_WaitFlag(&I2C1->SR1, (1U << 2)))
        return 0;

    I2C1->CR1 |= (1U << 9);

    while (timeout--)
    {
        if ((I2C1->SR2 & (1U << 1)) == 0)
            return 1;
    }

    return 0;
}

static uint8_t I2C1_ReadRegs(uint8_t reg, uint8_t *data, uint8_t len)
{
    uint32_t timeout = 100000;

    // START
    I2C1->CR1 |= (1U << 8);

    if (!I2C1_WaitFlag(&I2C1->SR1, (1U << 0)))
        return 0;

    // Gửi địa chỉ ghi
    I2C1->DR = MPU6050_ADDR << 1;

    if (!I2C1_WaitFlag(&I2C1->SR1, (1U << 1)))
        return 0;

    (void)I2C1->SR1;
    (void)I2C1->SR2;

    // Gửi địa chỉ thanh ghi
    I2C1->DR = reg;

    if (!I2C1_WaitFlag(&I2C1->SR1, (1U << 7)))
        return 0;

    if (!I2C1_WaitFlag(&I2C1->SR1, (1U << 2)))
        return 0;

    // START lặp lại
    I2C1->CR1 |= (1U << 8);

    if (!I2C1_WaitFlag(&I2C1->SR1, (1U << 0)))
        return 0;

    // Gửi địa chỉ đọc
    I2C1->DR = (MPU6050_ADDR << 1) | 1U;

    if (!I2C1_WaitFlag(&I2C1->SR1, (1U << 1)))
        return 0;

    (void)I2C1->SR1;
    (void)I2C1->SR2;

    for (uint8_t i = 0; i < len; i++)
    {
        if (i == len - 1)
        {
            I2C1->CR1 &= ~(1U << 10); // ACK = 0
            I2C1->CR1 |= (1U << 9);   // STOP
        }
        else
        {
            I2C1->CR1 |= (1U << 10);  // ACK = 1
        }

        if (!I2C1_WaitFlag(&I2C1->SR1, (1U << 6)))
            return 0;

        data[i] = I2C1->DR;
    }

    I2C1->CR1 |= (1U << 10);

    while (timeout--)
    {
        if ((I2C1->SR2 & (1U << 1)) == 0)
            return 1;
    }

    return 0;
}

uint8_t MPU6050_Init(void)
{
    uint8_t who_am_i;

    I2C1_Init();

    if (!I2C1_ReadRegs(MPU6050_WHO_AM_I, &who_am_i, 1))
        return 0;

    if (who_am_i != 0x68)
        return 0;

    if (!I2C1_WriteReg(MPU6050_PWR_MGMT_1, 0x00))
        return 0;

    return 1;
}

uint8_t MPU6050_ReadAll(MPU6050_Data *data)
{
    uint8_t buffer[14];

    if (!I2C1_ReadRegs(MPU6050_ACCEL_XOUT, buffer, 14))
        return 0;

    data->ax = (int16_t)((buffer[0] << 8) | buffer[1]);
    data->ay = (int16_t)((buffer[2] << 8) | buffer[3]);
    data->az = (int16_t)((buffer[4] << 8) | buffer[5]);

    data->gx = (int16_t)((buffer[8] << 8) | buffer[9]);
    data->gy = (int16_t)((buffer[10] << 8) | buffer[11]);
    data->gz = (int16_t)((buffer[12] << 8) | buffer[13]);

    return 1;
}
