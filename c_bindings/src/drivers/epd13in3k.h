#ifndef EPD13IN3K_H
#define EPD13IN3K_H
#include "epd_common.h"
#ifdef __cplusplus
extern "C" {
#endif
#define EPD13IN3K_WIDTH 960
#define EPD13IN3K_HEIGHT 680
int  EPD_epd13in3k_Init(EPD *epd);
#define EPD_EPD13IN3K_TYPE BWR
void EPD_epd13in3k_Clear(EPD *epd);
void EPD_epd13in3k_Display(EPD *epd, const uint8_t *black, const uint8_t *red);
void EPD_epd13in3k_Sleep(EPD *epd);
#ifdef __cplusplus
}
#endif
#endif
