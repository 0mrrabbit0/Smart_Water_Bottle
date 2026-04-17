/**
 * @file    bsp_battery.h
 * @brief   Single-cell Li-ion battery voltage monitor (PA0 / ADC1_CH0)
 *
 * Voltage divider R1=R2=100k (1:1), so VBAT = ADC_raw * 3.3V * 2 / 4095.
 * Capacity is mapped from voltage via lookup table in bsp_battery.c.
 */

#ifndef __BSP_BATTERY_H__
#define __BSP_BATTERY_H__

#include <stdint.h>

void     BSP_Battery_Init(void);
uint16_t BSP_Battery_ReadRaw(void);
uint16_t BSP_Battery_GetVoltage(void);  /**< mV */
uint8_t  BSP_Battery_GetCapacity(void); /**< 0-100 */

#endif
