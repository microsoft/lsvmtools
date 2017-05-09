/*
**==============================================================================
**
** LSVMTools 
** 
** MIT License
** 
** Copyright (c) Microsoft Corporation. All rights reserved.
** 
** Permission is hereby granted, free of charge, to any person obtaining a copy
** of this software and associated documentation files (the "Software"), to deal
** in the Software without restriction, including without limitation the rights
** to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
** copies of the Software, and to permit persons to whom the Software is
** furnished to do so, subject to the following conditions:
** 
** The above copyright notice and this permission notice shall be included in 
** all copies or substantial portions of the Software.
** 
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
** LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
** OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
** SOFTWARE
**
**==============================================================================
*/
#define POSIXEFI_SUPPRESS_DEFINITIONS
#include <time.h>
#include <stddef.h>
# include <string.h>

#if defined(BUILD_EFI)
# include <efi.h>
# include <efilib.h>
#endif

void __posix_panic(const char* func);

#if defined(BUILD_EFI)

static UINT64 _secs_since_epoch(
    UINT64 year,
    UINT64 month,
    UINT64 day,
    UINT64 hours,
    UINT64 minutes,
    UINT64 seconds)
{
    UINT64 tmp = (14 - month) / 12;
    UINT64 y = year + 4800 - tmp;
    UINT64 m = month + 12 * tmp - 3;
    UINT64 julianDays =
        day +
        ((153 * m + 2) / 5) +
        (y * 365) +
        (y / 4) -
        (y / 100) +
        (y / 400) -
        32045;
    UINT64 posixDays = julianDays - 2440588;

    return
        (posixDays * 24 * 60 * 60) +
        (hours * 60 * 60) +
        (minutes * 60) +
        seconds;
}

posix_time_t posix_time(posix_time_t *timer)
{
    EFI_TIME time;
    posix_time_t t;

    if (timer)
        *timer = 0;

    if (uefi_call_wrapper(RT->GetTime, 2, &time, NULL) != EFI_SUCCESS)
    {
        __posix_panic("time:1");
        return 0;
    }

    t = _secs_since_epoch(
        time.Year,
        time.Month,
        time.Day,
        time.Hour,
        time.Minute,
        time.Second);

    if (timer)
        *timer = t;

    return t;
}

struct posix_tm *posix_gmtime_r(const posix_time_t *timep, struct posix_tm *result)
{
    EFI_TIME time;
    struct posix_tm t;

    if (!timep || !result)
    {
        __posix_panic("gmtime_r:1");
        return NULL;
    }

    if (uefi_call_wrapper(RT->GetTime, 2, &time, NULL) != EFI_SUCCESS)
    {
        __posix_panic("gmtime_r:2");
        return NULL;
    }

    /* Initialze fields supported by EFI */
    t.tm_sec = time.Second;
    t.tm_min = time.Minute;
    t.tm_hour = time.Hour;
    t.tm_mday = time.Day;
    t.tm_mon = time.Month;
    t.tm_year = time.Year;
    t.tm_isdst = time.Daylight;

    /* Currently openssl does not use these fields */
    t.__tm_wday_unsupported = 0;
    t.__tm_yday_unsupported = 0;

    return posix_memcpy(result, &t, sizeof(t));
}

struct posix_tm *posix_localtime(const posix_time_t *timep)
{
    __posix_panic("localtime");
    return NULL;
}

#endif /* defined(BUILD_EFI) */
