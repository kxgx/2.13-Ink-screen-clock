/**
 * @file    epd2in13_V2.c
 * @brief   Waveshare 2.13" e-Paper V2 driver (SSD1680 with LUT)
 */

#include "epd2in13_V2.h"
#include <stdlib.h>
#include <string.h>

static const uint8_t lut_full_update[76] = {
    0x80,0x60,0x40,0x00,0x00,0x00,0x00, 0x10,0x60,0x20,0x00,0x00,0x00,0x00,
    0x80,0x60,0x40,0x00,0x00,0x00,0x00, 0x10,0x60,0x20,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x03,0x03,0x00,0x00,0x02, 0x09,0x09,0x00,0x00,0x02,
    0x03,0x03,0x00,0x00,0x02, 0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00, 0x15,0x41,0xA8,0x32,0x30,0x0A,
};

static const uint8_t lut_partial_update[76] = {
    0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x80,0x00,0x00,0x00,0x00,0x00,0x00,
    0x40,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x0A,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00, 0x15,0x41,0xA8,0x32,0x30,0x0A,
};

#define CMD_DEEP_SLEEP      0x10
#define CMD_DATA_ENTRY      0x11
#define CMD_SW_RESET        0x12
#define CMD_MASTER_ACT      0x20
#define CMD_DISP_UPDATE2    0x22
#define CMD_WRITE_RAM_BW    0x24
#define CMD_WRITE_RAM_RED   0x26
#define CMD_VCOM_VOLTAGE    0x2C
#define CMD_LUT_VCOM        0x32
#define CMD_BORDER           0x3C
#define CMD_DUMMY_LINE      0x3A
#define CMD_GATE_TIME       0x3B
#define CMD_SET_RAM_X_CTR   0x4E
#define CMD_SET_RAM_Y_CTR   0x4F

static void _turn_on_display(EPD *epd) {
    epd_send_command(epd, CMD_DISP_UPDATE2); epd_send_data(epd, 0xC7);
    epd_send_command(epd, CMD_MASTER_ACT); epd_wait_busy(epd);
}

static void _turn_on_display_part(EPD *epd) {
    epd_send_command(epd, CMD_DISP_UPDATE2); epd_send_data(epd, 0x0C);
    epd_send_command(epd, CMD_MASTER_ACT); epd_wait_busy(epd);
}

int EPD_2in13_V2_Init(EPD *epd, int update_mode) {
    if (!epd) return EPD_ERROR;
    epd->width = EPD2IN13_V2_WIDTH; epd->height = EPD2IN13_V2_HEIGHT;
    epd->rst_pin = EPD_RST_PIN; epd->dc_pin = EPD_DC_PIN;
    epd->cs_pin = EPD_CS_PIN; epd->busy_pin = EPD_BUSY_PIN; epd->pwr_pin = EPD_PWR_PIN;
    if (epd_setup_default_hal(epd) != EPD_OK) return EPD_ERROR;

    epd_reset(epd);
    if (update_mode == EPD_FULL_UPDATE) {
        epd_wait_busy(epd);
        epd_send_command(epd, CMD_SW_RESET); epd_wait_busy(epd);

        epd_send_command(epd, 0x74); epd_send_data(epd, 0x54);
        epd_send_command(epd, 0x7E); epd_send_data(epd, 0x3B);

        epd_send_command(epd, 0x01); epd_send_data(epd, 0xF9); epd_send_data(epd, 0x00); epd_send_data(epd, 0x00);
        epd_send_command(epd, CMD_DATA_ENTRY); epd_send_data(epd, 0x01);

        epd_send_command(epd, 0x44); epd_send_data(epd, 0x00); epd_send_data(epd, 0x0F);
        epd_send_command(epd, 0x45); epd_send_data(epd, 0xF9); epd_send_data(epd, 0x00); epd_send_data(epd, 0x00); epd_send_data(epd, 0x00);

        epd_send_command(epd, CMD_BORDER); epd_send_data(epd, 0x03);
        epd_send_command(epd, CMD_VCOM_VOLTAGE); epd_send_data(epd, 0x55);

        epd_send_command(epd, 0x03); epd_send_data(epd, lut_full_update[70]);
        epd_send_command(epd, 0x04); epd_send_data(epd, lut_full_update[71]); epd_send_data(epd, lut_full_update[72]); epd_send_data(epd, lut_full_update[73]);
        epd_send_command(epd, CMD_DUMMY_LINE); epd_send_data(epd, lut_full_update[74]);
        epd_send_command(epd, CMD_GATE_TIME); epd_send_data(epd, lut_full_update[75]);

        epd_send_command(epd, CMD_LUT_VCOM);
        for (int i = 0; i < 70; i++) epd_send_data(epd, lut_full_update[i]);

        epd_send_command(epd, CMD_SET_RAM_X_CTR); epd_send_data(epd, 0x00);
        epd_send_command(epd, CMD_SET_RAM_Y_CTR); epd_send_data(epd, 0xF9); epd_send_data(epd, 0x00);
        epd_wait_busy(epd);
    } else {
        epd_send_command(epd, CMD_VCOM_VOLTAGE); epd_send_data(epd, 0x26);
        epd_wait_busy(epd);

        epd_send_command(epd, CMD_LUT_VCOM);
        for (int i = 0; i < 70; i++) epd_send_data(epd, lut_partial_update[i]);

        epd_send_command(epd, 0x37);
        epd_send_data(epd, 0x00); epd_send_data(epd, 0x00); epd_send_data(epd, 0x00); epd_send_data(epd, 0x00);
        epd_send_data(epd, 0x40); epd_send_data(epd, 0x00); epd_send_data(epd, 0x00);

        epd_send_command(epd, CMD_DISP_UPDATE2); epd_send_data(epd, 0xC0);
        epd_send_command(epd, CMD_MASTER_ACT); epd_wait_busy(epd);

        epd_send_command(epd, CMD_BORDER); epd_send_data(epd, 0x01);
    }
    return EPD_OK;
}

void EPD_2in13_V2_Clear(EPD *epd, uint8_t color) {
    if (!epd) return;
    int sz = epd_linewidth(epd->width) * epd->height;
    uint8_t *buf = (uint8_t *)malloc(sz);
    if (!buf) return;
    memset(buf, color, sz);
    epd_send_command(epd, CMD_WRITE_RAM_BW);
    epd_send_data_buf(epd, buf, sz);
    free(buf);
    _turn_on_display(epd);
}

void EPD_2in13_V2_Display(EPD *epd, const uint8_t *image) {
    if (!epd || !image) return;
    epd_send_command(epd, CMD_WRITE_RAM_BW);
    epd_send_data_buf(epd, image, epd_linewidth(epd->width) * epd->height);
    _turn_on_display(epd);
}

void EPD_2in13_V2_DisplayPartial(EPD *epd, const uint8_t *image) {
    if (!epd || !image) return;
    int lw = epd_linewidth(epd->width);
    int sz = epd->height * lw;
    uint8_t *buf = (uint8_t *)malloc(sz);
    if (!buf) return;
    for (int i = 0; i < sz; i++) buf[i] = ~image[i];

    epd_send_command(epd, CMD_WRITE_RAM_BW);
    epd_send_data_buf(epd, image, sz);
    epd_send_command(epd, CMD_WRITE_RAM_RED);
    epd_send_data_buf(epd, buf, sz);
    free(buf);
    _turn_on_display_part(epd);
}

void EPD_2in13_V2_DisplayPartBaseImage(EPD *epd, const uint8_t *image) {
    if (!epd || !image) return;
    int sz = epd_linewidth(epd->width) * epd->height;
    epd_send_command(epd, CMD_WRITE_RAM_BW);
    epd_send_data_buf(epd, image, sz);
    epd_send_command(epd, CMD_WRITE_RAM_RED);
    epd_send_data_buf(epd, image, sz);
    _turn_on_display(epd);
}

void EPD_2in13_V2_Sleep(EPD *epd) {
    if (!epd) return;
    epd_send_command(epd, CMD_DEEP_SLEEP); epd_send_data(epd, 0x03);
    epd->delay_ms(2000);
    epd_teardown_default_hal(epd);
}
