#ifndef EPD2IN66G_H
#define EPD2IN66G_H
#include "epd_common.h"
#ifdef __cplusplus
extern "C" {
#endif
#define EPD2IN66G_WIDTH 152
#define EPD2IN66G_HEIGHT 296
int  EPD_epd2in66g_Init(EPD *epd);
#define EPD_EPD2IN66G_TYPE BW
void EPD_epd2in66g_Clear(EPD *epd, uint8_t color);
void EPD_epd2in66g_Display(EPD *epd, const uint8_t *image);
void EPD_epd2in66g_Sleep(EPD *epd);
#ifdef __cplusplus
}
#endif
#endif
