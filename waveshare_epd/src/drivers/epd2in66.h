#ifndef EPD2IN66_H
#define EPD2IN66_H
#include "epd_common.h"
#ifdef __cplusplus
extern "C" {
#endif
#define EPD2IN66_WIDTH 152
#define EPD2IN66_HEIGHT 296
int  EPD_epd2in66_Init(EPD *epd);
#define EPD_EPD2IN66_TYPE BW
void EPD_epd2in66_Clear(EPD *epd, uint8_t color);
void EPD_epd2in66_Display(EPD *epd, const uint8_t *image);
void EPD_epd2in66_Sleep(EPD *epd);
#ifdef __cplusplus
}
#endif
#endif
