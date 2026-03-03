/**
 * @file    app_lock.c
 * @brief   Cup lid lock/unlock logic with password sequence
 */

#include "app_lock.h"
#include "app_display.h"
#include "app_task.h"
#include "bsp.h"

/* Default unlock password: press KEY1 when digit 3, 7, 5 appears */
static const uint8_t s_password[UNLOCK_PASSWORD_LEN] = {3, 7, 5};
static uint8_t s_fail_count = 0;

/* Cooldown after unlock: prevent immediate re-lock (in Task_Lock cycles, ~100ms each) */
#define UNLOCK_COOLDOWN_CYCLES  30   /* 30 * 100ms = 3 seconds */
static uint8_t s_unlock_cooldown = 0;

static void run_unlock_sequence(void)
{
    for (uint8_t step = 0; step < UNLOCK_PASSWORD_LEN; step++) {
        uint8_t correct = 0;

        /* Show prompt for this step */
        App_Display_UnlockPrompt(step + 1, UNLOCK_PASSWORD_LEN);
        vTaskDelay(pdMS_TO_TICKS(500));

        /* Display digits 1-9, user presses KEY1 at correct digit */
        for (uint8_t digit = 1; digit <= 9; digit++) {
            App_Display_UnlockDigit(digit);

            TickType_t start = xTaskGetTickCount();
            while ((xTaskGetTickCount() - start) < pdMS_TO_TICKS(UNLOCK_DIGIT_DISPLAY_MS)) {
                key_event_t evt;
                if (xQueueReceive(g_queue_key, &evt, pdMS_TO_TICKS(50)) == pdTRUE) {
                    if (evt == KEY_EVENT_KEY1_SHORT) {
                        if (digit == s_password[step]) {
                            correct = 1;
                            BSP_Buzzer_Beep(50, 0, 1);
                        } else {
                            /* Wrong digit */
                            s_fail_count++;
                            BSP_Buzzer_Beep(200, 100, 3);
                            goto unlock_failed;
                        }
                        break;
                    }
                }
            }
            if (correct) break;
        }

        if (!correct) {
            /* Timeout - no button pressed during 1-9 cycle */
            s_fail_count++;
            BSP_Buzzer_Beep(200, 100, 2);
            goto unlock_failed;
        }
    }

    /* All steps correct - unlock! */
    BSP_Servo_Unlock();
    g_lock_state      = LOCK_STATE_UNLOCKED;
    g_system_state    = SYS_STATE_NORMAL;
    s_fail_count      = 0;
    s_unlock_cooldown = UNLOCK_COOLDOWN_CYCLES;
    BSP_Buzzer_Beep(100, 50, 2);
    return;

unlock_failed:
    if (s_fail_count >= UNLOCK_MAX_FAILURES) {
        g_lock_state   = LOCK_STATE_ALARM;
        g_system_state = SYS_STATE_ALARM;
    } else {
        g_system_state = SYS_STATE_NORMAL;
        /* Remain locked */
    }
}

void App_Lock_Init(void)
{
    g_lock_state   = LOCK_STATE_UNLOCKED;
    g_system_state = SYS_STATE_NORMAL;
    s_fail_count   = 0;
}

void App_Lock_Process(key_event_t key_event)
{
    switch (g_system_state) {
    case SYS_STATE_NORMAL:
        /* Cooldown after unlock: count down before allowing auto-lock */
        if (s_unlock_cooldown > 0) {
            s_unlock_cooldown--;
            break;
        }

        /* Auto-lock: IR detects lid closed + currently unlocked */
        if (BSP_IR_IsBlocked() && g_lock_state == LOCK_STATE_UNLOCKED) {
            BSP_Servo_Lock();
            g_lock_state = LOCK_STATE_LOCKED;
            BSP_Buzzer_Beep(100, 0, 1);
        }

        /* Long press KEY1 to enter unlock mode */
        if (key_event == KEY_EVENT_KEY1_LONG && g_lock_state == LOCK_STATE_LOCKED) {
            g_system_state = SYS_STATE_UNLOCK_MODE;
            run_unlock_sequence();
        }
        break;

    case SYS_STATE_UNLOCK_MODE:
        /* Handled inside run_unlock_sequence */
        break;

    case SYS_STATE_ALARM:
        /* Buzzer alarm - toggle every cycle (~100ms) */
        BSP_Buzzer_Toggle();

        /* Long press KEY2 to reset alarm */
        if (key_event == KEY_EVENT_KEY2_LONG) {
            BSP_Buzzer_Off();
            s_fail_count   = 0;
            g_lock_state   = LOCK_STATE_LOCKED;
            g_system_state = SYS_STATE_NORMAL;
        }
        break;
    }
}

lock_state_t App_Lock_GetState(void)
{
    return g_lock_state;
}
