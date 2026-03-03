/**
 * @file    bsp_buzzer.h
 * @brief   Buzzer driver
 */

#ifndef __BSP_BUZZER_H__
#define __BSP_BUZZER_H__

#include <stdint.h>

void BSP_Buzzer_Init(void);
void BSP_Buzzer_On(void);
void BSP_Buzzer_Off(void);
void BSP_Buzzer_Toggle(void);
void BSP_Buzzer_Beep(uint16_t on_ms, uint16_t off_ms, uint8_t count);

#endif /* __BSP_BUZZER_H__ */
