#ifndef EPD1IN54C_H
#define EPD1IN54C_H
#include "epd_common.h"
#ifdef __cplusplus
extern "C" {
#endif
#define EPD1IN54C_WIDTH 200
#define EPD1IN54C_HEIGHT 200
int  EPD_epd1in54c_Init(EPD *epd);
#define EPD_EPD1IN54C_TYPE BWR
void EPD_epd1in54c_Clear(EPD *epd);
void EPD_epd1in54c_Display(EPD *epd, const uint8_t *black, const uint8_t *red);
void EPD_epd1in54c_Sleep(EPD *epd);
#ifdef __cplusplus
}
#endif
#endif
