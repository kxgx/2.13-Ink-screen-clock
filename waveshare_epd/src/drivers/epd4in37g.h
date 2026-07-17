#ifndef EPD4IN37G_H
#define EPD4IN37G_H
#include "epd_common.h"
#ifdef __cplusplus
extern "C" {
#endif
#define EPD4IN37G_WIDTH 512
#define EPD4IN37G_HEIGHT 368
int  EPD_epd4in37g_Init(EPD *epd);
#define EPD_EPD4IN37G_TYPE BW
void EPD_epd4in37g_Clear(EPD *epd, uint8_t color);
void EPD_epd4in37g_Display(EPD *epd, const uint8_t *image);
void EPD_epd4in37g_Sleep(EPD *epd);
#ifdef __cplusplus
}
#endif
#endif
