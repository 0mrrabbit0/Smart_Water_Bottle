/**
 * @file    bsp_gpio.h
 * @brief   Central GPIO pin definition macros for Smart Water Bottle
 */

#ifndef __BSP_GPIO_H__
#define __BSP_GPIO_H__

#include "stm32f1xx_hal.h"

/* ---- DS18B20 (PA8, Open-Drain) ---- */
#define DS18B20_PORT            GPIOA
#define DS18B20_PIN             GPIO_PIN_8

#define DS18B20_SET()           HAL_GPIO_WritePin(DS18B20_PORT, DS18B20_PIN, GPIO_PIN_SET)
#define DS18B20_CLR()           HAL_GPIO_WritePin(DS18B20_PORT, DS18B20_PIN, GPIO_PIN_RESET)
#define DS18B20_READ()          HAL_GPIO_ReadPin(DS18B20_PORT, DS18B20_PIN)

#define DS18B20_OUT_MODE()      do { \
    GPIO_InitTypeDef gpio = {0}; \
    gpio.Pin   = DS18B20_PIN; \
    gpio.Mode  = GPIO_MODE_OUTPUT_OD; \
    gpio.Speed = GPIO_SPEED_FREQ_HIGH; \
    HAL_GPIO_Init(DS18B20_PORT, &gpio); \
} while(0)

#define DS18B20_IN_MODE()       do { \
    GPIO_InitTypeDef gpio = {0}; \
    gpio.Pin   = DS18B20_PIN; \
    gpio.Mode  = GPIO_MODE_INPUT; \
    gpio.Pull  = GPIO_PULLUP; \
    HAL_GPIO_Init(DS18B20_PORT, &gpio); \
} while(0)

/* ---- Battery Voltage Monitoring (PA0, ADC1_CH0) ---- */
#define VBAT_ADC_PORT           GPIOA
#define VBAT_ADC_PIN            GPIO_PIN_0
#define VBAT_ADC_CHANNEL        ADC_CHANNEL_0
/* Voltage divider: R1=100kΩ, R2=100kΩ (1:1), so VBAT = VBAT_ADC * 2 */

/* ---- TDS Sensor (Digital USART3 on PB10/PB11) ---- */
/* TDS USART pins defined below with TP4056 section */

/* ---- M04 Non-contact Capacitive Water Level Sensors (active LOW, open-drain) ---- */
#define WATER_LEVEL_1_PORT      GPIOA
#define WATER_LEVEL_1_PIN       GPIO_PIN_1  /* Lowest sensor */

#define WATER_LEVEL_2_PORT      GPIOB
#define WATER_LEVEL_2_PIN       GPIO_PIN_0  /* Middle sensor */

#define WATER_LEVEL_3_PORT      GPIOB
#define WATER_LEVEL_3_PIN       GPIO_PIN_1  /* Highest sensor */

/* ---- Keys (active LOW) ---- */
#define KEY1_PORT               GPIOA
#define KEY1_PIN                GPIO_PIN_4

#define KEY2_PORT               GPIOA
#define KEY2_PIN                GPIO_PIN_5

/* ---- IR Distance Sensor GP2Y0A21YK0F (PA7, ADC1 Channel 7, analog output) ---- */
#define IR_SENSOR_PORT          GPIOA
#define IR_SENSOR_PIN           GPIO_PIN_7
#define IR_ADC_CHANNEL          ADC_CHANNEL_7
#define IR_CLOSE_THRESHOLD      1000    /* ADC raw threshold: above = lid closed (<~40cm) */

/* ---- Servo Motor (PA6, TIM3_CH1) ---- */
#define SERVO_PORT              GPIOA
#define SERVO_PIN               GPIO_PIN_6
#define SERVO_TIM               TIM3
#define SERVO_CHANNEL           TIM_CHANNEL_1
#define SERVO_TIM_PSC           (72 - 1)
#define SERVO_TIM_ARR           (20000 - 1)

/* ---- Buzzer (PB5, active HIGH) ---- */
#define BUZZER_PORT             GPIOB
#define BUZZER_PIN              GPIO_PIN_5

/* ---- OLED SSD1306 Software I2C (4-wire connection) ---- */
/* Hardware connection (7-pin OLED module, only use 4 wires):
 *   D0 (SCL)  -> PB6
 *   D1 (SDA)  -> PB7
 *   VCC       -> 3.3V
 *   GND       -> GND
 *   (RES/DC/CS pins are not connected in I2C mode)
 */
#define OLED_SCL_PORT           GPIOB
#define OLED_SCL_PIN            GPIO_PIN_6
#define OLED_SDA_PORT           GPIOB
#define OLED_SDA_PIN            GPIO_PIN_7

#define OLED_SCL_SET()          HAL_GPIO_WritePin(OLED_SCL_PORT, OLED_SCL_PIN, GPIO_PIN_SET)
#define OLED_SCL_CLR()          HAL_GPIO_WritePin(OLED_SCL_PORT, OLED_SCL_PIN, GPIO_PIN_RESET)
#define OLED_SDA_SET()          HAL_GPIO_WritePin(OLED_SDA_PORT, OLED_SDA_PIN, GPIO_PIN_SET)
#define OLED_SDA_CLR()          HAL_GPIO_WritePin(OLED_SDA_PORT, OLED_SDA_PIN, GPIO_PIN_RESET)
#define OLED_SDA_READ()         HAL_GPIO_ReadPin(OLED_SDA_PORT, OLED_SDA_PIN)

/* I2C address: 0x78 (7-bit 0x3C << 1), try 0x7A if 0x78 doesn't work */
#define OLED_I2C_ADDR           0x78

/* ---- Bluetooth HC-05 (USART2) ---- */
#define BT_USART                USART2
#define BT_BAUDRATE             9600
#define BT_TX_PORT              GPIOA
#define BT_TX_PIN               GPIO_PIN_2
#define BT_RX_PORT              GPIOA
#define BT_RX_PIN               GPIO_PIN_3

/* ---- Debug UART (USART1) ---- */
#define DEBUG_USART             USART1
#define DEBUG_BAUDRATE          115200
#define DEBUG_TX_PORT           GPIOA
#define DEBUG_TX_PIN            GPIO_PIN_9
#define DEBUG_RX_PORT           GPIOA
#define DEBUG_RX_PIN            GPIO_PIN_10

/* ---- TDS Sensor USART3 ---- */
#define TDS_USART               USART3
#define TDS_BAUDRATE            9600
#define TDS_TX_PORT             GPIOB
#define TDS_TX_PIN              GPIO_PIN_10
#define TDS_RX_PORT             GPIOB
#define TDS_RX_PIN              GPIO_PIN_11

/* ---- TP4056 Charger (active LOW) ---- */
#define TP4056_CHRG_PORT        GPIOB
#define TP4056_CHRG_PIN         GPIO_PIN_12
#define TP4056_STDBY_PORT       GPIOB
#define TP4056_STDBY_PIN        GPIO_PIN_13

/* ---- Onboard LED (PC13, active LOW) ---- */
#define LED_PORT                GPIOC
#define LED_PIN                 GPIO_PIN_13

#endif /* __BSP_GPIO_H__ */
