/**
 * @file    bsp_debug_uart.h
 * @brief   Debug UART (USART1) with serial key simulation
 *
 * Send single ASCII characters via serial terminal (115200 8N1) to simulate keys:
 *   '1' = KEY1 short press
 *   '2' = KEY1 long press
 *   '3' = KEY2 short press
 *   '4' = KEY2 long press
 */

#ifndef __BSP_DEBUG_UART_H__
#define __BSP_DEBUG_UART_H__

#include <stdint.h>

void BSP_DebugUart_Init(void);
void BSP_DebugUart_SendString(const char *str);

#endif /* __BSP_DEBUG_UART_H__ */
