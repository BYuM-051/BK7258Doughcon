/*
 * Copyright (c) 2025 BekenCorp. All rights reserved.
 * 
 * This software is proprietary and confidential. No part of this software may be
 * reproduced, distributed, or transmitted in any form or by any means, including
 * photocopying, recording, or other electronic or mechanical methods, without the
 * prior written permission of BekenCorp, except in the case of brief quotations
 * embodied in critical reviews and certain other noncommercial uses permitted
 * by copyright law.
 * 
 * For permission requests, write to BekenCorp at armino_support@bekencorp.com.

 * Author: Beken LVGL Designer Tool
*/ 
#ifndef HAL_USE_EMULATOR
#include "common/bk_typedef.h"
#include <driver/aon_rtc.h>   /* bk_rtc_gettimeofday — BK7258 time(NULL)은 AON RTC와 연결 안 됨 */
#include "hardware_hal.h"     /* hal_rtc_get — DS1338Z 배터리 백업 RTC 직접 읽기 */
#endif
#include "lvgl.h"
#include "beken_ui.h"
#include <stdio.h>
#include <time.h>

/* forward declaration — defined below */
static void _update_date_labels(void);

/* BK7258 하드웨어는 time(NULL)이 AON RTC와 연결되지 않으므로
 * bk_rtc_gettimeofday 를 직접 사용한다.
 * 에뮬레이터는 호스트 PC의 time(NULL) 사용. */
static time_t _rtc_now(void)
{
#ifdef HAL_USE_EMULATOR
    return time(NULL);
#else
    struct timeval tv;
    bk_rtc_gettimeofday(&tv, NULL);
    return tv.tv_sec;
#endif
}

/* Digital clock global timer and instance management */
#define MAX_DIGITAL_CLOCKS 32

typedef struct {
    lv_obj_t *label;      /* label object pointer */
    int show_second;      /* show second */
    int use_ampm;         /* use ampm */
} lv_digital_clock_inst_t;

static lv_timer_t *g_digital_clock_timer = NULL;
static lv_digital_clock_inst_t g_digital_clock_instances[MAX_DIGITAL_CLOCKS];
static int g_digital_clock_count = 0;
static int g_global_hour = 0;
static int g_global_minute = 0;
static int g_global_second = 0;
static char g_global_meridiem[4] = "AM";
static int g_timer_initialized = 0;

/**
 * @brief format time string
 * @param buf output buffer
 * @param hour hour
 * @param minute minute
 * @param second second (if show_second is 0, it will be ignored)
 * @param use_ampm use ampm
 * @param show_second show second
 */
static void format_time_string(char *buf, int hour, int minute, int second, int use_ampm, int show_second)
{
    int display_hour = hour;
    const char *meridiem = "";
    
    if (use_ampm) {
        if (hour == 0) {
            display_hour = 12;
            meridiem = "AM";
        } else if (hour < 12) {
            display_hour = hour;
            meridiem = "AM";
        } else if (hour == 12) {
            display_hour = 12;
            meridiem = "PM";
        } else {
            display_hour = hour - 12;
            meridiem = "PM";
        }
        
        if (show_second) {
            lv_snprintf(buf, 32, "%02d:%02d:%02d %s", display_hour, minute, second, meridiem);
        } else {
            lv_snprintf(buf, 32, "%02d:%02d %s", display_hour, minute, meridiem);
        }
    } else {
        if (show_second) {
            lv_snprintf(buf, 32, "%02d:%02d:%02d", hour, minute, second);
        } else {
            lv_snprintf(buf, 32, "%02d:%02d", hour, minute);
        }
    }
}

/**
* @brief global digital clock timer callback
 * @param timer LVGL timer object
 */
void lv_digital_clock_timer(lv_timer_t *timer)
{
    int i;
    char time_str[32];
    static uint32_t s_rtc_sync_tick = 0;
    bool _hw_synced = false;

    /* 5분마다 DS1338Z(배터리 백업 RTC)에서 시각 재동기화.
     * 소프트웨어 카운터(g_global_second++)는 lv_tick 주기 오차로 시간이 누적 편차됨.
     * 하드웨어 RTC를 주기적으로 읽어 보정. 동기화 성공 시 이번 틱 소프트 증가 건너뜀. */
#ifndef HAL_USE_EMULATOR
    if (lv_tick_elaps(s_rtc_sync_tick) >= 300000u) {
        int yr, mo, day, hr, mn, sc;
        if (hal_rtc_get(&yr, &mo, &day, &hr, &mn, &sc) && yr >= 2020 && yr <= 2099) {
            s_rtc_sync_tick = lv_tick_get();
            g_global_hour   = hr;
            g_global_minute = mn;
            g_global_second = sc;
            if (g_global_hour < 12) lv_strcpy(g_global_meridiem, "AM");
            else                     lv_strcpy(g_global_meridiem, "PM");
            _update_date_labels();
            _hw_synced = true;
        }
    }
#endif

    if (!_hw_synced) {
    g_global_second++;
    if (g_global_second >= 60) {
        g_global_second = 0;
        g_global_minute++;
    }
    if (g_global_minute >= 60) {
        g_global_minute = 0;
        g_global_hour++;
        _update_date_labels(); /* 매 시간마다 날짜 동기화 */
    }
    if (g_global_hour >= 24) {
        g_global_hour = 0;
    }

    if (g_global_hour == 0) {
        lv_strcpy(g_global_meridiem, "AM");
    } else if (g_global_hour == 12) {
        lv_strcpy(g_global_meridiem, "PM");
    }
    }
    
    i = 0;
    while (i < g_digital_clock_count) {
        if (g_digital_clock_instances[i].label != NULL && 
            lv_obj_is_valid(g_digital_clock_instances[i].label) &&
            lv_obj_check_type(g_digital_clock_instances[i].label, &lv_label_class)) {
            format_time_string(time_str, g_global_hour, g_global_minute, g_global_second,
                             g_digital_clock_instances[i].use_ampm,
                             g_digital_clock_instances[i].show_second);
            /* 텍스트 변경 시에만 set — 동일 텍스트 반복 호출이 lv_obj_invalidate를 유발하여
             * 반투명 스크린(popuptime bg_opa=60%) 과의 피드백 루프로 배경 깨짐 발생 */
            {
                const char *_cur = lv_label_get_text(g_digital_clock_instances[i].label);
                if (!_cur || lv_strcmp(_cur, time_str) != 0)
                    lv_label_set_text(g_digital_clock_instances[i].label, time_str);
            }
            i++;
        } else {
            int j;
            for (j = i; j < g_digital_clock_count - 1; j++) {
                g_digital_clock_instances[j] = g_digital_clock_instances[j + 1];
            }
            g_digital_clock_count--;
        }
    }
}

/**
 * @brief register digital_clock instance (called when create)
 * @param label label object pointer
 * @param show_second show second
 * @param use_ampm use ampm
 * @param hour initial hour (optional, -1 means not specified, only the first instance is valid)
 * @param minute initial minute (optional, -1 means not specified, only the first instance is valid)
 * @param second initial second (optional, -1 means not specified, only the first instance is valid)
 */
void lv_digital_clock_register(lv_obj_t *label, int show_second, int use_ampm, int hour, int minute, int second)
{
    if (g_digital_clock_count >= MAX_DIGITAL_CLOCKS) {
        return;
    }
    
    if (g_timer_initialized == 0) {
        {
            time_t _now = _rtc_now();
            struct tm *_t = localtime(&_now);
            g_global_hour   = _t->tm_hour;
            g_global_minute = _t->tm_min;
            g_global_second = _t->tm_sec;
        }

        if (use_ampm) {
            if (g_global_hour < 12) {
                lv_strcpy(g_global_meridiem, "AM");
            } else {
                lv_strcpy(g_global_meridiem, "PM");
            }
        }

        if (g_digital_clock_timer == NULL) {
            g_digital_clock_timer = lv_timer_create(lv_digital_clock_timer, 1000, NULL);
        }
        g_timer_initialized = 1;
    }

    g_digital_clock_instances[g_digital_clock_count].label = label;
    g_digital_clock_instances[g_digital_clock_count].show_second = show_second;
    g_digital_clock_instances[g_digital_clock_count].use_ampm = use_ampm;
    g_digital_clock_count++;
}

/**
 * @brief unregister digital_clock instance (called when destroy)
 * @param label label object pointer
 */
void lv_digital_clock_unregister(lv_obj_t *label)
{
    int i, j;
    for (i = 0; i < g_digital_clock_count; i++) {
        if (g_digital_clock_instances[i].label == label) {
            for (j = i; j < g_digital_clock_count - 1; j++) {
                g_digital_clock_instances[j] = g_digital_clock_instances[j + 1];
            }
            g_digital_clock_count--;
            break;
        }
    }
}

/* lv_timer_pause/resume API 유무와 무관하게 동작:
 * 카운트를 0으로 만들면 타이머가 1초마다 여전히 fire하지만 어떤 라벨도
 * 업데이트하지 않으므로 dirty area가 발생하지 않는다. */
static int g_clock_saved_count = 0;
void lv_digital_clock_pause(void)
{
    if (g_clock_saved_count == 0 && g_digital_clock_count > 0) {
        g_clock_saved_count = g_digital_clock_count;
        g_digital_clock_count = 0;
    }
}
void lv_digital_clock_resume(void)
{
    if (g_clock_saved_count > 0) {
        g_digital_clock_count = g_clock_saved_count;
        g_clock_saved_count = 0;
    }
}

/* ── 날짜 레이블 지원 (YYYY.MM.DD) ──────────────────────────────── */
#include <stdio.h>

#define MAX_DATE_LABELS 8
static lv_obj_t *g_date_labels[MAX_DATE_LABELS];
static int        g_date_label_count = 0;

static void _update_date_labels(void)
{
    time_t    now = _rtc_now();
    struct tm *t  = localtime(&now);
    char       buf[16];
    int i, j;

    lv_snprintf(buf, sizeof(buf), "%04d.%02d.%02d",
                t->tm_year + 1900, t->tm_mon + 1, t->tm_mday);

    i = 0;
    while (i < g_date_label_count) {
        if (g_date_labels[i] && lv_obj_is_valid(g_date_labels[i])) {
            const char *cur = lv_label_get_text(g_date_labels[i]);
            if (!cur || lv_strcmp(cur, buf) != 0)
                lv_label_set_text(g_date_labels[i], buf);
            i++;
        } else {
            for (j = i; j < g_date_label_count - 1; j++)
                g_date_labels[j] = g_date_labels[j + 1];
            g_date_label_count--;
        }
    }
}

/**
 * @brief register a date label — updates every minute via the clock timer
 * @param label lv_label object to show YYYY.MM.DD
 */
void lv_digital_date_register(lv_obj_t *label)
{
    int i, j;
    /* compact stale entries before registering */
    i = 0;
    while (i < g_date_label_count) {
        if (g_date_labels[i] && lv_obj_is_valid(g_date_labels[i])) {
            i++;
        } else {
            for (j = i; j < g_date_label_count - 1; j++)
                g_date_labels[j] = g_date_labels[j + 1];
            g_date_label_count--;
        }
    }
    if (!label || g_date_label_count >= MAX_DATE_LABELS) return;
    for (i = 0; i < g_date_label_count; i++)
        if (g_date_labels[i] == label) return;
    g_date_labels[g_date_label_count++] = label;
    _update_date_labels();
}

/**
 * @brief sync clock globals from system time — call after hal_rtc_set
 */
void lv_digital_clock_sync(void)
{
    time_t    now = _rtc_now();
    struct tm *t  = localtime(&now);
    g_global_hour   = t->tm_hour;
    g_global_minute = t->tm_min;
    g_global_second = t->tm_sec;
    if (g_global_hour < 12) lv_strcpy(g_global_meridiem, "AM");
    else                     lv_strcpy(g_global_meridiem, "PM");
    _update_date_labels();
}

/**
 * @brief set clock globals and date labels from explicit values (emulator-safe)
 * Call after hal_rtc_set() so the display reflects the new time immediately
 * without relying on time(NULL) being updated.
 */
void lv_digital_clock_set_datetime(int yr, int mo, int day, int hr, int mn, int sc)
{
    char buf[16];
    int i, j;

    g_global_hour   = hr;
    g_global_minute = mn;
    g_global_second = sc;
    if (g_global_hour < 12) lv_strcpy(g_global_meridiem, "AM");
    else                     lv_strcpy(g_global_meridiem, "PM");

    lv_snprintf(buf, sizeof(buf), "%04d.%02d.%02d", yr, mo, day);
    i = 0;
    while (i < g_date_label_count) {
        if (g_date_labels[i] && lv_obj_is_valid(g_date_labels[i])) {
            lv_label_set_text(g_date_labels[i], buf);
            i++;
        } else {
            for (j = i; j < g_date_label_count - 1; j++)
                g_date_labels[j] = g_date_labels[j + 1];
            g_date_label_count--;
        }
    }
}

/**
 * @brief unregister a date label
 */
void lv_digital_date_unregister(lv_obj_t *label)
{
    int i, j;
    for (i = 0; i < g_date_label_count; i++) {
        if (g_date_labels[i] == label) {
            for (j = i; j < g_date_label_count - 1; j++)
                g_date_labels[j] = g_date_labels[j + 1];
            g_date_label_count--;
            return;
        }
    }
}
