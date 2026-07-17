/**
 * @file    clock.c
 * @brief   C port of main.py - Ink screen clock with weather display
 *
 * Uses the waveshare_epd C library for 2.13" V4 e-Paper display (122x250).
 * Font rendering via stb_truetype.h (zero dependencies)
 *
 * Build:
 *   cd clock && make clock
 *
 * Run (on Raspberry Pi):
 *   sudo ./build/epd_clock
 */

/* NOTE: This code targets Raspberry Pi (Linux).
 * Required packages on RPi:
 *   sudo apt install libfreetype6-dev
 * These headers are Linux-specific and won't lint on Windows. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#include "waveshare_epd.h"

/* =========================================================================
 * Display and framebuffer constants
 *
 * The Python code draws on a 250x122 canvas (Image.new('1', (250, 122))),
 * then rotates 180°, and the Python driver's getbuffer() rotates 90° CCW.
 *
 * Coordinate mapping: Python (px, py) in 250x122
 *   �?display buffer (dx = 121-py, dy = px) in 122x250
 *
 * We draw directly into fb[250][122] (1bpp packed), then convert to
 * the display buffer format (122x250, 16 bytes/row).
 * ========================================================================= */
#define FB_WIDTH   250
#define FB_HEIGHT  122
#define FB_ROW     ((FB_WIDTH + 7) / 8)  /* 32 bytes per row */
#define FB_SIZE    (FB_ROW * FB_HEIGHT)   /* 3904 bytes */

#define DISP_WIDTH  122
#define DISP_HEIGHT 250
#define DISP_ROW    ((DISP_WIDTH + 7) / 8)  /* 16 bytes per row */
#define DISP_SIZE   (DISP_ROW * DISP_HEIGHT) /* 4000 bytes */

/* Font and data paths - relative to clock/ working directory */
#define FONT_PATH_TTC  "../pic/Font.ttc"
#define FONT_PATH_DSEG "../pic/DSEG7Modern-Bold.ttf"

/* Weather JSON path */
#define WEATHER_JSON_PATH "../bin/weather.json"

/* =========================================================================
 * Global framebuffer
 * ========================================================================= */
static uint8_t fb[FB_SIZE];              /* 250x122 drawing canvas */
static uint8_t display_buf[DISP_SIZE];    /* 122x250 final buffer for EPD */

/* Global EPD for signal handler cleanup */
static EPD *g_epd = NULL;
static volatile int g_running = 1;

/* =========================================================================
 * Font globals (stb_truetype)
 * ========================================================================= */
static stbtt_fontinfo font_ttc;
static stbtt_fontinfo font_dseg;
static unsigned char *font_ttc_buf = NULL;
static unsigned char *font_dseg_buf = NULL;

/* Font sizes in pt, matching Python */
#define FONT_SIZE_DATE   15   /* font02 */
#define FONT_SIZE_TIME   38   /* font03 - DSEG7Modern-Bold */
#define FONT_SIZE_SMALL  10   /* font04 */
#define FONT_SIZE_IP     12   /* font05 */
#define FONT_SIZE_WEATHER 13  /* font06 */

/* =========================================================================
 * Framebuffer drawing primitives
 * ========================================================================= */

/* Clear entire framebuffer to color (0xFF=white, 0x00=black) */
static void fb_clear(uint8_t color) {
    memset(fb, color, FB_SIZE);
}

/* Set a single pixel in the framebuffer.
 * color: 0 = black, 1 = white */
static void fb_set_pixel(int x, int y, int color) {
    if (x < 0 || x >= FB_WIDTH || y < 0 || y >= FB_HEIGHT) return;
    int byte_idx = y * FB_ROW + (x / 8);
    int bit_idx  = 7 - (x % 8);
    if (color) {
        fb[byte_idx] |= (1 << bit_idx);
    } else {
        fb[byte_idx] &= ~(1 << bit_idx);
    }
}

/* Fill a rectangle */
static void fb_fill_rect(int x, int y, int w, int h, int color) {
    for (int dy = 0; dy < h; dy++) {
        for (int dx = 0; dx < w; dx++) {
            fb_set_pixel(x + dx, y + dy, color);
        }
    }
}

/* Bresenham line drawing */
static void fb_draw_line(int x0, int y0, int x1, int y1, int color) {
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy, e2;

    while (1) {
        fb_set_pixel(x0, y0, color);
        if (x0 == x1 && y0 == y1) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

/* Draw ellipse outline using midpoint algorithm */
static void fb_draw_ellipse(int cx, int cy, int rx, int ry, int outline, int fill) {
    if (fill >= 0) {
        /* Filled ellipse: draw horizontal lines between left and right edges */
        int rx2 = rx * rx, ry2 = ry * ry;
        /* Midpoint algorithm for filling */
        int x = 0, y = ry;
        int dx = 0, dy = 2 * rx2 * y;
        int d1 = ry2 - rx2 * ry + rx2 / 4;

        /* Region 1 */
        while (dx < dy) {
            fb_draw_line(cx - x, cy - y, cx + x, cy - y, fill);
            fb_draw_line(cx - x, cy + y, cx + x, cy + y, fill);
            x++;
            dx += 2 * ry2;
            if (d1 < 0) {
                d1 += dx + ry2;
            } else {
                y--;
                dy -= 2 * rx2;
                d1 += dx - dy + ry2;
            }
        }

        /* Region 2 */
        int d2 = (int)(ry2 * (x + 0.5) * (x + 0.5) + rx2 * (y - 1) * (y - 1) - (long)rx2 * ry2);
        while (y >= 0) {
            fb_draw_line(cx - x, cy - y, cx + x, cy - y, fill);
            fb_draw_line(cx - x, cy + y, cx + x, cy + y, fill);
            y--;
            dy -= 2 * rx2;
            if (d2 > 0) {
                d2 += rx2 - dy;
            } else {
                x++;
                dx += 2 * ry2;
                d2 += dx - dy + rx2;
            }
        }
    }
    if (outline >= 0) {
        /* Outline ellipse using midpoint */
        int rx2 = rx * rx, ry2 = ry * ry;
        int x = 0, y = ry;
        int dx = 0, dy = 2 * rx2 * y;
        int d1 = ry2 - rx2 * ry + rx2 / 4;

        while (dx < dy) {
            fb_set_pixel(cx + x, cy + y, outline);
            fb_set_pixel(cx - x, cy + y, outline);
            fb_set_pixel(cx + x, cy - y, outline);
            fb_set_pixel(cx - x, cy - y, outline);
            x++;
            dx += 2 * ry2;
            if (d1 < 0) {
                d1 += dx + ry2;
            } else {
                y--;
                dy -= 2 * rx2;
                d1 += dx - dy + ry2;
            }
        }
        int d2 = (int)(ry2 * (x + 0.5) * (x + 0.5) + rx2 * (y - 1) * (y - 1) - (long)rx2 * ry2);
        while (y >= 0) {
            fb_set_pixel(cx + x, cy + y, outline);
            fb_set_pixel(cx - x, cy + y, outline);
            fb_set_pixel(cx + x, cy - y, outline);
            fb_set_pixel(cx - x, cy - y, outline);
            y--;
            dy -= 2 * rx2;
            if (d2 > 0) {
                d2 += rx2 - dy;
            } else {
                x++;
                dx += 2 * ry2;
                d2 += dx - dy + rx2;
            }
        }
    }
}

/* =========================================================================
 * Font rendering (stb_truetype)
 * ========================================================================= */

/* Initialize fonts and load all fonts */
static int ft_init(void) {
    FILE *fp = fopen(FONT_PATH_TTC, "rb");
    if (!fp) {
        fprintf(stderr, "ERROR: Cannot open %s\n", FONT_PATH_TTC);
        return -1;
    }

    fseek(fp, 0, SEEK_END);
    long fsize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    font_ttc_buf = (unsigned char *)malloc(fsize);
    if (!font_ttc_buf) {
        fclose(fp);
        return -1;
    }

    size_t nread = fread(font_ttc_buf, 1, fsize, fp);
    fclose(fp);

    if (nread != (size_t)fsize) {
        free(font_ttc_buf);
        return -1;
    }

    int ttc_offset = stbtt_GetFontOffsetForIndex(font_ttc_buf, 0);
    if (ttc_offset < 0 || !stbtt_InitFont(&font_ttc, font_ttc_buf, ttc_offset)) {
        fprintf(stderr, "ERROR: Cannot load %s\n", FONT_PATH_TTC);
        free(font_ttc_buf);
        return -1;
    }

    fp = fopen(FONT_PATH_DSEG, "rb");
    if (!fp) {
        fprintf(stderr, "ERROR: Cannot open %s\n", FONT_PATH_DSEG);
        return -1;
    }

    fseek(fp, 0, SEEK_END);
    fsize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    font_dseg_buf = (unsigned char *)malloc(fsize);
    if (!font_dseg_buf) {
        fclose(fp);
        return -1;
    }

    nread = fread(font_dseg_buf, 1, fsize, fp);
    fclose(fp);

    if (nread != (size_t)fsize) {
        free(font_dseg_buf);
        return -1;
    }

    if (!stbtt_InitFont(&font_dseg, font_dseg_buf, 0)) {
        fprintf(stderr, "ERROR: Cannot load %s\n", FONT_PATH_DSEG);
        free(font_dseg_buf);
        return -1;
    }

    return 0;
}

/* Clean up fonts resources */
static void ft_cleanup(void) {
    if (font_dseg_buf) free(font_dseg_buf);
    if (font_ttc_buf)  free(font_ttc_buf);
}

/* Render text using stb_truetype (zero-dependency) into the framebuffer.
 * font_size: point size
 * use_dseg: 1 for digital font, 0 for Chinese font
 * color: 0 = black text, 1 = white text
 */
static void ft_render_text(int x, int y, const char *text, int font_size,
                           int use_dseg, int color) {
    stbtt_fontinfo *font = use_dseg ? &font_dseg : &font_ttc;

    /* Set character size in points (72dpi) */
    float scale = stbtt_ScaleForPixelHeight(font, font_size);

    /* Pillow: y = text top. stb_truetype: y = baseline. Compensate. */
    int ascent, descent, lineGap;
    stbtt_GetFontVMetrics(font, &ascent, &descent, &lineGap);
    int baseline_y = y + (int)(ascent * scale);

    int pen_x = x;
    const char *p = text;

    while (*p) {
        /* Handle multi-byte UTF-8 characters */
        unsigned int charcode;
        int advance = 0;

        if ((*p & 0x80) == 0) {
            /* ASCII: 1 byte */
            charcode = (unsigned char)*p;
            advance = 1;
        } else if ((*p & 0xE0) == 0xC0) {
            /* 2-byte UTF-8 */
            charcode = ((unsigned char)p[0] & 0x1F) << 6;
            charcode |= ((unsigned char)p[1] & 0x3F);
            advance = 2;
        } else if ((*p & 0xF0) == 0xE0) {
            /* 3-byte UTF-8 (Chinese characters) */
            charcode = ((unsigned char)p[0] & 0x0F) << 12;
            charcode |= ((unsigned char)p[1] & 0x3F) << 6;
            charcode |= ((unsigned char)p[2] & 0x3F);
            advance = 3;
        } else if ((*p & 0xF8) == 0xF0) {
            /* 4-byte UTF-8 */
            charcode = ((unsigned char)p[0] & 0x07) << 18;
            charcode |= ((unsigned char)p[1] & 0x3F) << 12;
            charcode |= ((unsigned char)p[2] & 0x3F) << 6;
            charcode |= ((unsigned char)p[3] & 0x3F);
            advance = 4;
        } else {
            /* Invalid, skip */
            p++;
            continue;
        }

        /* Load and render glyph with monochrome target */
        int glyph = stbtt_FindGlyphIndex(font, charcode);
        if (glyph == 0) goto next_char;

        int advance_width;
        int left_side_bearing;
        stbtt_GetGlyphHMetrics(font, glyph, &advance_width, &left_side_bearing);

        int x0, y0, x1, y1;
        stbtt_GetGlyphBitmapBox(font, glyph, scale, scale, &x0, &y0, &x1, &y1);

        int w = x1 - x0;
        int h = y1 - y0;

        unsigned char *bitmap = (unsigned char *)malloc(w * h);
        if (!bitmap) goto next_char;

        stbtt_MakeGlyphBitmap(font, bitmap, w, h, w, scale, scale, glyph);

        int gx = pen_x + (int)(left_side_bearing * scale);
        int gy = baseline_y - y1;  /* y1 is top of glyph in stb coords (positive up) */

        for (int row = 0; row < h; row++) {
            for (int col = 0; col < w; col++) {
                int pixel = bitmap[row * w + col];

                if (color == 0) {
                    /* Black text: draw black pixels */
                    if (pixel) fb_set_pixel(gx + col, gy + row, 0);
                } else {
                    /* White text: draw white pixels */
                    if (pixel) fb_set_pixel(gx + col, gy + row, 1);
                }
            }
        }

        free(bitmap);

        pen_x += advance_width * scale;

next_char:
        p += advance;
    }
}

/* =========================================================================
 * Convert framebuffer (250x122) to display buffer (122x250)
 *
 * Mapping: Python (px, py) �?display (dx=121-py, dy=px)
 * ========================================================================= */
static void fb_to_display(void) {
    memset(display_buf, 0xFF, DISP_SIZE);  /* Start all white */

    for (int py = 0; py < FB_HEIGHT; py++) {
        for (int px = 0; px < FB_WIDTH; px++) {
            int dx = FB_HEIGHT - 1 - py;   /* 121 - py */
            int dy = px;

            /* Get source pixel (0=black in Pillow '1' mode) */
            int src_byte = py * FB_ROW + (px / 8);
            int src_bit  = 7 - (px % 8);
            int pixel = (fb[src_byte] >> src_bit) & 1;

            if (pixel == 0) {
                /* Black pixel �?clear bit in display buffer */
                int dst_byte = dy * DISP_ROW + (dx / 8);
                int dst_bit  = 7 - (dx % 8);
                display_buf[dst_byte] &= ~(1 << dst_bit);
            }
        }
    }
}

/* =========================================================================
 * System functions (matching Python counterparts)
 * ========================================================================= */

/* Flag to track hwclock sync (matching has_set_system_time) */
static int has_set_system_time = 0;

/* Execute a shell command and return the first line of output.
 * Caller must free the returned string. */
static char *shell_output(const char *cmd) {
    FILE *fp = popen(cmd, "r");
    if (!fp) return NULL;

    static char buf[256];
    char *result = NULL;
    if (fgets(buf, sizeof(buf), fp)) {
        /* Strip trailing newline */
        size_t len = strlen(buf);
        while (len > 0 && (buf[len-1] == '\n' || buf[len-1] == '\r'))
            buf[--len] = '\0';
        result = strdup(buf);
    }
    pclose(fp);
    return result;
}

/* Set system time from hardware clock (matching set_system_time_from_hwclock) */
static int set_system_time_from_hwclock(void) {
    int ret = system("sudo hwclock --hctosys 2>/dev/null");
    usleep(100000);  /* 0.1s delay for time update */
    return (ret == 0);
}

/* Get current time string HH:MM (matching get_time) */
static void get_time_str(char *buf, size_t bufsize) {
    if (!has_set_system_time) {
        set_system_time_from_hwclock();
        has_set_system_time = 1;
    }
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    strftime(buf, bufsize, "%H:%M", tm_info);
}

/* Get current date string (matching get_date but lunar via Python) */
static void get_date_str(char *buf, size_t bufsize) {
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);

    /* Gregorian date */
    char gregorian[64];
    strftime(gregorian, sizeof(gregorian), "%Y年%m月%d日", tm_info);

    /* Weekday in Chinese */
    static const char *weekdays[] = {
        "星期一", "星期二", "星期三", "星期四", "星期五", "星期六", "星期日"
    };
    const char *weekday = weekdays[tm_info->tm_wday];

    /* Lunar date via Python */
    char lunar[32] = "";
    char *lunar_output = shell_output(
        "python3 -c \"from borax.calendars.lunardate import LunarDate; "
        "print(LunarDate.today().strftime('农历%M月%D日'))\" 2>/dev/null");
    if (lunar_output) {
        snprintf(lunar, sizeof(lunar), "%s", lunar_output);
        free(lunar_output);
    }

    snprintf(buf, bufsize, "%s%s%s", gregorian, weekday, lunar);
}

/* Get IPv4 address (matching Get_ipv4_address) */
static void get_ip_str(char *buf, size_t bufsize) {
    char *ip = shell_output(
        "hostname -I 2>/dev/null | grep -oE '[0-9]{1,3}(\\.[0-9]{1,3}){3}' "
        "| grep -v '^172\\.' | head -1");
    if (ip && strlen(ip) > 0) {
        snprintf(buf, bufsize, "%s", ip);
        free(ip);
    } else {
        snprintf(buf, bufsize, "获取失败");
        free(ip);
    }
}

/* Get battery power (matching power_battery with caching) */
static char last_power[16] = "";
static time_t last_power_time = 0;

static void get_power_str(char *buf, size_t bufsize) {
    time_t now = time(NULL);

    /* Cache for 3 minutes */
    if ((now - last_power_time < 180) && last_power[0] != '\0') {
        snprintf(buf, bufsize, "%s", last_power);
        return;
    }

    char *power = shell_output(
        "echo \"get battery\" | nc -q 0 127.0.0.1 8423 2>/dev/null | "
        "awk -F':' '{print int($2)}'");
    if (power) {
        snprintf(last_power, sizeof(last_power), "%s%%", power);
        free(power);
    } else if (last_power[0] == '\0') {
        snprintf(last_power, sizeof(last_power), "0%%");
    }
    last_power_time = now;
    snprintf(buf, bufsize, "%s", last_power);
}

/* =========================================================================
 * Weather JSON parsing (simple string parser, no external JSON library)
 * ========================================================================= */

/* Extract a string value from a simple JSON key-value pair.
 * Handles format: "key":"value" or "key": "value" */
static int json_get_str(const char *json, const char *key, char *out, size_t outsize) {
    char search[64];
    snprintf(search, sizeof(search), "\"%s\"", key);

    const char *pos = strstr(json, search);
    if (!pos) return -1;

    pos += strlen(search);

    /* Skip whitespace and colon */
    while (*pos && (*pos == ' ' || *pos == ':')) pos++;

    /* Skip opening quote */
    if (*pos == '"') pos++;
    else return -1;

    /* Copy until closing quote */
    size_t i = 0;
    while (*pos && *pos != '"' && *pos != '\n' && i < outsize - 1) {
        out[i++] = *pos++;
    }
    out[i] = '\0';
    return 0;
}

/* Read weather.json and get all fields */
typedef struct {
    char cityname[32];
    char temp[16];
    char weather[32];
    char wd[16];
    char time_str[16];
    char date_str[16];
    char sd[16];
} WeatherData;

static int read_weather(WeatherData *w) {
    memset(w, 0, sizeof(*w));

    FILE *fp = fopen(WEATHER_JSON_PATH, "r");
    if (!fp) return -1;

    fseek(fp, 0, SEEK_END);
    long fsize = ftell(fp);
    if (fsize <= 0 || fsize > 4096) { fclose(fp); return -1; }
    fseek(fp, 0, SEEK_SET);

    char *buf = (char *)malloc(fsize + 1);
    if (!buf) { fclose(fp); return -1; }

    size_t nread = fread(buf, 1, fsize, fp);
    buf[nread] = '\0';
    fclose(fp);

    if (nread == 0) { free(buf); return -1; }

    json_get_str(buf, "cityname", w->cityname, sizeof(w->cityname));
    json_get_str(buf, "temp",      w->temp,     sizeof(w->temp));
    json_get_str(buf, "weather",   w->weather,  sizeof(w->weather));
    json_get_str(buf, "WD",        w->wd,       sizeof(w->wd));
    json_get_str(buf, "time",      w->time_str, sizeof(w->time_str));
    json_get_str(buf, "date",      w->date_str, sizeof(w->date_str));
    json_get_str(buf, "SD",        w->sd,       sizeof(w->sd));

    free(buf);
    return 0;
}

/* =========================================================================
 * Display content drawing (matching Python display functions)
 * ========================================================================= */

/* Cached values for partial refresh comparison */
static char cached_date[128]    = "";
static char cached_time[8]      = "";
static char cached_ip[32]       = "";
static char cached_power[16]    = "";
static char cached_weather_w[32]  = "";
static char cached_weather_t[32]  = "";
static char cached_weather_h[32]  = "";
static char cached_weather_c[32]  = "";
static char cached_weather_u[32]  = "";

/* Draw bottom edge bar (matching Bottom_edge) */
static void draw_bottom_edge(void) {
    /* Black bar across bottom */
    fb_fill_rect(0, 105, FB_WIDTH, 17, 0);

    /* Battery icon outline */
    fb_draw_line(126, 109, 154, 109, 1);  /* top */
    fb_draw_line(126, 110, 126, 119, 1);  /* left */
    fb_draw_line(127, 119, 154, 119, 1);  /* bottom */
    fb_draw_line(154, 110, 154, 118, 1);  /* right */
    /* Battery bump */
    fb_draw_line(155, 112, 157, 112, 1);
    fb_draw_line(155, 116, 157, 116, 1);
    fb_draw_line(157, 113, 157, 115, 1);

    /* Battery percentage text */
    char power[16];
    get_power_str(power, sizeof(power));
    snprintf(cached_power, sizeof(cached_power), "%s", power);
    ft_render_text(129, 108, power, FONT_SIZE_SMALL, 0, 1);

    /* Clock icon (ellipse + hands) */
    fb_draw_ellipse(199, 113, 7, 6, 1, 1);   /* filled white circle, white outline */
    fb_draw_line(199, 109, 199, 114, 0);       /* hour hand (black) */
    fb_draw_line(200, 114, 204, 114, 0);        /* minute hand (black) */

    /* IP address */
    char ip[32];
    get_ip_str(ip, sizeof(ip));
    snprintf(cached_ip, sizeof(cached_ip), "%s", ip);
    char ip_text[64];
    snprintf(ip_text, sizeof(ip_text), "IP:%s", ip);
    ft_render_text(10, 107, ip_text, FONT_SIZE_IP, 0, 1);
}

/* Draw weather section (matching Weather) */
static void draw_weather(void) {
    WeatherData w;
    if (read_weather(&w) != 0) return;

    /* Cache values */
    snprintf(cached_weather_w, sizeof(cached_weather_w), "%s", w.weather);
    snprintf(cached_weather_t, sizeof(cached_weather_t), "%s", w.temp);
    snprintf(cached_weather_h, sizeof(cached_weather_h), "%s", w.sd);
    snprintf(cached_weather_c, sizeof(cached_weather_c), "%s", w.cityname);
    snprintf(cached_weather_u, sizeof(cached_weather_u), "%s", w.time_str);

    /* Prefix labels */
    ft_render_text(150, 25, "天气:", FONT_SIZE_WEATHER, 0, 0);
    ft_render_text(150, 45, "温度:", FONT_SIZE_WEATHER, 0, 0);
    ft_render_text(150, 65, "湿度:", FONT_SIZE_WEATHER, 0, 0);
    ft_render_text(150, 85, "城市:", FONT_SIZE_WEATHER, 0, 0);

    /* Weather data */
    char temp_str[32];
    snprintf(temp_str, sizeof(temp_str), "%s°C", w.temp);
    ft_render_text(191, 25, w.weather,  FONT_SIZE_WEATHER, 0, 0);
    ft_render_text(191, 45, temp_str,    FONT_SIZE_WEATHER, 0, 0);
    ft_render_text(191, 65, w.sd,        FONT_SIZE_WEATHER, 0, 0);
    ft_render_text(191, 85, w.cityname,  FONT_SIZE_WEATHER, 0, 0);

    /* Weather update time in bottom bar */
    ft_render_text(211, 107, w.time_str, FONT_SIZE_IP, 0, 1);
}

/* Full refresh (matching Basic_refresh) */
static void basic_refresh(EPD *epd) {
    fb_clear(0xFF);  /* White background */

    /* Date line */
    get_date_str(cached_date, sizeof(cached_date));
    ft_render_text(2, 2, cached_date, FONT_SIZE_DATE, 0, 0);

    /* Time display */
    get_time_str(cached_time, sizeof(cached_time));
    ft_render_text(5, 40, cached_time, FONT_SIZE_TIME, 1, 0);

    /* Bottom edge */
    draw_bottom_edge();

    /* Weather */
    draw_weather();

    /* Convert and display */
    fb_to_display();
    EPD_2in13_V4_Display(epd, display_buf);
}

/* Partial refresh loop (matching Partial_refresh) */
static void partial_refresh(EPD *epd) {
    /* Set base image for partial refresh */
    fb_to_display();
    EPD_2in13_V4_DisplayPartBaseImage(epd, display_buf);

    /* Re-init for partial refresh */
    EPD_2in13_V4_Init(epd);

    while (1) {
        int need_refresh = 0;

        /* ---- Time check ---- */
        char current_time[8];
        get_time_str(current_time, sizeof(current_time));
        if (strcmp(current_time, cached_time) != 0) {
            /* Erase old time area - Python: (5,40,133,82) inclusive */
            fb_fill_rect(5, 40, 129, 43, 1);
            ft_render_text(5, 40, current_time, FONT_SIZE_TIME, 1, 0);
            snprintf(cached_time, sizeof(cached_time), "%s", current_time);
            need_refresh = 1;
        }

        /* ---- Date check ---- */
        char current_date[128];
        get_date_str(current_date, sizeof(current_date));
        if (strcmp(current_date, cached_date) != 0) {
            fb_fill_rect(2, 2, 249, 15, 1);
            ft_render_text(2, 2, current_date, FONT_SIZE_DATE, 0, 0);
            snprintf(cached_date, sizeof(cached_date), "%s", current_date);
            need_refresh = 1;
        }

        /* ---- IP check ---- */
        char current_ip[32];
        get_ip_str(current_ip, sizeof(current_ip));
        if (strcmp(current_ip, cached_ip) != 0) {
            fb_fill_rect(1, 107, 123, 14, 0);  /* black background */
            char ip_text[64];
            snprintf(ip_text, sizeof(ip_text), "IP:%s", current_ip);
            ft_render_text(10, 107, ip_text, FONT_SIZE_IP, 0, 1);
            snprintf(cached_ip, sizeof(cached_ip), "%s", current_ip);
            need_refresh = 1;
        }

        /* ---- Weather update ---- */
        WeatherData w;
        if (read_weather(&w) == 0) {
            /* Weather description */
            if (strcmp(w.weather, cached_weather_w) != 0) {
                fb_fill_rect(191, 25, 59, 14, 1);
                ft_render_text(191, 25, w.weather, FONT_SIZE_WEATHER, 0, 0);
                snprintf(cached_weather_w, sizeof(cached_weather_w), "%s", w.weather);
                need_refresh = 1;
            }

            /* Temperature */
            char temp_str[32];
            snprintf(temp_str, sizeof(temp_str), "%s°C", w.temp);
            if (strcmp(temp_str, cached_weather_t) != 0) {
                fb_fill_rect(191, 45, 59, 13, 1);
                ft_render_text(191, 45, temp_str, FONT_SIZE_WEATHER, 0, 0);
                snprintf(cached_weather_t, sizeof(cached_weather_t), "%s", temp_str);
                need_refresh = 1;
            }

            /* Humidity */
            if (strcmp(w.sd, cached_weather_h) != 0) {
                fb_fill_rect(191, 65, 59, 13, 1);
                ft_render_text(191, 65, w.sd, FONT_SIZE_WEATHER, 0, 0);
                snprintf(cached_weather_h, sizeof(cached_weather_h), "%s", w.sd);
                need_refresh = 1;
            }

            /* City */
            if (strcmp(w.cityname, cached_weather_c) != 0) {
                fb_fill_rect(191, 85, 59, 14, 1);
                ft_render_text(191, 85, w.cityname, FONT_SIZE_WEATHER, 0, 0);
                snprintf(cached_weather_c, sizeof(cached_weather_c), "%s", w.cityname);
                need_refresh = 1;
            }

            /* Update time */
            if (strcmp(w.time_str, cached_weather_u) != 0) {
                fb_fill_rect(211, 107, 38, 12, 0);
                ft_render_text(211, 107, w.time_str, FONT_SIZE_IP, 0, 1);
                snprintf(cached_weather_u, sizeof(cached_weather_u), "%s", w.time_str);
                need_refresh = 1;
            }
        }

        /* ---- Battery check ---- */
        char current_power[16];
        get_power_str(current_power, sizeof(current_power));
        if (strcmp(current_power, cached_power) != 0) {
            fb_fill_rect(128, 110, 26, 8, 0);
            ft_render_text(129, 108, current_power, FONT_SIZE_SMALL, 0, 1);
            snprintf(cached_power, sizeof(cached_power), "%s", current_power);
            need_refresh = 1;
        }

        /* Perform partial refresh if anything changed */
        if (need_refresh) {
            fb_to_display();
            /* Strong partial refresh (5 times, matching Local_strong_brush) */
            for (int i = 0; i < 5; i++) {
                EPD_2in13_V4_DisplayPartial(epd, display_buf);
            }
        }

        /* Short sleep to avoid busy-waiting (Python checks continuously) */
        usleep(200000);  /* 200ms */
    }
}

/* =========================================================================
 * Signal handler and weather update
 * ========================================================================= */

/* Clean screen and exit on signal (matching clean.py) */
static void sig_handler(int sig) {
    (void)sig;
    g_running = 0;
    if (g_epd) {
        fprintf(stderr, "\nCaught signal %d, cleaning display...\n", sig);
        EPD_2in13_V4_Init(g_epd);
        EPD_2in13_V4_Clear(g_epd, EPD_WHITE);
        EPD_2in13_V4_Sleep(g_epd);
    }
    ft_cleanup();
    _exit(0);
}

/* Weather update thread: launch weather.py once, it self-schedules every 30min */
static void *weather_update_thread(void *arg) {
    (void)arg;
    printf("Starting weather updater...\n");
    system("python3 ../bin/weather.py 2>/dev/null &");
    /* weather.py runs forever with its own Timer; nothing more to do */
    return NULL;
}

/* =========================================================================
 * Main entry point
 * ========================================================================= */
int main(void) {
    EPD epd;
    int retry_interval = 180;

    signal(SIGINT, sig_handler);
    signal(SIGTERM, sig_handler);

    if (ft_init() != 0) {
        fprintf(stderr, "ERROR: Font initialization failed.\n");
        return 1;
    }

    printf("Ink Screen Clock - C Version\n");
    printf("=============================\n");

    /* Start weather update thread */
    pthread_t weather_tid;
    pthread_create(&weather_tid, NULL, weather_update_thread, NULL);
    pthread_detach(weather_tid);

    while (1) {
        printf("Initializing 2.13in V4 e-Paper display...\n");

        if (EPD_2in13_V4_Init(&epd) != EPD_OK) {
            fprintf(stderr, "ERROR: Failed to initialize. Retrying in %ds...\n",
                    retry_interval);
            sleep(retry_interval);
            continue;
        }

        g_epd = &epd;
        printf("  Width: %d px, Height: %d px\n", epd.width, epd.height);

        printf("Performing full refresh...\n");
        basic_refresh(&epd);

        printf("Entering partial refresh loop...\n");
        partial_refresh(&epd);

        g_epd = NULL;
        EPD_2in13_V4_Init(&epd);
        EPD_2in13_V4_Clear(&epd, EPD_WHITE);
        EPD_2in13_V4_Sleep(&epd);

        printf("Partial refresh exited, retrying...\n");
        sleep(retry_interval);
    }

    ft_cleanup();
    return 0;
}
