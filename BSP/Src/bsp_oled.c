/**
 * @file    bsp_oled.c
 * @brief   SSD1306 128x64 OLED driver (Software I2C)
 *
 * Hardware connection (4-wire I2C OLED):
 *   D0 (SCL) -> PB6
 *   D1 (SDA) -> PB7
 *   VCC      -> 3.3V
 *   GND      -> GND
 *   (Other pins like RES/DC/CS are not used in I2C mode)
 */

#include "stm32f1xx_hal.h"
#include "bsp_gpio.h"
#include "bsp_oled.h"
#include "bsp_oled_font.h"
#include <string.h>

/* Forward declarations */
void BSP_DelayUs(uint32_t us);
void BSP_DelayMs(uint32_t ms);

#define OLED_WIDTH   128
#define OLED_HEIGHT  64
#define OLED_PAGES   8

static uint8_t s_gram[OLED_PAGES][OLED_WIDTH];
static uint8_t s_gram_shadow[OLED_PAGES][OLED_WIDTH];

/* ---- Software I2C ---- */

static void I2C_Delay(void)
{
    /* ~5us delay for ~100kHz I2C (more stable than 2us) */
    for (volatile uint16_t i = 0; i < 20; i++);
}

static void I2C_Start(void)
{
    OLED_SDA_SET();
    OLED_SCL_SET();
    I2C_Delay();
    OLED_SDA_CLR();
    I2C_Delay();
    OLED_SCL_CLR();
    I2C_Delay();
}

static void I2C_Stop(void)
{
    OLED_SCL_CLR();
    OLED_SDA_CLR();
    I2C_Delay();
    OLED_SCL_SET();
    I2C_Delay();
    OLED_SDA_SET();
    I2C_Delay();
}

static uint8_t I2C_WaitAck(void)
{
    uint8_t ack;

    OLED_SDA_SET();  /* Release SDA for slave ACK */
    I2C_Delay();
    OLED_SCL_SET();
    I2C_Delay();

    ack = OLED_SDA_READ();

    OLED_SCL_CLR();
    I2C_Delay();

    return ack;  /* 0 = ACK, 1 = NACK */
}

static void I2C_WriteByte(uint8_t byte)
{
    for (uint8_t i = 0; i < 8; i++) {
        OLED_SCL_CLR();
        I2C_Delay();

        if (byte & 0x80) {
            OLED_SDA_SET();
        } else {
            OLED_SDA_CLR();
        }

        byte <<= 1;
        I2C_Delay();

        OLED_SCL_SET();
        I2C_Delay();
    }

    OLED_SCL_CLR();
    I2C_Delay();
}

static void OLED_WriteCmd(uint8_t cmd)
{
    I2C_Start();
    I2C_WriteByte(OLED_I2C_ADDR);
    I2C_WaitAck();
    I2C_WriteByte(0x00); /* Co=0, D/C=0 (command) */
    I2C_WaitAck();
    I2C_WriteByte(cmd);
    I2C_WaitAck();
    I2C_Stop();
}

static void OLED_WriteData(uint8_t data)
{
    I2C_Start();
    I2C_WriteByte(OLED_I2C_ADDR);
    I2C_WaitAck();
    I2C_WriteByte(0x40); /* Co=0, D/C=1 (data) */
    I2C_WaitAck();
    I2C_WriteByte(data);
    I2C_WaitAck();
    I2C_Stop();
}

/* ---- Public API ---- */

void BSP_OLED_Init(void)
{
    /* Wait for OLED power stabilization - critical! */
    BSP_DelayMs(200);

    /* Initialization sequence for SSD1306 128x64 */
    OLED_WriteCmd(0xAE); /* Display OFF */

    OLED_WriteCmd(0xD5); /* Set display clock divide ratio/oscillator frequency */
    OLED_WriteCmd(0x80); /* Default ratio */

    OLED_WriteCmd(0xA8); /* Set multiplex ratio */
    OLED_WriteCmd(0x3F); /* 1/64 duty (64 rows) */

    OLED_WriteCmd(0xD3); /* Set display offset */
    OLED_WriteCmd(0x00); /* No offset */

    OLED_WriteCmd(0x40); /* Set display start line to 0 */

    OLED_WriteCmd(0x8D); /* Charge pump setting */
    OLED_WriteCmd(0x14); /* Enable charge pump (MUST for 3.3V operation) */

    OLED_WriteCmd(0x20); /* Set memory addressing mode */
    OLED_WriteCmd(0x02); /* Page addressing mode */

    OLED_WriteCmd(0xA1); /* Set segment re-map: col127->SEG0 */
    OLED_WriteCmd(0xC8); /* Set COM output scan direction: remapped */

    OLED_WriteCmd(0xDA); /* Set COM pins hardware configuration */
    OLED_WriteCmd(0x12); /* Alternative COM pin config, disable COM L/R remap */

    OLED_WriteCmd(0x81); /* Set contrast control */
    OLED_WriteCmd(0xFF); /* Maximum brightness */

    OLED_WriteCmd(0xD9); /* Set pre-charge period */
    OLED_WriteCmd(0xF1); /* Phase1=15 clocks, Phase2=1 clock */

    OLED_WriteCmd(0xDB); /* Set VCOMH deselect level */
    OLED_WriteCmd(0x40); /* ~0.77*Vcc */

    OLED_WriteCmd(0xA4); /* Entire display ON (resume to RAM content) */
    OLED_WriteCmd(0xA6); /* Set normal display (not inverted) */

    OLED_WriteCmd(0xAF); /* Display ON */

    BSP_DelayMs(100);

    /* Initialize: shadow=0xFF so first Update() sends all pages */
    memset(s_gram, 0x00, sizeof(s_gram));
    memset(s_gram_shadow, 0xFF, sizeof(s_gram_shadow));
    BSP_OLED_Update();
}

void BSP_OLED_Update(void)
{
    for (uint8_t page = 0; page < OLED_PAGES; page++) {
        /* Skip pages that haven't changed */
        if (memcmp(s_gram[page], s_gram_shadow[page], OLED_WIDTH) == 0)
            continue;

        OLED_WriteCmd(0xB0 + page); /* Set page address */
        OLED_WriteCmd(0x00);        /* Set lower column address */
        OLED_WriteCmd(0x10);        /* Set higher column address */

        /* Send entire page in one I2C transaction (128x faster than per-byte) */
        I2C_Start();
        I2C_WriteByte(OLED_I2C_ADDR);
        I2C_WaitAck();
        I2C_WriteByte(0x40); /* Co=0, D/C=1 (data) */
        I2C_WaitAck();
        for (uint8_t col = 0; col < OLED_WIDTH; col++) {
            I2C_WriteByte(s_gram[page][col]);
            I2C_WaitAck();
        }
        I2C_Stop();

        /* Update shadow */
        memcpy(s_gram_shadow[page], s_gram[page], OLED_WIDTH);
    }
}

void BSP_OLED_Clear(void)
{
    memset(s_gram, 0x00, sizeof(s_gram));
}

void BSP_OLED_Fill(uint8_t data)
{
    memset(s_gram, data, sizeof(s_gram));
}

void BSP_OLED_DrawPoint(uint8_t x, uint8_t y, uint8_t mode)
{
    if (x >= OLED_WIDTH || y >= OLED_HEIGHT) return;

    uint8_t page = y / 8;
    uint8_t bit  = y % 8;

    if (mode) {
        s_gram[page][x] |= (1 << bit);
    } else {
        s_gram[page][x] &= ~(1 << bit);
    }
}

void BSP_OLED_ShowChar(uint8_t x, uint8_t y, char ch, uint8_t size)
{
    uint8_t c = (uint8_t)ch - 0x20;
    if (c > 95) return;

    if (size == 16) {
        /* 8x16 font */
        uint8_t page = y / 8;
        if (page >= OLED_PAGES) return;
        for (uint8_t i = 0; i < 8; i++) {
            if (x + i < OLED_WIDTH) {
                s_gram[page][x + i] = F8x16[c][i];
                if (page + 1 < OLED_PAGES) {
                    s_gram[page + 1][x + i] = F8x16[c][i + 8];
                }
            }
        }
    } else {
        /* 6x8 font (size == 12 or default) */
        uint8_t page = y / 8;
        if (page >= OLED_PAGES) return;
        for (uint8_t i = 0; i < 6; i++) {
            if (x + i < OLED_WIDTH) {
                s_gram[page][x + i] = F6x8[c][i];
            }
        }
    }
}

void BSP_OLED_ShowString(uint8_t x, uint8_t y, const char *str, uint8_t size)
{
    uint8_t char_width = (size == 16) ? 8 : 6;

    while (*str) {
        if (x + char_width > OLED_WIDTH) {
            x = 0;
            y += size;
        }
        if (y >= OLED_HEIGHT) break;
        BSP_OLED_ShowChar(x, y, *str, size);
        x += char_width;
        str++;
    }
}

void BSP_OLED_ShowNum(uint8_t x, uint8_t y, uint32_t num, uint8_t len, uint8_t size)
{
    char buf[12];
    uint8_t i = 0;
    uint8_t char_width = (size == 16) ? 8 : 6;

    if (len > 10) len = 10;

    for (uint8_t j = 0; j < len; j++) {
        buf[len - 1 - j] = '0' + (num % 10);
        num /= 10;
    }
    buf[len] = '\0';

    for (i = 0; i < len; i++) {
        BSP_OLED_ShowChar(x + i * char_width, y, buf[i], size);
    }
}
