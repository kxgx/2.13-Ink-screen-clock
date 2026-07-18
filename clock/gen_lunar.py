#!/usr/bin/env python3
"""Generate compact lunar calendar data table for 1900-2100."""

from borax.calendars.lunardate import LunarDate

START_YEAR = 1900
END_YEAR = 2100

# Encoding per year (unsigned int):
#   bits 0-3:  leap month number (0=none)
#   bits 4-15: months 1-12, 1=big(30d) 0=small(29d)
#   bit 16:    leap month size (0=29d, 1=30d)

data = []
for year in range(START_YEAR, END_YEAR + 1):
    # Try each month with leap=True to detect leap month
    leap_month = 0
    leap_size = 0
    for m in range(1, 13):
        try:
            ld = LunarDate(year, m, 1, True)
            leap_month = m
            last = LunarDate.last_day(year, m, True)
            leap_size = 30 if last.day == 30 else 29
            break  # only one leap month per year
        except:
            continue

    # Build regular month bitmap
    bitmap = 0
    for m in range(1, 13):
        last = LunarDate.last_day(year, m, False)
        if last.day == 30:
            bitmap |= (1 << (3 + m))

    encoded = bitmap | (leap_month & 0xF)
    if leap_month and leap_size == 30:
        encoded |= (1 << 16)

    data.append(encoded)
    print(f"  0x{encoded:06x},  // {year} 闰{leap_month or '无'}月({leap_size or '-'}d)")

print(f"// Total: {len(data)} entries (1900-2100)")
