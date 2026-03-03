/**
 * @file    bsp_servo.c
 * @brief   SG90 servo motor driver (TIM3 PWM)
 */

#include "bsp.h"

static TIM_HandleTypeDef s_htim3;

void BSP_Servo_Init(void)
{
    GPIO_InitTypeDef gpio = {0};
    TIM_OC_InitTypeDef oc = {0};

    /* Enable clocks */
    __HAL_RCC_TIM3_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    /* PA6 as AF push-pull for TIM3_CH1 */
    gpio.Pin   = SERVO_PIN;
    gpio.Mode  = GPIO_MODE_AF_PP;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(SERVO_PORT, &gpio);

    /* TIM3: 72MHz / 72 = 1MHz, period = 20000 -> 50Hz */
    s_htim3.Instance               = SERVO_TIM;
    s_htim3.Init.Prescaler         = SERVO_TIM_PSC;
    s_htim3.Init.CounterMode       = TIM_COUNTERMODE_UP;
    s_htim3.Init.Period            = SERVO_TIM_ARR;
    s_htim3.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
    s_htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
    HAL_TIM_PWM_Init(&s_htim3);

    /* PWM Mode 1, initial pulse 1500us (90 degrees / center) */
    oc.OCMode     = TIM_OCMODE_PWM1;
    oc.Pulse      = 1500;
    oc.OCPolarity = TIM_OCPOLARITY_HIGH;
    oc.OCFastMode = TIM_OCFAST_DISABLE;
    HAL_TIM_PWM_ConfigChannel(&s_htim3, &oc, SERVO_CHANNEL);

    HAL_TIM_PWM_Start(&s_htim3, SERVO_CHANNEL);
}

void BSP_Servo_SetAngle(uint8_t angle)
{
    if (angle > 180) angle = 180;

    /* 0deg=500us, 180deg=2500us, 1MHz tick */
    uint16_t pulse = 500 + (uint32_t)angle * 2000 / 180;
    __HAL_TIM_SET_COMPARE(&s_htim3, SERVO_CHANNEL, pulse);
}

void BSP_Servo_Lock(void)
{
    BSP_Servo_SetAngle(0);
}

void BSP_Servo_Unlock(void)
{
    BSP_Servo_SetAngle(90);
}
