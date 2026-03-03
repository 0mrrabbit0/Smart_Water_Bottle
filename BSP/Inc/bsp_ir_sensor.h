/**
 * @file    bsp_ir_sensor.h
 * @brief   GP2Y0A21YK0F IR distance sensor driver (analog output, ADC)
 */

#ifndef __BSP_IR_SENSOR_H__
#define __BSP_IR_SENSOR_H__

#include <stdint.h>

void     BSP_IR_Init(void);
uint16_t BSP_IR_ReadRaw(void);
uint8_t  BSP_IR_IsBlocked(void);

#endif /* __BSP_IR_SENSOR_H__ */
