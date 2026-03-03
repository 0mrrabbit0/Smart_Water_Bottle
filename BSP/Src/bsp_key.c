/**
 * @file    bsp_key.c
 * @brief   Button driver with debounce and long-press detection
 */

#include "bsp.h"
#include "bsp_key.h"

typedef struct {
    GPIO_TypeDef *port;
    uint16_t      pin;
    uint8_t       pressed;
    uint16_t      count;
    uint8_t       long_triggered;
} key_state_t;

static key_state_t s_key1;
static key_state_t s_key2;

void BSP_Key_Init(void)
{
    /* GPIO already configured in bsp_gpio.c */
    s_key1.port = KEY1_PORT;
    s_key1.pin  = KEY1_PIN;
    s_key1.pressed = 0;
    s_key1.count = 0;
    s_key1.long_triggered = 0;

    s_key2.port = KEY2_PORT;
    s_key2.pin  = KEY2_PIN;
    s_key2.pressed = 0;
    s_key2.count = 0;
    s_key2.long_triggered = 0;
}

static key_event_t scan_single_key(key_state_t *key, key_event_t short_evt, key_event_t long_evt)
{
    key_event_t event = KEY_EVENT_NONE;
    uint8_t cur = (HAL_GPIO_ReadPin(key->port, key->pin) == GPIO_PIN_RESET) ? 1 : 0;

    if (cur) {
        /* Button is pressed */
        key->count++;
        if (key->count >= KEY_LONG_PRESS_THRESHOLD && !key->long_triggered) {
            key->long_triggered = 1;
            event = long_evt;
        }
    } else {
        /* Button released */
        if (key->pressed && key->count >= KEY_DEBOUNCE_THRESHOLD && !key->long_triggered) {
            event = short_evt;
        }
        key->count = 0;
        key->long_triggered = 0;
    }

    key->pressed = cur;
    return event;
}

key_event_t BSP_Key_Scan(void)
{
    key_event_t evt;

    /* KEY1 has higher priority */
    evt = scan_single_key(&s_key1, KEY_EVENT_KEY1_SHORT, KEY_EVENT_KEY1_LONG);
    if (evt != KEY_EVENT_NONE) return evt;

    evt = scan_single_key(&s_key2, KEY_EVENT_KEY2_SHORT, KEY_EVENT_KEY2_LONG);
    return evt;
}
