/**
 * @file    bsp_tds.h
 * @brief   Digital TDS sensor driver (USART3 communication)
 *
 * This driver supports multi-parameter water quality sensor module
 * with UART communication (9600bps). Provides TDS, EC, salinity,
 * specific gravity, temperature, and hardness readings.
 *
 * Protocol (9600bps, 8N1):
 * Query: A0 00 00 00 00 00 A0 (7 bytes)
 * Response: AA [TDS-H TDS-L] [EC-4 EC-3 EC-2 EC-1] [SAL-H SAL-L]
 *           [SG-H SG-L] [TEM-H TEM-L] [HAR-H HAR-L] [CRC8] (16 bytes)
 */

#ifndef __BSP_TDS_H__
#define __BSP_TDS_H__

#include <stdint.h>

/**
 * @brief TDS sensor data packet structure
 */
typedef struct {
    uint16_t tds;          /**< TDS value in ppm */
    uint32_t ec;           /**< Electrical Conductivity in us/cm (microsiemens/cm) */
    uint16_t salinity;     /**< Salinity in 0.01% (e.g., 400 = 4.00%) */
    uint16_t sg;           /**< Specific Gravity in 0.0001 (e.g., 10250 = 1.0250) */
    uint16_t temperature;  /**< Temperature in 0.01°C (e.g., 2550 = 25.50°C) */
    uint16_t hardness;     /**< Hardness in ppm */
    uint8_t  valid;        /**< Data valid flag (1=valid, 0=invalid/timeout) */
} tds_sensor_data_t;

void     BSP_TDS_Init(void);
uint8_t  BSP_TDS_ReadData(tds_sensor_data_t *data);
uint16_t BSP_TDS_ReadValue(void);  /**< Legacy compatibility: returns TDS only */

#endif /* __BSP_TDS_H__ */
