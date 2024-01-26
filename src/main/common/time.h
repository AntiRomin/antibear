#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "platform.h"

// time difference, 32 bits always sufficient
typedef int32_t timeDelta_t;
// millisecond time
typedef uint32_t timeMs_t ;
// microsecond time
#ifdef USE_64BIT_TIME
typedef uint64_t timeUs_t;
#define TIMEUS_MAX UINT64_MAX
#else
typedef uint32_t timeUs_t;
#define TIMEUS_MAX UINT32_MAX
#endif

#define TIMEZONE_OFFSET_MINUTES_MIN -780    // -13 hours
#define TIMEZONE_OFFSET_MINUTES_MAX 780     // +13 hours

// Milliseconds since Jan 1 1970
typedef int64_t rtcTime_t;

typedef struct dateTime_s {
    // full year
    uint16_t year;
    // 1-12
    uint8_t month;
    // 1-31
    uint8_t date;
    // 1-7
    uint8_t wday;
    // 0-23
    uint8_t hours;
    // 0-59
    uint8_t minutes;
    // 0-59
    uint8_t seconds;
    // 0-999
    uint16_t millis;
} dateTime_t;

#define FORMATTED_DATE_TIME_BUFSIZE 30

#define RTC_DEFAULT_HOUR            0
#define RTC_DEFAULT_MINUTE          0
#define RTC_DEFAULT_SECOND          0
#define RTC_DEFAULT_WEEKDAY         RTC_WEEKDAY_SATURDAY
#define RTC_DEFAULT_MONTH           RTC_MONTH_JANUARY
#define RTC_DEFAULT_DATE            1
#define RTC_DEFAULT_YEAR            24  // (2024 - 2000)

rtcTime_t rtcTimeMake(int32_t secs, uint16_t millis);
int32_t rtcTimeGetSeconds(rtcTime_t *t);
uint16_t rtcTimeGetMillis(rtcTime_t *t);

// buf must be at least FORMATTED_DATE_TIME_BUFSIZE
bool dateTimeFormatUTC(char *buf, dateTime_t *dt);
bool dateTimeFormatLocal(char *buf, dateTime_t *dt);
bool dateTimeFormatLocalShort(char *buf, dateTime_t *dt);

void dateTimeUTCToLocal(dateTime_t *utcDateTime, dateTime_t *localDateTime);

// dateTimeSplitFormatted splits a formatted date into its date
// and time parts. Note that the string pointed by formatted will
// be modified and will become invalid after calling this function.
bool dateTimeSplitFormatted(char *formatted, char **date, char **time);

bool rtcHasTime(void);

bool rtcGet(rtcTime_t *t);
bool rtcSet(rtcTime_t *t);

bool rtcGetDateTime(dateTime_t *dt);
bool rtcSetDateTime(dateTime_t *dt);

typedef enum {
    RTC_TIMEZONE_UTC_N12 = -120,
    RTC_TIMEZONE_UTC_N11 = -110,
    RTC_TIMEZONE_UTC_N10 = -100,
    RTC_TIMEZONE_UTC_N9 = -90,
    RTC_TIMEZONE_UTC_N8 = -80,
    RTC_TIMEZONE_UTC_N7 = -70,
    RTC_TIMEZONE_UTC_N6 = -60,
    RTC_TIMEZONE_UTC_N5 = -50,
    RTC_TIMEZONE_UTC_N4 = -40,
    RTC_TIMEZONE_UTC_N3 = -30,
    RTC_TIMEZONE_UTC_N2 = -20,
    RTC_TIMEZONE_UTC_N1 = -10,
    RTC_TIMEZONE_UTC_BASE = 0,
    RTC_TIMEZONE_UTC_P1 = 10,
    RTC_TIMEZONE_UTC_P2 = 20,
    RTC_TIMEZONE_UTC_P3 = 30,
    RTC_TIMEZONE_UTC_P35 = 35,
    RTC_TIMEZONE_UTC_P4 = 40,
    RTC_TIMEZONE_UTC_P5 = 50,
    RTC_TIMEZONE_UTC_P55 = 55,
    RTC_TIMEZONE_UTC_P6 = 60,
    RTC_TIMEZONE_UTC_P7 = 70,
    RTC_TIMEZONE_UTC_P8 = 80,
    RTC_TIMEZONE_UTC_P9 = 90,
    RTC_TIMEZONE_UTC_P10 = 100,
    RTC_TIMEZONE_UTC_P11 = 110,
    RTC_TIMEZONE_UTC_P12 = 120,
} RTC_TIMEZONE_UTC_e;

bool rtcSetTimezone(RTC_TIMEZONE_UTC_e *tz);
bool rtcGetTimezone(RTC_TIMEZONE_UTC_e *tz);