/**
 * @file    epd2in13_V3.h
 * @brief   Waveshare 2.13" e-Paper V3 driver (122x250, B/W, SSD1680)
 */

#ifndef EPD2IN13_V3_H
#define EPD2IN13_V3_H

#include "epd_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define EPD2IN13_V3_WIDTH   122
#define EPD2IN13_V3_HEIGHT  250

int  EPD_2in13_V3_Init(EPD *epd);
void EPD_2in13_V3_Clear(EPD *epd, uint8_t color);
void EPD_2in13_V3_Display(EPD *epd, const uint8_t *image);
void EPD_2in13_V3_DisplayPartial(EPD *epd, const uint8_t *image);
void EPD_2in13_V3_DisplayPartBaseImage(EPD *epd, const uint8_t *image);
void EPD_2in13_V3_Sleep(EPD *epd);

#ifdef __cplusplus
}
#endif
#endif /* EPD2IN13_V3_H */
