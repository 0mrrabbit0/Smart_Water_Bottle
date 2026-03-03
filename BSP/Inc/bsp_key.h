/**
 * @file    bsp_key.h
 * @brief   Button/key driver with debounce and long-press detection
 */

#ifndef __BSP_KEY_H__
#define __BSP_KEY_H__

#include "app_config.h"

#define KEY_DEBOUNCE_THRESHOLD      (2)     /* 2 scans = 40ms */
#define KEY_LONG_PRESS_THRESHOLD    (100)   /* 100 scans = 2000ms */

void        BSP_Key_Init(void);
key_event_t BSP_Key_Scan(void);

#endif /* __BSP_KEY_H__ */
