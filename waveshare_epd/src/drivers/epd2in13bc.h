#ifndef EPD2IN13BC_H
#define EPD2IN13BC_H
#include "epd_common.h"
#ifdef __cplusplus
extern "C" {
#endif
#define EPD2IN13BC_WIDTH 104
#define EPD2IN13BC_HEIGHT 212
int  EPD_2in13bc_Init(EPD *epd);
void EPD_2in13bc_Clear(EPD *epd);
void EPD_2in13bc_Display(EPD *epd, const uint8_t *black, const uint8_t *red);
void EPD_2in13bc_Sleep(EPD *epd);
#ifdef __cplusplus
}
#endif
#endif
