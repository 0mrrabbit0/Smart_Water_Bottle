/**
 * @file    bsp.h
 * @brief   BSP unified header - includes all BSP modules
 */

#ifndef __BSP_H__
#define __BSP_H__

#include "stm32f1xx_hal.h"
#include "bsp_gpio.h"
#include "bsp_ds18b20.h"
#include "bsp_oled.h"
#include "bsp_tds.h"
#include "bsp_water_level.h"
#include "bsp_servo.h"
#include "bsp_buzzer.h"
#include "bsp_ir_sensor.h"
#include "bsp_bluetooth.h"
#include "bsp_key.h"
#include "bsp_rtc.h"
#include "bsp_battery.h"
#include "bsp_debug_uart.h"

void BSP_Init(void);
void BSP_DelayUs(uint32_t us);
void BSP_DelayMs(uint32_t ms);
uint16_t BSP_ADC1_ReadChannel(uint32_t channel);

#endif /* __BSP_H__ */
