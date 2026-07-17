#ifndef EPD1IN02_H
#define EPD1IN02_H
#include "epd_common.h"
#ifdef __cplusplus
extern "C" {
#endif
#define EPD1IN02_WIDTH 128
#define EPD1IN02_HEIGHT 80
int  EPD_epd1in02_Init(EPD *epd);
#define EPD_EPD1IN02_TYPE BW
void EPD_epd1in02_Clear(EPD *epd, uint8_t color);
void EPD_epd1in02_Display(EPD *epd, const uint8_t *image);
void EPD_epd1in02_Sleep(EPD *epd);
#ifdef __cplusplus
}
#endif
#endif
