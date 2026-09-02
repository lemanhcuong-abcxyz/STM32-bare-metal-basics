#include <stdint.h>
#include <stdio.h>
#include "stm32f4xx.h"

void UART2_Init(uint32_t baudrate) {
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN;

    GPIOA->MODER &= ~((3U << (2 * 2)) | (3U << (3 * 2)));
    GPIOA->MODER |=  ((2U << (2 * 2)) | (2U << (3 * 2)));

    GPIOA->AFR[0] &= ~((0xFU << (2 * 4)) | (0xFU << (3 * 4)));
    GPIOA->AFR[0] |=  ((7U << (2 * 4)) | (7U << (3 * 4)));

    uint32_t pclk1 = 16000000U;
    USART2->BRR = (pclk1 + (baudrate / 2U)) / baudrate;

    USART2->CR1 |= USART_CR1_TE | USART_CR1_RE | USART_CR1_UE;
}

void UART2_SendChar(char c) {
    while (!(USART2->SR & USART_SR_TXE));
    USART2->DR = (uint8_t)c;
}

int _write(int file, char *ptr, int len) {
    (void)file;
    for (int i = 0; i < len; i++) {
        if (ptr[i] == '\n') {
            UART2_SendChar('\r');
        }
        UART2_SendChar(ptr[i]);
    }
    return len;
}

int main(void) {
    setvbuf(stdout, NULL, _IONBF, 0);

    UART2_Init(115200);

    printf("\n=====================================\n");
    printf("  CHAO MANG BAN DEN VOI STM32F411RE! \n");
    printf("=====================================\n");

    int counter = 0;
    while (1) {
        printf("[DEBUG] Uptime: %d giay\n", counter++);
        for (volatile int i = 0; i < 2000000; i++);
    }
}
