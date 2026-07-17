#ifndef EPD2IN15B_H
#define EPD2IN15B_H
#include "epd_common.h"
#ifdef __cplusplus
extern "C" {
#endif
#define EPD2IN15B_WIDTH 128
#define EPD2IN15B_HEIGHT 128
int  EPD_epd2in15b_Init(EPD *epd);
#define EPD_EPD2IN15B_TYPE BWR
void EPD_epd2in15b_Clear(EPD *epd);
void EPD_epd2in15b_Display(EPD *epd, const uint8_t *black, const uint8_t *red);
void EPD_epd2in15b_Sleep(EPD *epd);
#ifdef __cplusplus
}
#endif
#endif
