/**
 * @file    bsp_battery.c
 * @brief   Battery voltage monitor and capacity lookup
 */

#include "bsp.h"

#define VBAT_SAMPLES        10
#define VREF_MV             3300
#define ADC_RESOLUTION      4095
#define VOLTAGE_DIVIDER     2     /* R1=R2 => measured voltage is half of VBAT */

/* Battery voltage-to-capacity lookup table (mV vs %) */
static const uint16_t BATTERY_LUT[][2] = {
    {4350, 100},  /* 4.35V = 100% */
    {4200, 90},   /* 4.20V = 90% */
    {4000, 70},   /* 4.00V = 70% */
    {3800, 50},   /* 3.80V = 50% */
    {3600, 30},   /* 3.60V = 30% */
    {3400, 10},   /* 3.40V = 10% */
    {3200, 0}     /* 3.20V = 0% */
};

#define LUT_SIZE (sizeof(BATTERY_LUT) / sizeof(BATTERY_LUT[0]))

void BSP_Battery_Init(void)
{
}

uint16_t BSP_Battery_ReadRaw(void)
{
    uint32_t sum = 0;

    for (uint8_t i = 0; i < VBAT_SAMPLES; i++) {
        sum += BSP_ADC1_ReadChannel(VBAT_ADC_CHANNEL);
    }

    return (uint16_t)(sum / VBAT_SAMPLES);
}

uint16_t BSP_Battery_GetVoltage(void)
{
    uint16_t adc_raw = BSP_Battery_ReadRaw();

    uint32_t vbat_adc_mv = ((uint32_t)adc_raw * VREF_MV) / ADC_RESOLUTION;
    uint16_t vbat_mv = (uint16_t)(vbat_adc_mv * VOLTAGE_DIVIDER);

    return vbat_mv;
}

uint8_t BSP_Battery_GetCapacity(void)
{
    uint16_t voltage_mv = BSP_Battery_GetVoltage();

    if (voltage_mv >= BATTERY_LUT[0][0]) {
        return 100;
    }
    if (voltage_mv <= BATTERY_LUT[LUT_SIZE - 1][0]) {
        return 0;
    }

    for (uint8_t i = 0; i < LUT_SIZE - 1; i++) {
        uint16_t v_high = BATTERY_LUT[i][0];
        uint16_t v_low  = BATTERY_LUT[i + 1][0];
        uint8_t  p_high = BATTERY_LUT[i][1];
        uint8_t  p_low  = BATTERY_LUT[i + 1][1];

        if (voltage_mv <= v_high && voltage_mv >= v_low) {
            int32_t capacity = p_low + ((int32_t)(voltage_mv - v_low) * (p_high - p_low)) / (v_high - v_low);
            return (uint8_t)capacity;
        }
    }

    return 0;
}
