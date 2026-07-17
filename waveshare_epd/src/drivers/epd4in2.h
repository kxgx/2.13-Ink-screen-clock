#ifndef EPD4IN2_H
#define EPD4IN2_H
#include "epd_common.h"
#ifdef __cplusplus
extern "C" {
#endif
#define EPD4IN2_WIDTH 400
#define EPD4IN2_HEIGHT 300
int  EPD_epd4in2_Init(EPD *epd);
#define EPD_EPD4IN2_TYPE BW
void EPD_epd4in2_Clear(EPD *epd, uint8_t color);
void EPD_epd4in2_Display(EPD *epd, const uint8_t *image);
void EPD_epd4in2_Sleep(EPD *epd);
#ifdef __cplusplus
}
#endif
#endif
