#include "rtc_sync.h"
#include "settings.h"
#include "hardware_hal.h"
#include "lvgl.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <driver/aon_rtc.h>

#define TAG "[rtc_sync.c] "
// #define bk_printf(fmt, ...) do {if(0) printf(fmt, ##__VA_ARGS__); } while(0) // disable printf

/* 2020-01-01 00:00:00 UTC — minimum valid epoch */
#define EPOCH_2020  1577836800UL
/* 2100-01-01 00:00:00 UTC — upper sanity bound (0xFFFFFFFF uninitialized flash > this) */
#define EPOCH_2100  4102444800UL

/* epoch이 유효한지 검사: 2020~2100 범위 내여야 함.
 * 상한이 없으면 0xFFFFFFFF(미초기화 flash) = 4294967295 가 EPOCH_2020 보다 커서
 * 유효하다고 오판되고, 32비트 RTC에 저장 시 -1 로 잘려 1969-12-31 이 된다. */
static bool _epoch_valid(unsigned long ep)
{
    return ep >= EPOCH_2020 && ep <= EPOCH_2100;
}

bool rtc_is_valid(void)
{
    struct timeval tv;
    bk_rtc_gettimeofday(&tv, NULL);
    return _epoch_valid((unsigned long)tv.tv_sec);
}

/* 2026-01-01 00:00:00 UTC — compile-time fallback when no epoch saved in flash */
#define EPOCH_2026  1767225600UL

void rtc_sync_init(void)
{
    bk_printf(TAG "[RTC] rtc_sync_init start\n");

    /* DS1338Z is battery-backed: try it first */
    int yr, mo, day, hr, mn, sc;
    bool from_ds1338z = hal_rtc_get(&yr, &mo, &day, &hr, &mn, &sc);
    bk_printf(TAG "[RTC] hal_rtc_get after init: %04d-%02d-%02d %02d:%02d:%02d  valid=%d  src=%s\n",
           yr, mo, day, hr, mn, sc, (yr >= 2020 && yr <= 2099) ? 1 : 0,
           from_ds1338z ? "DS1338Z" : "AON");

    if (yr >= 2020 && yr <= 2099) {
        if (from_ds1338z) {
            /* DS1338Z has valid battery-backed time — sync AON RTC */
            hal_rtc_set(yr, mo, day, hr, mn, sc);
            bk_printf(TAG "[RTC] init source: DS1338Z  %04d-%02d-%02d %02d:%02d:%02d\n",
                   yr, mo, day, hr, mn, sc);
        } else {
            /* AON RTC fallback — DS1338Z failed.
             * AON may have been reset on power cycle or set to a stale value.
             * Compare with flash-saved epoch and use whichever is newer. */
            const char *v = settings_get_str("rtc_epoch");
            unsigned long flash_ep = (v && v[0] != '\0') ? strtoul(v, NULL, 10) : 0;

            struct tm t0 = {0};
            t0.tm_year = yr - 1900; t0.tm_mon = mo - 1; t0.tm_mday = day;
            t0.tm_hour = hr; t0.tm_min = mn; t0.tm_sec = sc; t0.tm_isdst = -1;
            unsigned long aon_ep = (unsigned long)mktime(&t0);

            bk_printf(TAG "[RTC] AON epoch=%lu  flash epoch=%lu (key=\"%s\"  valid=%d)\n",
                   aon_ep, flash_ep, v ? v : "(null)", _epoch_valid(flash_ep) ? 1 : 0);

            if (_epoch_valid(flash_ep) && flash_ep > aon_ep) {
                /* Flash epoch is newer — use it to avoid time regressing after reboot */
                struct timeval tv = { .tv_sec = (time_t)flash_ep, .tv_usec = 0 };
                bk_rtc_settimeofday(&tv, NULL);
                time_t ep_t = (time_t)flash_ep;
                struct tm *ft = localtime(&ep_t);
                if (ft) {
                    hal_rtc_set(ft->tm_year + 1900, ft->tm_mon + 1, ft->tm_mday,
                                ft->tm_hour, ft->tm_min, ft->tm_sec);
                    bk_printf(TAG "[RTC] init source: flash (newer by %lus)  %04d-%02d-%02d %02d:%02d:%02d\n",
                           flash_ep - aon_ep,
                           ft->tm_year + 1900, ft->tm_mon + 1, ft->tm_mday,
                           ft->tm_hour, ft->tm_min, ft->tm_sec);
                }
            } else {
                hal_rtc_set(yr, mo, day, hr, mn, sc);
                bk_printf(TAG "[RTC] init source: AON  %04d-%02d-%02d %02d:%02d:%02d\n",
                       yr, mo, day, hr, mn, sc);
            }
        }
        return;
    }

    /* DS1338Z not available or unset — fall back to flash-saved epoch */
    const char *v = settings_get_str("rtc_epoch");
    unsigned long epoch = (v && v[0] != '\0') ? strtoul(v, NULL, 10) : 0;
    bk_printf(TAG "[RTC] flash epoch key=\"%s\" parsed=%lu  valid=%d\n",
           v ? v : "(null)", epoch, _epoch_valid(epoch) ? 1 : 0);

    if (_epoch_valid(epoch)) {
        struct timeval tv = { .tv_sec = (time_t)epoch, .tv_usec = 0 };
        bk_rtc_settimeofday(&tv, NULL);
        /* Also program DS1338Z from flash epoch so it keeps time going forward */
        time_t ep_t = (time_t)epoch;
        struct tm *t = localtime(&ep_t);
        if (t) {
            hal_rtc_set(t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
                        t->tm_hour, t->tm_min, t->tm_sec);
            bk_printf(TAG "[RTC] init source: flash  %04d-%02d-%02d %02d:%02d:%02d (epoch=%lu)\n",
                   t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
                   t->tm_hour, t->tm_min, t->tm_sec, epoch);
        }
    } else {
        /* 유효하지 않은 epoch(미초기화 0xFFFFFFFF 포함) — 2026-01-01 00:00:00 으로 초기화 */
        struct timeval tv = { .tv_sec = (time_t)EPOCH_2026, .tv_usec = 0 };
        bk_rtc_settimeofday(&tv, NULL);
        hal_rtc_set(2026, 1, 1, 0, 0, 0);
        settings_set_str("rtc_epoch", "1767225600");
        settings_save_dirty();
        bk_printf(TAG "[RTC] init source: FALLBACK 2026-01-01 (bad epoch was %lu)\n", epoch);
    }

    /* Final state verify */
    {
        struct timeval tv2;
        bk_rtc_gettimeofday(&tv2, NULL);
        struct tm *tf = localtime(&tv2.tv_sec);
        if (tf) {
            bk_printf(TAG "[RTC] AON RTC after init: %04d-%02d-%02d %02d:%02d:%02d (epoch=%lu)\n",
                   tf->tm_year + 1900, tf->tm_mon + 1, tf->tm_mday,
                   tf->tm_hour, tf->tm_min, tf->tm_sec, (unsigned long)tv2.tv_sec);
        }
    }
}

void rtc_sync_from_mcu(int yr2, int mo, int day, int hr, int mn, int sc)
{
    bk_printf(TAG "[RTC] rtc_sync_from_mcu: 20%02d-%02d-%02d %02d:%02d:%02d\n",
           yr2, mo, day, hr, mn, sc);

    if (yr2 < 20 || yr2 > 99) { bk_printf(TAG "[RTC] MCU sync REJECT: yr2=%d out of range\n", yr2); return; }
    if (mo  < 1  || mo  > 12) { bk_printf(TAG "[RTC] MCU sync REJECT: mo=%d out of range\n", mo);  return; }
    if (day < 1  || day > 31) { bk_printf(TAG "[RTC] MCU sync REJECT: day=%d out of range\n", day); return; }
    if (hr  > 23 || mn  > 59 || sc > 59) {
        bk_printf(TAG "[RTC] MCU sync REJECT: time %02d:%02d:%02d out of range\n", hr, mn, sc);
        return;
    }

    struct tm t = {0};
    t.tm_year  = yr2 + 100;  /* years since 1900 */
    t.tm_mon   = mo - 1;     /* 0..11 */
    t.tm_mday  = day;
    t.tm_hour  = hr;
    t.tm_min   = mn;
    t.tm_sec   = sc;
    t.tm_isdst = -1;

    time_t epoch = mktime(&t);
    if (epoch == (time_t)-1 || !_epoch_valid((unsigned long)epoch)) {
        bk_printf(TAG "[RTC] MCU sync REJECT: mktime failed or epoch %lu out of valid range\n",
               (unsigned long)epoch);
        return;
    }

    /* Write DS1338Z and sync AON RTC */
    hal_rtc_set(2000 + yr2, mo, day, hr, mn, sc);

    char buf[16];
    snprintf(buf, sizeof(buf), "%lu", (unsigned long)epoch);
    settings_set_str("rtc_epoch", buf);
    settings_save_dirty();

    bk_printf(TAG "[RTC] MCU sync OK: 20%02d-%02d-%02d %02d:%02d:%02d (epoch=%lu) saved\n",
           yr2, mo, day, hr, mn, sc, (unsigned long)epoch);
}

void rtc_sync_save_now(void)
{
    if (!rtc_is_valid()) return;
    struct timeval tv;
    bk_rtc_gettimeofday(&tv, NULL);
    char buf[16];
    snprintf(buf, sizeof(buf), "%lu", (unsigned long)tv.tv_sec);
    settings_set_str("rtc_epoch", buf);
    settings_save_dirty();
    bk_printf(TAG "[RTC] epoch saved immediately (user set): %lu\n", (unsigned long)tv.tv_sec);
}

void rtc_sync_periodic_save(void)
{
    static uint32_t s_last_save_tick = 0;
    if (lv_tick_elaps(s_last_save_tick) < 60000) return;
    s_last_save_tick = lv_tick_get();

    if (!rtc_is_valid()) return;

    struct timeval tv;
    bk_rtc_gettimeofday(&tv, NULL);
    char buf[16];
    snprintf(buf, sizeof(buf), "%lu", (unsigned long)tv.tv_sec);
    settings_set_str("rtc_epoch", buf);
    settings_save_dirty();
}
