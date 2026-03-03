/**
 * @file    bsp_ds18b20.c
 * @brief   DS18B20 temperature sensor driver (OneWire protocol)
 */

#include "bsp.h"

#define DS18B20_CMD_SKIP_ROM        0xCC
#define DS18B20_CMD_CONVERT_T       0x44
#define DS18B20_CMD_READ_SCRATCHPAD 0xBE

static uint8_t DS18B20_Reset(void)
{
    uint8_t presence;

    DS18B20_OUT_MODE();
    DS18B20_CLR();
    BSP_DelayUs(480);
    DS18B20_SET();
    BSP_DelayUs(60);

    DS18B20_IN_MODE();
    presence = (DS18B20_READ() == GPIO_PIN_RESET) ? 1 : 0;
    BSP_DelayUs(420);
    DS18B20_OUT_MODE();
    DS18B20_SET();

    return presence;
}

static void DS18B20_WriteBit(uint8_t bit)
{
    DS18B20_OUT_MODE();
    DS18B20_CLR();
    BSP_DelayUs(2);

    if (bit) {
        DS18B20_SET();
    }
    BSP_DelayUs(60);
    DS18B20_SET();
    BSP_DelayUs(2);
}

static uint8_t DS18B20_ReadBit(void)
{
    uint8_t bit;

    DS18B20_OUT_MODE();
    DS18B20_CLR();
    BSP_DelayUs(2);
    DS18B20_SET();

    DS18B20_IN_MODE();
    BSP_DelayUs(12);
    bit = (DS18B20_READ() == GPIO_PIN_SET) ? 1 : 0;
    BSP_DelayUs(50);

    return bit;
}

static void DS18B20_WriteByte(uint8_t byte)
{
    for (uint8_t i = 0; i < 8; i++) {
        DS18B20_WriteBit(byte & 0x01);
        byte >>= 1;
    }
}

static uint8_t DS18B20_ReadByte(void)
{
    uint8_t byte = 0;
    for (uint8_t i = 0; i < 8; i++) {
        byte >>= 1;
        if (DS18B20_ReadBit()) {
            byte |= 0x80;
        }
    }
    return byte;
}

int BSP_DS18B20_Init(void)
{
    return DS18B20_Reset() ? 0 : -1;
}

float BSP_DS18B20_ReadTemp(void)
{
    uint8_t temp_l, temp_h;
    int16_t raw;

    if (!DS18B20_Reset()) {
        return -999.0f;
    }

    DS18B20_WriteByte(DS18B20_CMD_SKIP_ROM);
    DS18B20_WriteByte(DS18B20_CMD_CONVERT_T);

    /* Wait for conversion (750ms for 12-bit) */
    BSP_DelayMs(750);

    if (!DS18B20_Reset()) {
        return -999.0f;
    }

    DS18B20_WriteByte(DS18B20_CMD_SKIP_ROM);
    DS18B20_WriteByte(DS18B20_CMD_READ_SCRATCHPAD);

    temp_l = DS18B20_ReadByte();
    temp_h = DS18B20_ReadByte();

    raw = (int16_t)((temp_h << 8) | temp_l);

    return raw / 16.0f;
}
