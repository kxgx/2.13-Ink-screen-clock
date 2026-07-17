#ifndef EPD1IN54_H
#define EPD1IN54_H
#include "epd_common.h"
#ifdef __cplusplus
extern "C" {
#endif
#define EPD1IN54_WIDTH 200
#define EPD1IN54_HEIGHT 200
int  EPD_epd1in54_Init(EPD *epd);
#define EPD_EPD1IN54_TYPE BW
void EPD_epd1in54_Clear(EPD *epd, uint8_t color);
void EPD_epd1in54_Display(EPD *epd, const uint8_t *image);
void EPD_epd1in54_Sleep(EPD *epd);
#ifdef __cplusplus
}
#endif
#endif
