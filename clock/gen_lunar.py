#!/usr/bin/env python3
"""Generate compact lunar calendar data table for 1900-2100."""

from borax.calendars.lunardate import LunarDate
from datetime import date, timedelta

START_YEAR = 1900
END_YEAR = 2100

# Encoding per year (unsigned int):
#   bits 0-3:  leap month number (0=none)
#   bits 4-15: months 1-12, 1=big(30d) 0=small(29d)
#   bit 16:    leap month size (0=29d, 1=30d)

data = []
for year in range(START_YEAR, END_YEAR + 1):
    # Get Chinese New Year date for this year
    cny = LunarDate(year, 1, 1).to_solar_date()
    if year < END_YEAR:
        next_cny = LunarDate(year + 1, 1, 1).to_solar_date()
        days_in_year = (next_cny - cny).days
    else:
        # Last year: sum all month days + leap if any
        days_in_year = sum(LunarDate.last_day(year, m, False).day for m in range(1, 13))
        for m in range(1, 13):
            try:
                last = LunarDate.last_day(year, m, True)
                if last:
                    days_in_year += last.day
                    break
            except:
                pass

    # Find leap month by scanning day by day
    leap_month = 0
    leap_size = 0
    for i in range(days_in_year):
        d = cny + timedelta(days=i)
        ld = LunarDate.from_solar_date(d.year, d.month, d.day)
        if ld.leap:
            leap_month = ld.month
            last = LunarDate.last_day(year, ld.month, True)
            leap_size = 30 if last.day == 30 else 29
            break

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
    total = 12
    if leap_month:
        total += 1
    print(f"  0x{encoded:06x},  // {year} 闰{leap_month or '无'}月({leap_size or '-'}d) {total}个月 {days_in_year}天")

print(f"// Total: {len(data)} entries (1900-2100)")
