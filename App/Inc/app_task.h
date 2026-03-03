/**
 * @file    app_task.h
 * @brief   FreeRTOS task declarations and shared resources
 */

#ifndef __APP_TASK_H__
#define __APP_TASK_H__

#include "app_config.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

/* Shared data */
extern sensor_data_t             g_sensor_data;
extern SemaphoreHandle_t         g_mutex_sensor;
extern QueueHandle_t             g_queue_key;
extern volatile system_state_t   g_system_state;
extern volatile lock_state_t     g_lock_state;

/* Task API */
void App_Task_Create(void);

/* Task functions */
void Task_Key(void *pvParameters);
void Task_Sensor(void *pvParameters);
void Task_Display(void *pvParameters);
void Task_Lock(void *pvParameters);
void Task_Bluetooth(void *pvParameters);

#endif /* __APP_TASK_H__ */
