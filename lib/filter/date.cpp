/*
 Copyright (©) 2003-2015 Teus Benschop.
 
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 3 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */


#include <filter/date.h>
#include <database/config/general.h>


// Gets the second within the minute from the seconds since the Unix epoch.
int filter_string_date_numerical_second (int seconds)
{
  time_t tt = seconds;
  tm utc_tm = *gmtime(&tt);
  int second = utc_tm.tm_sec;
  return second;
}


// Gets the minute within the hour from the seconds since the Unix epoch.
int filter_string_date_numerical_minute (int seconds)
{
  time_t tt = seconds;
  tm utc_tm = *gmtime(&tt);
  int minute = utc_tm.tm_min;
  return minute;
}


// Gets the hour within the day from the seconds since the Unix epoch.
int filter_string_date_numerical_hour (int seconds)
{
  time_t tt = seconds;
  tm utc_tm = *gmtime(&tt);
  int hour = utc_tm.tm_hour;
  return hour;
}


// The numerical day of the month from 1 to 31.
int filter_string_date_numerical_month_day (int seconds)
{
  time_t tt = seconds;
  tm utc_tm = *gmtime(&tt);
  int day = utc_tm.tm_mday;
  return day;
}


// The numerical day of the week: 0 (for Sunday) through 6 (for Saturday)
int filter_string_date_numerical_week_day (int seconds) // Todo
{
  time_t tt = seconds;
  tm utc_tm = *gmtime(&tt);
  int day = utc_tm.tm_wday;
  return day;
}


// A C++ equivalent for PHP's date ("n") function.
// Numeric representation of a month: 1 through 12.
int filter_string_date_numerical_month ()
{
  auto now = chrono::system_clock::now ();
  time_t tt = chrono::system_clock::to_time_t (now);
#ifdef WIN32
  tm utc_tm;
  gmtime_s(&utc_tm, &tt);
#else
  tm utc_tm = *gmtime(&tt);
#endif
  int month = utc_tm.tm_mon + 1;
  return month;
}


// A C++ equivalent for PHP's date ("Y") function.
// A full numeric representation of a year, 4 digits: 2014.
int filter_string_date_numerical_year ()
{
  auto now = chrono::system_clock::now ();
  time_t tt = chrono::system_clock::to_time_t (now);
#ifdef WIN32
  tm utc_tm;
  gmtime_s(&utc_tm, &tt);
#else
  tm utc_tm = *gmtime(&tt);
#endif
  // Get years since 1900, and correct to get years since birth of Christ.
  int year = utc_tm.tm_year + 1900;
  return year;
}


// This function gives the number of microseconds within the current second.
int filter_string_date_numerical_microseconds ()
{
  auto now = chrono::system_clock::now ();
  auto duration = now.time_since_epoch ();
  auto microseconds = chrono::duration_cast<std::chrono::microseconds>(duration).count();
  int usecs = microseconds % 1000000;
  return usecs;
}


// This function returns the seconds since the Unix epoch, which is 1 January 1970 UTC.
int filter_string_date_seconds_since_epoch ()
{
  auto now = chrono::system_clock::now ();
  auto duration = now.time_since_epoch ();
  int seconds = (int) chrono::duration_cast<std::chrono::seconds>(duration).count();
  return seconds;
}


// This function takes the "seconds" parameter,
// corrects it according to the local timezone,
// and returns it.
int filter_string_date_local_seconds (int seconds)
{
  int offset = Database_Config_General::getTimezone ();
  seconds += (offset * 3600);
  return seconds;
}


bool filter_date_is_first_working_day_of_month (int monthday, int weekday) // Todo test.
{
  if (monthday == 1) {
    if (weekday == 1) return true;
    if (weekday == 2) return true;
    if (weekday == 3) return true;
    if (weekday == 4) return true;
    if (weekday == 5) return true;
  }
  if (weekday == 1) {
    if (monthday == 2) return true;
    if (monthday == 3) return true;
  }
  return false;
}


int filter_date_get_last_business_day_of_month (int year, int month) // Todo test.
{
  /* Todo
  $time = mktime (0, 0, 0, $month, 1, $year);
  $lastday = date ("t", $time);
  $time = mktime (0, 0, 0, $month, $lastday, $year);
  $lastweekday = date ("w", $time);
  if ($lastweekday == 0) $lastday -= 2;
  if ($lastweekday == 6) $lastday--;
  return $lastday;
   */
  return 30;
}


bool filter_date_is_business_day (int year, int month, int day) // Todo test.
{
  /* Todo
  $time = mktime (0, 0, 0, $month, $day, $year);
  $weekday = date ("w", $time);
  return in_array ($weekday, array (1, 2, 3, 4, 5));
   */
  return false;
}

