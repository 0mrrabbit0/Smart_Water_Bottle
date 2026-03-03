/**
 * @file    app_sensor.h
 * @brief   Sensor data acquisition interface
 */

#ifndef __APP_SENSOR_H__
#define __APP_SENSOR_H__

#include "app_config.h"

void          App_Sensor_Update(void);
sensor_data_t App_Sensor_GetData(void);

#endif /* __APP_SENSOR_H__ */
