#include "epd2in13b_V4.h"
#include <stdlib.h>
#include <string.h>

int EPD_2in13b_V4_Init(EPD *epd) {
    if (!epd) return EPD_ERROR;
    epd->width=EPD2IN13B_V4_WIDTH; epd->height=EPD2IN13B_V4_HEIGHT;
    epd->rst_pin=EPD_RST_PIN; epd->dc_pin=EPD_DC_PIN; epd->cs_pin=EPD_CS_PIN; epd->busy_pin=EPD_BUSY_PIN; epd->pwr_pin=EPD_PWR_PIN;
    if (epd_setup_default_hal(epd)!=EPD_OK) return EPD_ERROR;
    epd_reset(epd);
    epd_wait_busy(epd);
    epd_send_command(epd,0x12); epd_wait_busy(epd);
    epd_send_command(epd,0x01); epd_send_data(epd,0xF9); epd_send_data(epd,0x00); epd_send_data(epd,0x00);
    epd_send_command(epd,0x11); epd_send_data(epd,0x03);
    epd_send_command(epd,0x44); epd_send_data(epd,0x00); epd_send_data(epd,0x0F);
    epd_send_command(epd,0x45); epd_send_data(epd,0x00); epd_send_data(epd,0x00); epd_send_data(epd,0xF9); epd_send_data(epd,0x00);
    epd_send_command(epd,0x4E); epd_send_data(epd,0x00);
    epd_send_command(epd,0x4F); epd_send_data(epd,0x00); epd_send_data(epd,0x00);
    epd_send_command(epd,0x3C); epd_send_data(epd,0x05);
    epd_send_command(epd,0x18); epd_send_data(epd,0x80);
    epd_send_command(epd,0x21); epd_send_data(epd,0x80); epd_send_data(epd,0x80);
    epd_wait_busy(epd);
    return EPD_OK;
}

void EPD_2in13b_V4_Clear(EPD *epd) {
    if (!epd) return;
    int lw=epd_linewidth(epd->width); int sz=lw*epd->height;
    uint8_t *buf=(uint8_t*)malloc(sz); if(!buf)return; memset(buf,0xFF,sz);
    epd_send_command(epd,0x24); epd_send_data_buf(epd,buf,sz);
    epd_send_command(epd,0x26); epd_send_data_buf(epd,buf,sz);
    free(buf);
    epd_send_command(epd,0x20); epd_wait_busy(epd);
}

void EPD_2in13b_V4_Display(EPD *epd, const uint8_t *black, const uint8_t *red) {
    if (!epd||!black||!red) return;
    int sz=epd_linewidth(epd->width)*epd->height;
    epd_send_command(epd,0x24); epd_send_data_buf(epd,black,sz);
    epd_send_command(epd,0x26); epd_send_data_buf(epd,red,sz);
    epd_send_command(epd,0x20); epd_wait_busy(epd);
}

void EPD_2in13b_V4_Sleep(EPD *epd) {
    if (!epd) return;
    epd_send_command(epd,0x10); epd_send_data(epd,0x01);
    epd->delay_ms(2000); epd_teardown_default_hal(epd);
}
