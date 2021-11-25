/****************************************************************************************************************//**
*   @file               rtc.cc
*   @brief              Internal implementation os the Real Time Clock interface
*   @author             Adam Clark (hobbyos@eryjus.com)
*   @date               2021-Nov-21
*   @since              v0.0.14
*
*   @copyright          Copyright (c)  2017-2021 -- Adam Clark\n
*                       Licensed under "THE BEER-WARE LICENSE"\n
*                       See \ref LICENSE.md for details.
*
*   This file contains the internal implementation of the Real Time Clock hardware interface.
*
* ------------------------------------------------------------------------------------------------------------------
*
*   |     Date    | Tracker |  Version | Pgmr | Description
*   |:-----------:|:-------:|:--------:|:----:|:--------------------------------------------------------------------
*   | 2021-Nov-21 | Initial |  v0.0.14 | ADCL | Initial version
*
*///=================================================================================================================



#include "types.h"
#include "boot-interface.h"
#include "cpu.h"

#include <errno.h>
#include <time.h>



const long secsPerMin = 60;
const long secsPerHour = secsPerMin * 60;
const long secsPerDay = secsPerHour * 24;
const long secsPerYear = secsPerDay * 365;
const int epochYear = 1970;
const int daysPerMonth[] = {
    0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};



#define RTC_SEC     0x00
#define RTC_MIN     0x02
#define RTC_HR      0x04
#define RTC_DAY     0x07
#define RTC_MON     0x08
#define RTC_YR      0x09
#define RTC_STSA    0x0a
#define RTC_STSB    0x0b

#define CMOS_REG    0x70
#define CMOS_DATA   0x71



extern "C" Return_t RtcInit(BootInterface_t *b)
{
    return 0;
}


static inline bool UpdateInProgress(void)
{
    OUTB(CMOS_REG, RTC_STSA);
    return (INB(CMOS_DATA) & 0x80) != 0;
}


static inline uint8_t GetRtcRegister(int reg)
{
    OUTB(CMOS_REG, reg);
    return INB(CMOS_DATA);
}


extern "C" time_t rtc_GetTime(void)
{
    uint8_t sec, svSec;
    uint8_t min, svMin;
    uint8_t hr, svHr;
    uint8_t day, svDay;
    uint8_t mon, svMon;
    int year, svYear;
    uint8_t regB;

    while (UpdateInProgress()) {}               // -- Loop while we know an update is in progress

    sec = GetRtcRegister(RTC_SEC);
    min = GetRtcRegister(RTC_MIN);
    hr = GetRtcRegister(RTC_HR);
    day = GetRtcRegister(RTC_DAY);
    mon = GetRtcRegister(RTC_MON);
    year = GetRtcRegister(RTC_YR);

    do {
        svSec = sec;
        svMin = min;
        svHr = hr;
        svDay = day;
        svMon = mon;
        svYear = year;

        while (UpdateInProgress()) {}           // -- wait for no update again

        sec = GetRtcRegister(RTC_SEC);
        min = GetRtcRegister(RTC_MIN);
        hr = GetRtcRegister(RTC_HR);
        day = GetRtcRegister(RTC_DAY);
        mon = GetRtcRegister(RTC_MON);
        year = GetRtcRegister(RTC_YR);
    } while (sec != svSec || min != svMin || hr != svHr || day != svDay || mon != svMon || year != svYear);

    regB = GetRtcRegister(RTC_STSB);

    if ((regB & 0x04) == 0) {
        // -- need to convert BDC numbers
        sec = (sec & 0x0f) + ((sec / 16) * 10);
        min = (min & 0x0f) + ((min / 16) * 10);
        hr = (hr & 0x0f) + (((hr & 0x70) / 16) * 10) | (hr & 0x80);
        day = (day & 0x0f) + ((day / 16) * 10);
        mon = (mon & 0x0f) + ((mon / 16) * 10);
        year = (year & 0x0f) + ((year / 16) * 10);
    }

    // -- convert 12 hour clock to 24 hour is required
    if ((regB & 0x02) == 0 && (hr & 0x80)) {
        hr = ((hr & 0x7f) + 12) % 24;
    }

    // -- calculate a full 4-digit year
    year += (CURRENT_YEAR / 100) * 100;
    if (year < CURRENT_YEAR) year += 100;

    // -- Now, convert the date/time into seconds
    time_t rv = 0;

    svYear = epochYear;

    while (svYear < year) {
        rv += secsPerYear;
        // -- adjust for leap years
        if (svYear % 4 == 0) rv += secsPerDay;
        if (svYear % 100 == 0) rv -= secsPerDay;
        if (svYear % 400 == 0) rv += secsPerDay;
        svYear ++;
    }

    svMon = 0;

    while (svMon < mon) {
        rv += (daysPerMonth[svMon] * secsPerDay);

        if (svMon == 3) {               // -- adjust for leap year
            if (year % 4 == 0) rv += secsPerDay;
            if (year % 100 == 0) rv -= secsPerDay;
            if (year % 400 == 0) rv += secsPerDay;
        }

        svMon ++;
    }

    rv += ((day - 1) * secsPerDay);
    rv += (hr * secsPerHour);
    rv += (min * secsPerMin);
    rv += sec;

    return rv;
}


