/**
 * @file    epd2in13_V2.h
 * @brief   Waveshare 2.13" e-Paper V2 driver (122x250, B/W)
 */

#ifndef EPD2IN13_V2_H
#define EPD2IN13_V2_H
#include "epd_common.h"
#ifdef __cplusplus
extern "C" {
#endif

#define EPD2IN13_V2_WIDTH   122
#define EPD2IN13_V2_HEIGHT  250

int  EPD_2in13_V2_Init(EPD *epd, int update_mode);
void EPD_2in13_V2_Clear(EPD *epd, uint8_t color);
void EPD_2in13_V2_Display(EPD *epd, const uint8_t *image);
void EPD_2in13_V2_DisplayPartial(EPD *epd, const uint8_t *image);
void EPD_2in13_V2_DisplayPartBaseImage(EPD *epd, const uint8_t *image);
void EPD_2in13_V2_Sleep(EPD *epd);

#ifdef __cplusplus
}
#endif
#endif /* EPD2IN13_V2_H */
