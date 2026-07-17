/**
 * @file    epd2in13d.h
 * @brief   Waveshare 2.13" D e-Paper driver (104x212, B/W, IL3820)
 */

#ifndef EPD2IN13D_H
#define EPD2IN13D_H
#include "epd_common.h"
#ifdef __cplusplus
extern "C" {
#endif

#define EPD2IN13D_WIDTH   104
#define EPD2IN13D_HEIGHT  212

int  EPD_2in13d_Init(EPD *epd);
void EPD_2in13d_Clear(EPD *epd);
void EPD_2in13d_Display(EPD *epd, const uint8_t *image);
void EPD_2in13d_DisplayPartial(EPD *epd, const uint8_t *image);
void EPD_2in13d_Sleep(EPD *epd);

#ifdef __cplusplus
}
#endif
#endif /* EPD2IN13D_H */
