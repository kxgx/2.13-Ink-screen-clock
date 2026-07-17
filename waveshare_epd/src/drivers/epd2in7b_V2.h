#ifndef EPD2IN7B_V2_H
#define EPD2IN7B_V2_H
#include "epd_common.h"
#ifdef __cplusplus
extern "C" {
#endif
#define EPD2IN7B_V2_WIDTH 176
#define EPD2IN7B_V2_HEIGHT 264
int  EPD_2in7b_V2_Init(EPD *epd);
void EPD_2in7b_V2_Clear(EPD *epd);
void EPD_2in7b_V2_Display(EPD *epd, const uint8_t *black, const uint8_t *red);
void EPD_2in7b_V2_Sleep(EPD *epd);
#ifdef __cplusplus
}
#endif
#endif
