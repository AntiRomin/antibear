#pragma once

#include "common/time.h"

typedef enum {
    RTC_BKP_ID_MAGICNUMBER = RTC_BKP_DR0,
} RTC_BKP_ID_e;

void rtcInit(void);

uint32_t rtcBkpRead(RTC_BKP_ID_e id);
void rtcBkpWrite(RTC_BKP_ID_e id, uint32_t data);

bool rtcGetRaw(dateTime_t *dt);
bool rtcSetRaw(dateTime_t *dt);

int16_t rtcGetTimezoneOffsetMinutes(void);