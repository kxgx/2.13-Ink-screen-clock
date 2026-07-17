#ifndef EPD7IN5B_HD_H
#define EPD7IN5B_HD_H
#include "epd_common.h"
#ifdef __cplusplus
extern "C" {
#endif
#define EPD7IN5B_HD_WIDTH 880
#define EPD7IN5B_HD_HEIGHT 528
int  EPD_epd7in5b_HD_Init(EPD *epd);
#define EPD_EPD7IN5B_HD_TYPE BWR
void EPD_epd7in5b_HD_Clear(EPD *epd);
void EPD_epd7in5b_HD_Display(EPD *epd, const uint8_t *black, const uint8_t *red);
void EPD_epd7in5b_HD_Sleep(EPD *epd);
#ifdef __cplusplus
}
#endif
#endif
