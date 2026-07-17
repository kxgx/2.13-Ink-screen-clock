#ifndef EPD2IN9D_H
#define EPD2IN9D_H
#include "epd_common.h"
#ifdef __cplusplus
extern "C" {
#endif
#define EPD2IN9D_WIDTH 128
#define EPD2IN9D_HEIGHT 296
int  EPD_2in9d_Init(EPD *epd);
void EPD_2in9d_Clear(EPD *epd);
void EPD_2in9d_Display(EPD *epd, const uint8_t *image);
void EPD_2in9d_Sleep(EPD *epd);
#ifdef __cplusplus
}
#endif
#endif
