#ifndef EPD2IN13G_H
#define EPD2IN13G_H
#include "epd_common.h"
#ifdef __cplusplus
extern "C" {
#endif
#define EPD2IN13G_WIDTH 122
#define EPD2IN13G_HEIGHT 250
int  EPD_2in13g_Init(EPD *epd);
void EPD_2in13g_Clear(EPD *epd, uint8_t color);
void EPD_2in13g_Display(EPD *epd, const uint8_t *image);
void EPD_2in13g_Sleep(EPD *epd);
#ifdef __cplusplus
}
#endif
#endif
