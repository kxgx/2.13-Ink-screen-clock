/**
 * @file    api.c
 * @brief   Minimal HTTP API server (no external library)
 *
 * Listens on port 8080, serves JSON API and static files.
 * Routes:
 *   GET  /               JSON data
 *   GET  /layout.html    Layout control panel
 *   GET  /pic/...        Static files (fonts)
 *   POST /layout         Preview layout changes
 *   POST /layout/apply   Commit layout changes
 *   POST /layout/cancel  Discard pending changes
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
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

/* Pending layout — POST /layout writes here, /layout/apply commits */
static Layout g_pending;
static int g_has_pending = 0;  /* 1 = there are unapplied changes */

/* ================================================================== */
/* Get current layout (active or pending) for GET response             */
/* ================================================================== */
static Layout *current_layout(void) {
    return g_has_pending ? &g_pending : &g_layout;
}

/* ------------------------------------------------------------------ */
/* Build the JSON payload.  All strings are already UTF‑8 on RPi.     */
/* ------------------------------------------------------------------ */
static void build_json(char *buf, int bufsize, int show_pending) {
    Layout *l = current_layout();
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
        "\"bar\":{\"y\":%d,\"h\":%d},"
        "\"%s\":%s"
        "}",
        l->screen_w, l->screen_h,
        cached_time, l->time_x, l->time_y, l->time_pt,
        esc_date, l->date_x, l->date_y, l->date_pt,
        esc_w, l->w_data_x, l->w_data_y[0],
        esc_t, l->w_data_x, l->w_data_y[1],
        esc_h, l->w_data_x, l->w_data_y[2],
        esc_c, l->w_data_x, l->w_data_y[3],
        cached_weather_u, l->w_upd_x, l->w_upd_y, l->weather_pt,
        cached_power, l->bat_x, l->bat_y, l->small_pt,
        l->bat_frame_x, l->bat_frame_y, l->bat_frame_w, l->bat_frame_h,
        cached_ip, l->ip_x, l->ip_y, l->ip_pt,
        l->bar_y, l->bar_h,
        show_pending ? "pending" : "active", show_pending ? "true" : "false");

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

/* Serve a static file from disk (CWD = clock/) */
static void send_file(int fd, const char *path, const char *mime) {
    FILE *fp = fopen(path, "rb");
    if (!fp) { send_response(fd, 404, "text/plain", "{\"error\":\"not found\"}"); return; }

    struct stat st;
    fstat(fileno(fp), &st);

    char header[512];
    snprintf(header, sizeof(header),
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %ld\r\n"
        "Connection: close\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "\r\n",
        mime, (long)st.st_size);
    write(fd, header, strlen(header));

    char buf[4096];
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), fp)) > 0)
        write(fd, buf, n);
    fclose(fp);
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
/* POST /layout — preview: store in pending                          */
/* ------------------------------------------------------------------ */
static void handle_preview(int fd, const char *body) {
    if (!g_has_pending) {
        g_pending = g_layout;  /* start from current active */
        g_has_pending = 1;
    }
    #define S(k, f) g_pending.f = json_get_int(body, k, g_pending.f)
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
    S("time_pt", time_pt); S("date_pt", date_pt);
    S("weather_pt", weather_pt); S("small_pt", small_pt); S("ip_pt", ip_pt);
    #undef S

    char json[2048];
    build_json(json, sizeof(json), 1);
    send_response(fd, 200, "application/json", json);
}

/* ------------------------------------------------------------------ */
/* POST /layout/apply — commit pending to active, save, refresh       */
/* ------------------------------------------------------------------ */
static void handle_apply(int fd) {
    if (!g_has_pending) {
        send_response(fd, 200, "application/json", "{\"status\":\"no pending changes\"}");
        return;
    }
    g_layout = g_pending;
    layout_save(&g_layout);
    g_has_pending = 0;
    g_layout_refresh = 1;

    char json[2048];
    build_json(json, sizeof(json), 0);
    send_response(fd, 200, "application/json", json);
}

/* ------------------------------------------------------------------ */
/* POST /layout/cancel — discard pending, revert to active            */
/* ------------------------------------------------------------------ */
static void handle_cancel(int fd) {
    g_has_pending = 0;
    char json[2048];
    build_json(json, sizeof(json), 0);
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
            if (strncmp(buf, "POST /layout/apply", 18) == 0) {
                handle_apply(client_fd);
            } else if (strncmp(buf, "POST /layout/cancel", 19) == 0) {
                handle_cancel(client_fd);
            } else if (strncmp(buf, "POST /layout", 12) == 0) {
                char *body = strstr(buf, "\r\n\r\n");
                handle_preview(client_fd, body ? body + 4 : "{}");
            } else if (strncmp(buf, "GET /layout.html", 16) == 0) {
                send_file(client_fd, "../clock/layout.html", "text/html; charset=utf-8");
            } else if (strncmp(buf, "GET /pic/", 9) == 0) {
                /* Extract path after /pic/, only allow DSEG font */
                if (strstr(buf, "DSEG7Modern-Bold.ttf"))
                    send_file(client_fd, "../pic/DSEG7Modern-Bold.ttf", "font/ttf");
                else
                    send_response(client_fd, 404, "text/plain", "{\"error\":\"not found\"}");
            } else if (strncmp(buf, "GET / ", 6) == 0 || strncmp(buf, "GET /\r\n", 7) == 0) {
                char json[2048];
                build_json(json, sizeof(json), g_has_pending);
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
