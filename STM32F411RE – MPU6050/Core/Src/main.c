
#include "stm32f4xx.h"
#include <math.h>
#include <stdint.h>

typedef struct {
    int16_t ax;
    int16_t ay;
    int16_t az;
    int16_t gx;
    int16_t gy;
    int16_t gz;
} MPU6050_Data;

void I2C1_Init(void);
void I2C1_WriteReg(uint8_t reg, uint8_t data);
uint8_t I2C1_ReadReg(uint8_t reg);
void MPU6050_Init(void);
void MPU6050_ReadData(MPU6050_Data *data);
void SysTick_Init(void);
void MPU6050_CalibrateGyro(void);
void USART2_Init(void);
void USART2_SendChar(char c);
void USART2_SendString(char *str);
void USART2_SendFloat(float value);
volatile uint32_t tick_10ms = 0;
float gx_offset = 0.0f;
float gy_offset = 0.0f;
float gz_offset = 0.0f;
int main(void)
{
    MPU6050_Data sensor;
    uint8_t who_am_i;
    uint32_t last_tick = 0;

    float ax_g, ay_g, az_g;
    float gx_dps, gy_dps;
    float roll_acc, pitch_acc;
    float roll = 0.0f;
    float pitch = 0.0f;
    float dt = 0.01f;
    float alpha = 0.98f;

    RCC->AHB1ENR |= (1U << 0);

    GPIOA->MODER &= ~(3U << (5 * 2));
    GPIOA->MODER |= (1U << (5 * 2));

    I2C1_Init();
    MPU6050_Init();
    SysTick_Init();
    USART2_Init();

    USART2_SendString("UART OK\r\n");

    who_am_i = I2C1_ReadReg(0x75);

    if (who_am_i == 0x68)
    {
        GPIOA->ODR |= (1U << 5);

        MPU6050_CalibrateGyro();

        USART2_SendString("MPU6050 OK\r\n");

        while (1)
        {
            if (tick_10ms != last_tick)
            {
                last_tick = tick_10ms;

                MPU6050_ReadData(&sensor);

                ax_g = sensor.ax / 16384.0f;
                ay_g = sensor.ay / 16384.0f;
                az_g = sensor.az / 16384.0f;

                gx_dps = sensor.gx / 131.0f - gx_offset;
                gy_dps = sensor.gy / 131.0f - gy_offset;

                roll_acc = atan2f(ay_g, az_g) * 57.29578f;

                pitch_acc = atan2f(
                    -ax_g,
                    sqrtf(ay_g * ay_g + az_g * az_g)
                ) * 57.29578f;

                roll = alpha * (roll + gx_dps * dt)
                       + (1.0f - alpha) * roll_acc;

                pitch = alpha * (pitch + gy_dps * dt)
                        + (1.0f - alpha) * pitch_acc;

                USART2_SendString("Roll: ");
                USART2_SendFloat(roll);
                USART2_SendString(" Pitch: ");
                USART2_SendFloat(pitch);
                USART2_SendString("\r\n");
            }
        }
    }
    else
    {
        GPIOA->ODR &= ~(1U << 5);

        USART2_SendString("MPU6050 ERROR\r\n");
        USART2_SendString("WHO_AM_I: ");
        USART2_SendFloat((float)who_am_i);
        USART2_SendString("\r\n");

        while (1)
        {
        }
    }
}

void SysTick_Init(void)
{
    SysTick->LOAD = 159999;
    SysTick->VAL = 0;
    SysTick->CTRL = (1U << 2) | (1U << 1) | (1U << 0);
}

void I2C1_Init(void)
{
    RCC->AHB1ENR |= (1U << 1);
    RCC->APB1ENR |= (1U << 21);

    GPIOB->MODER &= ~((3U << 16) | (3U << 18));
    GPIOB->MODER |= (2U << 16) | (2U << 18);

    GPIOB->OTYPER |= (1U << 8) | (1U << 9);

    GPIOB->PUPDR &= ~((3U << 16) | (3U << 18));
    GPIOB->PUPDR |= (1U << 16) | (1U << 18);

    GPIOB->AFR[1] &= ~((15U << 0) | (15U << 4));
    GPIOB->AFR[1] |= (4U << 0) | (4U << 4);

    I2C1->CR1 = 0;
    I2C1->CR2 = 16;
    I2C1->CCR = 80;
    I2C1->TRISE = 17;

    I2C1->CR1 |= (1U << 0);
}

void I2C1_WriteReg(uint8_t reg, uint8_t data)
{
    uint32_t timeout = 100000;

    while ((I2C1->SR2 & (1U << 1)) && --timeout);
    if (timeout == 0) return;

    I2C1->CR1 |= (1U << 8);

    timeout = 100000;
    while (!(I2C1->SR1 & (1U << 0)) && --timeout);
    if (timeout == 0) return;

    I2C1->DR = 0xD0;

    timeout = 100000;
    while (!(I2C1->SR1 & (1U << 1)) && --timeout);
    if (timeout == 0) return;

    (void)I2C1->SR1;
    (void)I2C1->SR2;

    I2C1->DR = reg;

    timeout = 100000;
    while (!(I2C1->SR1 & (1U << 7)) && --timeout);
    if (timeout == 0) return;

    I2C1->DR = data;

    timeout = 100000;
    while (!(I2C1->SR1 & (1U << 2)) && --timeout);
    if (timeout == 0) return;

    I2C1->CR1 |= (1U << 9);
}

uint8_t I2C1_ReadReg(uint8_t reg)
{
    uint32_t timeout;
    uint8_t data = 0;

    timeout = 100000;
    while ((I2C1->SR2 & (1U << 1)) && --timeout);
    if (timeout == 0) return 0xFF;

    I2C1->CR1 |= (1U << 8);

    timeout = 100000;
    while (!(I2C1->SR1 & (1U << 0)) && --timeout);
    if (timeout == 0) return 0xFF;

    I2C1->DR = 0xD0;

    timeout = 100000;
    while (!(I2C1->SR1 & (1U << 1)) && --timeout);
    if (timeout == 0) return 0xFF;

    (void)I2C1->SR1;
    (void)I2C1->SR2;

    I2C1->DR = reg;

    timeout = 100000;
    while (!(I2C1->SR1 & (1U << 2)) && --timeout);
    if (timeout == 0) return 0xFF;

    I2C1->CR1 |= (1U << 8);

    timeout = 100000;
    while (!(I2C1->SR1 & (1U << 0)) && --timeout);
    if (timeout == 0) return 0xFF;

    I2C1->DR = 0xD1;

    timeout = 100000;
    while (!(I2C1->SR1 & (1U << 1)) && --timeout);
    if (timeout == 0) return 0xFF;

    I2C1->CR1 &= ~(1U << 10);

    (void)I2C1->SR1;
    (void)I2C1->SR2;

    I2C1->CR1 |= (1U << 9);

    timeout = 100000;
    while (!(I2C1->SR1 & (1U << 6)) && --timeout);
    if (timeout == 0) return 0xFF;

    data = I2C1->DR;

    I2C1->CR1 |= (1U << 10);

    return data;
}

void MPU6050_Init(void)
{
    I2C1_WriteReg(0x6B, 0x00);
    I2C1_WriteReg(0x19, 0x09);
    I2C1_WriteReg(0x1A, 0x03);
    I2C1_WriteReg(0x1B, 0x00);
    I2C1_WriteReg(0x1C, 0x00);
}

void MPU6050_ReadData(MPU6050_Data *data)
{
    uint8_t buffer[14];

    for (uint8_t i = 0; i < 14; i++)
    {
        buffer[i] = I2C1_ReadReg(0x3B + i);
    }

    data->ax = (int16_t)((buffer[0] << 8) | buffer[1]);
    data->ay = (int16_t)((buffer[2] << 8) | buffer[3]);
    data->az = (int16_t)((buffer[4] << 8) | buffer[5]);

    data->gx = (int16_t)((buffer[8] << 8) | buffer[9]);
    data->gy = (int16_t)((buffer[10] << 8) | buffer[11]);
    data->gz = (int16_t)((buffer[12] << 8) | buffer[13]);
}

void USART2_Init(void)
{
    RCC->AHB1ENR |= (1U << 0);
    RCC->APB1ENR |= (1U << 17);

    GPIOA->MODER &= ~((3U << 4) | (3U << 6));
    GPIOA->MODER |= (2U << 4) | (2U << 6);

    GPIOA->AFR[0] &= ~((15U << 8) | (15U << 12));
    GPIOA->AFR[0] |= (7U << 8) | (7U << 12);

    USART2->BRR = 139;
    USART2->CR1 = (1U << 3) | (1U << 13);
}

void USART2_SendChar(char c)
{
    while (!(USART2->SR & (1U << 7)));
    USART2->DR = c;
}

void USART2_SendString(char *str)
{
    while (*str)
    {
        USART2_SendChar(*str);
        str++;
    }
}

void USART2_SendFloat(float value)
{
    int32_t integer;
    uint32_t decimal;

    if (value < 0)
    {
        USART2_SendChar('-');
        value = -value;
    }

    integer = (int32_t)value;
    decimal = (uint32_t)((value - integer) * 100.0f);

    char buffer[12];
    uint8_t i = 0;

    if (integer == 0)
    {
        buffer[i++] = '0';
    }
    else
    {
        char temp[10];
        uint8_t j = 0;

        while (integer > 0)
        {
            temp[j++] = (integer % 10) + '0';
            integer /= 10;
        }

        while (j > 0)
        {
            buffer[i++] = temp[--j];
        }
    }

    buffer[i++] = '.';
    buffer[i++] = (decimal / 10) + '0';
    buffer[i++] = (decimal % 10) + '0';
    buffer[i] = '\0';

    USART2_SendString(buffer);
}

void MPU6050_CalibrateGyro(void)
{
    MPU6050_Data sensor;
    int32_t sum_x = 0;
    int32_t sum_y = 0;
    int32_t sum_z = 0;
    uint32_t last_tick = tick_10ms;

    USART2_SendString("Calibrating...\r\n");

    for (int i = 0; i < 500; i++)
    {
        while (tick_10ms == last_tick);

        last_tick = tick_10ms;

        MPU6050_ReadData(&sensor);

        sum_x += sensor.gx;
        sum_y += sensor.gy;
        sum_z += sensor.gz;
    }

    gx_offset = (float)sum_x / 500.0f / 131.0f;
    gy_offset = (float)sum_y / 500.0f / 131.0f;
    gz_offset = (float)sum_z / 500.0f / 131.0f;

    USART2_SendString("Calibration done\r\n");
}
