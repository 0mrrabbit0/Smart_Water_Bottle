/**
 * @file    bsp_water_level.c
 * @brief   M04 Non-contact capacitive water level sensor driver
 *
 * Pin configuration (configured in bsp_gpio.c):
 * - PA1: Water Level 1 (lowest sensor) - Input Pull-Up
 * - PB0: Water Level 2 (middle sensor) - Input Pull-Up
 * - PB1: Water Level 3 (highest sensor) - Input Pull-Up
 *
 * M04 output behavior:
 * - When water is detected: Output LOW (pulled to GND)
 * - When no water: Output HIGH (pulled up by internal pull-up resistor)
 *
 * Water level detection logic:
 * - Level 0 (Empty):  None of the sensors detect water
 * - Level 1 (Low):    Only sensor 1 detects water
 * - Level 2 (Medium): Sensors 1 and 2 detect water
 * - Level 3 (Full):   All three sensors detect water
 */

#include "bsp.h"

void BSP_WaterLevel_Init(void)
{
    /* GPIO already configured in bsp_gpio.c as Input Pull-Up */
}

/**
 * @brief Read current water level
 * @return Water level: 0=empty, 1=low, 2=medium, 3=full
 */
uint8_t BSP_WaterLevel_Read(void)
{
    /* M04 outputs LOW when water detected, HIGH when no water */
    uint8_t s1 = (HAL_GPIO_ReadPin(WATER_LEVEL_1_PORT, WATER_LEVEL_1_PIN) == GPIO_PIN_RESET) ? 1 : 0;
    uint8_t s2 = (HAL_GPIO_ReadPin(WATER_LEVEL_2_PORT, WATER_LEVEL_2_PIN) == GPIO_PIN_RESET) ? 1 : 0;
    uint8_t s3 = (HAL_GPIO_ReadPin(WATER_LEVEL_3_PORT, WATER_LEVEL_3_PIN) == GPIO_PIN_RESET) ? 1 : 0;

    /* Water rises from bottom to top: s1 triggers first, then s2, then s3 */
    if (s1 && s2 && s3) return 3; /* Full: all sensors submerged */
    if (s1 && s2)       return 2; /* Medium: sensors 1 and 2 submerged */
    if (s1)             return 1; /* Low: only sensor 1 submerged */
    return 0;                     /* Empty: no sensors detect water */
}
