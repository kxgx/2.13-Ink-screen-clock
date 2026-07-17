#ifndef EPD2IN36G_H
#define EPD2IN36G_H
#include "epd_common.h"
#ifdef __cplusplus
extern "C" {
#endif
#define EPD2IN36G_WIDTH 128
#define EPD2IN36G_HEIGHT 296
int  EPD_epd2in36g_Init(EPD *epd);
#define EPD_EPD2IN36G_TYPE BW
void EPD_epd2in36g_Clear(EPD *epd, uint8_t color);
void EPD_epd2in36g_Display(EPD *epd, const uint8_t *image);
void EPD_epd2in36g_Sleep(EPD *epd);
#ifdef __cplusplus
}
#endif
#endif
