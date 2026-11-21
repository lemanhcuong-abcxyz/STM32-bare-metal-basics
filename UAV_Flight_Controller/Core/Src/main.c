#include "main.h"
#include "mpu6050.h"
#include "pid.h"
#include "mixer.h"
#include "motor_output.h"

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>

/* ================= HANDLES ================= */

I2C_HandleTypeDef hi2c1;
UART_HandleTypeDef huart2;
TIM_HandleTypeDef htim3;

/* ================= DATA ================= */

MPU6050_Data imu;

PID_Controller roll_pid;
PID_Controller pitch_pid;
PID_Controller yaw_pid;

Mixer_Output mixer;

/* ================= FLIGHT STATE ================= */

typedef enum
{
    FC_DISARMED = 0,
    FC_ARMED
} FC_State;

static FC_State flight_state = FC_DISARMED;

/* ================= PARAMETERS ================= */

#define CONTROL_LOOP_MS       5U
#define DEBUG_INTERVAL_MS     200U

#define MOTOR_MIN_PWM        1000U
#define MOTOR_MAX_PWM        2000U

#define THROTTLE_MIN        0.0f
#define THROTTLE_MAX        1000.0f

#define MAX_ANGLE_DEG       45.0f

/* ================= VARIABLES ================= */

static float throttle_command = 0.0f;

static uint32_t previous_loop_tick = 0U;
static uint32_t previous_debug_tick = 0U;

static uint8_t sensor_ok = 0U;
static uint8_t failsafe_active = 0U;

static char uart_buffer[200];

/* ================= PROTOTYPES ================= */

void SystemClock_Config(void);

static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_TIM3_Init(void);

static void UART_Print(const char *message);
static void UART_Process(void);
static void Debug_Print(void);

static uint8_t Sensor_Check(void);

static void Flight_Disarm(void);
static void Flight_Arm(void);

/* ================= MAIN ================= */

int main(void)
{
    HAL_Init();

    SystemClock_Config();

    MX_GPIO_Init();
    MX_I2C1_Init();
    MX_USART2_UART_Init();
    MX_TIM3_Init();

    /* Initialize motor outputs */

    Motor_Init();
    Motor_Stop();

    UART_Print("\r\n");
    UART_Print("==============================\r\n");
    UART_Print("UAV FLIGHT CONTROLLER\r\n");
    UART_Print("STM32F411RE\r\n");
    UART_Print("CONTROL LOOP: 200 Hz\r\n");
    UART_Print("==============================\r\n");

    /* ================= PID INIT ================= */

    PID_Init(&roll_pid,
            2.0f,
            0.0f,
            0.15f,
            100.0f,
            50.0f);

    PID_Init(&pitch_pid,
            2.0f,
            0.0f,
            0.15f,
            100.0f,
            50.0f);

    PID_Init(&yaw_pid,
            1.0f,
            0.0f,
            0.0f,
            100.0f,
            50.0f);

    roll_pid.setpoint = 0.0f;
    pitch_pid.setpoint = 0.0f;
    yaw_pid.setpoint = 0.0f;

    /* ================= MIXER INIT ================= */

    mixer.throttle = 0.0f;
    mixer.roll = 0.0f;
    mixer.pitch = 0.0f;
    mixer.yaw = 0.0f;

    mixer.motor1 = 0.0f;
    mixer.motor2 = 0.0f;
    mixer.motor3 = 0.0f;
    mixer.motor4 = 0.0f;

    Mixer_Calculate(&mixer);

    /* ================= MPU6050 INIT ================= */

    UART_Print("Initializing MPU6050...\r\n");

    MPU6050_Init(&hi2c1);

    HAL_Delay(100);

    MPU6050_Read(&imu);

    MPU6050_CalculateAngles(&imu, 0.005f);

    UART_Print("MPU initialization sequence complete.\r\n");

    UART_Print("Gyro calibration...\r\n");

    MPU6050_CalibrateGyro();

    UART_Print("Calibration sequence complete.\r\n");

    /* ================= SAFE STATE ================= */

    Flight_Disarm();

    previous_loop_tick = HAL_GetTick();
    previous_debug_tick = HAL_GetTick();

    UART_Print("\r\n");
    UART_Print("COMMANDS:\r\n");
    UART_Print("A = ARM\r\n");
    UART_Print("D = DISARM\r\n");
    UART_Print("Current state: DISARMED\r\n");

    /* ================= CONTROL LOOP ================= */

    while (1)
    {
        uint32_t now = HAL_GetTick();

        /* Non-blocking UART processing */

        UART_Process();

        /* 200 Hz nominal control loop */

        if ((now - previous_loop_tick) < CONTROL_LOOP_MS)
        {
            continue;
        }

        previous_loop_tick = now;

        float dt = CONTROL_LOOP_MS / 1000.0f;

        /* 1. Read IMU */

        MPU6050_Read(&imu);

        /* 2. Calculate attitude */

        MPU6050_CalculateAngles(&imu, dt);

        /* 3. Validate sensor data */

        sensor_ok = Sensor_Check();

        if (!sensor_ok)
        {
            failsafe_active = 1U;
            Flight_Disarm();
        }

        /* 4. Failsafe */

        if (failsafe_active != 0U)
        {
            Flight_Disarm();
            Motor_Stop();

            Debug_Print();

            continue;
        }

        /* 5. DISARMED */

        if (flight_state == FC_DISARMED)
        {
            Motor_Stop();

            roll_pid.setpoint = 0.0f;
            pitch_pid.setpoint = 0.0f;
            yaw_pid.setpoint = 0.0f;

            PID_Reset(&roll_pid);
            PID_Reset(&pitch_pid);
            PID_Reset(&yaw_pid);

            Debug_Print();

            continue;
        }

        /* 6. ARMED */

        if (flight_state == FC_ARMED)
        {
            /*
             * Receiver not installed.
             * Throttle remains zero.
             */

            throttle_command = 0.0f;

            /* Roll PID */

            float roll_output =
                PID_Update(&roll_pid,
                          imu.roll,
                          dt);

            /* Pitch PID */

            float pitch_output =
                PID_Update(&pitch_pid,
                          imu.pitch,
                          dt);

            /* Yaw rate PID */

            float yaw_output =
                PID_Update(&yaw_pid,
                          imu.gyro_yaw,
                          dt);

            /* 7. Mixer */

            mixer.throttle = throttle_command;
            mixer.roll = roll_output;
            mixer.pitch = pitch_output;
            mixer.yaw = yaw_output;

            Mixer_Calculate(&mixer);

            /* 8. Motor outputs */

            if (throttle_command <= THROTTLE_MIN)
            {
                Motor_Stop();
            }
            else
            {
                Motor_Set_From_Mixer(
                    mixer.motor1,
                    mixer.motor2,
                    mixer.motor3,
                    mixer.motor4);
            }
        }

        /* 9. Debug */

        Debug_Print();
    }
}

/* ================= SENSOR CHECK ================= */

static uint8_t Sensor_Check(void)
{
    if (!isfinite(imu.roll) ||
        !isfinite(imu.pitch) ||
        !isfinite(imu.gyro_roll) ||
        !isfinite(imu.gyro_pitch) ||
        !isfinite(imu.gyro_yaw))
    {
        return 0U;
    }

    if (fabsf(imu.roll) > MAX_ANGLE_DEG)
    {
        return 0U;
    }

    if (fabsf(imu.pitch) > MAX_ANGLE_DEG)
    {
        return 0U;
    }

    if (fabsf(imu.gyro_roll) > 2000.0f ||
        fabsf(imu.gyro_pitch) > 2000.0f ||
        fabsf(imu.gyro_yaw) > 2000.0f)
    {
        return 0U;
    }

    return 1U;
}

/* ================= ARM ================= */

static void Flight_Arm(void)
{
    if (sensor_ok == 0U)
    {
        UART_Print("ARM REJECTED: SENSOR INVALID\r\n");
        return;
    }

    if (fabsf(imu.roll) > 10.0f ||
        fabsf(imu.pitch) > 10.0f)
    {
        UART_Print("ARM REJECTED: ANGLE TOO HIGH\r\n");
        return;
    }

    if (failsafe_active != 0U)
    {
        UART_Print("ARM REJECTED: FAILSAFE ACTIVE\r\n");
        return;
    }

    flight_state = FC_ARMED;

    PID_Reset(&roll_pid);
    PID_Reset(&pitch_pid);
    PID_Reset(&yaw_pid);

    Motor_Stop();

    UART_Print("ARM STATE ENABLED\r\n");
    UART_Print("THROTTLE IS ZERO\r\n");
}

/* ================= DISARM ================= */

static void Flight_Disarm(void)
{
    flight_state = FC_DISARMED;

    throttle_command = 0.0f;

    PID_Reset(&roll_pid);
    PID_Reset(&pitch_pid);
    PID_Reset(&yaw_pid);

    Motor_Stop();
}

/* ================= UART ================= */

static void UART_Print(const char *message)
{
    HAL_UART_Transmit(&huart2,
                     (uint8_t *)message,
                     strlen(message),
                     100U);
}

static void UART_Process(void)
{
    uint8_t command;

    if (HAL_UART_Receive(&huart2,
                         &command,
                         1U,
                         0U) == HAL_OK)
    {
        switch (command)
        {
            case 'A':
            case 'a':

                Flight_Arm();

                break;

            case 'D':
            case 'd':

                Flight_Disarm();

                failsafe_active = 0U;

                UART_Print("DISARMED\r\n");

                break;

            default:

                break;
        }
    }
}

/* ================= DEBUG ================= */

static void Debug_Print(void)
{
    uint32_t now = HAL_GetTick();

    if ((now - previous_debug_tick) < DEBUG_INTERVAL_MS)
    {
        return;
    }

    previous_debug_tick = now;

    int roll10 = (int)(imu.roll * 10.0f);
    int pitch10 = (int)(imu.pitch * 10.0f);

    int gyroR10 = (int)(imu.gyro_roll * 10.0f);
    int gyroP10 = (int)(imu.gyro_pitch * 10.0f);
    int gyroY10 = (int)(imu.gyro_yaw * 10.0f);

    int m1 = (int)mixer.motor1;
    int m2 = (int)mixer.motor2;
    int m3 = (int)mixer.motor3;
    int m4 = (int)mixer.motor4;

    snprintf(uart_buffer,
             sizeof(uart_buffer),

             "STATE:%s "
             "R:%d.%d "
             "P:%d.%d "
             "GR:%d.%d "
             "GP:%d.%d "
             "GY:%d.%d "
             "M:%d,%d,%d,%d "
             "FS:%d\r\n",

             (flight_state == FC_ARMED)
             ? "ARMED"
             : "DISARMED",

             roll10 / 10,
             abs(roll10 % 10),

             pitch10 / 10,
             abs(pitch10 % 10),

             gyroR10 / 10,
             abs(gyroR10 % 10),

             gyroP10 / 10,
             abs(gyroP10 % 10),

             gyroY10 / 10,
             abs(gyroY10 % 10),

             m1, m2, m3, m4,

             failsafe_active);

    UART_Print(uart_buffer);
}

/* ================= CLOCK ================= */

void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    RCC_OscInitStruct.OscillatorType =
        RCC_OSCILLATORTYPE_HSI;

    RCC_OscInitStruct.HSIState =
        RCC_HSI_ON;

    RCC_OscInitStruct.HSICalibrationValue =
        RCC_HSICALIBRATION_DEFAULT;

    RCC_OscInitStruct.PLL.PLLState =
        RCC_PLL_NONE;

    HAL_RCC_OscConfig(&RCC_OscInitStruct);

    RCC_ClkInitStruct.ClockType =
        RCC_CLOCKTYPE_HCLK |
        RCC_CLOCKTYPE_SYSCLK |
        RCC_CLOCKTYPE_PCLK1 |
        RCC_CLOCKTYPE_PCLK2;

    RCC_ClkInitStruct.SYSCLKSource =
        RCC_SYSCLKSOURCE_HSI;

    RCC_ClkInitStruct.AHBCLKDivider =
        RCC_SYSCLK_DIV1;

    RCC_ClkInitStruct.APB1CLKDivider =
        RCC_HCLK_DIV1;

    RCC_ClkInitStruct.APB2CLKDivider =
        RCC_HCLK_DIV1;

    HAL_RCC_ClockConfig(&RCC_ClkInitStruct,
                        FLASH_LATENCY_0);
}

/* ================= I2C1 ================= */

static void MX_I2C1_Init(void)
{
    hi2c1.Instance = I2C1;

    hi2c1.Init.ClockSpeed = 100000;
    hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;

    hi2c1.Init.OwnAddress1 = 0;

    hi2c1.Init.AddressingMode =
        I2C_ADDRESSINGMODE_7BIT;

    hi2c1.Init.DualAddressMode =
        I2C_DUALADDRESS_DISABLE;

    hi2c1.Init.OwnAddress2 = 0;

    hi2c1.Init.GeneralCallMode =
        I2C_GENERALCALL_DISABLE;

    hi2c1.Init.NoStretchMode =
        I2C_NOSTRETCH_DISABLE;

    HAL_I2C_Init(&hi2c1);
}

/* ================= USART2 ================= */

static void MX_USART2_UART_Init(void)
{
    huart2.Instance = USART2;

    huart2.Init.BaudRate = 115200;

    huart2.Init.WordLength =
        UART_WORDLENGTH_8B;

    huart2.Init.StopBits =
        UART_STOPBITS_1;

    huart2.Init.Parity =
        UART_PARITY_NONE;

    huart2.Init.Mode =
        UART_MODE_TX_RX;

    huart2.Init.HwFlowCtl =
        UART_HWCONTROL_NONE;

    huart2.Init.OverSampling =
        UART_OVERSAMPLING_16;

    HAL_UART_Init(&huart2);
}

/* ================= TIM3 PWM ================= */

static void MX_TIM3_Init(void)
{
    TIM_OC_InitTypeDef sConfigOC = {0};

    /*
     * Explicitly enable TIM3 clock.
     * MSP callbacks remain in stm32f4xx_hal_msp.c.
     */

    __HAL_RCC_TIM3_CLK_ENABLE();

    htim3.Instance = TIM3;

    /*
     * HSI = 16 MHz
     * PSC = 15 -> 1 MHz timer counter
     * ARR = 19999 -> 20 ms period
     * PWM frequency = 50 Hz
     */

    htim3.Init.Prescaler = 15;

    htim3.Init.CounterMode =
        TIM_COUNTERMODE_UP;

    htim3.Init.Period = 19999;

    htim3.Init.ClockDivision =
        TIM_CLOCKDIVISION_DIV1;

    htim3.Init.AutoReloadPreload =
        TIM_AUTORELOAD_PRELOAD_DISABLE;

    HAL_TIM_PWM_Init(&htim3);

    sConfigOC.OCMode =
        TIM_OCMODE_PWM1;

    sConfigOC.Pulse = 1000;

    sConfigOC.OCPolarity =
        TIM_OCPOLARITY_HIGH;

    sConfigOC.OCFastMode =
        TIM_OCFAST_DISABLE;

    HAL_TIM_PWM_ConfigChannel(
        &htim3,
        &sConfigOC,
        TIM_CHANNEL_1);

    HAL_TIM_PWM_ConfigChannel(
        &htim3,
        &sConfigOC,
        TIM_CHANNEL_2);

    HAL_TIM_PWM_ConfigChannel(
        &htim3,
        &sConfigOC,
        TIM_CHANNEL_3);

    HAL_TIM_PWM_ConfigChannel(
        &htim3,
        &sConfigOC,
        TIM_CHANNEL_4);
}

/* ================= GPIO ================= */

static void MX_GPIO_Init(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* TIM3 CH1 - PA6 */

    GPIO_InitStruct.Pin = GPIO_PIN_6;

    GPIO_InitStruct.Mode =
        GPIO_MODE_AF_PP;

    GPIO_InitStruct.Pull =
        GPIO_NOPULL;

    GPIO_InitStruct.Speed =
        GPIO_SPEED_FREQ_LOW;

    GPIO_InitStruct.Alternate =
        GPIO_AF2_TIM3;

    HAL_GPIO_Init(GPIOA,
                  &GPIO_InitStruct);

    /* TIM3 CH2 - PA7 */

    GPIO_InitStruct.Pin = GPIO_PIN_7;

    HAL_GPIO_Init(GPIOA,
                  &GPIO_InitStruct);

    /* TIM3 CH3 - PB0 */

    GPIO_InitStruct.Pin = GPIO_PIN_0;

    HAL_GPIO_Init(GPIOB,
                  &GPIO_InitStruct);

    /* TIM3 CH4 - PB1 */

    GPIO_InitStruct.Pin = GPIO_PIN_1;

    HAL_GPIO_Init(GPIOB,
                  &GPIO_InitStruct);
}

/*
 * HAL_I2C_MspInit()
 * HAL_UART_MspInit()
 * HAL_TIM_PWM_MspInit()
 *
 * These functions are intentionally NOT defined here.
 *
 * They are handled by:
 * Core/Src/stm32f4xx_hal_msp.c
 */

/* ================= ERROR ================= */

void Error_Handler(void)
{
    Motor_Stop();

    __disable_irq();

    while (1)
    {
        Motor_Stop();
    }
}
