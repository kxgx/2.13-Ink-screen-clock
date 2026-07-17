#ifndef EPD2IN15G_H
#define EPD2IN15G_H
#include "epd_common.h"
#ifdef __cplusplus
extern "C" {
#endif
#define EPD2IN15G_WIDTH 128
#define EPD2IN15G_HEIGHT 128
int  EPD_epd2in15g_Init(EPD *epd);
#define EPD_EPD2IN15G_TYPE BW
void EPD_epd2in15g_Clear(EPD *epd, uint8_t color);
void EPD_epd2in15g_Display(EPD *epd, const uint8_t *image);
void EPD_epd2in15g_Sleep(EPD *epd);
#ifdef __cplusplus
}
#endif
#endif
