/**
 * @file    main.c
 * @brief   Example: Waveshare e-Paper C Library usage
 *
 * Compile:
 *   cd c_bindings && make example
 *
 * Run (on Raspberry Pi):
 *   sudo ./build/epd_example
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "waveshare_epd.h"

/* =========================================================================
 * Helper: create a checkerboard pattern for testing
 * ========================================================================= */
static void fill_checkerboard(uint8_t *buf, int width, int height) {
    int lw = epd_linewidth(width);
    int square = 16; /* pixels per checker square */

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int cx = x / square;
            int cy = y / square;
            int pixel = ((cx + cy) & 1) ? 0 : 1; /* 0=black, 1=white */

            if (pixel) {
                /* Set bit to 1 (white) - buffer is init'd to 0xFF, so skip */
            } else {
                /* Set bit to 0 (black) */
                int byte_idx = (x / 8) + y * lw;
                int bit_idx  = 7 - (x % 8); /* MSB first */
                buf[byte_idx] &= ~(1 << bit_idx);
            }
        }
    }
}

/* =========================================================================
 * Main - demonstrate 2.13" V4 e-Paper display
 * ========================================================================= */
int main(void) {
    EPD epd;

    printf("Waveshare e-Paper C Library - Example\n");
    printf("=====================================\n\n");

    /* ---- Initialize 2.13" V4 display ---- */
    printf("Initializing 2.13\" V4 e-Paper display...\n");

    if (EPD_2in13_V4_Init(&epd) != EPD_OK) {
        fprintf(stderr, "ERROR: Failed to initialize display.\n");
        fprintf(stderr, "Make sure SPI is enabled and GPIO pins are correct.\n");
        return 1;
    }

    printf("  Width:  %d px\n", epd.width);
    printf("  Height: %d px\n", epd.height);
    printf("  Buffer: %d bytes\n\n", epd_linewidth(epd.width) * epd.height);

    /* ---- Clear screen ---- */
    printf("Clearing display...\n");
    EPD_2in13_V4_Clear(&epd, EPD_WHITE);

    /* ---- Draw checkerboard pattern ---- */
    printf("Drawing checkerboard pattern...\n");

    int bufsize = epd_linewidth(epd.width) * epd.height;
    uint8_t *image = (uint8_t *)malloc(bufsize);
    if (!image) {
        fprintf(stderr, "ERROR: malloc failed\n");
        EPD_2in13_V4_Sleep(&epd);
        return 1;
    }

    /* Initialize buffer to all white */
    memset(image, 0xFF, bufsize);

    /* Draw checkerboard */
    fill_checkerboard(image, epd.width, epd.height);

    /* Display the image */
    EPD_2in13_V4_Display(&epd, image);

    printf("Checkerboard displayed.\n");

    /* ---- Wait 5 seconds ---- */
    printf("Waiting 5 seconds...\n");
    epd.delay_ms(5000);

    /* ---- Clear screen again ---- */
    printf("Clearing display...\n");
    EPD_2in13_V4_Clear(&epd, EPD_WHITE);

    /* ---- Put display to sleep ---- */
    printf("Putting display to sleep...\n");
    EPD_2in13_V4_Sleep(&epd);

    /* Clean up */
    free(image);

    printf("\nDone!\n");
    return 0;
}
