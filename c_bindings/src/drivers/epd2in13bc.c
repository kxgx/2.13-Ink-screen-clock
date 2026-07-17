#include "epd2in13bc.h"
#include <stdlib.h>
#include <string.h>

static void _busy(EPD *epd){ while(epd->digital_read(epd->busy_pin)==0)epd->delay_ms(100); }

int EPD_2in13bc_Init(EPD *epd) {
    if(!epd)return EPD_ERROR;
    epd->width=EPD2IN13BC_WIDTH; epd->height=EPD2IN13BC_HEIGHT;
    epd->rst_pin=EPD_RST_PIN; epd->dc_pin=EPD_DC_PIN; epd->cs_pin=EPD_CS_PIN; epd->busy_pin=EPD_BUSY_PIN; epd->pwr_pin=EPD_PWR_PIN;
    if(epd_setup_default_hal(epd)!=EPD_OK)return EPD_ERROR;
    epd_reset(epd);
    epd_send_command(epd,0x06); epd_send_data(epd,0x17); epd_send_data(epd,0x17); epd_send_data(epd,0x17);
    epd_send_command(epd,0x04); _busy(epd);
    epd_send_command(epd,0x00); epd_send_data(epd,0x8F);
    epd_send_command(epd,0x50); epd_send_data(epd,0xF0);
    epd_send_command(epd,0x61); epd_send_data(epd,epd->width&0xFF); epd_send_data(epd,epd->height>>8); epd_send_data(epd,epd->height&0xFF);
    return EPD_OK;
}

void EPD_2in13bc_Clear(EPD *epd) {
    if(!epd){return;}int sz=epd->width*epd->height/8;
    epd_send_command(epd,0x10); for(int i=0;i<sz;i++)epd_send_data(epd,0xFF);
    epd_send_command(epd,0x13); for(int i=0;i<sz;i++)epd_send_data(epd,0xFF);
    epd_send_command(epd,0x12); _busy(epd);
}

void EPD_2in13bc_Display(EPD *epd, const uint8_t *black, const uint8_t *red) {
    if(!epd||!black||!red){return;}int sz=epd->width*epd->height/8;
    epd_send_command(epd,0x10); for(int i=0;i<sz;i++)epd_send_data(epd,black[i]);
    epd_send_command(epd,0x13); for(int i=0;i<sz;i++)epd_send_data(epd,red[i]);
    epd_send_command(epd,0x12); _busy(epd);
}

void EPD_2in13bc_Sleep(EPD *epd) {
    if(!epd)return;
    epd_send_command(epd,0x02); _busy(epd);
    epd_send_command(epd,0x07); epd_send_data(epd,0xA5);
    epd->delay_ms(2000); epd_teardown_default_hal(epd);
}
