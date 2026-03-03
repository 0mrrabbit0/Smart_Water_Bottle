/**
 * @file    bsp_battery.c
 * @brief   Battery voltage monitoring and capacity calculation driver
 *
 * This driver reads battery voltage via ADC and calculates remaining capacity.
 * Uses voltage divider (1:1, 100k each) to measure single-cell Li-ion battery.
 *
 * Uses shared ADC1 handle from bsp.c (BSP_ADC1_ReadChannel).
 */

#include "bsp.h"

#define VBAT_SAMPLES        10    /* Number of ADC samples to average */
#define VREF_MV             3300  /* ADC reference voltage in mV (3.3V) */
#define ADC_RESOLUTION      4095  /* 12-bit ADC max value */
#define VOLTAGE_DIVIDER     2     /* Voltage divider ratio (R1=R2, so 1:1 means divide by 2) */

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
    /* ADC1 initialized in BSP_ADC1_Init() (bsp.c) */
    /* GPIO configured as analog in BSP_GPIO_Init() (bsp_gpio.c) */
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
