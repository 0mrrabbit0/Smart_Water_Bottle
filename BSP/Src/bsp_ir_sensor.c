/**
 * @file    bsp_ir_sensor.c
 * @brief   GP2Y0A21YK0F IR distance sensor driver (analog output, ADC)
 *
 * GP2Y0A21YK0F outputs 0.4~3.1V for 80~10cm.
 * Closer object -> higher voltage -> higher ADC raw value.
 * Below ~10cm the output is non-monotonic but still well above
 * the "no object" baseline, so a raw threshold works for lid detection.
 *
 * Uses shared ADC1 handle from bsp.c (BSP_ADC1_ReadChannel).
 */

#include "bsp.h"

void BSP_IR_Init(void)
{
    /* ADC1 initialized in BSP_ADC1_Init() (bsp.c) */
    /* GPIO configured as analog in BSP_GPIO_Init() (bsp_gpio.c) */
}

uint16_t BSP_IR_ReadRaw(void)
{
    uint32_t sum = 0;
    for (uint8_t i = 0; i < 5; i++) {
        sum += BSP_ADC1_ReadChannel(IR_ADC_CHANNEL);
    }
    return (uint16_t)(sum / 5);
}

uint8_t BSP_IR_IsBlocked(void)
{
    return (BSP_IR_ReadRaw() > IR_CLOSE_THRESHOLD) ? 1 : 0;
}
