/**
 * @file    layout.h
 * @brief   Persistent layout configuration — all display-element positions
 */

#ifndef LAYOUT_H
#define LAYOUT_H

/* One flat struct so the API can GET/PUT everything atomically. */
typedef struct {
    /* screen */
    int screen_w, screen_h;     /* read-only, set at init */

    /* time (DSEG font, pt=40) */
    int time_x, time_y;

    /* date (pt=14) */
    int date_x, date_y;

    /* weather labels */
    int w_label_x[4];
    int w_label_y[4];

    /* weather data */
    int w_data_x[4];
    int w_data_y[4];

    /* weather update time (pt=13) */
    int w_upd_x, w_upd_y;

    /* battery percentage (pt=10, inside frame) */
    int bat_x, bat_y;
    int bat_frame_x, bat_frame_y, bat_frame_w, bat_frame_h;

    /* IP address (pt=13) */
    int ip_x, ip_y;

    /* black bottom bar */
    int bar_y, bar_h;

    /* font sizes (pt) */
    int time_pt;       /* DSEG, default 40 */
    int date_pt;       /* default 14 */
    int weather_pt;    /* default 14 */
    int small_pt;      /* battery, default 10 */
    int ip_pt;         /* default 13 */

    /* font file paths (relative to pic/ directory) */
    char font_cn[128];    /* Chinese font, default "Font.ttc" */
    char font_time[128];  /* time font, default "DSEG7Modern-Bold.ttf" */
} Layout;

/* Filename for persistent storage (relative to CWD = clock/) */
#define LAYOUT_FILE  "layout.json"

/* Load layout from file, or initialise with defaults.  Returns 0 on
 * success, -1 if file is missing/broken (defaults are still set). */
int layout_init(Layout *l);

/* Save current layout to file.  Returns 0 on success. */
int layout_save(const Layout *l);

#endif /* LAYOUT_H */
