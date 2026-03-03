/**
 * @file    app_lock.h
 * @brief   Cup lid lock/unlock logic
 */

#ifndef __APP_LOCK_H__
#define __APP_LOCK_H__

#include "app_config.h"

void         App_Lock_Init(void);
void         App_Lock_Process(key_event_t key_event);
lock_state_t App_Lock_GetState(void);

#endif /* __APP_LOCK_H__ */
