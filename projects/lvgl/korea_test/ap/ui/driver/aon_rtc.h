/* driver/aon_rtc.h — BK7258 AON RTC stub for PC simulator
 * Provides bk_rtc_gettimeofday / bk_rtc_settimeofday backed by system clock.
 * struct timeval: MinGW provides it in <sys/time.h>; MSVC via <winsock2.h>.
 *
 * Hardware builds: this file is on the include path before the SDK header,
 * so we use #include_next to forward to the real ARMINO SDK implementation.
 */
#pragma once

#ifdef HAL_USE_EMULATOR

#include <time.h>
#ifdef _WIN32
#  include <sys/time.h>   /* MinGW-w64: struct timeval, gettimeofday */
#else
#  include <sys/time.h>
#endif

static inline int bk_rtc_gettimeofday(struct timeval *tv, void *tz)
{
    (void)tz;
    if (!tv) return -1;
    tv->tv_sec  = (long)time(NULL);
    tv->tv_usec = 0;
    return 0;
}

static inline int bk_rtc_settimeofday(const struct timeval *tv, const void *tz)
{
    (void)tv; (void)tz;
    return 0;  /* no-op: cannot set system time in simulator */
}

#else /* !HAL_USE_EMULATOR — hardware build */

/* This stub file is found first in the include path (ap/ui/ is listed
 * before the SDK sysroot in CMakeLists.txt).  Forward unconditionally to
 * the real ARMINO SDK header so hardware builds get the actual driver. */
#include_next <driver/aon_rtc.h>

#endif /* HAL_USE_EMULATOR */
