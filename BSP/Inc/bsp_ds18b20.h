/**
 * @file    bsp_ds18b20.h
 * @brief   DS18B20 temperature sensor driver (OneWire)
 */

#ifndef __BSP_DS18B20_H__
#define __BSP_DS18B20_H__

#include <stdint.h>

int   BSP_DS18B20_Init(void);
float BSP_DS18B20_ReadTemp(void);

#endif /* __BSP_DS18B20_H__ */
