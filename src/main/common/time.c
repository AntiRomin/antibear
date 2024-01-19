#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "platform.h"

#include "common/time.h"
#include "drivers/rtc.h"

// For the "modulo 4" arithmetic to work, we need a leap base year
#define REFERENCE_YEAR 2000
// Offset (seconds) from the UNIX epoch (1970-01-01) to 2000-01-01
#define EPOCH_2000_OFFSET 946684800
// January 1, 2000 was a saturday
#define BEGIN_WEEK (RTC_WEEKDAY_SATURDAY - 1)

#define MILLIS_PER_SECOND 1000

static const uint16_t dates[4][12] =
{
    {   0,  31,     60,     91,     121,    152,    182,    213,    244,    274,    305,    335},
    { 366,  397,    425,    456,    486,    517,    547,    578,    609,    639,    670,    700},
    { 731,  762,    790,    821,    851,    882,    912,    943,    974,    1004,   1035,   1065},
    {1096,  1127,   1155,   1186,   1216,   1247,   1277,   1308,   1339,   1369,   1400,   1430},
};

static rtcTime_t dateTimeToRtcTime(dateTime_t *dt)
{
    unsigned int second = dt->seconds;  // 0-59
    unsigned int minute = dt->minutes;  // 0-59
    unsigned int hour = dt->hours;      // 0-23
    unsigned int date = dt->date - 1;   // 0-30
    unsigned int month = dt->month - 1; // 0-11
    unsigned int year = dt->year - REFERENCE_YEAR; // 0-99
    int32_t unixTime = (((year / 4 * (365 * 4 + 1) + dates[year % 4][month] + date) * 24 + hour) * 60 + minute) + second + EPOCH_2000_OFFSET;
    return rtcTimeMake(unixTime, dt->millis);
}

static void rtcTimeToDateTime(dateTime_t *dt, rtcTime_t t)
{
    int32_t unixTime = t / MILLIS_PER_SECOND - EPOCH_2000_OFFSET;
    dt->seconds = unixTime % 60;
    unixTime /= 60;
    dt->minutes = unixTime % 60;
    unixTime /= 60;
    dt->hours = unixTime % 24;
    unixTime /= 24;

    dt->wday = ((BEGIN_WEEK + unixTime) % 7) + 1;

    unsigned int years = unixTime / (365 * 4 + 1) * 4;
    unixTime %= 365 * 4 + 1;

    unsigned int year;
    for (year = 3; year > 0; year--) {
        if (unixTime >= dates[year][0]) {
            break;
        }
    }

    unsigned int month;
    for (month = 11; month > 0; month--) {
        if (unixTime > dates[year][month]) {
            break;
        }
    }

    dt->year = years + year + REFERENCE_YEAR;
    dt->month = month + 1;
    dt->date = unixTime - dates[year][month] + 1;
    dt->millis = t % MILLIS_PER_SECOND;
}

static void rtcGetDefaultDateTime(dateTime_t *dateTime)
{
    dateTime->year = RTC_DEFAULT_YEAR;
    dateTime->month = RTC_DEFAULT_MONTH;
    dateTime->date = RTC_DEFAULT_DATE;
    dateTime->wday = RTC_DEFAULT_WEEKDAY;
    dateTime->hours = RTC_DEFAULT_HOUR;
    dateTime->minutes = RTC_DEFAULT_MINUTE;
    dateTime->seconds = RTC_DEFAULT_SECOND;
    dateTime->millis = 0;
}

static bool rtcIsDateTimeValid(dateTime_t *dateTime)
{
    return (dateTime->year >= REFERENCE_YEAR) &&
           (dateTime->month >= 1 && dateTime->month <= 12) &&
           (dateTime->date >= 1 && dateTime->date <= 31) &&
           (dateTime->hours <= 23) &&
           (dateTime->minutes <= 59) &&
           (dateTime->seconds <= 59) &&
           (dateTime->millis <= 999);
}

static void dateTimeWithOffset(dateTime_t *dateTimeOffset, dateTime_t *dateTimeInitial, int16_t minutes)
{
    rtcTime_t initialTime = dateTimeToRtcTime(dateTimeInitial);
    rtcTime_t offsetTime = rtcTimeMake(rtcTimeGetSeconds(&initialTime) + minutes * 60, rtcTimeGetMillis(&initialTime));
    rtcTimeToDateTime(dateTimeOffset, offsetTime);
}

static bool dateTimeFormat(char *buf, dateTime_t *dateTime, int16_t offsetMinutes, bool shortVersion)
{
    dateTime_t local;

    int tz_hours = 0;
    int tz_minutes = 0;
    bool retVal = true;

    // Apply offset if necessary
    if (offsetMinutes != 0) {
        tz_hours = offsetMinutes / 60;
        tz_minutes = abs(offsetMinutes % 60);
        dateTimeWithOffset(&local, dateTime, offsetMinutes);
        dateTime = &local;
    }

    if (!rtcIsDateTimeValid(dateTime)) {
        rtcGetDefaultDateTime(&local);
        dateTime = &local;
        retVal = false;
    }

    if (shortVersion) {
        sprintf(buf, "%04u-%02u-%02u %02u-%02u-%02u",
            dateTime->year, dateTime->month, dateTime->date,
            dateTime->hours, dateTime->minutes, dateTime->seconds);
    } else {
        // Changes to this format might require updates in
        // dateTimeSplitFormatted()
        // Datetime is in ISO_8601 format, https://en.wikipedia.org/wiki/ISO_8061
        sprintf(buf, "%04u-%02u-%02uT%02u:%02u:%02u.%03u%c%02d:%02d",
            dateTime->year, dateTime->month, dateTime->date,
            dateTime->hours, dateTime->minutes, dateTime->seconds, dateTime->millis,
            tz_hours >= 0 ? '+' : '-', abs(tz_hours), tz_minutes);
    }

    return retVal;
}

rtcTime_t rtcTimeMake(int32_t secs, uint16_t millis)
{
    return ((rtcTime_t)secs) * MILLIS_PER_SECOND + millis;
}

int32_t rtcTimeGetSeconds(rtcTime_t *t)
{
    return *t / MILLIS_PER_SECOND;
}

uint16_t rtcTimeGetMillis(rtcTime_t *t)
{
    return *t % MILLIS_PER_SECOND;
}

bool dateTimeFormatUTC(char *buf, dateTime_t *dt)
{
    return dateTimeFormat(buf, dt, 0, false);
}

bool dateTimeFormatLocal(char *buf, dateTime_t *dt)
{
    const int16_t timezoneOffset = rtcIsDateTimeValid(dt) ? rtcGetTimezoneOffsetMinutes() : 0;
    return dateTimeFormat(buf, dt, timezoneOffset, false);
}

bool dateTimeFormatLocalShort(char *buf, dateTime_t *dt)
{
    return dateTimeFormat(buf, dt, rtcGetTimezoneOffsetMinutes(), false);
}

void dateTimeUTCToLocal(dateTime_t *utcDateTime, dateTime_t *localDateTime)
{
    dateTimeWithOffset(localDateTime, utcDateTime, rtcGetTimezoneOffsetMinutes());
}

bool dateTimeSplitFormatted(char *formatted, char **date, char**time)
{
    // Just look for the T and replace it with a zero
    // Keep in sync with dateTimeFormat()
    for (char *p = formatted; *p; p++) {
        if (*p == 'T') {
            *date = formatted;
            *time = (p+1);
            *p = '\0';
            return true;
        }
    }
    return false;
}

bool rtcGet(rtcTime_t *t)
{
    dateTime_t dateTime = {0};

    if (!rtcGetRaw(&dateTime)) {
        return false;
    }

    dateTime.year += REFERENCE_YEAR;

    *t = dateTimeToRtcTime(&dateTime);
    return true;
}

bool rtcSet(rtcTime_t *t)
{
    dateTime_t dateTime = {0};
    
    rtcTimeToDateTime(&dateTime, *t);

    dateTime.year -= REFERENCE_YEAR;
    if (!rtcSetRaw(&dateTime)) {
        return false;
    }

    return true;
}

bool rtcGetDateTime(dateTime_t *dt)
{
    if (!rtcGetRaw(dt)) {
        rtcGetDefaultDateTime(dt);
        return false;
    }

    dt->year += REFERENCE_YEAR;
    return true;
}

bool rtcSetDateTime(dateTime_t *dt)
{
    dt->year -= REFERENCE_YEAR;
    return rtcSetRaw(dt);
}