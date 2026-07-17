#ifndef EPD7IN5BC_H
#define EPD7IN5BC_H
#include "epd_common.h"
#ifdef __cplusplus
extern "C" {
#endif
#define EPD7IN5BC_WIDTH 640
#define EPD7IN5BC_HEIGHT 384
int  EPD_epd7in5bc_Init(EPD *epd);
#define EPD_EPD7IN5BC_TYPE BWR
void EPD_epd7in5bc_Clear(EPD *epd);
void EPD_epd7in5bc_Display(EPD *epd, const uint8_t *black, const uint8_t *red);
void EPD_epd7in5bc_Sleep(EPD *epd);
#ifdef __cplusplus
}
#endif
#endif
