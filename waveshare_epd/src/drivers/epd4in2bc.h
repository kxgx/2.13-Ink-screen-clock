#ifndef EPD4IN2BC_H
#define EPD4IN2BC_H
#include "epd_common.h"
#ifdef __cplusplus
extern "C" {
#endif
#define EPD4IN2BC_WIDTH 400
#define EPD4IN2BC_HEIGHT 300
int  EPD_epd4in2bc_Init(EPD *epd);
#define EPD_EPD4IN2BC_TYPE BWR
void EPD_epd4in2bc_Clear(EPD *epd);
void EPD_epd4in2bc_Display(EPD *epd, const uint8_t *black, const uint8_t *red);
void EPD_epd4in2bc_Sleep(EPD *epd);
#ifdef __cplusplus
}
#endif
#endif
