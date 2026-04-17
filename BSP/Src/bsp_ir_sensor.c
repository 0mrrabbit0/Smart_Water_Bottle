/**
 * @file    bsp_ir_sensor.c
 * @brief   GP2Y0A21YK0F IR distance sensor (analog, PA1 / ADC1_CH1)
 *
 * Output is 0.4~3.1V over 80~10cm; closer object yields higher ADC reading.
 * Lid-closed detection uses a raw ADC threshold (IR_CLOSE_THRESHOLD).
 */

#include "bsp.h"

void BSP_IR_Init(void)
{
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
