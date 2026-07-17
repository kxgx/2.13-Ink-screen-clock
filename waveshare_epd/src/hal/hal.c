/**
 * @file    hal.c
 * @brief   Hardware Abstraction Layer - Linux spidev + sysfs GPIO backend
 *
 * Uses /dev/spidevX.Y for SPI and /sys/class/gpio for GPIO control.
 * Designed for Raspberry Pi and similar Linux SBCs.
 */

#include "hal.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/spi/spidev.h>

/*===========================================================================
 * Internal HAL context
 *===========================================================================*/
struct _HALContext {
    hal_backend_t backend;
    int           spi_fd;          /**< File descriptor for /dev/spidev */
    EPD          *epd;             /**< Associated EPD device */
};

/*===========================================================================
 * Static forward declarations for HAL callbacks
 *===========================================================================*/
static void spidev_digital_write(int pin, int value);
static int  spidev_digital_read(int pin);
static void spidev_delay_ms(unsigned int ms);
static void spidev_spi_writebyte(uint8_t data);
static void spidev_spi_writebuf(const uint8_t *data, size_t len);

/* Module-level state */
static HALContext *g_active_ctx = NULL;
static volatile uint32_t *g_gpio = NULL;  /* mmap'd GPIO registers */

/*===========================================================================
 * GPIO via /dev/gpiomem (works on all Pi OS versions)
 *===========================================================================*/
#define GPFSEL0   0
#define GPSET0    7
#define GPCLR0   10
#define GPLEV0   13

static int gpio_init(void) {
    if (g_gpio) return 0;
    int fd = open("/dev/gpiomem", O_RDWR | O_SYNC);
    if (fd < 0) {
        fprintf(stderr, "hal: cannot open /dev/gpiomem: %s\n", strerror(errno));
        return -1;
    }
    g_gpio = (volatile uint32_t *)mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);
    if (g_gpio == MAP_FAILED) {
        fprintf(stderr, "hal: mmap /dev/gpiomem failed: %s\n", strerror(errno));
        g_gpio = NULL;
        return -1;
    }
    return 0;
}

static void gpio_set_input(int pin) {
    if (!g_gpio || pin < 0 || pin > 27) return;
    int reg = pin / 10;
    int shift = (pin % 10) * 3;
    g_gpio[GPFSEL0 + reg] &= ~(7 << shift);  /* 000 = input */
}

static void gpio_set_output(int pin) {
    if (!g_gpio || pin < 0 || pin > 27) return;
    int reg = pin / 10;
    int shift = (pin % 10) * 3;
    g_gpio[GPFSEL0 + reg] = (g_gpio[GPFSEL0 + reg] & ~(7 << shift)) | (1 << shift);  /* 001 = output */
}

static void gpio_write(int pin, int value) {
    if (!g_gpio || pin < 0 || pin > 27) return;
    if (value)
        g_gpio[GPSET0] = 1 << pin;
    else
        g_gpio[GPCLR0] = 1 << pin;
}

static int gpio_read(int pin) {
    if (!g_gpio || pin < 0 || pin > 27) return -1;
    return (g_gpio[GPLEV0] >> pin) & 1;
}

static void gpio_cleanup(void) {
    if (g_gpio) {
        munmap((void *)g_gpio, 4096);
        g_gpio = NULL;
    }
}

/*===========================================================================
 * SPI spidev helpers
 *===========================================================================*/

static int spi_open(const char *device, uint32_t speed, uint8_t mode, uint8_t bits) {
    int fd = open(device, O_RDWR);
    if (fd < 0) {
        fprintf(stderr, "hal: cannot open %s: %s\n", device, strerror(errno));
        return -1;
    }

    if (ioctl(fd, SPI_IOC_WR_MODE, &mode) < 0) {
        fprintf(stderr, "hal: SPI_IOC_WR_MODE failed: %s\n", strerror(errno));
        close(fd);
        return -1;
    }

    if (ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits) < 0) {
        fprintf(stderr, "hal: SPI_IOC_WR_BITS_PER_WORD failed: %s\n", strerror(errno));
        close(fd);
        return -1;
    }

    if (ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed) < 0) {
        fprintf(stderr, "hal: SPI_IOC_WR_MAX_SPEED_HZ failed: %s\n", strerror(errno));
        close(fd);
        return -1;
    }

    return fd;
}

static int spi_transfer(int fd, const uint8_t *tx, uint8_t *rx, size_t len) {
    struct spi_ioc_transfer tr = {
        .tx_buf        = (unsigned long)tx,
        .rx_buf        = (unsigned long)rx,
        .len           = (uint32_t)len,
        .speed_hz      = EPD_SPI_SPEED,
        .delay_usecs   = 0,
        .bits_per_word = 8,
    };
    return ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
}

/*===========================================================================
 * HAL callback implementations (using global context)
 *===========================================================================*/

static void spidev_digital_write(int pin, int value) {
    gpio_write(pin, value);
}

static int spidev_digital_read(int pin) {
    return gpio_read(pin);
}

static void spidev_delay_ms(unsigned int ms) {
    struct timespec ts;
    ts.tv_sec  = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000000L;
    nanosleep(&ts, NULL);
}

static void spidev_spi_writebyte(uint8_t data) {
    if (g_active_ctx && g_active_ctx->spi_fd >= 0) {
        spi_transfer(g_active_ctx->spi_fd, &data, NULL, 1);
    }
}

static void spidev_spi_writebuf(const uint8_t *data, size_t len) {
    if (g_active_ctx && g_active_ctx->spi_fd >= 0 && data && len > 0) {
        spi_transfer(g_active_ctx->spi_fd, data, NULL, len);
    }
}

/*===========================================================================
 * Public HAL API
 *===========================================================================*/

HALContext *hal_create(hal_backend_t backend) {
    HALContext *ctx = (HALContext *)calloc(1, sizeof(HALContext));
    if (!ctx) return NULL;

    ctx->backend = backend;
    ctx->spi_fd  = -1;

    return ctx;
}

void hal_destroy(HALContext *ctx) {
    if (!ctx) return;

    if (ctx->spi_fd >= 0) {
        close(ctx->spi_fd);
        ctx->spi_fd = -1;
    }

    free(ctx);
    if (g_active_ctx == ctx) g_active_ctx = NULL;
}

int hal_module_init(HALContext *ctx, EPD *epd) {
    if (!ctx || !epd) return EPD_ERROR;

    g_active_ctx = ctx;
    ctx->epd = epd;

    /* Initialize GPIO via /dev/gpiomem */
    if (gpio_init() != 0) return EPD_ERROR;

    /* Setup GPIO pins */
    int pins[] = {epd->rst_pin, epd->dc_pin, epd->busy_pin, epd->pwr_pin};

    for (int i = 0; i < 4; i++) {
        int pin = pins[i];
        if (pin < 0 || pin > 27) continue;

        if (i == 2) { /* BUSY */
            gpio_set_input(pin);
        } else {
            gpio_set_output(pin);
        }
    }

    /* Power on */
    gpio_write(epd->pwr_pin, 1);

    /* Set initial output values */
    gpio_write(epd->rst_pin, 1);
    gpio_write(epd->dc_pin, 0);

    /* Open SPI device */
    char spi_dev[32];
    snprintf(spi_dev, sizeof(spi_dev), "/dev/spidev%d.%d",
             EPD_SPI_BUS, EPD_SPI_DEVICE);
    ctx->spi_fd = spi_open(spi_dev, EPD_SPI_SPEED, SPI_MODE_0, 8);
    if (ctx->spi_fd < 0) {
        fprintf(stderr, "hal: SPI open failed on %s\n", spi_dev);
        return EPD_ERROR;
    }

    /* Populate EPD with function pointers */
    epd->digital_write = spidev_digital_write;
    epd->digital_read  = spidev_digital_read;
    epd->delay_ms      = spidev_delay_ms;
    epd->spi_writebyte = spidev_spi_writebyte;
    epd->spi_writebuf  = spidev_spi_writebuf;

    return EPD_OK;
}

int hal_module_exit(HALContext *ctx) {
    if (!ctx) return EPD_ERROR;

    if (ctx->spi_fd >= 0) {
        close(ctx->spi_fd);
        ctx->spi_fd = -1;
    }

    /* Turn off power */
    if (ctx->epd && ctx->epd->pwr_pin >= 0) {
        gpio_write(ctx->epd->pwr_pin, 0);
    }

    gpio_cleanup();
    g_active_ctx = NULL;
    return EPD_OK;
}

void hal_get_callbacks(HALContext *ctx, EPD *epd) {
    if (!ctx || !epd) return;

    g_active_ctx = ctx;
    epd->digital_write = spidev_digital_write;
    epd->digital_read  = spidev_digital_read;
    epd->delay_ms      = spidev_delay_ms;
    epd->spi_writebyte = spidev_spi_writebyte;
    epd->spi_writebuf  = spidev_spi_writebuf;
}

/*===========================================================================
 * Convenience: one-shot default HAL setup
 *===========================================================================*/

int epd_setup_default_hal(EPD *epd) {
    if (!epd) return EPD_ERROR;

    HALContext *ctx = hal_create(HAL_BACKEND_SPIDEV);
    if (!ctx) return EPD_ERROR;

    return hal_module_init(ctx, epd);
}

int epd_teardown_default_hal(EPD *epd) {
    (void)epd;
    if (g_active_ctx) {
        hal_module_exit(g_active_ctx);
        hal_destroy(g_active_ctx);
    }
    return EPD_OK;
}

/*===========================================================================
 * Common EPD helper functions (used by all drivers)
 *===========================================================================*/

void epd_send_command(EPD *epd, uint8_t cmd) {
    if (!epd) return;
    epd->digital_write(epd->dc_pin, 0);
    epd->spi_writebyte(cmd);
}

void epd_send_data(EPD *epd, uint8_t data) {
    if (!epd) return;
    epd->digital_write(epd->dc_pin, 1);
    epd->spi_writebyte(data);
}

void epd_send_data_buf(EPD *epd, const uint8_t *data, size_t len) {
    if (!epd || !data || len == 0) return;
    epd->digital_write(epd->dc_pin, 1);
    epd->spi_writebuf(data, len);
}

void epd_reset(EPD *epd) {
    if (!epd) return;
    epd->digital_write(epd->rst_pin, 1);
    epd->delay_ms(200);
    epd->digital_write(epd->rst_pin, 0);
    epd->delay_ms(2);
    epd->digital_write(epd->rst_pin, 1);
    epd->delay_ms(200);
}

void epd_wait_busy(EPD *epd) {
    if (!epd) return;
    while (epd->digital_read(epd->busy_pin) == 1) {
        epd->delay_ms(100);
    }
}

/* END OF FILE */
