#include "motor.h"

#define IN1_PORT GPIOA
#define IN1_PIN  GPIO_PIN_0

#define IN2_PORT GPIOA
#define IN2_PIN  GPIO_PIN_1

#define IN3_PORT GPIOA
#define IN3_PIN  GPIO_PIN_4

#define IN4_PORT GPIOB
#define IN4_PIN  GPIO_PIN_0

#define STEPS_PER_REV 4096.0f
#define TIMER_TICK_US 100U

static const uint8_t sequence[8] = {
    0x1, 0x3, 0x2, 0x6,
    0x4, 0xC, 0x8, 0x9
};

static volatile uint8_t step_index = 0;
static volatile uint32_t step_interval_us = 0;
static volatile uint32_t elapsed_us = 0;
static volatile uint8_t motor_enabled = 0;

static void Motor_Output(uint8_t state)
{
    HAL_GPIO_WritePin(IN1_PORT, IN1_PIN,
        (state & 0x01) ? GPIO_PIN_SET : GPIO_PIN_RESET);

    HAL_GPIO_WritePin(IN2_PORT, IN2_PIN,
        (state & 0x02) ? GPIO_PIN_SET : GPIO_PIN_RESET);

    HAL_GPIO_WritePin(IN3_PORT, IN3_PIN,
        (state & 0x04) ? GPIO_PIN_SET : GPIO_PIN_RESET);

    HAL_GPIO_WritePin(IN4_PORT, IN4_PIN,
        (state & 0x08) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void Motor_Init(void)
{
    step_index = 0;
    elapsed_us = 0;
    step_interval_us = 0;
    motor_enabled = 0;

    Motor_Output(0);
}

void Motor_SetSpeed(float rpm)
{
    if (rpm <= 0.0f)
    {
        Motor_Stop();
        return;
    }

    if (rpm > 10.0f)
    {
        rpm = 10.0f;
    }

    step_interval_us =
        (uint32_t)(60000000.0f / (STEPS_PER_REV * rpm));

    if (step_interval_us == 0)
    {
        step_interval_us = 1;
    }

    elapsed_us = 0;
    motor_enabled = 1;
}

void Motor_Stop(void)
{
    motor_enabled = 0;
    elapsed_us = 0;

    Motor_Output(0);
}

void Motor_Update(void)
{
    if (!motor_enabled)
    {
        return;
    }

    elapsed_us += TIMER_TICK_US;

    if (elapsed_us >= step_interval_us)
    {
        elapsed_us = 0;

        step_index++;

        if (step_index >= 8)
        {
            step_index = 0;
        }

        Motor_Output(sequence[step_index]);
    }
}
