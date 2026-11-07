
#include "stm32f4xx.h"
#include "mpu6050.h"
#include "filters.h"
#include <math.h>
#include <stdint.h>

#define RAD_TO_DEG  57.2957795f
#define SAMPLE_MS   10U
#define ALPHA       0.98f
#define CAL_SAMPLES 300U

volatile uint32_t system_ms = 0U;

static float gyro_x_offset = 0.0f;
static float gyro_y_offset = 0.0f;

static void clock_gpio_uart_init(void);
static void uart2_init(void);
static void uart2_send_char(char c);
static void uart2_send_string(const char *s);
static void uart2_send_fixed2(float value);
static void uart2_send_csv(float t, float ra, float rc, float rk,
                           float pa, float pc, float pk);
static void delay_ms(uint32_t ms);
static void gyro_calibrate(void);
static void led_set(int on);

void SysTick_Handler(void)
{
    system_ms++;
}

int main(void)
{
    MPU6050_Data sensor;
    Kalman1D kalman_roll;
    Kalman1D kalman_pitch;

    float ax, ay, az;
    float gx, gy;
    float roll_acc, pitch_acc;
    float roll_comp = 0.0f;
    float pitch_comp = 0.0f;
    float roll_kalman, pitch_kalman;

    uint32_t last_sample;
    uint32_t sample_number = 0U;
    int first_sample = 1;

    clock_gpio_uart_init();
    uart2_init();

    if (SysTick_Config(SystemCoreClock / 1000U) != 0U)
    {
        uart2_send_string("ERROR: SysTick config\r\n");
        while (1) {}
    }

    I2C1_Init();
    delay_ms(100U);

    uart2_send_string("\r\nMPU6050 FILTER COMPARISON\r\n");
    uart2_send_string("Bare-metal CMSIS - no HAL\r\n");

    if (!MPU6050_Init())
    {
        uart2_send_string("ERROR: MPU6050 I2C init\r\n");
        led_set(0);
        while (1) {}
    }

    delay_ms(100U);

    if (MPU6050_ReadWhoAmI() != 0x68U)
    {
        uart2_send_string("ERROR: WHO_AM_I is not 0x68\r\n");
        led_set(0);
        while (1) {}
    }

    led_set(1);
    uart2_send_string("MPU6050 OK\r\n");

    gyro_calibrate();

    uart2_send_string(
        "time,roll_acc,roll_comp,roll_kalman,"
        "pitch_acc,pitch_comp,pitch_kalman\r\n"
    );

    last_sample = system_ms;

    while (1)
    {
        if ((uint32_t)(system_ms - last_sample) >= SAMPLE_MS)
        {
            uint32_t now = system_ms;
            float dt = (float)(now - last_sample) / 1000.0f;

            last_sample = now;

            if (!MPU6050_ReadData(&sensor))
            {
                uart2_send_string("ERROR: I2C read\r\n");
                led_set(0);
                continue;
            }

            ax = (float)sensor.ax / 16384.0f;
            ay = (float)sensor.ay / 16384.0f;
            az = (float)sensor.az / 16384.0f;

            gx = (float)sensor.gx / 131.0f - gyro_x_offset;
            gy = (float)sensor.gy / 131.0f - gyro_y_offset;

            roll_acc = atan2f(ay, az) * RAD_TO_DEG;

            pitch_acc =
                atan2f(-ax, sqrtf(ay * ay + az * az)) * RAD_TO_DEG;

            if (first_sample)
            {
                roll_comp = roll_acc;
                pitch_comp = pitch_acc;

                Kalman1D_Init(&kalman_roll, roll_acc);
                Kalman1D_Init(&kalman_pitch, pitch_acc);

                first_sample = 0;
            }
            else
            {
                roll_comp = Complementary_Update(
                    roll_comp, gx, roll_acc, dt, ALPHA
                );

                pitch_comp = Complementary_Update(
                    pitch_comp, gy, pitch_acc, dt, ALPHA
                );
            }

            roll_kalman = Kalman1D_Update(
                &kalman_roll, roll_acc, gx, dt
            );

            pitch_kalman = Kalman1D_Update(
                &kalman_pitch, pitch_acc, gy, dt
            );

            sample_number++;

            uart2_send_csv(
                (float)sample_number * 0.01f,
                roll_acc,
                roll_comp,
                roll_kalman,
                pitch_acc,
                pitch_comp,
                pitch_kalman
            );
        }
    }
}

static void clock_gpio_uart_init(void)
{
    RCC->AHB1ENR |= (1U << 0);

    GPIOA->MODER &= ~(3U << (5U * 2U));
    GPIOA->MODER |=  (1U << (5U * 2U));

    GPIOA->OTYPER &= ~(1U << 5U);

    GPIOA->OSPEEDR &= ~(3U << (5U * 2U));
    GPIOA->PUPDR &= ~(3U << (5U * 2U));

    led_set(0);
}

static void uart2_init(void)
{
    RCC->AHB1ENR |= (1U << 0);
    RCC->APB1ENR |= (1U << 17);

    GPIOA->MODER &= ~((3U << 4U) | (3U << 6U));
    GPIOA->MODER |=  ((2U << 4U) | (2U << 6U));

    GPIOA->AFR[0] &= ~((15U << 8U) | (15U << 12U));
    GPIOA->AFR[0] |=  ((7U << 8U) | (7U << 12U));

    USART2->CR1 = 0U;
    USART2->BRR = 139U;

    USART2->CR1 =
        (1U << 13U) |
        (1U << 3U)  |
        (1U << 2U);
}

static void uart2_send_char(char c)
{
    while ((USART2->SR & (1U << 7U)) == 0U) {}

    USART2->DR = (uint8_t)c;
}

static void uart2_send_string(const char *s)
{
    while (*s != '\0')
    {
        uart2_send_char(*s++);
    }
}

static void uart2_send_fixed2(float value)
{
    int32_t scaled;
    uint32_t magnitude;
    uint32_t whole;
    uint32_t fraction;

    if (value < 0.0f)
    {
        uart2_send_char('-');
        value = -value;
    }

    scaled = (int32_t)(value * 100.0f + 0.5f);
    magnitude = (uint32_t)scaled;

    whole = magnitude / 100U;
    fraction = magnitude % 100U;

    if (whole >= 1000U)
    {
        uart2_send_char((char)('0' + (whole / 1000U) % 10U));
    }

    if (whole >= 100U)
    {
        uart2_send_char((char)('0' + (whole / 100U) % 10U));
    }

    if (whole >= 10U)
    {
        uart2_send_char((char)('0' + (whole / 10U) % 10U));
    }

    uart2_send_char((char)('0' + whole % 10U));
    uart2_send_char('.');

    uart2_send_char((char)('0' + fraction / 10U));
    uart2_send_char((char)('0' + fraction % 10U));
}

static void uart2_send_csv(float t, float ra, float rc, float rk,
                           float pa, float pc, float pk)
{
    uart2_send_fixed2(t);
    uart2_send_char(',');

    uart2_send_fixed2(ra);
    uart2_send_char(',');

    uart2_send_fixed2(rc);
    uart2_send_char(',');

    uart2_send_fixed2(rk);
    uart2_send_char(',');

    uart2_send_fixed2(pa);
    uart2_send_char(',');

    uart2_send_fixed2(pc);
    uart2_send_char(',');

    uart2_send_fixed2(pk);
    uart2_send_string("\r\n");
}

static void delay_ms(uint32_t ms)
{
    uint32_t start = system_ms;

    while ((uint32_t)(system_ms - start) < ms) {}
}

static void gyro_calibrate(void)
{
    MPU6050_Data sample;

    int32_t sum_x = 0;
    int32_t sum_y = 0;

    uart2_send_string(
        "Keep sensor still: gyro calibration...\r\n"
    );

    for (uint32_t i = 0U; i < CAL_SAMPLES; i++)
    {
        if (MPU6050_ReadData(&sample))
        {
            sum_x += sample.gx;
            sum_y += sample.gy;
        }

        delay_ms(SAMPLE_MS);
    }

    gyro_x_offset =
        ((float)sum_x / (float)CAL_SAMPLES) / 131.0f;

    gyro_y_offset =
        ((float)sum_y / (float)CAL_SAMPLES) / 131.0f;

    uart2_send_string("Calibration done\r\n");
}

static void led_set(int on)
{
    if (on)
    {
        GPIOA->ODR |= (1U << 5U);
    }
    else
    {
        GPIOA->ODR &= ~(1U << 5U);
    }
}
