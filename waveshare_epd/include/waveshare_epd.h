/**
 * @file    waveshare_epd.h
 * @brief   Waveshare e-Paper C Library - Master header
 *
 * Include this single header to access all e-Paper display drivers
 * and the HAL abstraction layer.
 *
 * Usage:
 *   #include "waveshare_epd.h"
 *
 *   EPD epd;
 *   EPD_2in13_V4_Init(&epd);
 *   EPD_2in13_V4_Clear(&epd, EPD_WHITE);
 *   // ... draw image buffer ...
 *   EPD_2in13_V4_Display(&epd, image_buffer);
 *   EPD_2in13_V4_Sleep(&epd);
 *
 * Supported displays: 60+ Waveshare e-Paper models
 * Supported platforms: Raspberry Pi (Linux spidev + sysfs GPIO)
 */

#ifndef WAVESHARE_EPD_H
#define WAVESHARE_EPD_H

#include "epd_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================
 * All display drivers
 * ======================================================================== */

/* 1.02 inch */
#include "../src/drivers/epd1in02.h"

/* 1.54 inch */
#include "../src/drivers/epd1in54.h"
#include "../src/drivers/epd1in54_V2.h"
#include "../src/drivers/epd1in54b.h"
#include "../src/drivers/epd1in54b_V2.h"
#include "../src/drivers/epd1in54c.h"

/* 1.64 inch grayscale */
#include "../src/drivers/epd1in64g.h"

/* 2.13 inch */
#include "../src/drivers/epd2in13_V2.h"
#include "../src/drivers/epd2in13_V3.h"
#include "../src/drivers/epd2in13_V4.h"
#include "../src/drivers/epd2in13b_V3.h"
#include "../src/drivers/epd2in13b_V4.h"
#include "../src/drivers/epd2in13bc.h"
#include "../src/drivers/epd2in13d.h"
#include "../src/drivers/epd2in13g.h"

/* 2.15 inch */
#include "../src/drivers/epd2in15b.h"
#include "../src/drivers/epd2in15g.h"

/* 2.36 inch grayscale */
#include "../src/drivers/epd2in36g.h"

/* 2.66 inch */
#include "../src/drivers/epd2in66.h"
#include "../src/drivers/epd2in66b.h"
#include "../src/drivers/epd2in66g.h"

/* 2.7 inch */
#include "../src/drivers/epd2in7.h"
#include "../src/drivers/epd2in7_V2.h"
#include "../src/drivers/epd2in7b.h"
#include "../src/drivers/epd2in7b_V2.h"

/* 2.9 inch */
#include "../src/drivers/epd2in9.h"
#include "../src/drivers/epd2in9_V2.h"
#include "../src/drivers/epd2in9b_V3.h"
#include "../src/drivers/epd2in9b_V4.h"
#include "../src/drivers/epd2in9bc.h"
#include "../src/drivers/epd2in9d.h"

/* 3.0 inch grayscale */
#include "../src/drivers/epd3in0g.h"

/* 3.52 inch */
#include "../src/drivers/epd3in52.h"

/* 3.7 inch */
#include "../src/drivers/epd3in7.h"

/* 4.01 inch 7-color */
#include "../src/drivers/epd4in01f.h"

/* 4.2 inch */
#include "../src/drivers/epd4in2.h"
#include "../src/drivers/epd4in2_V2.h"
#include "../src/drivers/epd4in2b_V2.h"
#include "../src/drivers/epd4in2bc.h"

/* 4.26 inch */
#include "../src/drivers/epd4in26.h"

/* 4.37 inch grayscale */
#include "../src/drivers/epd4in37g.h"

/* 5.65 inch 7-color */
#include "../src/drivers/epd5in65f.h"

/* 5.79 inch */
#include "../src/drivers/epd5in79.h"
#include "../src/drivers/epd5in79b.h"
#include "../src/drivers/epd5in79g.h"

/* 5.83 inch */
#include "../src/drivers/epd5in83.h"
#include "../src/drivers/epd5in83_V2.h"
#include "../src/drivers/epd5in83b_V2.h"
#include "../src/drivers/epd5in83bc.h"

/* 7.3 inch */
#include "../src/drivers/epd7in3e.h"
#include "../src/drivers/epd7in3f.h"
#include "../src/drivers/epd7in3g.h"

/* 7.5 inch */
#include "../src/drivers/epd7in5.h"
#include "../src/drivers/epd7in5_HD.h"
#include "../src/drivers/epd7in5_V2.h"
#include "../src/drivers/epd7in5b_HD.h"
#include "../src/drivers/epd7in5b_V2.h"
#include "../src/drivers/epd7in5bc.h"

/* 13.3 inch */
#include "../src/drivers/epd13in3b.h"
#include "../src/drivers/epd13in3k.h"

#ifdef __cplusplus
}
#endif

#endif /* WAVESHARE_EPD_H */
