#ifndef EPD1IN54_V2_H
#define EPD1IN54_V2_H
#include "epd_common.h"
#ifdef __cplusplus
extern "C" {
#endif
#define EPD1IN54_V2_WIDTH 200
#define EPD1IN54_V2_HEIGHT 200
int  EPD_epd1in54_V2_Init(EPD *epd);
#define EPD_EPD1IN54_V2_TYPE BW
void EPD_epd1in54_V2_Clear(EPD *epd, uint8_t color);
void EPD_epd1in54_V2_Display(EPD *epd, const uint8_t *image);
void EPD_epd1in54_V2_Sleep(EPD *epd);
#ifdef __cplusplus
}
#endif
#endif
