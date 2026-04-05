/**
 * @file    bsp_gpio.c
 * @brief   GPIO initialization for all peripherals
 */

#include "bsp_gpio.h"

void BSP_GPIO_Init(void)
{
    GPIO_InitTypeDef gpio = {0};

    /* Enable GPIO clocks */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();

    /* --- DS18B20 (PA8) Open-Drain --- */
    gpio.Pin   = DS18B20_PIN;
    gpio.Mode  = GPIO_MODE_OUTPUT_OD;
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(DS18B20_PORT, &gpio);
    DS18B20_SET();

    /* --- Battery Voltage ADC (PA0) Analog --- */
    gpio.Pin  = VBAT_ADC_PIN;
    gpio.Mode = GPIO_MODE_ANALOG;
    gpio.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(VBAT_ADC_PORT, &gpio);

    /* --- IR Distance Sensor GP2Y0A21YK0F (PA1) Analog --- */
    gpio.Pin  = IR_SENSOR_PIN;
    gpio.Mode = GPIO_MODE_ANALOG;
    gpio.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(IR_SENSOR_PORT, &gpio);

    /* --- M04 Non-contact Water Level Sensors (PB0, PB1) Input Pull-Up --- */
    gpio.Mode = GPIO_MODE_INPUT;
    gpio.Pull = GPIO_PULLUP;

    gpio.Pin = WATER_LEVEL_LOW_PIN;
    HAL_GPIO_Init(WATER_LEVEL_LOW_PORT, &gpio);

    gpio.Pin = WATER_LEVEL_HIGH_PIN;
    HAL_GPIO_Init(WATER_LEVEL_HIGH_PORT, &gpio);

    /* --- Keys (PA4, PA5) Input Pull-Up --- */
    gpio.Pin = KEY1_PIN;
    HAL_GPIO_Init(KEY1_PORT, &gpio);

    gpio.Pin = KEY2_PIN;
    HAL_GPIO_Init(KEY2_PORT, &gpio);

    /* --- TP4056 Status (PB10, PB11) Input Pull-Up --- */
    gpio.Pin = TP4056_CHRG_PIN;
    HAL_GPIO_Init(TP4056_CHRG_PORT, &gpio);

    gpio.Pin = TP4056_STDBY_PIN;
    HAL_GPIO_Init(TP4056_STDBY_PORT, &gpio);

    /* --- Buzzer (PB5) Push-Pull Output --- */
    gpio.Pin   = BUZZER_PIN;
    gpio.Mode  = GPIO_MODE_OUTPUT_PP;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(BUZZER_PORT, &gpio);
    HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_RESET);

    /* --- OLED I2C (PB6 SCL, PB7 SDA) Open-Drain with Pull-Up --- */
    gpio.Mode  = GPIO_MODE_OUTPUT_OD;
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    gpio.Pull  = GPIO_PULLUP;

    gpio.Pin = OLED_SCL_PIN;
    HAL_GPIO_Init(OLED_SCL_PORT, &gpio);

    gpio.Pin = OLED_SDA_PIN;
    HAL_GPIO_Init(OLED_SDA_PORT, &gpio);

    OLED_SCL_SET();
    OLED_SDA_SET();

    /* --- LED (PC13) Push-Pull Output --- */
    gpio.Pin   = LED_PIN;
    gpio.Mode  = GPIO_MODE_OUTPUT_PP;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LED_PORT, &gpio);
    HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_SET); /* LED off (active LOW) */
}
