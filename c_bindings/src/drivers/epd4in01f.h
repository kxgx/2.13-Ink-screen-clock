#ifndef EPD4IN01F_H
#define EPD4IN01F_H
#include "epd_common.h"
#ifdef __cplusplus
extern "C" {
#endif
#define EPD4IN01F_WIDTH 640
#define EPD4IN01F_HEIGHT 400
int  EPD_epd4in01f_Init(EPD *epd);
#define EPD_EPD4IN01F_TYPE BWR
void EPD_epd4in01f_Clear(EPD *epd);
void EPD_epd4in01f_Display(EPD *epd, const uint8_t *black, const uint8_t *red);
void EPD_epd4in01f_Sleep(EPD *epd);
#ifdef __cplusplus
}
#endif
#endif
