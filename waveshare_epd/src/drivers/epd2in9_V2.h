#ifndef EPD2IN9_V2_H
#define EPD2IN9_V2_H
#include "epd_common.h"
#ifdef __cplusplus
extern "C" {
#endif
#define EPD2IN9_V2_WIDTH 128
#define EPD2IN9_V2_HEIGHT 296
int  EPD_2in9_V2_Init(EPD *epd);
void EPD_2in9_V2_Clear(EPD *epd, uint8_t color);
void EPD_2in9_V2_Display(EPD *epd, const uint8_t *image);
void EPD_2in9_V2_Sleep(EPD *epd);
#ifdef __cplusplus
}
#endif
#endif
