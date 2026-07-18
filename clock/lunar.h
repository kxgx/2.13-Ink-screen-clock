/**
 * @file    lunar.h
 * @brief   Pure C lunar calendar — zero external dependencies
 *
 * Computes Chinese lunar date from Gregorian date using a compact
 * encoding table (1900–2100).  ~800 bytes of read-only data.
 */

#ifndef LUNAR_H
#define LUNAR_H

/** Result of a lunar-date calculation.  All fields 1‑based. */
typedef struct {
    int year;            /* lunar year  (1900..2100)               */
    int month;           /* lunar month (1..12)                   */
    int day;             /* lunar day   (1..30)                   */
    int leap;            /* 1 if the month is a leap month         */
    int days_in_year;    /* total days in this lunar year          */
} LunarDate;

/* ----------------------------------------------------------------
 * Public API
 * ---------------------------------------------------------------- */

/** Convert a Gregorian date (year, month 1-12, day 1-31)
 *  to a Chinese lunar date.
 *  Returns 0 on success, -1 if the year is out of range (1900-2100).
 */
int lunar_from_solar(int g_year, int g_month, int g_day, LunarDate *out);

/** Format a LunarDate into a human-readable Chinese string.
 *  Example output: "农历正月初一" / "农历闰二月十五"
 *  Caller supplies buf (≥ 32 bytes recommended).  Returns buf.
 */
const char *lunar_format(const LunarDate *ld, char *buf, int bufsize);

#endif /* LUNAR_H */
