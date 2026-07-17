#ifndef EPD2IN7B_H
#define EPD2IN7B_H
#include "epd_common.h"
#ifdef __cplusplus
extern "C" {
#endif
#define EPD2IN7B_WIDTH 176
#define EPD2IN7B_HEIGHT 264
int  EPD_2in7b_Init(EPD *epd);
void EPD_2in7b_Clear(EPD *epd);
void EPD_2in7b_Display(EPD *epd, const uint8_t *black, const uint8_t *red);
void EPD_2in7b_Sleep(EPD *epd);
#ifdef __cplusplus
}
#endif
#endif
