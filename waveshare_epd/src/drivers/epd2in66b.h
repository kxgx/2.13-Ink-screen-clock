#ifndef EPD2IN66B_H
#define EPD2IN66B_H
#include "epd_common.h"
#ifdef __cplusplus
extern "C" {
#endif
#define EPD2IN66B_WIDTH 152
#define EPD2IN66B_HEIGHT 296
int  EPD_epd2in66b_Init(EPD *epd);
#define EPD_EPD2IN66B_TYPE BWR
void EPD_epd2in66b_Clear(EPD *epd);
void EPD_epd2in66b_Display(EPD *epd, const uint8_t *black, const uint8_t *red);
void EPD_epd2in66b_Sleep(EPD *epd);
#ifdef __cplusplus
}
#endif
#endif
