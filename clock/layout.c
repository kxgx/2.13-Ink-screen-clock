/**
 * @file    layout.c
 * @brief   Layout persistence — JSON read/write (hand-rolled, no library)
 */

#include "layout.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ================================================================== */
/* Default positions (mirrors current clock.c hard-coded values)        */
/* ================================================================== */
static void set_defaults(Layout *l) {
    l->screen_w = 250;
    l->screen_h = 122;

    l->time_x = -1;
    l->time_y = 40;

    l->date_x = 2;
    l->date_y = 2;

    for (int i = 0; i < 4; i++) l->w_label_x[i] = 150;
    l->w_label_y[0] = 25;
    l->w_label_y[1] = 45;
    l->w_label_y[2] = 65;
    l->w_label_y[3] = 85;

    for (int i = 0; i < 4; i++) l->w_data_x[i] = 195;
    l->w_data_y[0] = 25;
    l->w_data_y[1] = 45;
    l->w_data_y[2] = 65;
    l->w_data_y[3] = 85;

    l->w_upd_x = 211;
    l->w_upd_y = 109;

    l->bat_x = 128;
    l->bat_y = 108;
    l->bat_frame_x = 126;
    l->bat_frame_y = 108;
    l->bat_frame_w = 29;
    l->bat_frame_h = 11;

    l->ip_x = 10;
    l->ip_y = 109;

    l->bar_y = 105;
    l->bar_h = 17;

    l->time_pt = 40;
    l->date_pt = 14;
    l->weather_pt = 14;
    l->small_pt = 10;
    l->ip_pt = 13;

    strcpy(l->font_cn, "Font.ttc");
    strcpy(l->font_time, "DSEG7Modern-Bold.ttf");
}

/* ================================================================== */
/* Tiny JSON int-reader: extract integer value for a given key.         */
/* ================================================================== */
static int json_get_int(const char *json, const char *key, int fallback) {
    char search[64];
    snprintf(search, sizeof(search), "\"%s\"", key);
    const char *p = strstr(json, search);
    if (!p) return fallback;
    p += strlen(search);
    while (*p == ' ' || *p == ':' || *p == '\t') p++;
    int val = 0, sign = 1;
    if (*p == '-') { sign = -1; p++; }
    while (*p >= '0' && *p <= '9') { val = val * 10 + (*p - '0'); p++; }
    return val * sign;
}

/* extract string value for a given key */
static void json_get_str(const char *json, const char *key, char *out, int outsz, const char *fallback) {
    strcpy(out, fallback);
    char search[64];
    snprintf(search, sizeof(search), "\"%s\"", key);
    const char *p = strstr(json, search);
    if (!p) return;
    p += strlen(search);
    while (*p == ' ' || *p == ':' || *p == '\t') p++;
    if (*p != '"') return;
    p++;  /* skip opening quote */
    int i = 0;
    while (*p && *p != '"' && i < outsz - 1) {
        if (*p == '\\' && *(p+1)) p++;  /* skip escape */
        out[i++] = *p++;
    }
    out[i] = '\0';
}

/* ================================================================== */
int layout_init(Layout *l) {
    set_defaults(l);

    FILE *fp = fopen(LAYOUT_FILE, "r");
    if (!fp) return -1;   /* file missing — defaults are fine */

    fseek(fp, 0, SEEK_END);
    long sz = ftell(fp);
    if (sz <= 0 || sz > 8192) { fclose(fp); return -1; }
    fseek(fp, 0, SEEK_SET);

    char *buf = (char *)malloc(sz + 1);
    if (!buf) { fclose(fp); return -1; }
    size_t n = fread(buf, 1, sz, fp);
    buf[n] = '\0';
    fclose(fp);

    l->time_x = json_get_int(buf, "time_x", l->time_x);
    l->time_y = json_get_int(buf, "time_y", l->time_y);
    l->date_x = json_get_int(buf, "date_x", l->date_x);
    l->date_y = json_get_int(buf, "date_y", l->date_y);
    /* Backward compat: old "w_label_x" applies to all, individual overrides */
    { int v = json_get_int(buf, "w_label_x", -1);
      if (v >= 0) for (int i = 0; i < 4; i++) l->w_label_x[i] = v; }
    l->w_label_x[0] = json_get_int(buf, "w_label_x0", l->w_label_x[0]);
    l->w_label_x[1] = json_get_int(buf, "w_label_x1", l->w_label_x[1]);
    l->w_label_x[2] = json_get_int(buf, "w_label_x2", l->w_label_x[2]);
    l->w_label_x[3] = json_get_int(buf, "w_label_x3", l->w_label_x[3]);
    l->w_label_y[0] = json_get_int(buf, "w_label_y0", l->w_label_y[0]);
    l->w_label_y[1] = json_get_int(buf, "w_label_y1", l->w_label_y[1]);
    l->w_label_y[2] = json_get_int(buf, "w_label_y2", l->w_label_y[2]);
    l->w_label_y[3] = json_get_int(buf, "w_label_y3", l->w_label_y[3]);
    { int v = json_get_int(buf, "w_data_x", -1);
      if (v >= 0) for (int i = 0; i < 4; i++) l->w_data_x[i] = v; }
    l->w_data_x[0] = json_get_int(buf, "w_data_x0", l->w_data_x[0]);
    l->w_data_x[1] = json_get_int(buf, "w_data_x1", l->w_data_x[1]);
    l->w_data_x[2] = json_get_int(buf, "w_data_x2", l->w_data_x[2]);
    l->w_data_x[3] = json_get_int(buf, "w_data_x3", l->w_data_x[3]);
    l->w_data_y[0] = json_get_int(buf, "w_data_y0", l->w_data_y[0]);
    l->w_data_y[1] = json_get_int(buf, "w_data_y1", l->w_data_y[1]);
    l->w_data_y[2] = json_get_int(buf, "w_data_y2", l->w_data_y[2]);
    l->w_data_y[3] = json_get_int(buf, "w_data_y3", l->w_data_y[3]);
    l->w_upd_x = json_get_int(buf, "w_upd_x", l->w_upd_x);
    l->w_upd_y = json_get_int(buf, "w_upd_y", l->w_upd_y);
    l->bat_x = json_get_int(buf, "bat_x", l->bat_x);
    l->bat_y = json_get_int(buf, "bat_y", l->bat_y);
    l->bat_frame_x = json_get_int(buf, "bat_frame_x", l->bat_frame_x);
    l->bat_frame_y = json_get_int(buf, "bat_frame_y", l->bat_frame_y);
    l->bat_frame_w = json_get_int(buf, "bat_frame_w", l->bat_frame_w);
    l->bat_frame_h = json_get_int(buf, "bat_frame_h", l->bat_frame_h);
    l->ip_x = json_get_int(buf, "ip_x", l->ip_x);
    l->ip_y = json_get_int(buf, "ip_y", l->ip_y);
    l->bar_y = json_get_int(buf, "bar_y", l->bar_y);
    l->bar_h = json_get_int(buf, "bar_h", l->bar_h);

    l->time_pt    = json_get_int(buf, "time_pt",    l->time_pt);
    l->date_pt    = json_get_int(buf, "date_pt",    l->date_pt);
    l->weather_pt = json_get_int(buf, "weather_pt", l->weather_pt);
    l->small_pt   = json_get_int(buf, "small_pt",   l->small_pt);
    l->ip_pt      = json_get_int(buf, "ip_pt",      l->ip_pt);

    json_get_str(buf, "font_cn",   l->font_cn,   sizeof(l->font_cn),   l->font_cn);
    json_get_str(buf, "font_time", l->font_time, sizeof(l->font_time), l->font_time);

    free(buf);
    printf("Layout loaded from %s\n", LAYOUT_FILE);
    return 0;
}

/* ================================================================== */
int layout_save(const Layout *l) {
    FILE *fp = fopen(LAYOUT_FILE, "w");
    if (!fp) return -1;

    fprintf(fp,
        "{\n"
        "  \"time_x\":%d,\"time_y\":%d,\n"
        "  \"date_x\":%d,\"date_y\":%d,\n"
        "  \"w_label_x0\":%d,\"w_label_x1\":%d,\"w_label_x2\":%d,\"w_label_x3\":%d,\n"
        "  \"w_label_y0\":%d,\"w_label_y1\":%d,\"w_label_y2\":%d,\"w_label_y3\":%d,\n"
        "  \"w_data_x0\":%d,\"w_data_x1\":%d,\"w_data_x2\":%d,\"w_data_x3\":%d,\n"
        "  \"w_data_y0\":%d,\"w_data_y1\":%d,\"w_data_y2\":%d,\"w_data_y3\":%d,\n"
        "  \"w_upd_x\":%d,\"w_upd_y\":%d,\n"
        "  \"bat_x\":%d,\"bat_y\":%d,\n"
        "  \"bat_frame_x\":%d,\"bat_frame_y\":%d,\"bat_frame_w\":%d,\"bat_frame_h\":%d,\n"
        "  \"ip_x\":%d,\"ip_y\":%d,\n"
        "  \"bar_y\":%d,\"bar_h\":%d,\n"
        "  \"time_pt\":%d,\"date_pt\":%d,\"weather_pt\":%d,\"small_pt\":%d,\"ip_pt\":%d,\n"
        "  \"font_cn\":\"%s\",\"font_time\":\"%s\"\n"
        "}\n",
        l->time_x, l->time_y,
        l->date_x, l->date_y,
        l->w_label_x[0], l->w_label_x[1], l->w_label_x[2], l->w_label_x[3],
        l->w_label_y[0], l->w_label_y[1], l->w_label_y[2], l->w_label_y[3],
        l->w_data_x[0], l->w_data_x[1], l->w_data_x[2], l->w_data_x[3],
        l->w_data_y[0], l->w_data_y[1], l->w_data_y[2], l->w_data_y[3],
        l->w_upd_x, l->w_upd_y,
        l->bat_x, l->bat_y,
        l->bat_frame_x, l->bat_frame_y, l->bat_frame_w, l->bat_frame_h,
        l->ip_x, l->ip_y,
        l->bar_y, l->bar_h,
        l->time_pt, l->date_pt, l->weather_pt, l->small_pt, l->ip_pt,
        l->font_cn, l->font_time);

    fclose(fp);
    printf("Layout saved to %s\n", LAYOUT_FILE);
    return 0;
}
