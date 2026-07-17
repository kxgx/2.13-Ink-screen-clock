#ifndef EPD2IN9BC_H
#define EPD2IN9BC_H
#include "epd_common.h"
#ifdef __cplusplus
extern "C" {
#endif
#define EPD2IN9BC_WIDTH 128
#define EPD2IN9BC_HEIGHT 296
int  EPD_2in9bc_Init(EPD *epd);
void EPD_2in9bc_Clear(EPD *epd);
void EPD_2in9bc_Display(EPD *epd, const uint8_t *black, const uint8_t *red);
void EPD_2in9bc_Sleep(EPD *epd);
#ifdef __cplusplus
}
#endif
#endif
