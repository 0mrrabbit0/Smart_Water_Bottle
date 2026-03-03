/**
 * @file    app_bluetooth.c
 * @brief   Bluetooth data transmission
 */

#include "app_bluetooth.h"
#include "app_sensor.h"
#include "bsp.h"
#include <stdio.h>

void App_BT_SendStatus(void)
{
    sensor_data_t data = App_Sensor_GetData();
    rtc_time_t time;
    char buf[64];

    BSP_RTC_GetTime(&time);

    int temp_int = (int)data.temperature;
    int temp_dec = (int)((data.temperature - temp_int) * 10);
    if (temp_dec < 0) temp_dec = -temp_dec;

    snprintf(buf, sizeof(buf),
             BT_HEADER "T=%d.%d,TDS=%u,WL=%u,%02u:%02u:%02u\r\n",
             temp_int, temp_dec,
             data.tds_value,
             data.water_level,
             time.hours, time.minutes, time.seconds);

    BSP_BT_SendString(buf);
}
