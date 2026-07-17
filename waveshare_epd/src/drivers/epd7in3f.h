#ifndef EPD7IN3F_H
#define EPD7IN3F_H
#include "epd_common.h"
#ifdef __cplusplus
extern "C" {
#endif
#define EPD7IN3F_WIDTH 800
#define EPD7IN3F_HEIGHT 480
int  EPD_epd7in3f_Init(EPD *epd);
#define EPD_EPD7IN3F_TYPE BW
void EPD_epd7in3f_Clear(EPD *epd, uint8_t color);
void EPD_epd7in3f_Display(EPD *epd, const uint8_t *image);
void EPD_epd7in3f_Sleep(EPD *epd);
#ifdef __cplusplus
}
#endif
#endif
