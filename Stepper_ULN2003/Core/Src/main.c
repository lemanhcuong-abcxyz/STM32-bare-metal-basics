
#include "stm32f4xx.h"
#include <stdint.h>

#define IN1 (1U << 0)  // PA0 -> A0 -> IN1
#define IN2 (1U << 1)  // PA1 -> A1 -> IN2
#define IN3 (1U << 4)  // PA4 -> A2 -> IN3
#define IN4 (1U << 0)  // PB0 -> A3 -> IN4

#define MOTOR_PINS_A (IN1 | IN2 | IN3)
#define MOTOR_PINS_B IN4

static const uint8_t step_sequence[8] = {
    0x1, 0x3, 0x2, 0x6,
    0x4, 0xC, 0x8, 0x9
};

#define STEP_DELAY_MS 5U

static void delay_ms(uint32_t ms)
{
    while (ms--) {
        for (volatile uint32_t i = 0; i < 4000U; i++) {
            __NOP();
        }
    }
}

static void gpio_init(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN |
                    RCC_AHB1ENR_GPIOBEN;
    (void)RCC->AHB1ENR;

    GPIOA->MODER &= ~((3U << 0) |
                      (3U << 2) |
                      (3U << 8));
    GPIOA->MODER |=  ((1U << 0) |
                      (1U << 2) |
                      (1U << 8));

    GPIOB->MODER &= ~(3U << 0);
    GPIOB->MODER |=  (1U << 0);

    GPIOA->OTYPER &= ~MOTOR_PINS_A;
    GPIOB->OTYPER &= ~MOTOR_PINS_B;

    GPIOA->PUPDR &= ~((3U << 0) |
                      (3U << 2) |
                      (3U << 8));
    GPIOB->PUPDR &= ~(3U << 0);

    GPIOA->BSRR = MOTOR_PINS_A << 16;
    GPIOB->BSRR = MOTOR_PINS_B << 16;
}

static void motor_write(uint8_t pattern)
{
    uint32_t set_a = 0U;
    uint32_t set_b = 0U;

    if (pattern & 0x1U) set_a |= IN1;
    if (pattern & 0x2U) set_a |= IN2;
    if (pattern & 0x4U) set_a |= IN3;
    if (pattern & 0x8U) set_b |= IN4;

    GPIOA->BSRR = (MOTOR_PINS_A << 16) | set_a;
    GPIOB->BSRR = (MOTOR_PINS_B << 16) | set_b;
}

int main(void)
{
    gpio_init();

    uint32_t index = 0U;

    while (1) {
        motor_write(step_sequence[index]);

        index++;
        if (index >= 8U) {
            index = 0U;
        }

        delay_ms(STEP_DELAY_MS);
    }
}