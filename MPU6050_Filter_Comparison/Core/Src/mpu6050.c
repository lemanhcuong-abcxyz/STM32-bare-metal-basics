#include "stm32f4xx.h"
#include "mpu6050.h"

#define MPU6050_ADDR_W  0xD0U
#define MPU6050_ADDR_R  0xD1U
#define I2C_TIMEOUT     1000000U

static int wait_sr1(uint32_t mask)
{
    uint32_t timeout = I2C_TIMEOUT;
    while ((I2C1->SR1 & mask) == 0U)
    {
        if (--timeout == 0U) return 0;
    }
    return 1;
}

static int wait_bus_free(void)
{
    uint32_t timeout = I2C_TIMEOUT;
    while ((I2C1->SR2 & (1U << 1)) != 0U)
    {
        if (--timeout == 0U) return 0;
    }
    return 1;
}

static void i2c_stop(void)
{
    I2C1->CR1 |= (1U << 9);
    I2C1->CR1 |= (1U << 10);
}

static int i2c_write_reg(uint8_t reg, uint8_t value)
{
    if (!wait_bus_free()) return 0;

    I2C1->CR1 |= (1U << 8);
    if (!wait_sr1(1U << 0)) return 0;

    I2C1->DR = MPU6050_ADDR_W;
    if (!wait_sr1(1U << 1)) return 0;
    (void)I2C1->SR1;
    (void)I2C1->SR2;

    I2C1->DR = reg;
    if (!wait_sr1(1U << 7)) return 0;

    I2C1->DR = value;
    if (!wait_sr1(1U << 2)) return 0;

    i2c_stop();
    return 1;
}

static int i2c_read_regs(uint8_t reg, uint8_t *data, uint8_t length)
{
    if ((data == 0) || (length < 1U)) return 0;
    if (!wait_bus_free()) return 0;

    I2C1->CR1 |= (1U << 10);
    I2C1->CR1 |= (1U << 8);
    if (!wait_sr1(1U << 0)) return 0;

    I2C1->DR = MPU6050_ADDR_W;
    if (!wait_sr1(1U << 1)) return 0;
    (void)I2C1->SR1;
    (void)I2C1->SR2;

    I2C1->DR = reg;
    if (!wait_sr1(1U << 7)) return 0;

    I2C1->CR1 |= (1U << 8);
    if (!wait_sr1(1U << 0)) return 0;

    I2C1->DR = MPU6050_ADDR_R;
    if (!wait_sr1(1U << 1)) return 0;

    if (length == 1U)
    {
        I2C1->CR1 &= ~(1U << 10);
        (void)I2C1->SR1;
        (void)I2C1->SR2;
        i2c_stop();
        if (!wait_sr1(1U << 6)) return 0;
        data[0] = (uint8_t)I2C1->DR;
        I2C1->CR1 |= (1U << 10);
        return 1;
    }

    (void)I2C1->SR1;
    (void)I2C1->SR2;
    I2C1->CR1 |= (1U << 10);

    if (length == 2U)
    {
        I2C1->CR1 |= (1U << 11);
        I2C1->CR1 &= ~(1U << 10);
        if (!wait_sr1(1U << 2)) return 0;
        i2c_stop();
        data[0] = (uint8_t)I2C1->DR;
        data[1] = (uint8_t)I2C1->DR;
        I2C1->CR1 &= ~(1U << 11);
        I2C1->CR1 |= (1U << 10);
        return 1;
    }

    for (uint8_t i = 0U; i < (uint8_t)(length - 3U); i++)
    {
        if (!wait_sr1(1U << 6)) return 0;
        data[i] = (uint8_t)I2C1->DR;
    }

    if (!wait_sr1(1U << 2)) return 0;
    I2C1->CR1 &= ~(1U << 10);
    data[length - 3U] = (uint8_t)I2C1->DR;

    if (!wait_sr1(1U << 2)) return 0;
    i2c_stop();
    data[length - 2U] = (uint8_t)I2C1->DR;
    data[length - 1U] = (uint8_t)I2C1->DR;

    I2C1->CR1 |= (1U << 10);
    return 1;
}

void I2C1_Init(void)
{
    RCC->AHB1ENR |= (1U << 1);       /* GPIOB clock */
    RCC->APB1ENR |= (1U << 21);      /* I2C1 clock */

    GPIOB->MODER &= ~((3U << 16) | (3U << 18));
    GPIOB->MODER |=  ((2U << 16) | (2U << 18));

    GPIOB->OTYPER |= (1U << 8) | (1U << 9);

    GPIOB->OSPEEDR &= ~((3U << 16) | (3U << 18));
    GPIOB->OSPEEDR |=  ((3U << 16) | (3U << 18));

    GPIOB->PUPDR &= ~((3U << 16) | (3U << 18));
    GPIOB->PUPDR |=  ((1U << 16) | (1U << 18));

    GPIOB->AFR[1] &= ~((15U << 0) | (15U << 4));
    GPIOB->AFR[1] |=  ((4U << 0) | (4U << 4));

    RCC->APB1RSTR |=  (1U << 21);
    RCC->APB1RSTR &= ~(1U << 21);

    I2C1->CR1 = 0U;
    I2C1->CR2 = 16U;                 /* PCLK1 = 16 MHz */
    I2C1->CCR = 80U;                 /* 100 kHz standard mode */
    I2C1->TRISE = 17U;
    I2C1->CR1 = (1U << 0);           /* PE */
}

uint8_t MPU6050_ReadWhoAmI(void)
{
    uint8_t value = 0xFFU;
    (void)i2c_read_regs(0x75U, &value, 1U);
    return value;
}

int MPU6050_Init(void)
{
    if (!i2c_write_reg(0x6BU, 0x00U)) return 0; /* Wake up */
    if (!i2c_write_reg(0x19U, 0x09U)) return 0; /* 100 Hz */
    if (!i2c_write_reg(0x1AU, 0x03U)) return 0; /* DLPF */
    if (!i2c_write_reg(0x1BU, 0x00U)) return 0; /* Gyro +/-250 dps */
    if (!i2c_write_reg(0x1CU, 0x00U)) return 0; /* Accel +/-2g */
    return 1;
}

int MPU6050_ReadData(MPU6050_Data *data)
{
    uint8_t b[14];

    if (data == 0) return 0;
    if (!i2c_read_regs(0x3BU, b, 14U)) return 0;

    data->ax = (int16_t)(((uint16_t)b[0] << 8) | b[1]);
    data->ay = (int16_t)(((uint16_t)b[2] << 8) | b[3]);
    data->az = (int16_t)(((uint16_t)b[4] << 8) | b[5]);
    data->gx = (int16_t)(((uint16_t)b[8] << 8) | b[9]);
    data->gy = (int16_t)(((uint16_t)b[10] << 8) | b[11]);
    data->gz = (int16_t)(((uint16_t)b[12] << 8) | b[13]);

    return 1;
}
