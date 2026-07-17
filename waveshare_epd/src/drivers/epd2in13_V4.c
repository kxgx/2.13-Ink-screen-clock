/**
 * @file    epd2in13_V4.c
 * @brief   Waveshare 2.13" e-Paper V4 driver implementation (SSD1680)
 */

#include "epd2in13_V4.h"
#include <stdlib.h>
#include <string.h>

/* =========================================================================
 * Register commands
 * ========================================================================= */
#define CMD_DRIVER_OUTPUT_CONTROL      0x01
#define CMD_DEEP_SLEEP                 0x10
#define CMD_DATA_ENTRY_MODE            0x11
#define CMD_SW_RESET                   0x12
#define CMD_TEMP_SENSOR_CONTROL        0x18
#define CMD_TEMP_SENSOR_WRITE          0x1A
#define CMD_MASTER_ACTIVATION          0x20
#define CMD_DISPLAY_UPDATE_CONTROL     0x21
#define CMD_DISPLAY_UPDATE_CONTROL2    0x22
#define CMD_WRITE_RAM_BW               0x24
#define CMD_WRITE_RAM_RED              0x26
#define CMD_BORDER_WAVEFORM            0x3C
#define CMD_SET_RAM_X                  0x44
#define CMD_SET_RAM_Y                  0x45
#define CMD_SET_RAM_X_COUNTER          0x4E
#define CMD_SET_RAM_Y_COUNTER          0x4F

/* =========================================================================
 * Internal helpers
 * ========================================================================= */
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
    epd_send_command(epd, CMD_SET_RAM_X_COUNTER);
    epd_send_data(epd, x & 0xFF);

    epd_send_command(epd, CMD_SET_RAM_Y_COUNTER);
    epd_send_data(epd, y & 0xFF);
    epd_send_data(epd, (y >> 8) & 0xFF);
}

static void _turn_on_display(EPD *epd) {
    epd_send_command(epd, CMD_DISPLAY_UPDATE_CONTROL2);
    epd_send_data(epd, 0xF7);
    epd_send_command(epd, CMD_MASTER_ACTIVATION);
    epd_wait_busy(epd);
}

static void _turn_on_display_fast(EPD *epd) {
    epd_send_command(epd, CMD_DISPLAY_UPDATE_CONTROL2);
    epd_send_data(epd, 0xC7);
    epd_send_command(epd, CMD_MASTER_ACTIVATION);
    epd_wait_busy(epd);
}

static void _turn_on_display_part(EPD *epd) {
    epd_send_command(epd, CMD_DISPLAY_UPDATE_CONTROL2);
    epd_send_data(epd, 0xFF);
    epd_send_command(epd, CMD_MASTER_ACTIVATION);
    epd_wait_busy(epd);
}

/* =========================================================================
 * Public API
 * ========================================================================= */

int EPD_2in13_V4_Init(EPD *epd) {
    if (!epd) return EPD_ERROR;

    epd->width    = EPD2IN13_V4_WIDTH;
    epd->height   = EPD2IN13_V4_HEIGHT;
    epd->rst_pin  = EPD_RST_PIN;
    epd->dc_pin   = EPD_DC_PIN;
    epd->cs_pin   = EPD_CS_PIN;
    epd->busy_pin = EPD_BUSY_PIN;
    epd->pwr_pin  = EPD_PWR_PIN;

    if (epd_setup_default_hal(epd) != EPD_OK) {
        return EPD_ERROR;
    }

    epd_reset(epd);
    epd_wait_busy(epd);

    epd_send_command(epd, CMD_SW_RESET);
    epd_wait_busy(epd);

    epd_send_command(epd, CMD_DRIVER_OUTPUT_CONTROL);
    epd_send_data(epd, 0xF9);
    epd_send_data(epd, 0x00);
    epd_send_data(epd, 0x00);

    epd_send_command(epd, CMD_DATA_ENTRY_MODE);
    epd_send_data(epd, 0x03);

    _set_window(epd, 0, 0, epd->width - 1, epd->height - 1);
    _set_cursor(epd, 0, 0);

    epd_send_command(epd, CMD_BORDER_WAVEFORM);
    epd_send_data(epd, 0x05);

    epd_send_command(epd, CMD_DISPLAY_UPDATE_CONTROL);
    epd_send_data(epd, 0x00);
    epd_send_data(epd, 0x80);

    epd_send_command(epd, CMD_TEMP_SENSOR_CONTROL);
    epd_send_data(epd, 0x80);

    epd_wait_busy(epd);
    return EPD_OK;
}

int EPD_2in13_V4_Init_Fast(EPD *epd) {
    if (!epd) return EPD_ERROR;

    epd->width    = EPD2IN13_V4_WIDTH;
    epd->height   = EPD2IN13_V4_HEIGHT;
    epd->rst_pin  = EPD_RST_PIN;
    epd->dc_pin   = EPD_DC_PIN;
    epd->cs_pin   = EPD_CS_PIN;
    epd->busy_pin = EPD_BUSY_PIN;
    epd->pwr_pin  = EPD_PWR_PIN;

    if (epd_setup_default_hal(epd) != EPD_OK) {
        return EPD_ERROR;
    }

    epd_reset(epd);

    epd_send_command(epd, CMD_SW_RESET);
    epd_wait_busy(epd);

    epd_send_command(epd, CMD_TEMP_SENSOR_CONTROL);
    epd_send_command(epd, 0x80);

    epd_send_command(epd, CMD_DATA_ENTRY_MODE);
    epd_send_data(epd, 0x03);

    _set_window(epd, 0, 0, epd->width - 1, epd->height - 1);
    _set_cursor(epd, 0, 0);

    epd_send_command(epd, CMD_DISPLAY_UPDATE_CONTROL2);
    epd_send_data(epd, 0xB1);
    epd_send_command(epd, CMD_MASTER_ACTIVATION);
    epd_wait_busy(epd);

    epd_send_command(epd, CMD_TEMP_SENSOR_WRITE);
    epd_send_data(epd, 0x64);
    epd_send_data(epd, 0x00);

    epd_send_command(epd, CMD_DISPLAY_UPDATE_CONTROL2);
    epd_send_data(epd, 0x91);
    epd_send_command(epd, CMD_MASTER_ACTIVATION);
    epd_wait_busy(epd);

    return EPD_OK;
}

void EPD_2in13_V4_Clear(EPD *epd, uint8_t color) {
    if (!epd) return;
    int lw = epd_linewidth(epd->width);
    int bufsize = epd->height * lw;

    /* Allocate on heap for large buffers on embedded systems */
    uint8_t *buf = (uint8_t *)malloc(bufsize);
    if (!buf) return;
    memset(buf, color, bufsize);

    epd_send_command(epd, CMD_WRITE_RAM_BW);
    epd_send_data_buf(epd, buf, bufsize);
    free(buf);

    _turn_on_display(epd);
}

void EPD_2in13_V4_Display(EPD *epd, const uint8_t *image) {
    if (!epd || !image) return;

    epd_send_command(epd, CMD_WRITE_RAM_BW);
    epd_send_data_buf(epd, image, epd_linewidth(epd->width) * epd->height);
    _turn_on_display(epd);
}

void EPD_2in13_V4_Display_Fast(EPD *epd, const uint8_t *image) {
    if (!epd || !image) return;

    epd_send_command(epd, CMD_WRITE_RAM_BW);
    epd_send_data_buf(epd, image, epd_linewidth(epd->width) * epd->height);
    _turn_on_display_fast(epd);
}

void EPD_2in13_V4_DisplayPartial(EPD *epd, const uint8_t *image) {
    if (!epd || !image) return;

    epd->digital_write(epd->rst_pin, 0);
    epd->delay_ms(1);
    epd->digital_write(epd->rst_pin, 1);

    epd_send_command(epd, CMD_BORDER_WAVEFORM);
    epd_send_data(epd, 0x80);

    epd_send_command(epd, CMD_DRIVER_OUTPUT_CONTROL);
    epd_send_data(epd, 0xF9);
    epd_send_data(epd, 0x00);
    epd_send_data(epd, 0x00);

    epd_send_command(epd, CMD_DATA_ENTRY_MODE);
    epd_send_data(epd, 0x03);

    _set_window(epd, 0, 0, epd->width - 1, epd->height - 1);
    _set_cursor(epd, 0, 0);

    epd_send_command(epd, CMD_WRITE_RAM_BW);
    epd_send_data_buf(epd, image, epd_linewidth(epd->width) * epd->height);
    _turn_on_display_part(epd);
}

void EPD_2in13_V4_DisplayPartBaseImage(EPD *epd, const uint8_t *image) {
    if (!epd || !image) return;
    int bufsize = epd_linewidth(epd->width) * epd->height;

    epd_send_command(epd, CMD_WRITE_RAM_BW);
    epd_send_data_buf(epd, image, bufsize);

    epd_send_command(epd, CMD_WRITE_RAM_RED);
    epd_send_data_buf(epd, image, bufsize);
    _turn_on_display(epd);
}

void EPD_2in13_V4_Sleep(EPD *epd) {
    if (!epd) return;

    epd_send_command(epd, CMD_DEEP_SLEEP);
    epd_send_data(epd, 0x01);

    epd->delay_ms(2000);
    epd_teardown_default_hal(epd);
}
