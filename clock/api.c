/**
 * @file    api.c
 * @brief   Minimal HTTP API server (no external library)
 *
 * Listens on port 8080, returns JSON with all current screen data.
 * Only handles GET / — no routing, no POST, no headers beyond the bare
 * minimum required by HTTP/1.1.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "layout.h"

#define API_PORT        8080
#define API_BACKLOG     4
#define API_BUF_SIZE    4096

/* ------------------------------------------------------------------ */
/* "Extern" – the main clock app keeps these strings current.          */
/* ------------------------------------------------------------------ */
extern char cached_date[128];
extern char cached_time[8];
extern char cached_ip[32];
extern char cached_power[16];
extern char cached_weather_w[32];
extern char cached_weather_t[32];
extern char cached_weather_h[32];
extern char cached_weather_c[32];
extern char cached_weather_u[32];

extern Layout g_layout;
extern volatile int g_layout_refresh;  /* 1 = trigger full refresh */

/* ------------------------------------------------------------------ */
/* Build the JSON payload.  All strings are already UTF‑8 on RPi.     */
/* ------------------------------------------------------------------ */
static void build_json(char *buf, int bufsize) {
    /* Escape minimal set: backslash and double-quote */
#define ESC(_s, _d) do { \
    const char *_src = (_s); char *_dst = (_d); \
    while (*_src) { \
        if (*_src == '\\' || *_src == '"') *_dst++ = '\\'; \
        *_dst++ = *_src++; \
    } *_dst = '\0'; \
} while(0)

    char esc_w[64], esc_t[64], esc_h[64], esc_c[64];
    ESC(cached_weather_w, esc_w);
    ESC(cached_weather_t, esc_t);
    ESC(cached_weather_h, esc_h);
    ESC(cached_weather_c, esc_c);

    /* Also escape date (contains Chinese, safe but belts-and-suspenders) */
    char esc_date[256];
    ESC(cached_date, esc_date);

    snprintf(buf, bufsize,
        "{"
        "\"screen\":{\"width\":%d,\"height\":%d},"
        "\"time\":{\"text\":\"%s\",\"x\":%d,\"y\":%d,\"pt\":%d,\"font\":\"DSEG\"},"
        "\"date\":{\"text\":\"%s\",\"x\":%d,\"y\":%d,\"pt\":%d},"
        "\"weather\":{"
            "\"desc\":{\"text\":\"%s\",\"x\":%d,\"y\":%d},"
            "\"temp\":{\"text\":\"%s\",\"x\":%d,\"y\":%d},"
            "\"humidity\":{\"text\":\"%s\",\"x\":%d,\"y\":%d},"
            "\"city\":{\"text\":\"%s\",\"x\":%d,\"y\":%d},"
            "\"updated\":{\"text\":\"%s\",\"x\":%d,\"y\":%d},"
            "\"labels\":[\"天气:\",\"温度:\",\"湿度:\",\"城市:\"],"
            "\"pt\":%d"
        "},"
        "\"battery\":{\"text\":\"%s\",\"x\":%d,\"y\":%d,\"pt\":%d,"
            "\"frame\":{\"x\":%d,\"y\":%d,\"w\":%d,\"h\":%d}},"
        "\"ip\":{\"text\":\"%s\",\"x\":%d,\"y\":%d,\"pt\":%d},"
        "\"bar\":{\"y\":%d,\"h\":%d}"
        "}",
        g_layout.screen_w, g_layout.screen_h,
        cached_time, g_layout.time_x, g_layout.time_y, 40,
        esc_date, g_layout.date_x, g_layout.date_y, 14,
        esc_w, g_layout.w_data_x, g_layout.w_data_y[0],
        esc_t, g_layout.w_data_x, g_layout.w_data_y[1],
        esc_h, g_layout.w_data_x, g_layout.w_data_y[2],
        esc_c, g_layout.w_data_x, g_layout.w_data_y[3],
        cached_weather_u, g_layout.w_upd_x, g_layout.w_upd_y, 14,
        cached_power, g_layout.bat_x, g_layout.bat_y, 10,
        g_layout.bat_frame_x, g_layout.bat_frame_y, g_layout.bat_frame_w, g_layout.bat_frame_h,
        cached_ip, g_layout.ip_x, g_layout.ip_y, 13,
        g_layout.bar_y, g_layout.bar_h);

#undef ESC
}

/* ------------------------------------------------------------------ */
/* Minimal HTTP response helper.                                       */
/* ------------------------------------------------------------------ */
static void send_response(int fd, int code, const char *mime, const char *body) {
    char header[512];
    int body_len = (int)strlen(body);
    snprintf(header, sizeof(header),
        "HTTP/1.1 %d OK\r\n"
        "Content-Type: %s; charset=utf-8\r\n"
        "Content-Length: %d\r\n"
        "Connection: close\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "\r\n",
        code, mime, body_len);
    write(fd, header, strlen(header));
    write(fd, body, body_len);
}

/* ------------------------------------------------------------------ */
/* Tiny JSON int parser (same as layout.c)                             */
/* ------------------------------------------------------------------ */
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

/* ------------------------------------------------------------------ */
/* POST /layout — parse JSON, update, save, trigger full refresh       */
/* ------------------------------------------------------------------ */
static void handle_post(int fd, const char *body) {
    Layout tmp = g_layout;
    #define S(k, f) tmp.f = json_get_int(body, k, tmp.f)
    S("time_x", time_x); S("time_y", time_y);
    S("date_x", date_x); S("date_y", date_y);
    S("w_label_x", w_label_x);
    S("w_label_y0", w_label_y[0]); S("w_label_y1", w_label_y[1]);
    S("w_label_y2", w_label_y[2]); S("w_label_y3", w_label_y[3]);
    S("w_data_x", w_data_x);
    S("w_data_y0", w_data_y[0]); S("w_data_y1", w_data_y[1]);
    S("w_data_y2", w_data_y[2]); S("w_data_y3", w_data_y[3]);
    S("w_upd_x", w_upd_x); S("w_upd_y", w_upd_y);
    S("bat_x", bat_x); S("bat_y", bat_y);
    S("bat_frame_x", bat_frame_x); S("bat_frame_y", bat_frame_y);
    S("bat_frame_w", bat_frame_w); S("bat_frame_h", bat_frame_h);
    S("ip_x", ip_x); S("ip_y", ip_y);
    S("bar_y", bar_y); S("bar_h", bar_h);
    #undef S

    g_layout = tmp;
    layout_save(&g_layout);
    g_layout_refresh = 1;

    char json[2048];
    build_json(json, sizeof(json));
    send_response(fd, 200, "application/json", json);
}

/* ------------------------------------------------------------------ */
/* Thread entry point — accept loop.                                   */
/* ------------------------------------------------------------------ */
void api_server_start(void) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("API: socket");
        return;
    }

    int opt = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr = {
        .sin_family      = AF_INET,
        .sin_addr.s_addr = INADDR_ANY,
        .sin_port        = htons(API_PORT),
    };

    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("API: bind");
        close(sock);
        return;
    }

    if (listen(sock, API_BACKLOG) < 0) {
        perror("API: listen");
        close(sock);
        return;
    }

    printf("API server listening on port %d\n", API_PORT);

    char buf[API_BUF_SIZE];

    while (1) {
        struct sockaddr_in client;
        socklen_t clen = sizeof(client);
        int client_fd = accept(sock, (struct sockaddr *)&client, &clen);
        if (client_fd < 0) {
            perror("API: accept");
            continue;
        }

        ssize_t n = read(client_fd, buf, API_BUF_SIZE - 1);
        if (n > 0) {
            buf[n] = '\0';

            /* Routing */
            if (strncmp(buf, "POST /layout", 12) == 0) {
                /* Extract body (after \r\n\r\n) */
                char *body = strstr(buf, "\r\n\r\n");
                handle_post(client_fd, body ? body + 4 : "{}");
            } else if (strncmp(buf, "GET / ", 6) == 0 || strncmp(buf, "GET /\r\n", 7) == 0) {
                char json[2048];
                build_json(json, sizeof(json));
                send_response(client_fd, 200, "application/json", json);
            } else {
                send_response(client_fd, 404, "text/plain", "{\"error\":\"not found\"}");
            }
        }

        close(client_fd);
    }

    /* Never reached */
    close(sock);
}
