/**
 * @file    app_sensor.c
 * @brief   Sensor data acquisition with mutex protection
 */

#include "app_sensor.h"
#include "app_task.h"
#include "bsp.h"

void App_Sensor_Update(void)
{
    sensor_data_t data;

    data.temperature      = BSP_DS18B20_ReadTemp();
    data.tds_value        = BSP_TDS_ReadValue();
    data.water_level      = BSP_WaterLevel_Read();
    data.battery_capacity = BSP_Battery_GetCapacity();

    if (xSemaphoreTake(g_mutex_sensor, pdMS_TO_TICKS(50)) == pdTRUE) {
        g_sensor_data = data;
        xSemaphoreGive(g_mutex_sensor);
    }
}

sensor_data_t App_Sensor_GetData(void)
{
    sensor_data_t data = {0};

    if (xSemaphoreTake(g_mutex_sensor, pdMS_TO_TICKS(50)) == pdTRUE) {
        data = g_sensor_data;
        xSemaphoreGive(g_mutex_sensor);
    }

    return data;
}
