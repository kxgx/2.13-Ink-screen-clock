#ifndef EPD5IN79B_H
#define EPD5IN79B_H
#include "epd_common.h"
#ifdef __cplusplus
extern "C" {
#endif
#define EPD5IN79B_WIDTH 792
#define EPD5IN79B_HEIGHT 272
int  EPD_epd5in79b_Init(EPD *epd);
#define EPD_EPD5IN79B_TYPE BWR
void EPD_epd5in79b_Clear(EPD *epd);
void EPD_epd5in79b_Display(EPD *epd, const uint8_t *black, const uint8_t *red);
void EPD_epd5in79b_Sleep(EPD *epd);
#ifdef __cplusplus
}
#endif
#endif
