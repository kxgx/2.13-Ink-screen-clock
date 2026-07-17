/**
 * @file    epd_common.h
 * @brief   Waveshare e-Paper C bindings - common types and definitions
 * @author  Based on Waveshare e-Paper Python drivers
 */

#ifndef EPD_COMMON_H
#define EPD_COMMON_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/*===========================================================================
 * Return codes
 *===========================================================================*/
#define EPD_OK        0
#define EPD_ERROR    -1

/*===========================================================================
 * Color constants (for monochrome displays)
 *===========================================================================*/
#define EPD_WHITE    0xFF
#define EPD_BLACK    0x00

/*===========================================================================
 * Update mode constants
 *===========================================================================*/
#define EPD_FULL_UPDATE  0
#define EPD_PART_UPDATE  1

/*===========================================================================
 * Default GPIO pin definitions (Raspberry Pi BCM numbering)
 * Can be overridden at compile time or runtime.
 *===========================================================================*/
#ifndef EPD_RST_PIN
#define EPD_RST_PIN    17
#endif

#ifndef EPD_DC_PIN
#define EPD_DC_PIN     25
#endif

#ifndef EPD_CS_PIN
#define EPD_CS_PIN     8
#endif

#ifndef EPD_BUSY_PIN
#define EPD_BUSY_PIN   24
#endif

#ifndef EPD_PWR_PIN
#define EPD_PWR_PIN    18
#endif

#ifndef EPD_SPI_BUS
#define EPD_SPI_BUS    0
#endif

#ifndef EPD_SPI_DEVICE
#define EPD_SPI_DEVICE 0
#endif

#ifndef EPD_SPI_SPEED
#define EPD_SPI_SPEED  4000000
#endif

/*===========================================================================
 * HAL function pointer types
 * These allow the HAL implementation to be swapped at runtime.
 *===========================================================================*/

/** Write a value (0 or 1) to a GPIO pin */
typedef void (*hal_digital_write_t)(int pin, int value);

/** Read a value (0 or 1) from a GPIO pin */
typedef int  (*hal_digital_read_t)(int pin);

/** Delay for specified milliseconds */
typedef void (*hal_delay_ms_t)(unsigned int ms);

/** Write a single byte via SPI */
typedef void (*hal_spi_writebyte_t)(uint8_t data);

/** Write a buffer of bytes via SPI (one CS cycle for entire buffer) */
typedef void (*hal_spi_writebuf_t)(const uint8_t *data, size_t len);

/*===========================================================================
 * EPD Device structure
 *
 * Each driver populates this structure with:
 *   - dimensions (width, height)
 *   - pin assignments
 *   - LUT table pointers (if applicable)
 *   - HAL function pointers
 *   - private driver-specific data
 *===========================================================================*/
typedef struct _EPD {
    /* Display dimensions */
    int width;
    int height;

    /* GPIO pin assignments */
    int rst_pin;
    int dc_pin;
    int cs_pin;
    int busy_pin;
    int pwr_pin;

    /* HAL function pointers */
    hal_digital_write_t  digital_write;
    hal_digital_read_t   digital_read;
    hal_delay_ms_t       delay_ms;
    hal_spi_writebyte_t  spi_writebyte;
    hal_spi_writebuf_t   spi_writebuf;

    /* Driver-specific LUT tables (set by each driver's init) */
    const uint8_t *lut_full;
    size_t         lut_full_size;
    const uint8_t *lut_partial;
    size_t         lut_partial_size;

    /* Private data - driver-specific state */
    void *driver_data;
} EPD;

/*===========================================================================
 * HAL setup functions (implemented in hal.c)
 *===========================================================================*/

/**
 * @brief Initialize hardware (SPI, GPIO) for the given EPD device.
 * @param epd  Pointer to EPD struct with pin/HAL configured.
 * @return EPD_OK on success, EPD_ERROR on failure.
 */
int hal_init(EPD *epd);

/**
 * @brief Clean up hardware resources (SPI, GPIO).
 * @param epd  Pointer to EPD struct.
 * @return EPD_OK on success.
 */
int hal_cleanup(EPD *epd);

/*===========================================================================
 * Default HAL implementation (Linux sysfs GPIO + spidev)
 * Used on Raspberry Pi and similar Linux SBCs.
 *===========================================================================*/

/**
 * @brief Populate an EPD struct with default HAL function pointers
 *        using Linux spidev + sysfs GPIO (or wiringPi/libgpiod).
 * @param epd  Pointer to EPD struct to configure.
 */
int epd_setup_default_hal(EPD *epd);

/*===========================================================================
 * EPD driver common helpers
 *===========================================================================*/

/** Send a command byte (DC=0) */
void epd_send_command(EPD *epd, uint8_t cmd);

/** Send a single data byte (DC=1) */
void epd_send_data(EPD *epd, uint8_t data);

/** Send a buffer of data bytes (DC=1, single CS cycle) */
void epd_send_data_buf(EPD *epd, const uint8_t *data, size_t len);

/** Hardware reset sequence */
void epd_reset(EPD *epd);

/** Wait until the busy pin goes low (idle) */
void epd_wait_busy(EPD *epd);

/** Calculate the line width in bytes for a given pixel width */
static inline int epd_linewidth(int w) {
    return (w % 8 == 0) ? (w / 8) : (w / 8 + 1);
}

#ifdef __cplusplus
}
#endif

#endif /* EPD_COMMON_H */
