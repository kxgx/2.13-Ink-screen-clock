#ifndef EPD7IN3E_H
#define EPD7IN3E_H
#include "epd_common.h"
#ifdef __cplusplus
extern "C" {
#endif
#define EPD7IN3E_WIDTH 800
#define EPD7IN3E_HEIGHT 480
int  EPD_epd7in3e_Init(EPD *epd);
#define EPD_EPD7IN3E_TYPE BW
void EPD_epd7in3e_Clear(EPD *epd, uint8_t color);
void EPD_epd7in3e_Display(EPD *epd, const uint8_t *image);
void EPD_epd7in3e_Sleep(EPD *epd);
#ifdef __cplusplus
}
#endif
#endif
