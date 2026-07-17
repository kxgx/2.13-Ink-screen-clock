#ifndef EPD7IN5_H
#define EPD7IN5_H
#include "epd_common.h"
#ifdef __cplusplus
extern "C" {
#endif
#define EPD7IN5_WIDTH 640
#define EPD7IN5_HEIGHT 384
int  EPD_epd7in5_Init(EPD *epd);
#define EPD_EPD7IN5_TYPE BW
void EPD_epd7in5_Clear(EPD *epd, uint8_t color);
void EPD_epd7in5_Display(EPD *epd, const uint8_t *image);
void EPD_epd7in5_Sleep(EPD *epd);
#ifdef __cplusplus
}
#endif
#endif
