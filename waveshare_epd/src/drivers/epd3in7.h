#ifndef EPD3IN7_H
#define EPD3IN7_H
#include "epd_common.h"
#ifdef __cplusplus
extern "C" {
#endif
#define EPD3IN7_WIDTH 280
#define EPD3IN7_HEIGHT 480
int  EPD_epd3in7_Init(EPD *epd);
#define EPD_EPD3IN7_TYPE BW
void EPD_epd3in7_Clear(EPD *epd, uint8_t color);
void EPD_epd3in7_Display(EPD *epd, const uint8_t *image);
void EPD_epd3in7_Sleep(EPD *epd);
#ifdef __cplusplus
}
#endif
#endif
