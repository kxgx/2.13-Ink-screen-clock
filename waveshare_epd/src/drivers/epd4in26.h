#ifndef EPD4IN26_H
#define EPD4IN26_H
#include "epd_common.h"
#ifdef __cplusplus
extern "C" {
#endif
#define EPD4IN26_WIDTH 800
#define EPD4IN26_HEIGHT 480
int  EPD_epd4in26_Init(EPD *epd);
#define EPD_EPD4IN26_TYPE BW
void EPD_epd4in26_Clear(EPD *epd, uint8_t color);
void EPD_epd4in26_Display(EPD *epd, const uint8_t *image);
void EPD_epd4in26_Sleep(EPD *epd);
#ifdef __cplusplus
}
#endif
#endif
