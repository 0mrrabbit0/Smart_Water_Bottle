/**
 * @file    bsp_buzzer.c
 * @brief   Buzzer driver
 */

#include "bsp.h"

void BSP_Buzzer_Init(void)
{
    /* GPIO already configured in bsp_gpio.c */
    BSP_Buzzer_Off();
}

void BSP_Buzzer_On(void)
{
    HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_SET);
}

void BSP_Buzzer_Off(void)
{
    HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_RESET);
}

void BSP_Buzzer_Toggle(void)
{
    HAL_GPIO_TogglePin(BUZZER_PORT, BUZZER_PIN);
}

void BSP_Buzzer_Beep(uint16_t on_ms, uint16_t off_ms, uint8_t count)
{
    for (uint8_t i = 0; i < count; i++) {
        BSP_Buzzer_On();
        HAL_Delay(on_ms);
        BSP_Buzzer_Off();
        if (off_ms > 0 && i < count - 1) {
            HAL_Delay(off_ms);
        }
    }
}
