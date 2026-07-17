/**
 * @file    clock.c
 * @brief   e-Paper Clock - C port of main.py using libwaveshare_epd
 *
 * Displays time, date, weather, battery, IP on 2.13" V4 e-Paper display.
 * Supports full refresh and partial refresh for minute-level updates.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "waveshare_epd.h"

/* =========================================================================
 * Display and layout constants (250x122, rotated 180°)
 * ========================================================================= */
#define W           250
#define H           122
#define BUF_SIZE    ((W/8) * H)  /* 4000 bytes */

/* =========================================================================
 * Embedded bitmap fonts
 * ========================================================================= */

/* ---- 5x8 pixel ASCII font (printable chars 0x20-0x7E) ---- */
static const uint8_t font5x8[][5] = {
    {0x00,0x00,0x00,0x00,0x00}, /*   */
    {0x00,0x00,0x5F,0x00,0x00}, /* ! */
    {0x00,0x07,0x00,0x07,0x00}, /* " */
    {0x14,0x7F,0x14,0x7F,0x14}, /* # */
    {0x24,0x2A,0x7F,0x2A,0x12}, /* $ */
    {0x23,0x13,0x08,0x64,0x62}, /* % */
    {0x36,0x49,0x55,0x22,0x50}, /* & */
    {0x00,0x05,0x03,0x00,0x00}, /* ' */
    {0x00,0x1C,0x22,0x41,0x00}, /* ( */
    {0x00,0x41,0x22,0x1C,0x00}, /* ) */
    {0x08,0x2A,0x1C,0x2A,0x08}, /* * */
    {0x08,0x08,0x3E,0x08,0x08}, /* + */
    {0x00,0x50,0x30,0x00,0x00}, /* , */
    {0x08,0x08,0x08,0x08,0x08}, /* - */
    {0x00,0x60,0x60,0x00,0x00}, /* . */
    {0x20,0x10,0x08,0x04,0x02}, /* / */
    {0x3E,0x51,0x49,0x45,0x3E}, /* 0 */
    {0x00,0x42,0x7F,0x40,0x00}, /* 1 */
    {0x42,0x61,0x51,0x49,0x46}, /* 2 */
    {0x21,0x41,0x45,0x4B,0x31}, /* 3 */
    {0x18,0x14,0x12,0x7F,0x10}, /* 4 */
    {0x27,0x45,0x45,0x45,0x39}, /* 5 */
    {0x3C,0x4A,0x49,0x49,0x30}, /* 6 */
    {0x01,0x71,0x09,0x05,0x03}, /* 7 */
    {0x36,0x49,0x49,0x49,0x36}, /* 8 */
    {0x06,0x49,0x49,0x29,0x1E}, /* 9 */
    {0x00,0x36,0x36,0x00,0x00}, /* : */
    {0x00,0x56,0x36,0x00,0x00}, /* ; */
    {0x00,0x08,0x14,0x22,0x41}, /* < */
    {0x14,0x14,0x14,0x14,0x14}, /* = */
    {0x41,0x22,0x14,0x08,0x00}, /* > */
    {0x02,0x01,0x51,0x09,0x06}, /* ? */
    {0x32,0x49,0x79,0x41,0x3E}, /* @ */
    {0x7E,0x11,0x11,0x11,0x7E}, /* A */
    {0x7F,0x49,0x49,0x49,0x36}, /* B */
    {0x3E,0x41,0x41,0x41,0x22}, /* C */
    {0x7F,0x41,0x41,0x22,0x1C}, /* D */
    {0x7F,0x49,0x49,0x49,0x41}, /* E */
    {0x7F,0x09,0x09,0x01,0x01}, /* F */
    {0x3E,0x41,0x41,0x51,0x32}, /* G */
    {0x7F,0x08,0x08,0x08,0x7F}, /* H */
    {0x00,0x41,0x7F,0x41,0x00}, /* I */
    {0x20,0x40,0x41,0x3F,0x01}, /* J */
    {0x7F,0x08,0x14,0x22,0x41}, /* K */
    {0x7F,0x40,0x40,0x40,0x40}, /* L */
    {0x7F,0x02,0x04,0x02,0x7F}, /* M */
    {0x7F,0x04,0x08,0x10,0x7F}, /* N */
    {0x3E,0x41,0x41,0x41,0x3E}, /* O */
    {0x7F,0x09,0x09,0x09,0x06}, /* P */
    {0x3E,0x41,0x51,0x21,0x5E}, /* Q */
    {0x7F,0x09,0x19,0x29,0x46}, /* R */
    {0x46,0x49,0x49,0x49,0x31}, /* S */
    {0x01,0x01,0x7F,0x01,0x01}, /* T */
    {0x3F,0x40,0x40,0x40,0x3F}, /* U */
    {0x1F,0x20,0x40,0x20,0x1F}, /* V */
    {0x7F,0x20,0x18,0x20,0x7F}, /* W */
    {0x63,0x14,0x08,0x14,0x63}, /* X */
    {0x03,0x04,0x78,0x04,0x03}, /* Y */
    {0x61,0x51,0x49,0x45,0x43}, /* Z */
    {0x00,0x00,0x7F,0x41,0x41}, /* [ */
    {0x02,0x04,0x08,0x10,0x20}, /* \ */
    {0x41,0x41,0x7F,0x00,0x00}, /* ] */
    {0x04,0x02,0x01,0x02,0x04}, /* ^ */
    {0x40,0x40,0x40,0x40,0x40}, /* _ */
    {0x00,0x01,0x02,0x04,0x00}, /* ` */
    {0x20,0x54,0x54,0x54,0x78}, /* a */
    {0x7F,0x48,0x44,0x44,0x38}, /* b */
    {0x38,0x44,0x44,0x44,0x20}, /* c */
    {0x38,0x44,0x44,0x48,0x7F}, /* d */
    {0x38,0x54,0x54,0x54,0x18}, /* e */
    {0x08,0x7E,0x09,0x01,0x02}, /* f */
    {0x08,0x14,0x54,0x54,0x3C}, /* g */
    {0x7F,0x08,0x04,0x04,0x78}, /* h */
    {0x00,0x44,0x7D,0x40,0x00}, /* i */
    {0x20,0x40,0x44,0x3D,0x00}, /* j */
    {0x00,0x7F,0x10,0x28,0x44}, /* k */
    {0x00,0x41,0x7F,0x40,0x00}, /* l */
    {0x7C,0x04,0x18,0x04,0x78}, /* m */
    {0x7C,0x08,0x04,0x04,0x78}, /* n */
    {0x38,0x44,0x44,0x44,0x38}, /* o */
    {0x7C,0x14,0x14,0x14,0x08}, /* p */
    {0x08,0x14,0x14,0x18,0x7C}, /* q */
    {0x7C,0x08,0x04,0x04,0x08}, /* r */
    {0x48,0x54,0x54,0x54,0x20}, /* s */
    {0x04,0x3F,0x44,0x40,0x20}, /* t */
    {0x3C,0x40,0x40,0x20,0x7C}, /* u */
    {0x1C,0x20,0x40,0x20,0x1C}, /* v */
    {0x3C,0x40,0x30,0x40,0x3C}, /* w */
    {0x44,0x28,0x10,0x28,0x44}, /* x */
    {0x0C,0x50,0x50,0x50,0x3C}, /* y */
    {0x44,0x64,0x54,0x4C,0x44}, /* z */
};

/* ---- Large 7-segment digit font (for time display, 3x5 block size = ~18x30px) ---- */
/* Each digit is defined as 7 segments. Each segment is a 3-wide block. */
typedef struct { int x, y, w, h; } segment_t;

static void draw_seg(uint8_t *buf, int bx, int by, int on) {
    /* Draw a horizontal or vertical segment at (bx,by) */
    /* Segments are 3 pixels wide, 1 pixel thick */
    int lw = W / 8;
    for (int p = 0; p < 3; p++) {
        int x = bx + p;
        if (x >= 0 && x < W && by >= 0 && by < H) {
            if (on) buf[(x / 8) + by * lw] |= (0x80 >> (x % 8));
        }
    }
}

static void draw_seg_v(uint8_t *buf, int bx, int by, int on) {
    int lw = W / 8;
    for (int p = 0; p < 3; p++) {
        int y = by + p;
        if (bx >= 0 && bx < W && y >= 0 && y < H) {
            if (on) buf[(bx / 8) + y * lw] |= (0x80 >> (bx % 8));
        }
    }
}

static void draw_7seg_char(uint8_t *buf, int x, int y, char ch) {
    /* Segment patterns for digits 0-9, ':' */
    /* Segments: A(T), B(TR), C(BR), D(B), E(BL), F(TL), G(MID) */
    static const uint8_t segs[12] = {
        0x3F, /* 0: ABCDEF */
        0x06, /* 1: BC */
        0x5B, /* 2: ABDEG */
        0x4F, /* 3: ABCDG */
        0x66, /* 4: BCFG */
        0x6D, /* 5: ACDFG */
        0x7D, /* 6: ACDEFG */
        0x07, /* 7: ABC */
        0x7F, /* 8: ABCDEFG */
        0x6F, /* 9: ABCDFG */
        0x00, /* : (colon, handled separately) */
        0x00,
    };

    int idx = (ch >= '0' && ch <= '9') ? (ch - '0') : 10;
    uint8_t s = segs[idx];
    int w2 = 2;  /* segment thickness */

    if (ch == ':') {
        /* Draw colon dots */
        int lw = W / 8;
        for (int dy = 3; dy < 5; dy++) {
            int yy = y + 10 + dy;
            if (yy >= 0 && yy < H) buf[(x / 8) + yy * lw] |= (0x80 >> (x % 8));
        }
        for (int dy = 3; dy < 5; dy++) {
            int yy = y + 16 + dy;
            if (yy >= 0 && yy < H) buf[(x / 8) + yy * lw] |= (0x80 >> (x % 8));
        }
        return;
    }

    /* Segment drawing (each segment is w2 pixels thick) */
    int lw = W / 8;
    /* A: top horizontal */
    for (int tx = 0; tx < 3; tx++) {
        int xx = x + tx;
        for (int ty = 0; ty < w2; ty++) {
            int yy = y + ty;
            if (s & 0x01 && xx >= 0 && xx < W && yy >= 0 && yy < H)
                buf[(xx/8) + yy*lw] |= (0x80 >> (xx%8));
        }
    }
    /* B: top-right vertical */
    for (int tx = 0; tx < w2; tx++) {
        int xx = x + 3;
        for (int ty = 0; ty < 3; ty++) {
            int yy = y + ty;
            if (s & 0x02 && xx >= 0 && xx < W && yy >= 0 && yy < H)
                buf[(xx/8) + yy*lw] |= (0x80 >> (xx%8));
        }
    }
    /* C: bottom-right vertical */
    for (int tx = 0; tx < w2; tx++) {
        int xx = x + 3;
        for (int ty = 0; ty < 3; ty++) {
            int yy = y + 4 + ty;
            if (s & 0x04 && xx >= 0 && xx < W && yy >= 0 && yy < H)
                buf[(xx/8) + yy*lw] |= (0x80 >> (xx%8));
        }
    }
    /* D: bottom horizontal */
    for (int tx = 0; tx < 3; tx++) {
        int xx = x + tx;
        for (int ty = 0; ty < w2; ty++) {
            int yy = y + 7 + ty;
            if (s & 0x08 && xx >= 0 && xx < W && yy >= 0 && yy < H)
                buf[(xx/8) + yy*lw] |= (0x80 >> (xx%8));
        }
    }
    /* E: bottom-left vertical */
    for (int tx = 0; tx < w2; tx++) {
        int xx = x;
        for (int ty = 0; ty < 3; ty++) {
            int yy = y + 4 + ty;
            if (s & 0x10 && xx >= 0 && xx < W && yy >= 0 && yy < H)
                buf[(xx/8) + yy*lw] |= (0x80 >> (xx%8));
        }
    }
    /* F: top-left vertical */
    for (int tx = 0; tx < w2; tx++) {
        int xx = x;
        for (int ty = 0; ty < 3; ty++) {
            int yy = y + ty;
            if (s & 0x20 && xx >= 0 && xx < W && yy >= 0 && yy < H)
                buf[(xx/8) + yy*lw] |= (0x80 >> (xx%8));
        }
    }
    /* G: middle horizontal */
    for (int tx = 0; tx < 3; tx++) {
        int xx = x + tx;
        for (int ty = 0; ty < w2; ty++) {
            int yy = y + 4 + ty;
            if (s & 0x40 && xx >= 0 && xx < W && yy >= 0 && yy < H)
                buf[(xx/8) + yy*lw] |= (0x80 >> (xx%8));
        }
    }
}

/* =========================================================================
 * Drawing primitives on 1bpp buffer
 * ========================================================================= */

/* Set pixel (x, y) to value (0=black, 1=white). Buffer is already 0xFF=white */
static void set_pixel(uint8_t *buf, int x, int y, int val) {
    if (x < 0 || x >= W || y < 0 || y >= H) return;
    int lw = W / 8;
    if (val)
        buf[(x / 8) + y * lw] |= (0x80 >> (x % 8));
    else
        buf[(x / 8) + y * lw] &= ~(0x80 >> (x % 8));
}

/* Draw horizontal line */
static void draw_hline(uint8_t *buf, int x1, int x2, int y, int val) {
    if (x1 > x2) { int t = x1; x1 = x2; x2 = t; }
    for (int x = x1; x <= x2; x++) set_pixel(buf, x, y, val);
}

/* Draw vertical line */
static void draw_vline(uint8_t *buf, int x, int y1, int y2, int val) {
    if (y1 > y2) { int t = y1; y1 = y2; y2 = t; }
    for (int y = y1; y <= y2; y++) set_pixel(buf, x, y, val);
}

/* Fill rectangle */
static void fill_rect(uint8_t *buf, int x1, int y1, int x2, int y2, int val) {
    if (x1 > x2) { int t = x1; x1 = x2; x2 = t; }
    if (y1 > y2) { int t = y1; y1 = y2; y2 = t; }
    for (int y = y1; y <= y2; y++)
        for (int x = x1; x <= x2; x++)
            set_pixel(buf, x, y, val);
}

/* Draw 5x8 character (from embedded font) */
static void draw_char(uint8_t *buf, int x, int y, char ch) {
    if (ch < 0x20 || ch > 0x7E) return;
    const uint8_t *glyph = font5x8[ch - 0x20];
    for (int col = 0; col < 5; col++) {
        uint8_t line = glyph[col];
        for (int row = 0; row < 8; row++) {
            if (line & (1 << row))
                set_pixel(buf, x + col, y + row, 0); /* black */
        }
    }
}

/* Draw 5x8 text string */
static void draw_text(uint8_t *buf, int x, int y, const char *text) {
    int cx = x;
    while (*text) {
        if (*text == '\n') { cx = x; y += 10; text++; continue; }
        draw_char(buf, cx, y, *text);
        cx += 6; /* 5 + 1 spacing */
        text++;
    }
}

/* Draw 7-segment time string (HH:MM) */
static void draw_time_7seg(uint8_t *buf, int x, int y, const char *time_str) {
    int cx = x;
    while (*time_str) {
        draw_7seg_char(buf, cx, y, *time_str);
        cx += (*time_str == ':') ? 3 : 6;
        time_str++;
    }
}

/* =========================================================================
 * System helpers
 * ========================================================================= */

/* Sync system time from hardware RTC */
static void sync_hwclock(void) {
    system("sudo hwclock --hctosys 2>/dev/null");
}

/* Get current time string "HH:MM" */
static void get_time_str(char *buf, size_t len) {
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    strftime(buf, len, "%H:%M", tm);
}

/* Get date string "YYYY年MM月DD日 星期X" */
static void get_date_str(char *buf, size_t len) {
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    static const char *wdays[] = {"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
    snprintf(buf, len, "%04d-%02d-%02d %s",
             tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
             wdays[tm->tm_wday]);
}

/* Get IPv4 address (excluding 172.x.x.x docker) */
static void get_ip_str(char *buf, size_t len) {
    FILE *fp = popen("hostname -I 2>/dev/null", "r");
    if (!fp) { snprintf(buf, len, "N/A"); return; }
    char ips[256] = {0};
    if (fgets(ips, sizeof(ips), fp)) {
        /* Get first non-172. IP */
        char *tok = strtok(ips, " \t\n");
        while (tok) {
            if (strncmp(tok, "172.", 4) != 0) {
                snprintf(buf, len, "IP:%s", tok);
                pclose(fp);
                return;
            }
            tok = strtok(NULL, " \t\n");
        }
    }
    pclose(fp);
    snprintf(buf, len, "IP:N/A");
}

/* Get battery percentage via netcat */
static void get_battery_str(char *buf, size_t len) {
    FILE *fp = popen("echo \"get battery\" | nc -q 0 127.0.0.1 8423 2>/dev/null", "r");
    if (!fp) { snprintf(buf, len, "--%%"); return; }
    char line[128] = {0};
    if (fgets(line, sizeof(line), fp)) {
        /* Parse "battery: XX" format */
        char *colon = strchr(line, ':');
        if (colon) {
            int pct = atoi(colon + 1);
            snprintf(buf, len, "%d%%", pct);
        } else {
            snprintf(buf, len, "--%%");
        }
    } else {
        snprintf(buf, len, "--%%");
    }
    pclose(fp);
}

/* Parse weather.json and get values */
static int parse_weather(char *temp_out, char *humid_out, char *weather_out,
                          char *city_out, char *update_out, size_t len) {
    FILE *fp = fopen("/root/2.13-Ink-screen-clock/bin/weather.json", "r");
    if (!fp) return -1;

    char json[4096] = {0};
    size_t n = fread(json, 1, sizeof(json) - 1, fp);
    fclose(fp);
    if (n == 0) return -1;
    json[n] = '\0';

    /* Simple JSON value extractor */
    #define EXTRACT(key, out) do { \
        char *p = strstr(json, "\"" key "\""); \
        if (p) { \
            p = strchr(p, ':'); \
            if (p) { \
                p++; while (*p == ' ' || *p == '"') p++; \
                char *end = p; \
                while (*end && *end != '"' && *end != ',' && *end != '\n' && *end != '}') end++; \
                size_t l = (size_t)(end - p); \
                if (l >= len) l = len - 1; \
                memcpy(out, p, l); out[l] = '\0'; \
            } \
        } \
    } while(0)

    EXTRACT("temp", temp_out);
    EXTRACT("SD", humid_out);
    EXTRACT("weather", weather_out);
    EXTRACT("cityname", city_out);
    EXTRACT("time", update_out);

    /* Format temperature and humidity */
    if (temp_out[0]) {
        char t[64]; snprintf(t, sizeof(t), "%sC", temp_out);
        strncpy(temp_out, t, len);
    }
    if (humid_out[0]) {
        char h[64]; snprintf(h, sizeof(h), "%s%%", humid_out);
        strncpy(humid_out, h, len);
    }
    return 0;
}

/* =========================================================================
 * Layout drawing
 * ========================================================================= */

/* Draw bottom bar (black background, white text) */
static void draw_bottom_bar(uint8_t *buf, const char *ip, const char *battery,
                             const char *weather_time) {
    /* Black bar from y=105 to y=121 */
    fill_rect(buf, 0, 105, W - 1, 121, 0);

    /* IP address on left */
    draw_text(buf, 10, 107, ip);

    /* Weather update time center-right */
    draw_text(buf, 150, 107, weather_time);

    /* Battery on right - draw simple battery icon + percentage */
    /* Battery outline: x=126..157, y=108..117 */
    draw_hline(buf, 126, 154, 108, 1);
    draw_vline(buf, 126, 108, 117, 1);
    draw_vline(buf, 154, 108, 117, 1);
    draw_hline(buf, 126, 154, 117, 1);
    /* Battery tip */
    draw_hline(buf, 155, 157, 110, 1);
    draw_hline(buf, 155, 157, 115, 1);
    draw_vline(buf, 157, 111, 114, 1);
    /* Battery level fill (simplified: text inside battery) */
    draw_text(buf, 129, 109, battery);
}

/* Draw weather info block (right side of screen) */
static void draw_weather_block(uint8_t *buf, const char *weather,
                                const char *temp, const char *humid, const char *city) {
    draw_text(buf, 150, 25, "W:");
    draw_text(buf, 191, 25, weather);

    draw_text(buf, 150, 45, "T:");
    draw_text(buf, 191, 45, temp);

    draw_text(buf, 150, 65, "H:");
    draw_text(buf, 191, 65, humid);

    draw_text(buf, 150, 85, "C:");
    draw_text(buf, 191, 85, city);
}

/* =========================================================================
 * Main program
 * ========================================================================= */

int main(void) {
    EPD epd;

    printf("e-Paper Clock (C version)\n");
    printf("========================\n");

    /* ---- Initialize display ---- */
    if (EPD_2in13_V4_Init(&epd) != EPD_OK) {
        fprintf(stderr, "ERROR: Failed to init display\n");
        return 1;
    }

    /* ---- Sync hardware clock ---- */
    sync_hwclock();

    /* ---- Buffer for display ---- */
    uint8_t *buf = (uint8_t *)malloc(BUF_SIZE);
    if (!buf) { EPD_2in13_V4_Sleep(&epd); return 1; }

    /* ---- Basic (full) refresh ---- */
    memset(buf, 0xFF, BUF_SIZE);

    /* Date line */
    char date_str[64];
    get_date_str(date_str, sizeof(date_str));
    draw_text(buf, 2, 2, date_str);

    /* Time (7-segment) */
    char time_str[16];
    get_time_str(time_str, sizeof(time_str));
    draw_time_7seg(buf, 5, 30, time_str);

    /* Weather */
    char temp[64] = "--C", humid[64] = "--%%", weather[64] = "--", city[64] = "--", wtime[64] = "--";
    parse_weather(temp, humid, weather, city, wtime, sizeof(temp));
    draw_weather_block(buf, weather, temp, humid, city);

    /* Bottom bar */
    char ip[64], battery[64];
    get_ip_str(ip, sizeof(ip));
    get_battery_str(battery, sizeof(battery));
    draw_bottom_bar(buf, ip, battery, wtime);

    /* Display */
    EPD_2in13_V4_Display(&epd, buf);
    printf("Full refresh done.\n");

    /* ---- Partial refresh loop (minute-level updates) ---- */
    EPD_2in13_V4_DisplayPartBaseImage(&epd, buf);

    char last_time[16] = "";
    char last_date[64] = "";
    char last_ip[64] = "";
    char last_battery[64] = "";
    char last_weather[64] = "", last_temp[64] = "", last_humid[64] = "";
    char last_city[64] = "", last_wtime[64] = "";

    int loop_count = 0;
    while (1) {
        /* Check time change */
        char new_time[16];
        get_time_str(new_time, sizeof(new_time));
        if (strcmp(new_time, last_time) != 0) {
            /* Erase old time */
            fill_rect(buf, 5, 30, 130, 70, 1);
            draw_time_7seg(buf, 5, 30, new_time);
            strncpy(last_time, new_time, sizeof(last_time));

            /* Strong partial refresh (5 times) */
            for (int i = 0; i < 5; i++)
                EPD_2in13_V4_DisplayPartial(&epd, buf);
            printf("Time updated: %s\n", new_time);
        }

        /* Check date change */
        char new_date[64];
        get_date_str(new_date, sizeof(new_date));
        if (strcmp(new_date, last_date) != 0) {
            fill_rect(buf, 2, 2, 250, 14, 1);
            draw_text(buf, 2, 2, new_date);
            strncpy(last_date, new_date, sizeof(last_date));
            for (int i = 0; i < 5; i++)
                EPD_2in13_V4_DisplayPartial(&epd, buf);
            printf("Date updated: %s\n", new_date);
        }

        /* Check IP change (every ~30 iterations = ~30 seconds) */
        if (loop_count % 30 == 0) {
            char new_ip[64];
            get_ip_str(new_ip, sizeof(new_ip));
            if (strcmp(new_ip, last_ip) != 0) {
                fill_rect(buf, 1, 107, 123, 120, 0);
                draw_text(buf, 10, 107, new_ip);
                strncpy(last_ip, new_ip, sizeof(last_ip));
                for (int i = 0; i < 5; i++)
                    EPD_2in13_V4_DisplayPartial(&epd, buf);
            }

            /* Check battery change */
            char new_bat[64];
            get_battery_str(new_bat, sizeof(new_bat));
            if (strcmp(new_bat, last_battery) != 0) {
                fill_rect(buf, 128, 108, 153, 117, 0);
                draw_text(buf, 129, 109, new_bat);
                strncpy(last_battery, new_bat, sizeof(last_battery));
                for (int i = 0; i < 5; i++)
                    EPD_2in13_V4_DisplayPartial(&epd, buf);
            }

            /* Check weather update */
            char nt[64] = "--C", nh[64] = "--%%", nw[64] = "--", nc[64] = "--", nwt[64] = "--";
            if (parse_weather(nt, nh, nw, nc, nwt, sizeof(nt)) == 0) {
                if (strcmp(nw, last_weather) != 0) {
                    fill_rect(buf, 191, 25, 249, 38, 1);
                    draw_text(buf, 191, 25, nw);
                    strncpy(last_weather, nw, sizeof(last_weather));
                    for (int i = 0; i < 5; i++) EPD_2in13_V4_DisplayPartial(&epd, buf);
                }
                if (strcmp(nt, last_temp) != 0) {
                    fill_rect(buf, 191, 45, 249, 57, 1);
                    draw_text(buf, 191, 45, nt);
                    strncpy(last_temp, nt, sizeof(last_temp));
                    for (int i = 0; i < 5; i++) EPD_2in13_V4_DisplayPartial(&epd, buf);
                }
                if (strcmp(nh, last_humid) != 0) {
                    fill_rect(buf, 191, 65, 249, 77, 1);
                    draw_text(buf, 191, 65, nh);
                    strncpy(last_humid, nh, sizeof(last_humid));
                    for (int i = 0; i < 5; i++) EPD_2in13_V4_DisplayPartial(&epd, buf);
                }
                if (strcmp(nc, last_city) != 0) {
                    fill_rect(buf, 191, 85, 249, 98, 1);
                    draw_text(buf, 191, 85, nc);
                    strncpy(last_city, nc, sizeof(last_city));
                    for (int i = 0; i < 5; i++) EPD_2in13_V4_DisplayPartial(&epd, buf);
                }
                if (strcmp(nwt, last_wtime) != 0) {
                    fill_rect(buf, 150, 107, 249, 118, 0);
                    draw_text(buf, 150, 107, nwt);
                    strncpy(last_wtime, nwt, sizeof(last_wtime));
                    for (int i = 0; i < 5; i++) EPD_2in13_V4_DisplayPartial(&epd, buf);
                }
            }
        }

        loop_count++;
        epd.delay_ms(1000); /* Check every second */
    }

    /* Never reaches here, but for completeness: */
    free(buf);
    EPD_2in13_V4_Sleep(&epd);
    return 0;
}
