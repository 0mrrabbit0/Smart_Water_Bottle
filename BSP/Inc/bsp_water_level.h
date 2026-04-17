/**
 * @file    bsp_water_level.h
 * @brief   M04 non-contact capacitive water level sensor driver (2-level)
 *
 * M04 sensor:
 * - Open-drain output, active LOW when water is detected.
 * - Two sensors mounted at low and high water marks.
 */

#ifndef __BSP_WATER_LEVEL_H__
#define __BSP_WATER_LEVEL_H__

#include <stdint.h>

void    BSP_WaterLevel_Init(void);
uint8_t BSP_WaterLevel_Read(void);  /**< Returns 0=empty, 1=low, 2=full */

#endif
