#ifndef EPD1IN54B_H
#define EPD1IN54B_H
#include "epd_common.h"
#ifdef __cplusplus
extern "C" {
#endif
#define EPD1IN54B_WIDTH 200
#define EPD1IN54B_HEIGHT 200
int  EPD_epd1in54b_Init(EPD *epd);
#define EPD_EPD1IN54B_TYPE BWR
void EPD_epd1in54b_Clear(EPD *epd);
void EPD_epd1in54b_Display(EPD *epd, const uint8_t *black, const uint8_t *red);
void EPD_epd1in54b_Sleep(EPD *epd);
#ifdef __cplusplus
}
#endif
#endif
