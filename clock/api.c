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
        "\"time\":\"%s\","
        "\"date\":\"%s\","
        "\"weather\":{"
            "\"desc\":\"%s\","
            "\"temp\":\"%s\","
            "\"humidity\":\"%s\","
            "\"city\":\"%s\","
            "\"updated\":\"%s\""
        "},"
        "\"battery\":\"%s\","
        "\"ip\":\"%s\""
        "}",
        cached_time, esc_date,
        esc_w, esc_t, esc_h, esc_c, cached_weather_u,
        cached_power, cached_ip);

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
