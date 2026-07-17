#include "epd2in13g.h"
#include <stdlib.h>
#include <string.h>

static void _busy(EPD *epd){ epd->delay_ms(100); while(epd->digital_read(epd->busy_pin)==0)epd->delay_ms(5); }

int EPD_2in13g_Init(EPD *epd) {
    if(!epd)return EPD_ERROR;
    epd->width=EPD2IN13G_WIDTH; epd->height=EPD2IN13G_HEIGHT;
    epd->rst_pin=EPD_RST_PIN; epd->dc_pin=EPD_DC_PIN; epd->cs_pin=EPD_CS_PIN; epd->busy_pin=EPD_BUSY_PIN; epd->pwr_pin=EPD_PWR_PIN;
    if(epd_setup_default_hal(epd)!=EPD_OK)return EPD_ERROR;
    epd_reset(epd); _busy(epd);
    epd_send_command(epd,0x4D); epd_send_data(epd,0x78);
    epd_send_command(epd,0x00); epd_send_data(epd,0x0F); epd_send_data(epd,0x29);
    epd_send_command(epd,0x01); epd_send_data(epd,0x07); epd_send_data(epd,0x00);
    epd_send_command(epd,0x03); epd_send_data(epd,0x10); epd_send_data(epd,0x54); epd_send_data(epd,0x44);
    epd_send_command(epd,0x06); epd_send_data(epd,0x05); epd_send_data(epd,0x00); epd_send_data(epd,0x3F); epd_send_data(epd,0x0A); epd_send_data(epd,0x25); epd_send_data(epd,0x12); epd_send_data(epd,0x1A);
    epd_send_command(epd,0x50); epd_send_data(epd,0x37);
    epd_send_command(epd,0x60); epd_send_data(epd,0x02); epd_send_data(epd,0x02);
    epd_send_command(epd,0x61); epd_send_data(epd,0); epd_send_data(epd,128); epd_send_data(epd,0); epd_send_data(epd,250);
    epd_send_command(epd,0xE7); epd_send_data(epd,0x1C);
    epd_send_command(epd,0xE3); epd_send_data(epd,0x22);
    epd_send_command(epd,0xB4); epd_send_data(epd,0xD0);
    epd_send_command(epd,0xB5); epd_send_data(epd,0x03);
    epd_send_command(epd,0xE9); epd_send_data(epd,0x01);
    epd_send_command(epd,0x30); epd_send_data(epd,0x08);
    epd_send_command(epd,0x04); _busy(epd);
    return EPD_OK;
}

void EPD_2in13g_Clear(EPD *epd, uint8_t color) {
    if(!epd)return; int w=128/4, h=epd->height;
    epd_send_command(epd,0x10);
    for(int j=0;j<h;j++) for(int i=0;i<w;i++) epd_send_data(epd,color);
    epd_send_command(epd,0x12); epd_send_data(epd,0x00); _busy(epd);
}

void EPD_2in13g_Display(EPD *epd, const uint8_t *image) {
    if(!epd||!image)return; int w=128/4, h=epd->height, srcW=epd->width/4;
    epd_send_command(epd,0x10);
    for(int j=0;j<h;j++){ for(int i=0;i<w;i++){ if(i<31)epd_send_data(epd,image[i+j*srcW]); else epd_send_data(epd,0x00); }}
    epd_send_command(epd,0x12); epd_send_data(epd,0x00); _busy(epd);
}

void EPD_2in13g_Sleep(EPD *epd) {
    if(!epd)return;
    epd_send_command(epd,0x02); _busy(epd); epd->delay_ms(100);
    epd_send_command(epd,0x07); epd_send_data(epd,0xA5);
    epd->delay_ms(2000); epd_teardown_default_hal(epd);
}
