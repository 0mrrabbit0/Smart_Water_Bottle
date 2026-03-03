/**
 * @file    bsp.c
 * @brief   BSP unified initialization and utility functions
 */

#include "bsp.h"

extern void BSP_GPIO_Init(void);

/* ---- Shared ADC1 Handle ---- */
static ADC_HandleTypeDef s_hadc1;

static void BSP_DWT_Init(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

static void BSP_ADC1_Init(void)
{
    __HAL_RCC_ADC1_CLK_ENABLE();

    s_hadc1.Instance                   = ADC1;
    s_hadc1.Init.DataAlign             = ADC_DATAALIGN_RIGHT;
    s_hadc1.Init.ScanConvMode          = ADC_SCAN_DISABLE;
    s_hadc1.Init.ContinuousConvMode    = DISABLE;
    s_hadc1.Init.NbrOfConversion       = 1;
    s_hadc1.Init.DiscontinuousConvMode = DISABLE;
    s_hadc1.Init.ExternalTrigConv      = ADC_SOFTWARE_START;

    HAL_ADC_Init(&s_hadc1);
    HAL_ADCEx_Calibration_Start(&s_hadc1);
}

uint16_t BSP_ADC1_ReadChannel(uint32_t channel)
{
    ADC_ChannelConfTypeDef cfg = {0};

    cfg.Channel      = channel;
    cfg.Rank         = ADC_REGULAR_RANK_1;
    cfg.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;

    HAL_ADC_ConfigChannel(&s_hadc1, &cfg);
    HAL_ADC_Start(&s_hadc1);

    if (HAL_ADC_PollForConversion(&s_hadc1, 100) == HAL_OK) {
        return (uint16_t)HAL_ADC_GetValue(&s_hadc1);
    }

    return 0;
}

void BSP_DelayUs(uint32_t us)
{
    uint32_t start = DWT->CYCCNT;
    uint32_t ticks = us * (SystemCoreClock / 1000000);
    while ((DWT->CYCCNT - start) < ticks);
}

void BSP_DelayMs(uint32_t ms)
{
    HAL_Delay(ms);
}

void BSP_Init(void)
{
    BSP_DWT_Init();
    BSP_GPIO_Init();
    BSP_ADC1_Init();
    BSP_OLED_Init();
    BSP_DS18B20_Init();
    BSP_TDS_Init();
    BSP_WaterLevel_Init();
    BSP_Servo_Init();
    BSP_Buzzer_Init();
    BSP_IR_Init();
    BSP_Battery_Init();
    BSP_Key_Init();
    BSP_BT_Init();
    BSP_RTC_Init();
    BSP_DebugUart_Init();
}
