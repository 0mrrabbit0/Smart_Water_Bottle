/**
 * @file    bsp_water_level.c
 * @brief   M04 Non-contact capacitive water level sensor driver (2-level)
 *
 * Pin configuration (configured in bsp_gpio.c):
 * - PB0: Water Level Low  - Input Pull-Up
 * - PB1: Water Level High - Input Pull-Up
 * (PA1 was reassigned to IR sensor)
 *
 * M04 output behavior:
 * - When water is detected: Output LOW (pulled to GND)
 * - When no water: Output HIGH (pulled up by internal pull-up resistor)
 *
 * Water level detection logic (2 sensors):
 * - Level 0 (Empty):  Neither sensor detects water
 * - Level 1 (Low):    Only low sensor detects water
 * - Level 2 (Full):   Both sensors detect water
 */

#include "bsp.h"

void BSP_WaterLevel_Init(void)
{
    /* GPIO already configured in bsp_gpio.c as Input Pull-Up */
}

/**
 * @brief Read current water level
 * @return Water level: 0=empty, 1=low, 2=full
 */
uint8_t BSP_WaterLevel_Read(void)
{
    uint8_t low  = (HAL_GPIO_ReadPin(WATER_LEVEL_LOW_PORT,  WATER_LEVEL_LOW_PIN)  == GPIO_PIN_RESET) ? 1 : 0;
    uint8_t high = (HAL_GPIO_ReadPin(WATER_LEVEL_HIGH_PORT, WATER_LEVEL_HIGH_PIN) == GPIO_PIN_RESET) ? 1 : 0;

    if (low && high) return 2; /* Full */
    if (low)         return 1; /* Low */
    return 0;                  /* Empty */
}
