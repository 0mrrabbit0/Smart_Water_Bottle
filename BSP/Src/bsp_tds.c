/**
 * @file    bsp_tds.c
 * @brief   Digital TDS / multi-parameter water quality sensor (USART3)
 *
 * Wire-level protocol is documented in bsp_tds.h.
 */

#include "bsp.h"
#include <string.h>

#define TDS_RX_BUF_SIZE   64
#define TDS_TIMEOUT_MS    1000

static UART_HandleTypeDef s_huart_tds;
static uint8_t s_rx_buffer[TDS_RX_BUF_SIZE];

/* Checksum used by the TDS protocol: arithmetic sum of all bytes, modulo 256 */
static uint8_t TDS_CalcChecksum(const uint8_t *data, uint16_t len)
{
    uint8_t sum = 0;
    for (uint16_t i = 0; i < len; i++) {
        sum += data[i];
    }
    return sum;
}

void BSP_TDS_Init(void)
{
    /* GPIO and UART clock enabled in HAL_UART_MspInit */
    s_huart_tds.Instance          = TDS_USART;
    s_huart_tds.Init.BaudRate     = TDS_BAUDRATE;
    s_huart_tds.Init.WordLength   = UART_WORDLENGTH_8B;
    s_huart_tds.Init.StopBits     = UART_STOPBITS_1;
    s_huart_tds.Init.Parity       = UART_PARITY_NONE;
    s_huart_tds.Init.Mode         = UART_MODE_TX_RX;
    s_huart_tds.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
    s_huart_tds.Init.OverSampling = UART_OVERSAMPLING_16;

    HAL_UART_Init(&s_huart_tds);
}

/* HAL MSP callback for USART3 */
void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
    GPIO_InitTypeDef gpio = {0};

    if (huart->Instance == USART3) {
        __HAL_RCC_USART3_CLK_ENABLE();
        __HAL_RCC_GPIOB_CLK_ENABLE();

        /* TX: PB10 */
        gpio.Pin   = TDS_TX_PIN;
        gpio.Mode  = GPIO_MODE_AF_PP;
        gpio.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(TDS_TX_PORT, &gpio);

        /* RX: PB11 */
        gpio.Pin  = TDS_RX_PIN;
        gpio.Mode = GPIO_MODE_INPUT;
        gpio.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(TDS_RX_PORT, &gpio);
    }
}

/**
 * @brief  Read full sensor frame from the TDS module.
 * @return 1 on success, 0 on timeout or checksum mismatch.
 */
uint8_t BSP_TDS_ReadData(tds_sensor_data_t *data)
{
    /* Query command: A0 00 00 00 00 00 A0 (7 bytes) */
    const uint8_t cmd[] = {0xA0, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA0};

    memset(s_rx_buffer, 0, sizeof(s_rx_buffer));
    memset(data, 0, sizeof(tds_sensor_data_t));

    if (HAL_UART_Transmit(&s_huart_tds, (uint8_t*)cmd, sizeof(cmd), 100) != HAL_OK) {
        return 0;
    }

    /* Sensor needs ~50 ms to process the query before responding */
    HAL_Delay(50);

    uint16_t rx_len = 0;
    uint32_t start_tick = HAL_GetTick();

    while (rx_len < TDS_RX_BUF_SIZE && (HAL_GetTick() - start_tick) < TDS_TIMEOUT_MS) {
        if (HAL_UART_Receive(&s_huart_tds, &s_rx_buffer[rx_len], 1, 10) == HAL_OK) {
            rx_len++;
            if (rx_len >= 1 && s_rx_buffer[0] == 0xAA) {
                if (HAL_UART_Receive(&s_huart_tds, &s_rx_buffer[rx_len], 15, 200) == HAL_OK) {
                    rx_len = 16;
                    break;
                }
            }
        }
    }

    if (rx_len != 16 || s_rx_buffer[0] != 0xAA) {
        return 0;
    }

    /* CRC8 = sum of bytes 0..14 must equal byte 15 */
    uint8_t calculated_crc = TDS_CalcChecksum(s_rx_buffer, 15);
    if (calculated_crc != s_rx_buffer[15]) {
        return 0;
    }

    /* Big-endian fields per the protocol header above */
    data->tds         = ((uint16_t)s_rx_buffer[1]  << 8) | s_rx_buffer[2];
    data->ec          = ((uint32_t)s_rx_buffer[3]  << 24) |
                        ((uint32_t)s_rx_buffer[4]  << 16) |
                        ((uint32_t)s_rx_buffer[5]  << 8)  |
                        s_rx_buffer[6];
    data->salinity    = ((uint16_t)s_rx_buffer[7]  << 8) | s_rx_buffer[8];
    data->sg          = ((uint16_t)s_rx_buffer[9]  << 8) | s_rx_buffer[10];
    data->temperature = ((uint16_t)s_rx_buffer[11] << 8) | s_rx_buffer[12];
    data->hardness    = ((uint16_t)s_rx_buffer[13] << 8) | s_rx_buffer[14];
    data->valid       = 1;

    return 1;
}

/**
 * @brief  Convenience wrapper that returns the TDS value (ppm) only.
 * @return TDS value, or 0 if the read failed.
 */
uint16_t BSP_TDS_ReadValue(void)
{
    tds_sensor_data_t data;

    if (BSP_TDS_ReadData(&data) && data.valid) {
        return data.tds;
    }

    return 0;
}
