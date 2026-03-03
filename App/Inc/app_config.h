/**
 * @file    app_config.h
 * @brief   Application configuration, shared data types and constants
 */

#ifndef __APP_CONFIG_H__
#define __APP_CONFIG_H__

#include <stdint.h>

/* ---- Shared Sensor Data ---- */
typedef struct {
    float    temperature;      /* DS18B20 Celsius */
    uint16_t tds_value;        /* TDS ppm */
    uint8_t  water_level;      /* 0=Empty 1=Low 2=Medium 3=Full */
    uint8_t  battery_capacity; /* Battery capacity 0-100% */
} sensor_data_t;

/* ---- System State ---- */
typedef enum {
    SYS_STATE_NORMAL = 0,
    SYS_STATE_UNLOCK_MODE,
    SYS_STATE_ALARM,
} system_state_t;

/* ---- Key Events ---- */
typedef enum {
    KEY_EVENT_NONE = 0,
    KEY_EVENT_KEY1_SHORT,
    KEY_EVENT_KEY1_LONG,
    KEY_EVENT_KEY2_SHORT,
    KEY_EVENT_KEY2_LONG,
} key_event_t;

/* ---- Lock State ---- */
typedef enum {
    LOCK_STATE_UNLOCKED = 0,
    LOCK_STATE_LOCKED,
    LOCK_STATE_ALARM,
} lock_state_t;

/* ---- Task Priorities (higher = more important) ---- */
#define TASK_KEY_PRIORITY           (4)
#define TASK_LOCK_PRIORITY          (3)
#define TASK_SENSOR_PRIORITY        (2)
#define TASK_DISPLAY_PRIORITY       (2)
#define TASK_BLUETOOTH_PRIORITY     (1)

/* ---- Task Stack Sizes (words) ---- */
#define TASK_KEY_STACK_SIZE         (128)
#define TASK_LOCK_STACK_SIZE        (384)
#define TASK_SENSOR_STACK_SIZE      (384)
#define TASK_DISPLAY_STACK_SIZE     (512)
#define TASK_BLUETOOTH_STACK_SIZE   (384)

/* ---- Task Periods (ms) ---- */
#define TASK_KEY_PERIOD_MS          (20)
#define TASK_SENSOR_PERIOD_MS       (500)
#define TASK_DISPLAY_PERIOD_MS      (200)
#define TASK_BLUETOOTH_PERIOD_MS    (2000)

/* ---- Unlock Config ---- */
#define UNLOCK_PASSWORD_LEN         (3)
#define UNLOCK_DIGIT_DISPLAY_MS     (1200)
#define UNLOCK_MAX_FAILURES         (3)

/* ---- Key Queue ---- */
#define KEY_QUEUE_DEPTH             (5)

/* ---- Bluetooth Header ---- */
#define BT_HEADER                   "SWB:"

#endif /* __APP_CONFIG_H__ */
