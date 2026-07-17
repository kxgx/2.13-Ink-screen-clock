#ifndef EPD7IN5B_V2_H
#define EPD7IN5B_V2_H
#include "epd_common.h"
#ifdef __cplusplus
extern "C" {
#endif
#define EPD7IN5B_V2_WIDTH 800
#define EPD7IN5B_V2_HEIGHT 480
int  EPD_epd7in5b_V2_Init(EPD *epd);
#define EPD_EPD7IN5B_V2_TYPE BWR
void EPD_epd7in5b_V2_Clear(EPD *epd);
void EPD_epd7in5b_V2_Display(EPD *epd, const uint8_t *black, const uint8_t *red);
void EPD_epd7in5b_V2_Sleep(EPD *epd);
#ifdef __cplusplus
}
#endif
#endif
