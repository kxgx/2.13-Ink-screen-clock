/**
 * @file    epd2in13_V3.c
 * @brief   Waveshare 2.13" e-Paper V3 driver (SSD1680 with LUT tables)
 */

#include "epd2in13_V3.h"
#include <stdlib.h>
#include <string.h>

/* =========================================================================
 * LUT tables
 * ========================================================================= */
static const uint8_t lut_partial_update[159] = {
    0x0,0x40,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
    0x80,0x80,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
    0x40,0x40,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
    0x0,0x80,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
    0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
    0x14,0x0,0x0,0x0,0x0,0x0,0x0,  
    0x1,0x0,0x0,0x0,0x0,0x0,0x0,
    0x1,0x0,0x0,0x0,0x0,0x0,0x0,
    0x0,0x0,0x0,0x0,0x0,0x0,0x0,
    0x0,0x0,0x0,0x0,0x0,0x0,0x0,
    0x0,0x0,0x0,0x0,0x0,0x0,0x0,
    0x0,0x0,0x0,0x0,0x0,0x0,0x0,
    0x0,0x0,0x0,0x0,0x0,0x0,0x0,
    0x0,0x0,0x0,0x0,0x0,0x0,0x0,
    0x0,0x0,0x0,0x0,0x0,0x0,0x0,
    0x0,0x0,0x0,0x0,0x0,0x0,0x0,
    0x0,0x0,0x0,0x0,0x0,0x0,0x0,
    0x22,0x22,0x22,0x22,0x22,0x22,0x0,0x0,0x0,
    0x22,0x17,0x41,0x00,0x32,0x36,
};

static const uint8_t lut_full_update[159] = {
    0x80,0x4A,0x40,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
    0x40,0x4A,0x80,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
    0x80,0x4A,0x40,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
    0x40,0x4A,0x80,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
    0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
    0xF,0x0,0x0,0x0,0x0,0x0,0x0,
    0xF,0x0,0x0,0xF,0x0,0x0,0x2,
    0xF,0x0,0x0,0x0,0x0,0x0,0x0,
    0x1,0x0,0x0,0x0,0x0,0x0,0x0,
    0x0,0x0,0x0,0x0,0x0,0x0,0x0,
    0x0,0x0,0x0,0x0,0x0,0x0,0x0,
    0x0,0x0,0x0,0x0,0x0,0x0,0x0,
    0x0,0x0,0x0,0x0,0x0,0x0,0x0,
    0x0,0x0,0x0,0x0,0x0,0x0,0x0,
    0x0,0x0,0x0,0x0,0x0,0x0,0x0,
    0x0,0x0,0x0,0x0,0x0,0x0,0x0,
    0x0,0x0,0x0,0x0,0x0,0x0,0x0,
    0x22,0x22,0x22,0x22,0x22,0x22,0x0,0x0,0x0,
    0x22,0x17,0x41,0x0,0x32,0x36,
};

/* Register commands */
#define CMD_DRIVER_OUTPUT      0x01
#define CMD_GATE_VOLTAGE       0x03
#define CMD_SOURCE_VOLTAGE     0x04
#define CMD_DEEP_SLEEP         0x10
#define CMD_DATA_ENTRY         0x11
#define CMD_SW_RESET           0x12
#define CMD_TEMP_SENSOR        0x18
#define CMD_MASTER_ACTIVATE    0x20
#define CMD_DISP_UPDATE_CTRL   0x21
#define CMD_DISP_UPDATE_CTRL2  0x22
#define CMD_WRITE_RAM_BW       0x24
#define CMD_WRITE_RAM_RED      0x26
#define CMD_VCOM_VOLTAGE       0x2C
#define CMD_LUT_FOR_VCOM       0x32
#define CMD_BORDER_WAVEFORM    0x3C
#define CMD_LUT_OPTION         0x3F
#define CMD_SET_RAM_X          0x44
#define CMD_SET_RAM_Y          0x45
#define CMD_SET_RAM_X_CTR      0x4E
#define CMD_SET_RAM_Y_CTR      0x4F

static void _set_window(EPD *epd, int xs, int ys, int xe, int ye) {
    epd_send_command(epd, CMD_SET_RAM_X);
    epd_send_data(epd, (xs >> 3) & 0xFF);
    epd_send_data(epd, (xe >> 3) & 0xFF);
    epd_send_command(epd, CMD_SET_RAM_Y);
    epd_send_data(epd, ys & 0xFF);
    epd_send_data(epd, (ys >> 8) & 0xFF);
    epd_send_data(epd, ye & 0xFF);
    epd_send_data(epd, (ye >> 8) & 0xFF);
}

static void _set_cursor(EPD *epd, int x, int y) {
    epd_send_command(epd, CMD_SET_RAM_X_CTR);
    epd_send_data(epd, x & 0xFF);
    epd_send_command(epd, CMD_SET_RAM_Y_CTR);
    epd_send_data(epd, y & 0xFF);
    epd_send_data(epd, (y >> 8) & 0xFF);
}

static void _lut(EPD *epd, const uint8_t *lut, size_t len) {
    epd_send_command(epd, CMD_LUT_FOR_VCOM);
    for (size_t i = 0; i < len && i < 153; i++) {
        epd_send_data(epd, lut[i]);
    }
    epd_wait_busy(epd);
}

static void _set_lut(EPD *epd, const uint8_t *lut) {
    _lut(epd, lut, 153);
    epd_send_command(epd, CMD_LUT_OPTION);
    epd_send_data(epd, lut[153]);
    epd_send_command(epd, CMD_GATE_VOLTAGE);
    epd_send_data(epd, lut[154]);
    epd_send_command(epd, CMD_SOURCE_VOLTAGE);
    epd_send_data(epd, lut[155]);
    epd_send_data(epd, lut[156]);
    epd_send_data(epd, lut[157]);
    epd_send_command(epd, CMD_VCOM_VOLTAGE);
    epd_send_data(epd, lut[158]);
}

static void _turn_on_display(EPD *epd) {
    epd_send_command(epd, CMD_DISP_UPDATE_CTRL2);
    epd_send_data(epd, 0xC7);
    epd_send_command(epd, CMD_MASTER_ACTIVATE);
    epd_wait_busy(epd);
}

static void _turn_on_display_part(EPD *epd) {
    epd_send_command(epd, CMD_DISP_UPDATE_CTRL2);
    epd_send_data(epd, 0x0F);
    epd_send_command(epd, CMD_MASTER_ACTIVATE);
    epd_wait_busy(epd);
}

int EPD_2in13_V3_Init(EPD *epd) {
    if (!epd) return EPD_ERROR;
    epd->width = EPD2IN13_V3_WIDTH; epd->height = EPD2IN13_V3_HEIGHT;
    epd->rst_pin = EPD_RST_PIN; epd->dc_pin = EPD_DC_PIN;
    epd->cs_pin = EPD_CS_PIN; epd->busy_pin = EPD_BUSY_PIN; epd->pwr_pin = EPD_PWR_PIN;
    if (epd_setup_default_hal(epd) != EPD_OK) return EPD_ERROR;

    epd_reset(epd);
    epd_wait_busy(epd);

    epd_send_command(epd, CMD_SW_RESET);
    epd_wait_busy(epd);

    epd_send_command(epd, CMD_DRIVER_OUTPUT);
    epd_send_data(epd, 0xF9); epd_send_data(epd, 0x00); epd_send_data(epd, 0x00);

    epd_send_command(epd, CMD_DATA_ENTRY);
    epd_send_data(epd, 0x03);

    _set_window(epd, 0, 0, epd->width - 1, epd->height - 1);
    _set_cursor(epd, 0, 0);

    epd_send_command(epd, CMD_BORDER_WAVEFORM);
    epd_send_data(epd, 0x05);

    epd_send_command(epd, CMD_DISP_UPDATE_CTRL);
    epd_send_data(epd, 0x00); epd_send_data(epd, 0x80);

    epd_send_command(epd, CMD_TEMP_SENSOR);
    epd_send_data(epd, 0x80);

    epd_wait_busy(epd);
    _set_lut(epd, lut_full_update);
    return EPD_OK;
}

void EPD_2in13_V3_Clear(EPD *epd, uint8_t color) {
    if (!epd) return;
    int lw = epd_linewidth(epd->width);
    uint8_t *buf = (uint8_t *)malloc(epd->height * lw);
    if (!buf) return;
    memset(buf, color, epd->height * lw);
    epd_send_command(epd, CMD_WRITE_RAM_BW);
    epd_send_data_buf(epd, buf, epd->height * lw);
    free(buf);
    _turn_on_display(epd);
}

void EPD_2in13_V3_Display(EPD *epd, const uint8_t *image) {
    if (!epd || !image) return;
    epd_send_command(epd, CMD_WRITE_RAM_BW);
    epd_send_data_buf(epd, image, epd_linewidth(epd->width) * epd->height);
    _turn_on_display(epd);
}

void EPD_2in13_V3_DisplayPartial(EPD *epd, const uint8_t *image) {
    if (!epd || !image) return;

    epd->digital_write(epd->rst_pin, 0); epd->delay_ms(1);
    epd->digital_write(epd->rst_pin, 1);

    _set_lut(epd, lut_partial_update);
    epd_send_command(epd, 0x37);
    for (int i = 0; i < 10; i++) epd_send_data(epd, (i == 5) ? 0x40 : 0x00);

    epd_send_command(epd, CMD_BORDER_WAVEFORM);
    epd_send_data(epd, 0x80);

    epd_send_command(epd, CMD_DISP_UPDATE_CTRL2);
    epd_send_data(epd, 0xC0);
    epd_send_command(epd, CMD_MASTER_ACTIVATE);
    epd_wait_busy(epd);

    _set_window(epd, 0, 0, epd->width - 1, epd->height - 1);
    _set_cursor(epd, 0, 0);

    epd_send_command(epd, CMD_WRITE_RAM_BW);
    epd_send_data_buf(epd, image, epd_linewidth(epd->width) * epd->height);
    _turn_on_display_part(epd);
}

void EPD_2in13_V3_DisplayPartBaseImage(EPD *epd, const uint8_t *image) {
    if (!epd || !image) return;
    int sz = epd_linewidth(epd->width) * epd->height;
    epd_send_command(epd, CMD_WRITE_RAM_BW);
    epd_send_data_buf(epd, image, sz);
    epd_send_command(epd, CMD_WRITE_RAM_RED);
    epd_send_data_buf(epd, image, sz);
    _turn_on_display(epd);
}

void EPD_2in13_V3_Sleep(EPD *epd) {
    if (!epd) return;
    epd_send_command(epd, CMD_DEEP_SLEEP);
    epd_send_data(epd, 0x01);
    epd->delay_ms(2000);
    epd_teardown_default_hal(epd);
}
