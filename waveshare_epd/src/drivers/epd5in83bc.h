#ifndef EPD5IN83BC_H
#define EPD5IN83BC_H
#include "epd_common.h"
#ifdef __cplusplus
extern "C" {
#endif
#define EPD5IN83BC_WIDTH 600
#define EPD5IN83BC_HEIGHT 448
int  EPD_epd5in83bc_Init(EPD *epd);
#define EPD_EPD5IN83BC_TYPE BWR
void EPD_epd5in83bc_Clear(EPD *epd);
void EPD_epd5in83bc_Display(EPD *epd, const uint8_t *black, const uint8_t *red);
void EPD_epd5in83bc_Sleep(EPD *epd);
#ifdef __cplusplus
}
#endif
#endif
