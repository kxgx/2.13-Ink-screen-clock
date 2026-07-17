#ifndef EPD5IN79G_H
#define EPD5IN79G_H
#include "epd_common.h"
#ifdef __cplusplus
extern "C" {
#endif
#define EPD5IN79G_WIDTH 792
#define EPD5IN79G_HEIGHT 272
int  EPD_epd5in79g_Init(EPD *epd);
#define EPD_EPD5IN79G_TYPE BW
void EPD_epd5in79g_Clear(EPD *epd, uint8_t color);
void EPD_epd5in79g_Display(EPD *epd, const uint8_t *image);
void EPD_epd5in79g_Sleep(EPD *epd);
#ifdef __cplusplus
}
#endif
#endif
