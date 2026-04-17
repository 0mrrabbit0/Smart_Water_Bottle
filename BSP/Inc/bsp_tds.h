/**
 * @file    bsp_tds.h
 * @brief   Multi-parameter digital water quality sensor (USART3, 9600 8N1)
 *
 * Reports TDS, EC, salinity, specific gravity, temperature and hardness.
 *
 * Query  (7 bytes):  A0 00 00 00 00 00 A0
 * Reply (16 bytes):  AA [TDS-H TDS-L] [EC-4 EC-3 EC-2 EC-1] [SAL-H SAL-L]
 *                       [SG-H SG-L] [TEM-H TEM-L] [HAR-H HAR-L] [CRC8]
 */

#ifndef __BSP_TDS_H__
#define __BSP_TDS_H__

#include <stdint.h>

typedef struct {
    uint16_t tds;          /**< TDS in ppm */
    uint32_t ec;           /**< Electrical conductivity in us/cm */
    uint16_t salinity;     /**< Salinity in 0.01% (400 = 4.00%) */
    uint16_t sg;           /**< Specific gravity in 0.0001 (10250 = 1.0250) */
    uint16_t temperature;  /**< Temperature in 0.01°C (2550 = 25.50°C) */
    uint16_t hardness;     /**< Hardness in ppm */
    uint8_t  valid;        /**< 1 = data valid, 0 = read failed */
} tds_sensor_data_t;

void     BSP_TDS_Init(void);
uint8_t  BSP_TDS_ReadData(tds_sensor_data_t *data);
uint16_t BSP_TDS_ReadValue(void);  /**< Returns TDS in ppm only */

#endif
