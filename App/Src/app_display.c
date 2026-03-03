/**
 * @file    app_display.c
 * @brief   OLED display application logic
 */

#include "app_display.h"
#include "app_sensor.h"
#include "app_task.h"
#include "bsp.h"
#include <stdio.h>
#include <string.h>

static const char *s_water_level_str[] = {
    "Empty", "Low", "Medium", "Full"
};

/**
 * @brief Draw battery icon in top-right corner with capacity indicator
 * @param capacity Battery capacity 0-100%
 *
 * Battery icon design (16x10 pixels at position 110,0):
 * ┌──────────┐─
 * │▓▓▓▓      │ │  <- Outer frame (14x8) + Positive terminal (2x4)
 * │▓▓▓▓      │ │     Inner fill based on capacity %
 * └──────────┘─
 */
static void DrawBatteryIcon(uint8_t capacity)
{
    const uint8_t x = 110;  /* Top-right corner */
    const uint8_t y = 0;
    const uint8_t width = 14;
    const uint8_t height = 8;
    const uint8_t terminal_width = 2;
    const uint8_t terminal_height = 4;

    /* Draw outer frame (14x8 rectangle) */
    for (uint8_t i = 0; i < width; i++) {
        BSP_OLED_DrawPoint(x + i, y, 1);              /* Top edge */
        BSP_OLED_DrawPoint(x + i, y + height - 1, 1); /* Bottom edge */
    }
    for (uint8_t i = 0; i < height; i++) {
        BSP_OLED_DrawPoint(x, y + i, 1);              /* Left edge */
        BSP_OLED_DrawPoint(x + width - 1, y + i, 1);  /* Right edge */
    }

    /* Draw positive terminal (2x4 at right side) */
    for (uint8_t i = 0; i < terminal_height; i++) {
        for (uint8_t j = 0; j < terminal_width; j++) {
            BSP_OLED_DrawPoint(x + width + j, y + 2 + i, 1);
        }
    }

    /* Draw inner fill based on capacity (fill from left to right) */
    if (capacity > 100) capacity = 100;
    uint8_t fill_width = ((width - 4) * capacity) / 100;  /* Leave 2-pixel margin on each side */

    for (uint8_t i = 0; i < fill_width; i++) {
        for (uint8_t j = 0; j < height - 4; j++) {  /* Leave 2-pixel margin top/bottom */
            BSP_OLED_DrawPoint(x + 2 + i, y + 2 + j, 1);
        }
    }

    /* Draw capacity percentage number below battery (small font) */
    char buf[5];
    snprintf(buf, sizeof(buf), "%u%%", capacity);
    BSP_OLED_ShowString(x - 2, y + height + 1, buf, 12);
}

void App_Display_Normal(void)
{
    sensor_data_t data = App_Sensor_GetData();
    rtc_time_t time;
    char buf[22];

    BSP_RTC_GetTime(&time);

    /* Clear buffer */
    BSP_OLED_Fill(0x00);

    /* Line 0: Temperature */
    int temp_int = (int)data.temperature;
    int temp_dec = (int)((data.temperature - temp_int) * 10);
    if (temp_dec < 0) temp_dec = -temp_dec;
    snprintf(buf, sizeof(buf), "Temp: %d.%d C", temp_int, temp_dec);
    BSP_OLED_ShowString(0, 0, buf, 12);

    /* Line 1: TDS */
    snprintf(buf, sizeof(buf), "TDS:  %u ppm", data.tds_value);
    BSP_OLED_ShowString(0, 16, buf, 12);

    /* Line 2: Water Level */
    uint8_t lvl = data.water_level;
    if (lvl > 3) lvl = 3;
    snprintf(buf, sizeof(buf), "Water: %s", s_water_level_str[lvl]);
    BSP_OLED_ShowString(0, 32, buf, 12);

    /* Line 3: Time */
    snprintf(buf, sizeof(buf), "%02u:%02u:%02u", time.hours, time.minutes, time.seconds);
    BSP_OLED_ShowString(0, 48, buf, 16);

    /* Draw battery icon in top-right corner */
    DrawBatteryIcon(data.battery_capacity);

    BSP_OLED_Update();
}

void App_Display_UnlockDigit(uint8_t digit)
{
    sensor_data_t data = App_Sensor_GetData();
    char buf[4];

    BSP_OLED_Fill(0x00);

    BSP_OLED_ShowString(16, 0, "UNLOCK MODE", 16);

    /* Large centered digit */
    snprintf(buf, sizeof(buf), "%u", digit);
    BSP_OLED_ShowString(56, 24, buf, 16);

    /* Draw battery icon in top-right corner */
    DrawBatteryIcon(data.battery_capacity);

    BSP_OLED_Update();
}

void App_Display_UnlockPrompt(uint8_t step, uint8_t total)
{
    sensor_data_t data = App_Sensor_GetData();
    char buf[22];

    BSP_OLED_Fill(0x00);
    BSP_OLED_ShowString(16, 0, "UNLOCK MODE", 16);

    snprintf(buf, sizeof(buf), "Step %u/%u", step, total);
    BSP_OLED_ShowString(28, 24, buf, 16);

    BSP_OLED_ShowString(8, 48, "Press at correct #", 12);

    /* Draw battery icon in top-right corner */
    DrawBatteryIcon(data.battery_capacity);

    BSP_OLED_Update();
}

void App_Display_Alarm(void)
{
    sensor_data_t data = App_Sensor_GetData();
    static uint8_t s_toggle = 0;

    BSP_OLED_Fill(0x00);

    if (s_toggle) {
        BSP_OLED_ShowString(16, 8, "!! ALARM !!", 16);
        BSP_OLED_ShowString(4, 32, "LOCKED - 3 FAILS", 12);
        BSP_OLED_ShowString(4, 48, "Hold KEY2 to reset", 12);
    }
    s_toggle = !s_toggle;

    /* Draw battery icon in top-right corner */
    DrawBatteryIcon(data.battery_capacity);

    BSP_OLED_Update();
}

void App_Display_Locked(void)
{
    sensor_data_t data = App_Sensor_GetData();
    rtc_time_t time;
    char buf[22];

    BSP_RTC_GetTime(&time);

    BSP_OLED_Fill(0x00);

    /* Show locked indicator */
    BSP_OLED_ShowString(32, 0, "LOCKED", 16);

    /* Still show basic info */
    int temp_int = (int)data.temperature;
    int temp_dec = (int)((data.temperature - temp_int) * 10);
    if (temp_dec < 0) temp_dec = -temp_dec;
    snprintf(buf, sizeof(buf), "T:%d.%d  TDS:%u", temp_int, temp_dec, data.tds_value);
    BSP_OLED_ShowString(0, 24, buf, 12);

    snprintf(buf, sizeof(buf), "%02u:%02u:%02u", time.hours, time.minutes, time.seconds);
    BSP_OLED_ShowString(28, 48, buf, 16);

    /* Draw battery icon in top-right corner */
    DrawBatteryIcon(data.battery_capacity);

    BSP_OLED_Update();
}
