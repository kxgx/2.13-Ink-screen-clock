#include "epd2in13b_V3.h"
#include <stdlib.h>
#include <string.h>

int EPD_2in13b_V3_Init(EPD *epd) {
    if (!epd) return EPD_ERROR;
    epd->width=EPD2IN13B_V3_WIDTH; epd->height=EPD2IN13B_V3_HEIGHT;
    epd->rst_pin=EPD_RST_PIN; epd->dc_pin=EPD_DC_PIN; epd->cs_pin=EPD_CS_PIN; epd->busy_pin=EPD_BUSY_PIN; epd->pwr_pin=EPD_PWR_PIN;
    if (epd_setup_default_hal(epd)!=EPD_OK) return EPD_ERROR;
    epd_reset(epd);
    epd_send_command(epd,0x04); while(epd->digital_read(epd->busy_pin)==0){epd_send_command(epd,0x71); epd->delay_ms(100);}
    epd_send_command(epd,0x00); epd_send_data(epd,0x0F); epd_send_data(epd,0x89);
    epd_send_command(epd,0x61); epd_send_data(epd,0x68); epd_send_data(epd,0x00); epd_send_data(epd,0xD4);
    epd_send_command(epd,0x50); epd_send_data(epd,0x77);
    return EPD_OK;
}

void EPD_2in13b_V3_Clear(EPD *epd) {
    if (!epd) return;
    int sz=epd->width*epd->height/8;
    epd_send_command(epd,0x10); for(int i=0;i<sz;i++)epd_send_data(epd,0xFF);
    epd_send_command(epd,0x13); for(int i=0;i<sz;i++)epd_send_data(epd,0xFF);
    epd_send_command(epd,0x12); epd->delay_ms(100);
    while(epd->digital_read(epd->busy_pin)==0){epd_send_command(epd,0x71); epd->delay_ms(100);}
}

void EPD_2in13b_V3_Display(EPD *epd, const uint8_t *black, const uint8_t *red) {
    if (!epd||!black||!red) return;
    int sz=epd->width*epd->height/8;
    epd_send_command(epd,0x10); for(int i=0;i<sz;i++)epd_send_data(epd,black[i]);
    epd_send_command(epd,0x13); for(int i=0;i<sz;i++)epd_send_data(epd,red[i]);
    epd_send_command(epd,0x12); epd->delay_ms(100);
    while(epd->digital_read(epd->busy_pin)==0){epd_send_command(epd,0x71); epd->delay_ms(100);}
}

void EPD_2in13b_V3_Sleep(EPD *epd) {
    if (!epd) return;
    epd_send_command(epd,0x50); epd_send_data(epd,0xF7);
    epd_send_command(epd,0x02); while(epd->digital_read(epd->busy_pin)==0){epd_send_command(epd,0x71); epd->delay_ms(100);}
    epd_send_command(epd,0x07); epd_send_data(epd,0xA5);
    epd->delay_ms(2000); epd_teardown_default_hal(epd);
}
