/**
 * @file    bsp_rtc.c
 * @brief   RTC driver with LSE and backup domain for STM32F1
 */

#include "bsp.h"

static RTC_HandleTypeDef s_hrtc;

/* Read RTC 32-bit counter directly (HAL keeps its equivalent helper static) */
static uint32_t BSP_RTC_ReadCounter(void)
{
    uint16_t high1, high2, low;
    do {
        high1 = RTC->CNTH;
        low   = RTC->CNTL;
        high2 = RTC->CNTH;
    } while (high1 != high2);
    return ((uint32_t)high1 << 16) | low;
}

/* Write RTC 32-bit counter directly (HAL keeps its equivalent helper static) */
static void BSP_RTC_WriteCounter(uint32_t value)
{
    while (!(RTC->CRL & RTC_CRL_RTOFF)) {}
    RTC->CRL |= RTC_CRL_CNF;
    RTC->CNTH = (uint16_t)(value >> 16);
    RTC->CNTL = (uint16_t)(value & 0xFFFF);
    RTC->CRL &= ~RTC_CRL_CNF;
    while (!(RTC->CRL & RTC_CRL_RTOFF)) {}
}

void BSP_RTC_Init(void)
{
    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_RCC_BKP_CLK_ENABLE();

    HAL_PWR_EnableBkUpAccess();

    s_hrtc.Instance = RTC;

    /* Skip re-init if the backup register magic shows RTC was set previously */
    if (HAL_RTCEx_BKUPRead(&s_hrtc, RTC_BKP_DR1) == RTC_BKP_MAGIC) {
        HAL_RTC_WaitForSynchro(&s_hrtc);
        return;
    }

    RCC_OscInitTypeDef osc = {0};
    osc.OscillatorType = RCC_OSCILLATORTYPE_LSE;
    osc.LSEState       = RCC_LSE_ON;
    osc.PLL.PLLState   = RCC_PLL_NONE;
    HAL_RCC_OscConfig(&osc);

    RCC_PeriphCLKInitTypeDef clk = {0};
    clk.PeriphClockSelection = RCC_PERIPHCLK_RTC;
    clk.RTCClockSelection    = RCC_RTCCLKSOURCE_LSE;
    HAL_RCCEx_PeriphCLKConfig(&clk);

    __HAL_RCC_RTC_ENABLE();

    s_hrtc.Instance            = RTC;
    s_hrtc.Init.AsynchPrediv   = RTC_AUTO_1_SECOND;
    s_hrtc.Init.OutPut         = RTC_OUTPUTSOURCE_NONE;
    HAL_RTC_Init(&s_hrtc);

    rtc_time_t default_time = {12, 0, 0};
    BSP_RTC_SetTime(&default_time);

    rtc_date_t default_date = {26, 1, 1, 3};   /* 2026-01-01 Wednesday */
    BSP_RTC_SetDate(&default_date);

    HAL_RTCEx_BKUPWrite(&s_hrtc, RTC_BKP_DR1, RTC_BKP_MAGIC);
}

void BSP_RTC_GetTime(rtc_time_t *time)
{
    uint32_t counter = BSP_RTC_ReadCounter();
    uint32_t day_seconds = counter % RTC_SECONDS_PER_DAY;

    time->hours   = (uint8_t)(day_seconds / 3600);
    time->minutes = (uint8_t)((day_seconds % 3600) / 60);
    time->seconds = (uint8_t)(day_seconds % 60);
}

void BSP_RTC_SetTime(const rtc_time_t *time)
{
    uint32_t counter = (uint32_t)time->hours * 3600
                     + (uint32_t)time->minutes * 60
                     + (uint32_t)time->seconds;

    /* Keep the existing day count so only time-of-day is updated */
    uint32_t old_counter = BSP_RTC_ReadCounter();
    uint32_t day_part = old_counter - (old_counter % RTC_SECONDS_PER_DAY);
    counter += day_part;

    BSP_RTC_WriteCounter(counter);
}

void BSP_RTC_GetDate(rtc_date_t *date)
{
    date->year    = (uint8_t)HAL_RTCEx_BKUPRead(&s_hrtc, RTC_BKP_DR2);
    date->month   = (uint8_t)HAL_RTCEx_BKUPRead(&s_hrtc, RTC_BKP_DR3);
    date->day     = (uint8_t)HAL_RTCEx_BKUPRead(&s_hrtc, RTC_BKP_DR4);
    date->weekday = (uint8_t)HAL_RTCEx_BKUPRead(&s_hrtc, RTC_BKP_DR5);
}

void BSP_RTC_SetDate(const rtc_date_t *date)
{
    HAL_RTCEx_BKUPWrite(&s_hrtc, RTC_BKP_DR2, date->year);
    HAL_RTCEx_BKUPWrite(&s_hrtc, RTC_BKP_DR3, date->month);
    HAL_RTCEx_BKUPWrite(&s_hrtc, RTC_BKP_DR4, date->day);
    HAL_RTCEx_BKUPWrite(&s_hrtc, RTC_BKP_DR5, date->weekday);
}
