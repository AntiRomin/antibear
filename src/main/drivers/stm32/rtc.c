#include <stdint.h>

#include "platform.h"
#include "common/time.h"
#include "drivers/rtc.h"

RTC_HandleTypeDef hrtc;

#define RTC_MAGICNUMBER         0x524D524D

#define RTC_ASYNCHPREDIV        31
#define RTC_SYNCHPREDIV         1023

#define RTC_DEFAULT_TIMEZONE    RTC_TIMEZONE_UTC_P8

static bool isStarted = false;
static RTC_TIMEZONE_UTC_e timezone = RTC_TIMEZONE_UTC_BASE;

static void rtcHardwareInit(void)
{
    /*##-1- Enable peripherals Clocks ##########################################*/
    /* Enable the RTC clock */
    __HAL_RCC_RTC_ENABLE();

    /*##-2- RTC parameter configuration ########################################*/
    hrtc.Instance = RTC;
    HAL_RTC_WaitForSynchro(&hrtc);

    hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
    hrtc.Init.AsynchPrediv = RTC_ASYNCHPREDIV;
    hrtc.Init.SynchPrediv = RTC_SYNCHPREDIV;
    hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
    HAL_RTC_Init(&hrtc);

    isStarted = true;
}

static void rtcTimerInit(void)
{
    RTC_TimeTypeDef sTime = {0};
    RTC_DateTypeDef sDate = {0};

    sTime.Hours = RTC_DEFAULT_HOUR;
    sTime.Minutes = RTC_DEFAULT_MINUTE;
    sTime.Seconds = RTC_DEFAULT_SECOND;
    sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    sTime.StoreOperation = RTC_STOREOPERATION_RESET;
    HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);

    sDate.WeekDay = RTC_DEFAULT_WEEKDAY;
    sDate.Month = RTC_DEFAULT_MONTH;
    sDate.Date = RTC_DEFAULT_DATE;
    sDate.Year = RTC_DEFAULT_YEAR;
    HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN);

    timezone = RTC_DEFAULT_TIMEZONE;

    rtcBkpWrite(RTC_BKP_ID_MAGICNUMBER, RTC_MAGICNUMBER);
    rtcBkpWrite(RTC_BKP_ID_TIMEZONE, RTC_DEFAULT_TIMEZONE);
}

void rtcInit(void)
{
    rtcHardwareInit();

    if (rtcBkpRead(RTC_BKP_ID_MAGICNUMBER) != RTC_MAGICNUMBER) {
        rtcTimerInit();
    } else {
        timezone = rtcBkpRead(RTC_BKP_ID_TIMEZONE);
    }
}

uint32_t rtcBkpRead(RTC_BKP_ID_e id)
{
    return HAL_RTCEx_BKUPRead(&hrtc, id);
}

void rtcBkpWrite(RTC_BKP_ID_e id, uint32_t data)
{
    HAL_RTCEx_BKUPWrite(&hrtc, id, data);
}

bool rtcHasTime(void)
{
    return isStarted;
}

bool rtcGetRaw(dateTime_t *dt)
{
    if (!rtcHasTime()) {
        return false;
    }

    RTC_TimeTypeDef sTime = {0};
    RTC_DateTypeDef sDate = {0};

    HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);

    dt->year = sDate.Year;
    dt->month = sDate.Month;
    dt->date = sDate.Date;
    dt->wday = sDate.WeekDay;

    dt->hours = sTime.Hours;
    dt->minutes = sTime.Minutes;
    dt->seconds = sTime.Seconds;
    dt->millis = ((uint32_t)RTC_SYNCHPREDIV - (uint32_t)sTime.SubSeconds) * 1000 / ((uint32_t)RTC_SYNCHPREDIV + 1);

    return true;
}

bool rtcSetRaw(dateTime_t *dt)
{
    if (!rtcHasTime()) {
        return false;
    }

    RTC_TimeTypeDef sTime = {0};
    RTC_DateTypeDef sDate = {0};

    sTime.Hours = dt->hours;
    sTime.Minutes = dt->minutes;
    sTime.Seconds = dt->seconds;
    HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);

    sDate.WeekDay = dt->wday;
    sDate.Month = dt->month;
    sDate.Date = dt->date;
    sDate.Year = dt->year;
    HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN);

    return true;
}

bool rtcSetTimezone(RTC_TIMEZONE_UTC_e *tz)
{
    if (!rtcHasTime()) {
        return false;
    }

    timezone = *tz;

    return true;
}

bool rtcGetTimezone(RTC_TIMEZONE_UTC_e *tz)
{
    if (!rtcHasTime()) {
        return false;
    }

    *tz = timezone;

    return true;
}

int16_t rtcGetTimezoneOffsetMinutes(void)
{
    return timezone * 6;
}