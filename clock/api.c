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
        "\"battery\":{\"text\":\"%s\",\"x\":%d,\"y\":%d,\"pt\":%d,\"frame\":{\"x\":%d,\"y\":%d,\"w\":%d,\"h\":%d}},"
        "\"ip\":{\"text\":\"%s\",\"x\":%d,\"y\":%d,\"pt\":%d}"
        "}",
        250, 122,
        cached_time, -1, 40, 40,
        esc_date, 2, 2, 14,
        esc_w, 195, 25, esc_t, 195, 45, esc_h, 195, 65, esc_c, 195, 85,
        cached_weather_u, 211, 109, 14,
        cached_power, 128, 108, 10, 126, 108, 29, 11,
        cached_ip, 10, 109, 13);

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

            /* Extremely rough routing: match "GET /" (or "GET / ") */
            if (strncmp(buf, "GET / ", 6) == 0 || strncmp(buf, "GET /\r\n", 7) == 0) {
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
