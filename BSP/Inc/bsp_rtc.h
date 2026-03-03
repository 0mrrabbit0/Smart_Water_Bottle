/**
 * @file    bsp_rtc.h
 * @brief   RTC driver with LSE and backup domain
 */

#ifndef __BSP_RTC_H__
#define __BSP_RTC_H__

#include <stdint.h>

typedef struct {
    uint8_t hours;
    uint8_t minutes;
    uint8_t seconds;
} rtc_time_t;

typedef struct {
    uint8_t year;       /* 0-99 (20xx) */
    uint8_t month;      /* 1-12 */
    uint8_t day;        /* 1-31 */
    uint8_t weekday;    /* 1-7 */
} rtc_date_t;

#define RTC_BKP_MAGIC       0xA5A5
#define RTC_SECONDS_PER_DAY 86400

void BSP_RTC_Init(void);
void BSP_RTC_GetTime(rtc_time_t *time);
void BSP_RTC_SetTime(const rtc_time_t *time);
void BSP_RTC_GetDate(rtc_date_t *date);
void BSP_RTC_SetDate(const rtc_date_t *date);

#endif /* __BSP_RTC_H__ */
