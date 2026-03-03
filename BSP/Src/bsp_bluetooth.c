/**
 * @file    bsp_bluetooth.c
 * @brief   HC-05 Bluetooth module driver (USART2)
 */

#include "bsp.h"
#include <string.h>

static UART_HandleTypeDef s_huart2;

void BSP_BT_Init(void)
{
    GPIO_InitTypeDef gpio = {0};

    __HAL_RCC_USART2_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    /* PA2 USART2_TX: AF push-pull */
    gpio.Pin   = BT_TX_PIN;
    gpio.Mode  = GPIO_MODE_AF_PP;
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(BT_TX_PORT, &gpio);

    /* PA3 USART2_RX: Input floating */
    gpio.Pin  = BT_RX_PIN;
    gpio.Mode = GPIO_MODE_INPUT;
    gpio.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(BT_RX_PORT, &gpio);

    s_huart2.Instance          = BT_USART;
    s_huart2.Init.BaudRate     = BT_BAUDRATE;
    s_huart2.Init.WordLength   = UART_WORDLENGTH_8B;
    s_huart2.Init.StopBits     = UART_STOPBITS_1;
    s_huart2.Init.Parity       = UART_PARITY_NONE;
    s_huart2.Init.Mode         = UART_MODE_TX_RX;
    s_huart2.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
    s_huart2.Init.OverSampling = UART_OVERSAMPLING_16;

    HAL_UART_Init(&s_huart2);
}

void BSP_BT_SendByte(uint8_t byte)
{
    HAL_UART_Transmit(&s_huart2, &byte, 1, 100);
}

void BSP_BT_SendString(const char *str)
{
    HAL_UART_Transmit(&s_huart2, (uint8_t *)str, (uint16_t)strlen(str), 500);
}

void BSP_BT_SendData(const uint8_t *data, uint16_t len)
{
    HAL_UART_Transmit(&s_huart2, (uint8_t *)data, len, 500);
}
