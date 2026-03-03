/**
 * @file    bsp_water_level.h
 * @brief   M04 Non-contact capacitive water level sensor driver
 *
 * M04 features:
 * - 3-level capacitive sensor (no contact with liquid required)
 * - Open-drain output (requires pull-up resistor)
 * - LOW output when water detected, HIGH when no water
 * - Typical response time: <500ms
 */

#ifndef __BSP_WATER_LEVEL_H__
#define __BSP_WATER_LEVEL_H__

#include <stdint.h>

void    BSP_WaterLevel_Init(void);
uint8_t BSP_WaterLevel_Read(void);  /**< Returns 0=empty, 1=low, 2=medium, 3=full */

#endif /* __BSP_WATER_LEVEL_H__ */
