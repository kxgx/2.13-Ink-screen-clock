#ifndef EPD7IN5_HD_H
#define EPD7IN5_HD_H
#include "epd_common.h"
#ifdef __cplusplus
extern "C" {
#endif
#define EPD7IN5_HD_WIDTH 880
#define EPD7IN5_HD_HEIGHT 528
int  EPD_epd7in5_HD_Init(EPD *epd);
#define EPD_EPD7IN5_HD_TYPE BW
void EPD_epd7in5_HD_Clear(EPD *epd, uint8_t color);
void EPD_epd7in5_HD_Display(EPD *epd, const uint8_t *image);
void EPD_epd7in5_HD_Sleep(EPD *epd);
#ifdef __cplusplus
}
#endif
#endif
