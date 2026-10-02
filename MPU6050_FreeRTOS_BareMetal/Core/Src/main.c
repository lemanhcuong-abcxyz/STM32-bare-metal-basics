#include "stm32f4xx.h"
#include "cmsis_os2.h"
#include "mpu6050.h"
#include <math.h>
#include <stdio.h>

typedef struct
{
    float roll;
    float pitch;
} Angle_Data;

/* MPU6050 data */
MPU6050_Data mpu_data;
MPU6050_Data processed_data;

/* Converted sensor data */
float ax_g, ay_g, az_g;
float gx_dps, gy_dps, gz_dps;

float roll_acc;
float pitch_acc;
float roll;
float pitch;

float dt = 0.01f;
float alpha = 0.98f;

Angle_Data angle_data;

osMessageQueueId_t AngleQueueHandle;

uint8_t angle_queue_status;

uint8_t mpu_status = 0;
uint8_t read_status = 0;
uint8_t queue_status = 0;
uint8_t process_status = 0;

/* Task handles */
osThreadId_t defaultTaskHandle;
osThreadId_t SensorTaskHandle;
osThreadId_t ProcessingTaskHandle;

osMessageQueueId_t SensorQueueHandle;

osThreadId_t UART_LogTaskHandle;

const osThreadAttr_t UART_LogTask_attributes = {
    .name = "UART_LogTask",
    .stack_size = 512 * 4,
    .priority = osPriorityNormal
};

Angle_Data uart_data;
uint8_t uart_queue_status;

/* Task attributes */
const osThreadAttr_t defaultTask_attributes = {
    .name = "defaultTask",
    .stack_size = 128 * 4,
    .priority = osPriorityNormal
};

const osThreadAttr_t SensorTask_attributes = {
    .name = "SensorTask",
    .stack_size = 512 * 4,
    .priority = osPriorityNormal
};

const osThreadAttr_t ProcessingTask_attributes = {
    .name = "ProcessingTask",
    .stack_size = 512 * 4,
    .priority = osPriorityNormal
};

/* Function prototypes */
void SystemClock_Config(void);
void StartDefaultTask(void *argument);
void StartSensorTask(void *argument);
void StartProcessingTask(void *argument);
void Error_Handler(void);
void StartUARTLogTask(void *argument);
void USART2_Init(void);
void USART2_SendChar(char c);
void USART2_SendString(char *str);

int main(void)
{
    /* Configure system clock */
    SystemClock_Config();

    /* Initialize FreeRTOS */
    osKernelInitialize();

    /* Create queue */
    SensorQueueHandle = osMessageQueueNew(
        10,
        sizeof(MPU6050_Data),
        NULL
    );

    if (SensorQueueHandle == NULL)
    {
        Error_Handler();
    }

    AngleQueueHandle = osMessageQueueNew(
        10,
        sizeof(Angle_Data),
        NULL
    );

    if (AngleQueueHandle == NULL)
    {
        Error_Handler();
    }

    /* Create tasks */
    defaultTaskHandle = osThreadNew(
        StartDefaultTask,
        NULL,
        &defaultTask_attributes
    );

    SensorTaskHandle = osThreadNew(
        StartSensorTask,
        NULL,
        &SensorTask_attributes
    );

    ProcessingTaskHandle = osThreadNew(
        StartProcessingTask,
        NULL,
        &ProcessingTask_attributes
    );

    if (defaultTaskHandle == NULL ||
        SensorTaskHandle == NULL ||
        ProcessingTaskHandle == NULL)
    {
        Error_Handler();
    }

    UART_LogTaskHandle = osThreadNew(
        StartUARTLogTask,
        NULL,
        &UART_LogTask_attributes
    );

    if (UART_LogTaskHandle == NULL)
    {
        Error_Handler();
    }

    /* Start scheduler */
    if (osKernelStart() != osOK)
    {
        Error_Handler();
    }

    while (1)
    {
    }
}

void USART2_Init(void)
{
    /* Enable GPIOA and USART2 clock */
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN;

    /* PA2, PA3 alternate function */
    GPIOA->MODER &= ~((3U << 4) | (3U << 6));
    GPIOA->MODER |=  ((2U << 4) | (2U << 6));

    /* AF7 for USART2 */
    GPIOA->AFR[0] &= ~((0xFU << 8) | (0xFU << 12));
    GPIOA->AFR[0] |=  ((7U << 8) | (7U << 12));

    /* Configure USART2 */
    USART2->CR1 = 0;
    USART2->BRR = 139;
    USART2->CR1 |= USART_CR1_TE | USART_CR1_RE;
    USART2->CR1 |= USART_CR1_UE;
}

void USART2_SendChar(char c)
{
    while (!(USART2->SR & USART_SR_TXE))
    {
    }

    USART2->DR = c;
}

void USART2_SendString(char *str)
{
    while (*str)
    {
        USART2_SendChar(*str++);
    }
}
/* System clock: HSI 16 MHz */
void SystemClock_Config(void)
{
    /* Enable HSI */
    RCC->CR |= RCC_CR_HSION;

    while ((RCC->CR & RCC_CR_HSIRDY) == 0)
    {
    }

    /* Disable PLL before changing clock configuration */
    RCC->CR &= ~RCC_CR_PLLON;

    while (RCC->CR & RCC_CR_PLLRDY)
    {
    }

    /* AHB = HCLK, APB1 = HCLK, APB2 = HCLK */
    RCC->CFGR &= ~(RCC_CFGR_HPRE |
                   RCC_CFGR_PPRE1 |
                   RCC_CFGR_PPRE2);

    /* Select HSI as system clock */
    RCC->CFGR &= ~RCC_CFGR_SW;

    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_HSI)
    {
    }

    /* Flash latency = 0 for 16 MHz */
    FLASH->ACR &= ~FLASH_ACR_LATENCY;

    /* Update CMSIS clock variable */
    SystemCoreClockUpdate();
}

/* Default task */
void StartDefaultTask(void *argument)
{
    (void)argument;

    for (;;)
    {
        osDelay(1);
    }
}

/* Sensor task */
void StartSensorTask(void *argument)
{
    (void)argument;

    mpu_status = MPU6050_Init();

    for (;;)
    {
        if (mpu_status == 1)
        {
            read_status = MPU6050_ReadAll(&mpu_data);

            if (read_status == 1)
            {
                queue_status = osMessageQueuePut(
                    SensorQueueHandle,
                    &mpu_data,
                    0,
                    0
                );
            }
        }

        osDelay(10);
    }
}

/* Processing task */
/* Processing task */
void StartProcessingTask(void *argument)
{
    (void)argument;

    static uint8_t filter_initialized = 0;

    for (;;)
    {
        process_status = osMessageQueueGet(
            SensorQueueHandle,
            &processed_data,
            NULL,
            osWaitForever
        );

        if (process_status == osOK)
        {
            /* Convert accelerometer data to g */
            ax_g = processed_data.ax / 16384.0f;
            ay_g = processed_data.ay / 16384.0f;
            az_g = processed_data.az / 16384.0f;

            /* Convert gyroscope data to degree/s */
            gx_dps = processed_data.gx / 131.0f;
            gy_dps = processed_data.gy / 131.0f;
            gz_dps = processed_data.gz / 131.0f;

            /* Calculate angles from accelerometer */
            roll_acc = atan2f(ay_g, az_g)
                     * 180.0f / 3.14159265f;

            pitch_acc = atan2f(
                -ax_g,
                sqrtf(ay_g * ay_g + az_g * az_g)
            ) * 180.0f / 3.14159265f;

            /* Send angle data to UART queue */
            angle_data.roll = roll;
            angle_data.pitch = pitch;

            angle_queue_status = osMessageQueuePut(
                AngleQueueHandle,
                &angle_data,
                0,
                0
            );
            /* Initialize filter */
            if (filter_initialized == 0)
            {
                roll = roll_acc;
                pitch = pitch_acc;
                filter_initialized = 1;
            }
            else
            {
                /* Complementary filter */
                roll = alpha * (roll + gx_dps * dt)
                     + (1.0f - alpha) * roll_acc;

                pitch = alpha * (pitch + gy_dps * dt)
                      + (1.0f - alpha) * pitch_acc;
            }
        }
    }
}

void StartUARTLogTask(void *argument)
{
    (void)argument;

    char buffer[80];

    USART2_Init();

    for (;;)
    {
        uart_queue_status = osMessageQueueGet(
            AngleQueueHandle,
            &uart_data,
            NULL,
            osWaitForever
        );

        if (uart_queue_status == osOK)
        {
            snprintf(
                buffer,
                sizeof(buffer),
                "Roll x100: %ld, Pitch x100: %ld\r\n",
                (long)(uart_data.roll * 100.0f),
                (long)(uart_data.pitch * 100.0f)
            );

            USART2_SendString(buffer);
        }
    }
}

/* Error handler */
void Error_Handler(void)
{
    __disable_irq();

    while (1)
    {
    }
}
