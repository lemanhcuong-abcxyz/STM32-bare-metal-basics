
#include "stm32f4xx.h"
#include "gps.h"
#include <stdint.h>

static void GPIO_Init(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    (void)RCC->AHB1ENR;

    GPIOA->MODER &= ~((3U << 4)  |
                      (3U << 6)  |
                      (3U << 18) |
                      (3U << 20));

    GPIOA->MODER |=  ((2U << 4)  |
                      (2U << 6)  |
                      (2U << 18) |
                      (2U << 20));

    GPIOA->AFR[0] &= ~((0xFU << 8) | (0xFU << 12));
    GPIOA->AFR[0] |=  ((7U << 8) | (7U << 12));

    GPIOA->AFR[1] &= ~((0xFU << 4) | (0xFU << 8));
    GPIOA->AFR[1] |=  ((7U << 4) | (7U << 8));
}

static void USART_Init(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN;

    (void)RCC->APB2ENR;
    (void)RCC->APB1ENR;

    USART1->CR1 = 0;
    USART1->CR2 = 0;
    USART1->CR3 = 0;
    USART1->BRR = 1667;
    USART1->CR1 = USART_CR1_RE | USART_CR1_UE;

    USART2->CR1 = 0;
    USART2->CR2 = 0;
    USART2->CR3 = 0;
    USART2->BRR = 139;
    USART2->CR1 = USART_CR1_TE | USART_CR1_UE;
}

static void UART2_SendChar(char c)
{
    while (!(USART2->SR & USART_SR_TXE)) {}
    USART2->DR = (uint8_t)c;
}

static void UART2_SendString(const char *s)
{
    while (*s) {
        UART2_SendChar(*s++);
    }
}

static void UART2_SendUnsigned(uint32_t value)
{
    char digits[10];
    uint32_t i = 0;

    do {
        digits[i++] = (char)('0' + value % 10U);
        value /= 10U;
    } while (value && i < sizeof(digits));

    while (i > 0) {
        UART2_SendChar(digits[--i]);
    }
}

static void UART2_SendCoordinate(int32_t value)
{
    uint32_t magnitude;

    if (value < 0) {
        UART2_SendChar('-');
        magnitude = (uint32_t)(-value);
    } else {
        magnitude = (uint32_t)value;
    }

    UART2_SendUnsigned(magnitude / 1000000U);
    UART2_SendChar('.');

    uint32_t fraction = magnitude % 1000000U;
    uint32_t divisor = 100000U;

    while (divisor > 0) {
        UART2_SendChar((char)('0' + fraction / divisor));
        fraction %= divisor;
        divisor /= 10U;
    }
}

int main(void)
{
    GPIO_Init();
    USART_Init();
    GPS_Init();

    UART2_SendString("GPS NMEA Parser Started\r\n");

    while (1) {
        if (USART1->SR & USART_SR_RXNE) {
            char data = (char)(USART1->DR & 0xFFU);
            GPS_ProcessByte(data);
        }

        GPS_Data gps = GPS_GetData();

        if (gps.fix) {
            UART2_SendString("GPS FIX VALID\r\nLatitude: ");
            UART2_SendCoordinate(gps.latitude);
            UART2_SendString("\r\nLongitude: ");
            UART2_SendCoordinate(gps.longitude);
            UART2_SendString("\r\n\r\n");

            while (USART1->SR & USART_SR_RXNE) {
                (void)USART1->DR;
            }

            gps.fix = 0;
        }
    }
}
