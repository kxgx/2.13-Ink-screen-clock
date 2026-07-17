#ifndef EPD2IN7_H
#define EPD2IN7_H
#include "epd_common.h"
#ifdef __cplusplus
extern "C" {
#endif
#define EPD2IN7_WIDTH 176
#define EPD2IN7_HEIGHT 264
int  EPD_2in7_Init(EPD *epd);
void EPD_2in7_Clear(EPD *epd, uint8_t color);
void EPD_2in7_Display(EPD *epd, const uint8_t *image);
void EPD_2in7_Sleep(EPD *epd);
#ifdef __cplusplus
}
#endif
#endif
