#include "motor_output.h"

extern TIM_HandleTypeDef htim3;

static Motor_Output motor;

static uint16_t Limit_PWM(uint16_t pwm)
{
    if (pwm < 1000U) return 1000U;
    if (pwm > 2000U) return 2000U;

    return pwm;
}

static float Limit_Mixer(float value)
{
    if (value < 0.0f) return 0.0f;
    if (value > 1000.0f) return 1000.0f;

    return value;
}

void Motor_Init(void)
{
    motor.motor1 = 1000U;
    motor.motor2 = 1000U;
    motor.motor3 = 1000U;
    motor.motor4 = 1000U;

    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_4);

    Motor_Stop();
}

void Motor_Set(uint16_t m1,
               uint16_t m2,
               uint16_t m3,
               uint16_t m4)
{
    motor.motor1 = Limit_PWM(m1);
    motor.motor2 = Limit_PWM(m2);
    motor.motor3 = Limit_PWM(m3);
    motor.motor4 = Limit_PWM(m4);

    __HAL_TIM_SET_COMPARE(&htim3,
                         TIM_CHANNEL_1,
                         motor.motor1);

    __HAL_TIM_SET_COMPARE(&htim3,
                         TIM_CHANNEL_2,
                         motor.motor2);

    __HAL_TIM_SET_COMPARE(&htim3,
                         TIM_CHANNEL_3,
                         motor.motor3);

    __HAL_TIM_SET_COMPARE(&htim3,
                         TIM_CHANNEL_4,
                         motor.motor4);
}

void Motor_Set_From_Mixer(float m1,
                          float m2,
                          float m3,
                          float m4)
{
    m1 = Limit_Mixer(m1);
    m2 = Limit_Mixer(m2);
    m3 = Limit_Mixer(m3);
    m4 = Limit_Mixer(m4);

    Motor_Set((uint16_t)(1000.0f + m1),
             (uint16_t)(1000.0f + m2),
             (uint16_t)(1000.0f + m3),
             (uint16_t)(1000.0f + m4));
}

void Motor_Stop(void)
{
    Motor_Set(1000U,
             1000U,
             1000U,
             1000U);
}
