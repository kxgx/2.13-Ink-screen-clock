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
#include <dirent.h>

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
        "\"time\":{\"text\":\"%s\",\"x\":%d,\"y\":%d,\"pt\":%d,\"font\":\"%s\"},"
        "\"date\":{\"text\":\"%s\",\"x\":%d,\"y\":%d,\"pt\":%d},"
        "\"weather\":{"
            "\"desc\":{\"text\":\"%s\",\"x\":%d,\"y\":%d},"
            "\"temp\":{\"text\":\"%s\",\"x\":%d,\"y\":%d},"
            "\"humidity\":{\"text\":\"%s\",\"x\":%d,\"y\":%d},"
            "\"city\":{\"text\":\"%s\",\"x\":%d,\"y\":%d},"
            "\"updated\":{\"text\":\"%s\",\"x\":%d,\"y\":%d},"
            "\"labels\":[\"天气:\",\"温度:\",\"湿度:\",\"城市:\"],"
            "\"label_x\":[%d,%d,%d,%d],\"label_y\":[%d,%d,%d,%d],"
            "\"pt\":%d"
        "},"
        "\"battery\":{\"text\":\"%s\",\"x\":%d,\"y\":%d,\"pt\":%d,"
            "\"frame\":{\"x\":%d,\"y\":%d,\"w\":%d,\"h\":%d}},"
        "\"ip\":{\"text\":\"%s\",\"x\":%d,\"y\":%d,\"pt\":%d},"
        "\"bar\":{\"y\":%d,\"h\":%d},"
        "\"font_cn\":\"%s\",\"font_time\":\"%s\","
        "\"%s\":%s"
        "}",
        l->screen_w, l->screen_h,
        cached_time, l->time_x, l->time_y, l->time_pt, l->font_time,
        esc_date, l->date_x, l->date_y, l->date_pt,
        esc_w, l->w_data_x[0], l->w_data_y[0],
        esc_t, l->w_data_x[1], l->w_data_y[1],
        esc_h, l->w_data_x[2], l->w_data_y[2],
        esc_c, l->w_data_x[3], l->w_data_y[3],
        cached_weather_u, l->w_upd_x, l->w_upd_y,
        l->w_label_x[0], l->w_label_x[1], l->w_label_x[2], l->w_label_x[3],
        l->w_label_y[0], l->w_label_y[1], l->w_label_y[2], l->w_label_y[3],
        l->weather_pt,
        cached_power, l->bat_x, l->bat_y, l->small_pt,
        l->bat_frame_x, l->bat_frame_y, l->bat_frame_w, l->bat_frame_h,
        cached_ip, l->ip_x, l->ip_y, l->ip_pt,
        l->bar_y, l->bar_h,
        l->font_cn, l->font_time,
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
    S("w_label_x0", w_label_x[0]); S("w_label_x1", w_label_x[1]);
    S("w_label_x2", w_label_x[2]); S("w_label_x3", w_label_x[3]);
    S("w_label_y0", w_label_y[0]); S("w_label_y1", w_label_y[1]);
    S("w_label_y2", w_label_y[2]); S("w_label_y3", w_label_y[3]);
    S("w_data_x0", w_data_x[0]); S("w_data_x1", w_data_x[1]);
    S("w_data_x2", w_data_x[2]); S("w_data_x3", w_data_x[3]);
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
    json_get_str(body, "font_cn",   g_pending.font_cn,   sizeof(g_pending.font_cn),   g_pending.font_cn);
    json_get_str(body, "font_time", g_pending.font_time, sizeof(g_pending.font_time), g_pending.font_time);

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
/* GET /fonts — list available font files in ../pic/                  */
/* ------------------------------------------------------------------ */
static void handle_fonts(int fd) {
    char json[2048];
    int pos = 0;
    pos += snprintf(json + pos, sizeof(json) - pos, "[");

    DIR *d = opendir("../pic");
    if (d) {
        struct dirent *ent;
        int first = 1;
        while ((ent = readdir(d)) != NULL) {
            const char *name = ent->d_name;
            int len = (int)strlen(name);
            /* Only .ttf and .ttc files */
            if (len > 4 && (strcasecmp(name + len - 4, ".ttf") == 0 ||
                            strcasecmp(name + len - 4, ".ttc") == 0)) {
                if (!first) pos += snprintf(json + pos, sizeof(json) - pos, ",");
                first = 0;
                pos += snprintf(json + pos, sizeof(json) - pos, "\"%s\"", name);
            }
        }
        closedir(d);
    }
    pos += snprintf(json + pos, sizeof(json) - pos, "]");
    send_response(fd, 200, "application/json", json);
}

/* ------------------------------------------------------------------ */
/* POST /fonts/upload/filename  — upload a font file (.ttf/.ttc)      */
/* ------------------------------------------------------------------ */
static void handle_upload(int fd, const char *req) {
    char fname[128] = {0};

    /* Extract filename from URL: POST /fonts/upload/filename.ttf */
    const char *path_start = strstr(req, "/fonts/upload/");
    if (!path_start) { send_response(fd, 400, "application/json", "{\"error\":\"missing filename\"}"); return; }
    path_start += 14;  /* skip "/fonts/upload/" */
    const char *path_end = strchr(path_start, ' ');
    if (!path_end) { send_response(fd, 400, "application/json", "{\"error\":\"bad request\"}"); return; }
    int name_len = path_end - path_start;
    if (name_len < 1 || name_len > 120) { send_response(fd, 400, "application/json", "{\"error\":\"filename too long\"}"); return; }
    memcpy(fname, path_start, name_len);

    /* Validate extension: .ttf or .ttc only */
    int elen = (int)strlen(fname);
    if (elen < 5) { send_response(fd, 400, "application/json", "{\"error\":\"invalid filename\"}"); return; }
    const char *ext = fname + elen - 4;
    if (strcasecmp(ext, ".ttf") != 0 && strcasecmp(ext, ".ttc") != 0) {
        send_response(fd, 400, "application/json", "{\"error\":\"only .ttf and .ttc allowed\"}"); return;
    }

    /* Parse Content-Length from headers */
    const char *cl = strstr(req, "Content-Length:");
    if (!cl) cl = strstr(req, "content-length:");
    int content_len = 0;
    if (cl) {
        cl += 15;  /* skip "Content-Length:" */
        while (*cl == ' ' || *cl == '\t') cl++;
        while (*cl >= '0' && *cl <= '9') { content_len = content_len * 10 + (*cl - '0'); cl++; }
    }
    if (content_len <= 0 || content_len > 20 * 1024 * 1024) {  /* max 20MB */
        send_response(fd, 400, "application/json", "{\"error\":\"invalid content length\"}"); return;
    }

    /* Find body start (after \r\n\r\n) in what we've already read */
    const char *body_start = strstr(req, "\r\n\r\n");
    int already_read = 0;
    if (body_start) {
        body_start += 4;
        already_read = (int)(req + strlen(req) - body_start);
        if (already_read < 0) already_read = 0;
        if (already_read > content_len) already_read = content_len;
    }

    /* Save to ../pic/ */
    char fpath[256];
    snprintf(fpath, sizeof(fpath), "../pic/%s", fname);
    FILE *fp = fopen(fpath, "wb");
    if (!fp) { send_response(fd, 500, "application/json", "{\"error\":\"cannot create file\"}"); return; }

    size_t total = 0;
    /* Write any bytes already in the buffer after \r\n\r\n */
    if (already_read > 0) {
        total += fwrite(body_start, 1, already_read, fp);
    }

    /* Read remaining body from socket in chunks */
    char chunk[8192];
    while (total < (size_t)content_len) {
        size_t remain = (size_t)content_len - total;
        size_t to_read = remain < sizeof(chunk) ? remain : sizeof(chunk);
        ssize_t nr = read(fd, chunk, to_read);
        if (nr <= 0) break;
        total += fwrite(chunk, 1, nr, fp);
    }
    fclose(fp);

    char json[256];
    snprintf(json, sizeof(json), "{\"ok\":true,\"name\":\"%s\",\"size\":%zu}", fname, total);
    send_response(fd, 200, "application/json", json);
    printf("  Font uploaded: %s (%zu bytes)\n", fname, total);
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
            if (strncmp(buf, "OPTIONS ", 8) == 0) {
                /* CORS preflight */
                const char *rsp =
                    "HTTP/1.1 204 No Content\r\n"
                    "Access-Control-Allow-Origin: *\r\n"
                    "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
                    "Access-Control-Allow-Headers: Content-Type, Content-Length\r\n"
                    "Access-Control-Max-Age: 86400\r\n"
                    "Connection: close\r\n"
                    "\r\n";
                write(client_fd, rsp, strlen(rsp));
            } else if (strncmp(buf, "POST /fonts/upload/", 19) == 0) {
                handle_upload(client_fd, buf);
            } else if (strncmp(buf, "POST /layout/apply", 18) == 0) {
                handle_apply(client_fd);
            } else if (strncmp(buf, "POST /layout/cancel", 19) == 0) {
                handle_cancel(client_fd);
            } else if (strncmp(buf, "POST /layout", 12) == 0) {
                char *body = strstr(buf, "\r\n\r\n");
                handle_preview(client_fd, body ? body + 4 : "{}");
            } else if (strncmp(buf, "GET /fonts", 10) == 0) {
                handle_fonts(client_fd);
            } else if (strncmp(buf, "GET /layout.html", 16) == 0) {
                send_file(client_fd, "../clock/layout.html", "text/html; charset=utf-8");
            } else if (strncmp(buf, "GET /pic/", 9) == 0) {
                /* Serve any font file from pic/ */
                char *start = strstr(buf, "/pic/") + 5;
                char *end = strchr(start, ' ');
                if (end) *end = '\0';
                char fpath[256];
                snprintf(fpath, sizeof(fpath), "../pic/%s", start);
                send_file(client_fd, fpath, "font/ttf");
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
