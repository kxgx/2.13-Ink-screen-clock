#ifndef EPD2IN9B_V4_H
#define EPD2IN9B_V4_H
#include "epd_common.h"
#ifdef __cplusplus
extern "C" {
#endif
#define EPD2IN9B_V4_WIDTH 128
#define EPD2IN9B_V4_HEIGHT 296
int  EPD_2in9b_V4_Init(EPD *epd);
void EPD_2in9b_V4_Clear(EPD *epd);
void EPD_2in9b_V4_Display(EPD *epd, const uint8_t *black, const uint8_t *red);
void EPD_2in9b_V4_Sleep(EPD *epd);
#ifdef __cplusplus
}
#endif
#endif
