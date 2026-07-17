#ifndef EPD5IN79_H
#define EPD5IN79_H
#include "epd_common.h"
#ifdef __cplusplus
extern "C" {
#endif
#define EPD5IN79_WIDTH 792
#define EPD5IN79_HEIGHT 272
int  EPD_epd5in79_Init(EPD *epd);
#define EPD_EPD5IN79_TYPE BW
void EPD_epd5in79_Clear(EPD *epd, uint8_t color);
void EPD_epd5in79_Display(EPD *epd, const uint8_t *image);
void EPD_epd5in79_Sleep(EPD *epd);
#ifdef __cplusplus
}
#endif
#endif
