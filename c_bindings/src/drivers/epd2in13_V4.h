/**
 * @file    epd2in13_V4.h
 * @brief   Waveshare 2.13" e-Paper V4 driver (122x250, B/W, SSD1680)
 */

#ifndef EPD2IN13_V4_H
#define EPD2IN13_V4_H

#include "epd_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define EPD2IN13_V4_WIDTH   122
#define EPD2IN13_V4_HEIGHT  250

/**
 * @brief Initialize the 2.13" V4 e-Paper display with default HAL.
 * @param epd  Pointer to EPD struct (will be populated).
 * @return EPD_OK on success.
 */
int EPD_2in13_V4_Init(EPD *epd);

/**
 * @brief Initialize with fast mode (built-in OTP LUT).
 * @param epd  Initialized EPD device.
 * @return EPD_OK on success.
 */
int EPD_2in13_V4_Init_Fast(EPD *epd);

/**
 * @brief Clear the display to a single color.
 * @param epd   EPD device.
 * @param color Fill color (0xFF = white, 0x00 = black).
 */
void EPD_2in13_V4_Clear(EPD *epd, uint8_t color);

/**
 * @brief Display an image buffer (full refresh).
 * @param epd   EPD device.
 * @param image Image buffer (width/8 * height bytes, 1bpp).
 */
void EPD_2in13_V4_Display(EPD *epd, const uint8_t *image);

/**
 * @brief Display with fast refresh.
 * @param epd   EPD device.
 * @param image Image buffer.
 */
void EPD_2in13_V4_Display_Fast(EPD *epd, const uint8_t *image);

/**
 * @brief Partial refresh display.
 * @param epd   EPD device.
 * @param image Image buffer.
 */
void EPD_2in13_V4_DisplayPartial(EPD *epd, const uint8_t *image);

/**
 * @brief Set a base image for subsequent partial refreshes.
 * @param epd   EPD device.
 * @param image Base image buffer.
 */
void EPD_2in13_V4_DisplayPartBaseImage(EPD *epd, const uint8_t *image);

/**
 * @brief Put the display into deep sleep.
 * @param epd  EPD device.
 */
void EPD_2in13_V4_Sleep(EPD *epd);

#ifdef __cplusplus
}
#endif

#endif /* EPD2IN13_V4_H */
