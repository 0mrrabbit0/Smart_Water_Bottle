/**
 * @file    app_task.c
 * @brief   FreeRTOS task creation and task function implementations
 */

#include "app_task.h"
#include "app_sensor.h"
#include "app_display.h"
#include "app_lock.h"
#include "app_bluetooth.h"
#include "bsp.h"

/* ---- Global Shared Resources ---- */
sensor_data_t             g_sensor_data;
SemaphoreHandle_t         g_mutex_sensor;
QueueHandle_t             g_queue_key;
volatile system_state_t   g_system_state = SYS_STATE_NORMAL;
volatile lock_state_t     g_lock_state   = LOCK_STATE_UNLOCKED;

/* ---- Task Handles ---- */
static TaskHandle_t s_h_key;
static TaskHandle_t s_h_sensor;
static TaskHandle_t s_h_display;
static TaskHandle_t s_h_lock;
static TaskHandle_t s_h_bluetooth;

void App_Task_Create(void)
{
    g_mutex_sensor = xSemaphoreCreateMutex();
    g_queue_key    = xQueueCreate(KEY_QUEUE_DEPTH, sizeof(key_event_t));

    App_Lock_Init();

    xTaskCreate(Task_Key,       "Key",       TASK_KEY_STACK_SIZE,
                NULL, TASK_KEY_PRIORITY,       &s_h_key);

    xTaskCreate(Task_Sensor,    "Sensor",    TASK_SENSOR_STACK_SIZE,
                NULL, TASK_SENSOR_PRIORITY,    &s_h_sensor);

    xTaskCreate(Task_Display,   "Display",   TASK_DISPLAY_STACK_SIZE,
                NULL, TASK_DISPLAY_PRIORITY,   &s_h_display);

    xTaskCreate(Task_Lock,      "Lock",      TASK_LOCK_STACK_SIZE,
                NULL, TASK_LOCK_PRIORITY,      &s_h_lock);

    xTaskCreate(Task_Bluetooth, "Bluetooth", TASK_BLUETOOTH_STACK_SIZE,
                NULL, TASK_BLUETOOTH_PRIORITY, &s_h_bluetooth);
}

/* ---- Task: Key Scanning (20ms period) ---- */
void Task_Key(void *pvParameters)
{
    (void)pvParameters;
    TickType_t last_wake = xTaskGetTickCount();

    for (;;) {
        key_event_t evt = BSP_Key_Scan();
        if (evt != KEY_EVENT_NONE) {
            xQueueSend(g_queue_key, &evt, 0);
        }
        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(TASK_KEY_PERIOD_MS));
    }
}

/* ---- Task: Sensor Reading (500ms period) ---- */
void Task_Sensor(void *pvParameters)
{
    (void)pvParameters;
    TickType_t last_wake = xTaskGetTickCount();

    for (;;) {
        App_Sensor_Update();
        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(TASK_SENSOR_PERIOD_MS));
    }
}

/* ---- Task: Display Update (200ms period) ---- */
void Task_Display(void *pvParameters)
{
    (void)pvParameters;
    TickType_t last_wake = xTaskGetTickCount();

    for (;;) {
        switch (g_system_state) {
        case SYS_STATE_NORMAL:
            if (g_lock_state == LOCK_STATE_LOCKED) {
                App_Display_Locked();
            } else {
                App_Display_Normal();
            }
            break;
        case SYS_STATE_ALARM:
            App_Display_Alarm();
            break;
        case SYS_STATE_UNLOCK_MODE:
            /* Display is driven by Task_Lock during the unlock sequence */
            break;
        }
        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(TASK_DISPLAY_PERIOD_MS));
    }
}

/* ---- Task: Lock Control (event-driven with timeout) ---- */
void Task_Lock(void *pvParameters)
{
    (void)pvParameters;
    key_event_t evt;

    for (;;) {
        /* Wake on a key event or every 100 ms to poll the IR sensor */
        if (xQueueReceive(g_queue_key, &evt, pdMS_TO_TICKS(100)) == pdTRUE) {
            App_Lock_Process(evt);
        } else {
            App_Lock_Process(KEY_EVENT_NONE);
        }
    }
}

/* ---- Task: Bluetooth Transmission (2000ms period) ---- */
void Task_Bluetooth(void *pvParameters)
{
    (void)pvParameters;
    TickType_t last_wake = xTaskGetTickCount();

    for (;;) {
        App_BT_SendStatus();
        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(TASK_BLUETOOTH_PERIOD_MS));
    }
}
