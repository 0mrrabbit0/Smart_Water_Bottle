/**
 * @file    bsp_oled.h
 * @brief   SSD1306 128x64 OLED driver (Software I2C)
 */

#ifndef __BSP_OLED_H__
#define __BSP_OLED_H__

#include <stdint.h>

void BSP_OLED_Init(void);
void BSP_OLED_Clear(void);
void BSP_OLED_Update(void);
void BSP_OLED_Fill(uint8_t data);
void BSP_OLED_DrawPoint(uint8_t x, uint8_t y, uint8_t mode);
void BSP_OLED_ShowChar(uint8_t x, uint8_t y, char ch, uint8_t size);
void BSP_OLED_ShowString(uint8_t x, uint8_t y, const char *str, uint8_t size);
void BSP_OLED_ShowNum(uint8_t x, uint8_t y, uint32_t num, uint8_t len, uint8_t size);

#endif /* __BSP_OLED_H__ */
