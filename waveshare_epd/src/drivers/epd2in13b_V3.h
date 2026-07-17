#ifndef EPD2IN13B_V3_H
#define EPD2IN13B_V3_H
#include "epd_common.h"
#ifdef __cplusplus
extern "C" {
#endif
#define EPD2IN13B_V3_WIDTH 104
#define EPD2IN13B_V3_HEIGHT 212
int  EPD_2in13b_V3_Init(EPD *epd);
void EPD_2in13b_V3_Clear(EPD *epd);
void EPD_2in13b_V3_Display(EPD *epd, const uint8_t *black, const uint8_t *red);
void EPD_2in13b_V3_Sleep(EPD *epd);
#ifdef __cplusplus
}
#endif
#endif
