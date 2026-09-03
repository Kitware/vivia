// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vgCalendarUtils_h
#define __vgCalendarUtils_h

#include <cstdint>

namespace vgCalendar
{

struct Date
{
  int Year;
  int Month; // 1 - 12
  int Day;   // 1 - 31
};

//-----------------------------------------------------------------------------
// Howard Hinnant's civil calendar algorithms; exact for any year of the
// proleptic Gregorian calendar.
inline int64_t daysFromCivil(const Date& date)
{
  const int64_t y = date.Year - (date.Month <= 2);
  const int64_t era = (y >= 0 ? y : y - 399) / 400;
  const int64_t yoe = y - era * 400;
  const int64_t doy =
    (153 * (date.Month + (date.Month > 2 ? -3 : 9)) + 2) / 5 + date.Day - 1;
  const int64_t doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
  return era * 146097 + doe - 719468;
}

//-----------------------------------------------------------------------------
inline Date civilFromDays(int64_t days)
{
  const int64_t z = days + 719468;
  const int64_t era = (z >= 0 ? z : z - 146096) / 146097;
  const int64_t doe = z - era * 146097;
  const int64_t yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
  const int64_t doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
  const int64_t mp = (5 * doy + 2) / 153;
  const int64_t d = doy - (153 * mp + 2) / 5 + 1;
  const int64_t m = mp + (mp < 10 ? 3 : -9);

  Date date;
  date.Year = static_cast<int>(yoe + era * 400 + (m <= 2));
  date.Month = static_cast<int>(m);
  date.Day = static_cast<int>(d);
  return date;
}

//-----------------------------------------------------------------------------
inline const char* monthAbbreviation(int month)
{
  static const char* const names[] =
    {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
    };
  return (month >= 1 && month <= 12 ? names[month - 1] : "");
}

} // namespace vgCalendar

#endif
