#include "stm32f4xx.h"

volatile uint32_t millis = 0;
volatile uint32_t last_press = 0;

void SysTick_Handler(void)
{
    millis++;
}

void EXTI15_10_IRQHandler(void)
{
     if (EXTI->PR & (1 << 13))
    {
         EXTI->PR = (1 << 13);

          if ((millis - last_press) >= 50)
        {
            last_press = millis;
            GPIOA->ODR ^= (1 << 5);
        }
    }
}

int main(void)
{
    RCC->AHB1ENR |= (1 << 0);

    RCC->AHB1ENR |= (1 << 2);

    RCC->APB2ENR |= (1 << 14);

    GPIOA->MODER &= ~(3 << (5 * 2));
    GPIOA->MODER |=  (1 << (5 * 2));

    GPIOC->MODER &= ~(3 << (13 * 2));

    GPIOC->PUPDR &= ~(3 << (13 * 2));
    GPIOC->PUPDR |=  (1 << (13 * 2));

    SYSCFG->EXTICR[3] &= ~(0xF << 4);

    SYSCFG->EXTICR[3] |= (0x2 << 4);

    EXTI->IMR |= (1 << 13);
    EXTI->FTSR |= (1 << 13);
    EXTI->RTSR &= ~(1 << 13);
    NVIC->ISER[1] |= (1 << 8);


    SysTick->LOAD = 16000 - 1;
    SysTick->VAL = 0;
    SysTick->CTRL = 7;

    while (1)
    {

    }
}
