#ifndef EPD3IN0G_H
#define EPD3IN0G_H
#include "epd_common.h"
#ifdef __cplusplus
extern "C" {
#endif
#define EPD3IN0G_WIDTH 168
#define EPD3IN0G_HEIGHT 400
int  EPD_epd3in0g_Init(EPD *epd);
#define EPD_EPD3IN0G_TYPE BW
void EPD_epd3in0g_Clear(EPD *epd, uint8_t color);
void EPD_epd3in0g_Display(EPD *epd, const uint8_t *image);
void EPD_epd3in0g_Sleep(EPD *epd);
#ifdef __cplusplus
}
#endif
#endif
