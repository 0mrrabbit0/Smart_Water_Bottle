/**
 * @file    bsp_water_level.c
 * @brief   M04 non-contact capacitive water level sensor driver (2-level)
 *
 * Pins (initialised in bsp_gpio.c as Input Pull-Up):
 *   PB0: low water mark
 *   PB1: high water mark
 *
 * M04 output is open-drain: LOW = water present, HIGH = no water.
 */

#include "bsp.h"

void BSP_WaterLevel_Init(void)
{
}

uint8_t BSP_WaterLevel_Read(void)
{
    uint8_t low  = (HAL_GPIO_ReadPin(WATER_LEVEL_LOW_PORT,  WATER_LEVEL_LOW_PIN)  == GPIO_PIN_RESET) ? 1 : 0;
    uint8_t high = (HAL_GPIO_ReadPin(WATER_LEVEL_HIGH_PORT, WATER_LEVEL_HIGH_PIN) == GPIO_PIN_RESET) ? 1 : 0;

    if (low && high) return 2;
    if (low)         return 1;
    return 0;
}
