/**
 * @file    bsp_debug_uart.c
 * @brief   Debug UART (USART1) with RX interrupt for serial key simulation
 *
 * Receives single ASCII characters and injects key events into g_queue_key:
 *   '1' = KEY1 short press    '2' = KEY1 long press
 *   '3' = KEY2 short press    '4' = KEY2 long press
 */

#include "bsp_debug_uart.h"
#include "bsp_gpio.h"
#include "stm32f1xx_hal.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "app_config.h"
#include <string.h>

extern QueueHandle_t g_queue_key;

static UART_HandleTypeDef s_huart1;

void BSP_DebugUart_Init(void)
{
    GPIO_InitTypeDef gpio = {0};

    __HAL_RCC_USART1_CLK_ENABLE();

    /* PA9 USART1_TX: AF push-pull */
    gpio.Pin   = DEBUG_TX_PIN;
    gpio.Mode  = GPIO_MODE_AF_PP;
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(DEBUG_TX_PORT, &gpio);

    /* PA10 USART1_RX: Input floating */
    gpio.Pin  = DEBUG_RX_PIN;
    gpio.Mode = GPIO_MODE_INPUT;
    gpio.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(DEBUG_RX_PORT, &gpio);

    s_huart1.Instance          = DEBUG_USART;
    s_huart1.Init.BaudRate     = DEBUG_BAUDRATE;
    s_huart1.Init.WordLength   = UART_WORDLENGTH_8B;
    s_huart1.Init.StopBits     = UART_STOPBITS_1;
    s_huart1.Init.Parity       = UART_PARITY_NONE;
    s_huart1.Init.Mode         = UART_MODE_TX_RX;
    s_huart1.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
    s_huart1.Init.OverSampling = UART_OVERSAMPLING_16;

    HAL_UART_Init(&s_huart1);

    /* RXNE interrupt at NVIC priority 6 (lower than configLIBRARY_MAX_SYSCALL = 5) */
    HAL_NVIC_SetPriority(USART1_IRQn, 6, 0);
    HAL_NVIC_EnableIRQ(USART1_IRQn);
    __HAL_UART_ENABLE_IT(&s_huart1, UART_IT_RXNE);

    /* Polled banner: scheduler has not started yet so the ISR is not used here */
    const char *msg =
        "\r\n[DBG] Serial Key Sim Ready (115200 8N1)\r\n"
        "  1=K1_SHORT  2=K1_LONG  3=K2_SHORT  4=K2_LONG\r\n";
    HAL_UART_Transmit(&s_huart1, (uint8_t *)msg, (uint16_t)strlen(msg), 200);
}

void BSP_DebugUart_SendString(const char *str)
{
    HAL_UART_Transmit(&s_huart1, (uint8_t *)str, (uint16_t)strlen(str), 500);
}

/* ---- ISR helper: polling send a string ---- */
static void ISR_SendStr(const char *s)
{
    while (*s) {
        while (!(s_huart1.Instance->SR & UART_FLAG_TXE));
        s_huart1.Instance->DR = (uint8_t)*s++;
    }
}

/* ---- USART1 IRQ: serial -> key event ---- */

void USART1_IRQHandler(void)
{
    /* RXNE: data received */
    if (__HAL_UART_GET_FLAG(&s_huart1, UART_FLAG_RXNE)) {
        uint8_t ch = (uint8_t)(s_huart1.Instance->DR & 0xFF);

        key_event_t evt = KEY_EVENT_NONE;

        switch (ch) {
        case '1': evt = KEY_EVENT_KEY1_SHORT; ISR_SendStr("[K1_SHORT]\r\n"); break;
        case '2': evt = KEY_EVENT_KEY1_LONG;  ISR_SendStr("[K1_LONG]\r\n");  break;
        case '3': evt = KEY_EVENT_KEY2_SHORT; ISR_SendStr("[K2_SHORT]\r\n"); break;
        case '4': evt = KEY_EVENT_KEY2_LONG;  ISR_SendStr("[K2_LONG]\r\n");  break;
        default: {
            /* Echo any unrecognised byte back as hex for diagnostics */
            const char hex[] = "0123456789ABCDEF";
            char buf[] = "[RX:0x??]\r\n";
            buf[5] = hex[(ch >> 4) & 0x0F];
            buf[6] = hex[ch & 0x0F];
            ISR_SendStr(buf);
            break;
        }
        }

        if (evt != KEY_EVENT_NONE && g_queue_key != NULL) {
            BaseType_t xHigherPriorityTaskWoken = pdFALSE;
            xQueueSendFromISR(g_queue_key, &evt, &xHigherPriorityTaskWoken);
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }

    /* Clear ORE by reading SR then DR (STM32F1 reference manual sequence) */
    if (__HAL_UART_GET_FLAG(&s_huart1, UART_FLAG_ORE)) {
        (void)s_huart1.Instance->SR;
        (void)s_huart1.Instance->DR;
    }
}
