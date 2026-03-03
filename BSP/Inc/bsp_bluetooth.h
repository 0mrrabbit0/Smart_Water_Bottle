/**
 * @file    bsp_bluetooth.h
 * @brief   HC-05 Bluetooth module driver (USART2)
 */

#ifndef __BSP_BLUETOOTH_H__
#define __BSP_BLUETOOTH_H__

#include <stdint.h>

void BSP_BT_Init(void);
void BSP_BT_SendByte(uint8_t byte);
void BSP_BT_SendString(const char *str);
void BSP_BT_SendData(const uint8_t *data, uint16_t len);

#endif /* __BSP_BLUETOOTH_H__ */
