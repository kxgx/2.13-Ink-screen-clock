#ifndef EPD1IN64G_H
#define EPD1IN64G_H
#include "epd_common.h"
#ifdef __cplusplus
extern "C" {
#endif
#define EPD1IN64G_WIDTH 168
#define EPD1IN64G_HEIGHT 168
int  EPD_epd1in64g_Init(EPD *epd);
#define EPD_EPD1IN64G_TYPE BW
void EPD_epd1in64g_Clear(EPD *epd, uint8_t color);
void EPD_epd1in64g_Display(EPD *epd, const uint8_t *image);
void EPD_epd1in64g_Sleep(EPD *epd);
#ifdef __cplusplus
}
#endif
#endif
