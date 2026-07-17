#ifndef EPD4IN2B_V2_H
#define EPD4IN2B_V2_H
#include "epd_common.h"
#ifdef __cplusplus
extern "C" {
#endif
#define EPD4IN2B_V2_WIDTH 400
#define EPD4IN2B_V2_HEIGHT 300
int  EPD_epd4in2b_V2_Init(EPD *epd);
#define EPD_EPD4IN2B_V2_TYPE BWR
void EPD_epd4in2b_V2_Clear(EPD *epd);
void EPD_epd4in2b_V2_Display(EPD *epd, const uint8_t *black, const uint8_t *red);
void EPD_epd4in2b_V2_Sleep(EPD *epd);
#ifdef __cplusplus
}
#endif
#endif
