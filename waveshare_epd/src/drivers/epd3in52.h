#ifndef EPD3IN52_H
#define EPD3IN52_H
#include "epd_common.h"
#ifdef __cplusplus
extern "C" {
#endif
#define EPD3IN52_WIDTH 240
#define EPD3IN52_HEIGHT 360
int  EPD_epd3in52_Init(EPD *epd);
#define EPD_EPD3IN52_TYPE BW
void EPD_epd3in52_Clear(EPD *epd, uint8_t color);
void EPD_epd3in52_Display(EPD *epd, const uint8_t *image);
void EPD_epd3in52_Sleep(EPD *epd);
#ifdef __cplusplus
}
#endif
#endif
