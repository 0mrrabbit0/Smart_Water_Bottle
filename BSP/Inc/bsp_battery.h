/**
 * @file    bsp_battery.h
 * @brief   Battery voltage monitoring and capacity calculation driver
 *
 * Hardware:
 * - Battery: Single-cell Li-ion (3.2V ~ 4.35V high voltage type)
 * - Voltage divider: R1=100kΩ, R2=100kΩ (1:1 ratio)
 * - ADC input: PA0 (ADC1_CH0)
 * - Connection: VBAT -> R1 -> VBAT_ADC (PA0) -> R2 -> GND
 *
 * Calculation:
 * - VBAT_ADC = VBAT / 2 (due to 1:1 voltage divider)
 * - VBAT = (ADC_raw / 4095) * 3.3V * 2
 *
 * Battery capacity curve (approximate):
 * - 4.35V = 100%
 * - 4.20V = 90%
 * - 4.00V = 70%
 * - 3.80V = 50%
 * - 3.60V = 30%
 * - 3.40V = 10%
 * - 3.20V = 0%
 */

#ifndef __BSP_BATTERY_H__
#define __BSP_BATTERY_H__

#include <stdint.h>

void     BSP_Battery_Init(void);
uint16_t BSP_Battery_ReadRaw(void);
uint16_t BSP_Battery_GetVoltage(void);  /**< Returns battery voltage in mV */
uint8_t  BSP_Battery_GetCapacity(void); /**< Returns battery capacity 0-100% */

#endif /* __BSP_BATTERY_H__ */
