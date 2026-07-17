#ifndef EPD5IN65F_H
#define EPD5IN65F_H
#include "epd_common.h"
#ifdef __cplusplus
extern "C" {
#endif
#define EPD5IN65F_WIDTH 600
#define EPD5IN65F_HEIGHT 448
int  EPD_epd5in65f_Init(EPD *epd);
#define EPD_EPD5IN65F_TYPE BWR
void EPD_epd5in65f_Clear(EPD *epd);
void EPD_epd5in65f_Display(EPD *epd, const uint8_t *black, const uint8_t *red);
void EPD_epd5in65f_Sleep(EPD *epd);
#ifdef __cplusplus
}
#endif
#endif
