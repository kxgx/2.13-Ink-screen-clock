/**
 * @file    hal.h
 * @brief   Hardware Abstraction Layer interface for e-Paper displays
 *
 * Supports multiple backends:
 *   - Linux spidev + sysfs GPIO (Raspberry Pi, etc.)
 *   - WiringPi
 *   - Custom (user-provided function pointers)
 */

#ifndef EPD_HAL_H
#define EPD_HAL_H

#include "epd_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/*===========================================================================
 * HAL backend types
 *===========================================================================*/
typedef enum {
    HAL_BACKEND_SPIDEV,   /**< Linux /dev/spidev + sysfs GPIO */
    HAL_BACKEND_WIRINGPI, /**< WiringPi library */
    HAL_BACKEND_CUSTOM,   /**< User-provided function pointers */
    HAL_BACKEND_NONE
} hal_backend_t;

/*===========================================================================
 * HAL context (opaque to drivers)
 *===========================================================================*/
typedef struct _HALContext HALContext;

/**
 * @brief Create a HAL context with the specified backend.
 * @param backend  Which backend to use.
 * @return New HALContext, or NULL on failure.
 */
HALContext *hal_create(hal_backend_t backend);

/**
 * @brief Destroy a HAL context and release all resources.
 * @param ctx  HAL context to destroy.
 */
void hal_destroy(HALContext *ctx);

/**
 * @brief Initialize hardware (open SPI, export GPIOs, set directions).
 * @param ctx  HAL context.
 * @param epd  EPD device to initialize for.
 * @return EPD_OK on success, EPD_ERROR on failure.
 */
int hal_module_init(HALContext *ctx, EPD *epd);

/**
 * @brief Clean up hardware (close SPI, unexport GPIOs).
 * @param ctx  HAL context.
 * @return EPD_OK on success.
 */
int hal_module_exit(HALContext *ctx);

/**
 * @brief Get the HAL function pointers from a context
 *        to populate an EPD struct.
 * @param ctx  HAL context.
 * @param epd  EPD struct to populate with HAL callbacks.
 */
void hal_get_callbacks(HALContext *ctx, EPD *epd);

/*===========================================================================
 * Default HAL initialization (backward-compatible)
 *===========================================================================*/

/**
 * @brief One-shot setup: create HAL context, init hardware, populate EPD.
 *
 * If ctx_out is not NULL, the context pointer is stored so you can
 * later call hal_teardown_default().
 *
 * @param epd      EPD struct to configure.
 * @param ctx_out  Optional pointer to store HALContext for cleanup.
 * @return EPD_OK on success.
 */
int epd_setup_default_hal(EPD *epd);

/**
 * @brief One-shot cleanup matching epd_setup_default_hal().
 * @param epd  EPD struct used during setup.
 * @return EPD_OK on success.
 */
int epd_teardown_default_hal(EPD *epd);

#ifdef __cplusplus
}
#endif

#endif /* EPD_HAL_H */
