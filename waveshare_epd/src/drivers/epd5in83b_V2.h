#ifndef EPD5IN83B_V2_H
#define EPD5IN83B_V2_H
#include "epd_common.h"
#ifdef __cplusplus
extern "C" {
#endif
#define EPD5IN83B_V2_WIDTH 648
#define EPD5IN83B_V2_HEIGHT 480
int  EPD_epd5in83b_V2_Init(EPD *epd);
#define EPD_EPD5IN83B_V2_TYPE BWR
void EPD_epd5in83b_V2_Clear(EPD *epd);
void EPD_epd5in83b_V2_Display(EPD *epd, const uint8_t *black, const uint8_t *red);
void EPD_epd5in83b_V2_Sleep(EPD *epd);
#ifdef __cplusplus
}
#endif
#endif
