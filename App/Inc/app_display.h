/**
 * @file    app_display.h
 * @brief   OLED display application interface
 */

#ifndef __APP_DISPLAY_H__
#define __APP_DISPLAY_H__

#include <stdint.h>

void App_Display_Normal(void);
void App_Display_UnlockDigit(uint8_t digit);
void App_Display_UnlockPrompt(uint8_t step, uint8_t total);
void App_Display_Alarm(void);
void App_Display_Locked(void);

#endif /* __APP_DISPLAY_H__ */
