#include "stm32f4xx.h"
#include <stdint.h>
#include <stdio.h>

void UART2_Init(void)
{
    RCC->AHB1ENR |= (1U << 0);

    GPIOA->MODER &= ~(3U << (2 * 2));
    GPIOA->MODER |=  (2U << (2 * 2));

    GPIOA->MODER &= ~(3U << (3 * 2));
    GPIOA->MODER |=  (2U << (3 * 2));

    GPIOA->AFR[0] &= ~(0xFU << (2 * 4));
    GPIOA->AFR[0] |=  (7U << (2 * 4));

    GPIOA->AFR[0] &= ~(0xFU << (3 * 4));
    GPIOA->AFR[0] |=  (7U << (3 * 4));

    RCC->APB1ENR |= (1U << 17);

    USART2->BRR = 139;

    USART2->CR1 |= (1U << 3);
    USART2->CR1 |= (1U << 13);
}

void UART2_SendChar(char c)
{
    while (!(USART2->SR & (1U << 7)))
    {
    }

    USART2->DR = c;
}

int _write(int file, char *ptr, int len)
{
    int i;

    for (i = 0; i < len; i++)
    {
        UART2_SendChar(ptr[i]);
    }

    return len;
}

void TIM3_PWM_Init(void)
{
    RCC->AHB1ENR |= (1U << 0);

    GPIOA->MODER &= ~(3U << (6 * 2));
    GPIOA->MODER |=  (2U << (6 * 2));

    GPIOA->AFR[0] &= ~(0xFU << (6 * 4));
    GPIOA->AFR[0] |=  (2U << (6 * 4));

    RCC->APB1ENR |= (1U << 1);

    TIM3->PSC = 15;
    TIM3->ARR = 999;
    TIM3->CCR1 = 500;

    TIM3->CCMR1 &= ~(3U << 0);
    TIM3->CCMR1 &= ~(7U << 4);
    TIM3->CCMR1 |=  (6U << 4);

    TIM3->CCER |= (1U << 0);

    TIM3->EGR |= (1U << 0);

    TIM3->CR1 |= (1U << 0);
}

void TIM2_InputCapture_Init(void)
{
    RCC->AHB1ENR |= (1U << 0);

    GPIOA->MODER &= ~(3U << (0 * 2));
    GPIOA->MODER |=  (2U << (0 * 2));

    GPIOA->AFR[0] &= ~(0xFU << (0 * 4));
    GPIOA->AFR[0] |=  (1U << (0 * 4));

    RCC->APB1ENR |= (1U << 0);

    TIM2->PSC = 15;
    TIM2->ARR = 0xFFFFFFFF;

    TIM2->CCMR1 &= ~(3U << 0);
    TIM2->CCMR1 |=  (1U << 0);

    TIM2->CCER &= ~(1U << 1);
    TIM2->CCER |=  (1U << 0);

    TIM2->EGR |= (1U << 0);

    TIM2->CR1 |= (1U << 0);
}

int main(void)
{
    uint32_t capture1;
    uint32_t capture2;
    uint32_t difference;
    uint32_t frequency;

    UART2_Init();

    TIM3_PWM_Init();

    TIM2_InputCapture_Init();

    while (!(TIM2->SR & (1U << 1)))
    {
    }

    capture1 = TIM2->CCR1;

    TIM2->SR &= ~(1U << 1);

    while (1)
    {
        while (!(TIM2->SR & (1U << 1)))
        {
        }

        capture2 = TIM2->CCR1;

        TIM2->SR &= ~(1U << 1);

        difference = capture2 - capture1;

        if (difference != 0)
        {
            frequency = 1000000U / difference;

            printf("PWM Frequency = %lu Hz\r\n",
                   (unsigned long)frequency);
        }

        capture1 = capture2;
    }
}
