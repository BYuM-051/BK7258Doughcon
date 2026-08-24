#include "lvgl.h"
#include "beken_ui.h"
#include "custom_func.h"
#include "event_runtime.h"
#include <stdio.h>
#include <string.h>


#include "beken_ui.h"
#include "ui_animations.h"
#include "ui_lang.h"
#include "settings.h"
#include "device_state.h"
#include "uart_comm.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>   // struct tm, time_t, localtime, mktime 등을 사용하기 위해 필수
#include "main_activity.h"
#include "hardware_hal.h"
#include <driver/aon_rtc.h>

extern bk_lv_ui_t bk_lv_tool_ui;
extern lv_obj_t *preRenderRoot;
extern void init_keypad_group();
extern void memory_save_to_slot(int slot);

static uint32_t s_last_click_automode = 0;

/* ── automodestart background prewarm ───────────────────────────────
 * Loads automodestart images one per tick while user is on automode screen.
 * Paths are built at start time to capture current lang+degree suffix. */
#define _AMS_PW_MAX 7
static lv_timer_t *s_ams_pw_timer = NULL;
static lv_obj_t   *s_ams_pw_dummy = NULL;
static int         s_ams_pw_idx   = 0;
static char        s_ams_pw_paths[_AMS_PW_MAX][128];
static int         s_ams_pw_count = 0;

static void _ams_pw_tick(lv_timer_t *t)
{
    if (s_ams_pw_idx >= s_ams_pw_count || !s_ams_pw_dummy) {
        lv_timer_delete(t); s_ams_pw_timer = NULL;
        if (s_ams_pw_dummy) { lv_obj_del(s_ams_pw_dummy); s_ams_pw_dummy = NULL; }
        printf("[PERF] automodestart prewarm done (%d imgs)\n", s_ams_pw_count);
        return;
    }
    _img_set_src_timed(s_ams_pw_dummy, s_ams_pw_paths[s_ams_pw_idx]);
    s_ams_pw_idx++;
}

static void _ams_pw_start(void)
{
#if !UI_PREWARM_ENABLE
    return;
#endif
    if (s_ams_pw_timer) return;
    if (s_ams_pw_idx >= s_ams_pw_count && s_ams_pw_count > 0) return;

    int lang = settings_get_int("LANGUAGE");
    int is_f = (strcmp(settings_get_str("Degree"), "\xc2\xb0""F") == 0) ? 1 : 0;
    const char *fsuf = is_f    ? "_f"       : "";
    const char *lsuf = (lang == 1) ? "_china" : (lang == 2) ? "_english" : "";

    s_ams_pw_count = 0;
    /* background JPEG — largest, must be first */
    snprintf(s_ams_pw_paths[s_ams_pw_count++], 128,
             "/images/auto_mode_start_bgi%s%s.jpg", fsuf, lsuf);
    /* GIF animation frames (lazy-created on first visit) */
    snprintf(s_ams_pw_paths[s_ams_pw_count++], 128, "/images/defrost_gif.png");
    snprintf(s_ams_pw_paths[s_ams_pw_count++], 128, "/images/fermentation1_gif.png");
    snprintf(s_ams_pw_paths[s_ams_pw_count++], 128, "/images/fermentation2_gif.png");
    /* UI buttons */
    snprintf(s_ams_pw_paths[s_ams_pw_count++], 128, "/images/stop_bt%s.png", lsuf);
    snprintf(s_ams_pw_paths[s_ams_pw_count++], 128, "/images/blackout%s.png", lsuf);
    /* tempbox variants — 파일명: tempbox[_f]_zero[_lang].png */
    snprintf(s_ams_pw_paths[s_ams_pw_count++], 128,
             "/images/tempbox%s_zero%s.png", fsuf, lsuf);

    s_ams_pw_idx   = 0;
    s_ams_pw_dummy = lv_image_create(lv_layer_top());
    lv_obj_add_flag(s_ams_pw_dummy, LV_OBJ_FLAG_HIDDEN);
    s_ams_pw_timer = lv_timer_create(_ams_pw_tick, 10, NULL);
}

static void _ams_pw_cancel(void)
{
    /* Stop only — keep s_ams_pw_idx/s_ams_pw_count so a mere navigate-away
     * doesn't force the whole sequence to redecode from scratch next visit.
     * Actual restart-from-scratch only happens via automode_ams_prewarm_reset(). */
    if (s_ams_pw_timer) { lv_timer_delete(s_ams_pw_timer); s_ams_pw_timer = NULL; }
    if (s_ams_pw_dummy) { lv_obj_del(s_ams_pw_dummy);      s_ams_pw_dummy = NULL; }
}

void automode_ams_prewarm_reset(void)
{
    _ams_pw_cancel();
    s_ams_pw_idx   = 0;
    s_ams_pw_count = 0;
}

/* ── memorymode background prewarm ─────────────────────────────────
 * Loads memorymode images one per tick while user is on automode screen.
 * memorymode is keep-alive so this only needs to succeed before first visit. */
#define _MM_PW_MAX 12
static const char * const s_mm_pw_paths[_MM_PW_MAX] = {
    "/images/memorymode_title.png",
    "/images/memory_title_line.png",
    "/images/memory_box.png",
    "/images/memory_left.png",
    "/images/memory_right.png",
    "/images/ok.png",
    "/images/delete.png",
    "/images/memory_check_on.png",
    "/images/memory_check_off.png",
    "/images/memroy1_on.png",
    "/images/memroy2_off.png",
    "/images/memroy3_off.png",
};
static lv_timer_t *s_mm_pw_timer = NULL;
static lv_obj_t   *s_mm_pw_dummy = NULL;
static int         s_mm_pw_idx   = 0;

static void _mm_pw_tick(lv_timer_t *t)
{
    if (s_mm_pw_idx >= _MM_PW_MAX || !s_mm_pw_dummy) {
        lv_timer_delete(t); s_mm_pw_timer = NULL;
        if (s_mm_pw_dummy) { lv_obj_del(s_mm_pw_dummy); s_mm_pw_dummy = NULL; }
        printf("[PERF] memorymode prewarm done (%d imgs)\n", _MM_PW_MAX);
        return;
    }
    _img_set_src_timed(s_mm_pw_dummy, s_mm_pw_paths[s_mm_pw_idx]);
    s_mm_pw_idx++;
}

void automode_mm_prewarm_start(void)
{
#if !UI_PREWARM_ENABLE
    return;
#endif
    if (s_mm_pw_timer) return;
    if (s_mm_pw_idx >= _MM_PW_MAX) return;
    s_mm_pw_dummy = lv_image_create(lv_layer_top());
    lv_obj_add_flag(s_mm_pw_dummy, LV_OBJ_FLAG_HIDDEN);
    s_mm_pw_timer = lv_timer_create(_mm_pw_tick, 10, NULL);
}

void automode_mm_prewarm_cancel(void)
{
    /* Stop only — keep s_mm_pw_idx so revisiting automode/main doesn't
     * force the 12-image memorymode prewarm to restart from scratch. */
    if (s_mm_pw_timer) { lv_timer_delete(s_mm_pw_timer); s_mm_pw_timer = NULL; }
    if (s_mm_pw_dummy) { lv_obj_del(s_mm_pw_dummy);      s_mm_pw_dummy = NULL; }
}
static uint32_t s_last_click_backbt   = 0;
static uint32_t s_last_click_hide_am  = 0;
static int      s_tci_automode        = 0;
static char     s_save_automode[32]   = {0};
static char     s_edit_buf_automode[32] = {0};
/* 당일완료(해동 AUTO TIME) 활성 여부 — _rclr_automode()가 매 입력마다 갱신,
 * _keypad_hide_automode()가 BasicCurrentSaveDefreezeTimeHour/Min 저장 여부 결정에 참조 */
static bool     s_auto_time_active_automode = false;

static int _days_in_month(int m, int y);

void automode_backbt_event_cb(lv_event_t *e);
void automode_startbt_event_cb(lv_event_t *e);
void automode_AutoModeCompleteYearBt_event_cb(lv_event_t *e);
void automode_AutoModeCompleteMonthBt_event_cb(lv_event_t *e);
void automode_AutoModeCompleteDayBt_event_cb(lv_event_t *e);
void automode_AutoModeCompleteHourBt_event_cb(lv_event_t *e);
void automode_AutoModeCompleteMinBt_event_cb(lv_event_t *e);
void automode_loadbt_event_cb(lv_event_t *e);
void automode_savebt_event_cb(lv_event_t *e);
void automode_AutoFreezeTempBt_event_cb(lv_event_t *e);
void automode_AutoDefrostTempBt_event_cb(lv_event_t *e);
void automode_AutoDefrostTimeHourBt_event_cb(lv_event_t *e);
void automode_AutoDefrostTimeMinBt_event_cb(lv_event_t *e);
void automode_AutoFermentation1TempBt_event_cb(lv_event_t *e);
void automode_AutoFermentation1HumidityBt_event_cb(lv_event_t *e);
void automode_AutoFermentation1TimeHourBt_event_cb(lv_event_t *e);
void automode_AutoFermentation1TimeMinBt_event_cb(lv_event_t *e);
void automode_AutoFermentation2TempBt_event_cb(lv_event_t *e);
void automode_AutoFermentation2HumidityBt_event_cb(lv_event_t *e);
void automode_AutoFermentation2TimeHourBt_event_cb(lv_event_t *e);
void automode_AutoFermentation2TimeMinBt_event_cb(lv_event_t *e);
void keypad_touch_event_cb(lv_event_t *e);
void automode_keypadhide_event_cb(lv_event_t *e);
void automode_load_event_cb(lv_event_t *e);
bool spare = false;
bool testcontrol = false;
// int  s_tci_automode = 0;
static lv_obj_t *_get_target_label_automode(bk_lv_ui_t *bk_ui)
{
    switch (s_tci_automode) {
        case 1: return bk_ui->automode_AutoFreezeTempTxt;
        case 2: return bk_ui->automode_AutoDefrostTempTxt;
        case 4: return bk_ui->automode_AutoDefrostTimeHourTxt;
        case 5: return bk_ui->automode_AutoDefrostTimeMinTxt;
        case 6: return bk_ui->automode_AutoFermentation1TempTxt;
        case 7: return bk_ui->automode_AutoFermentation1HumidityTxt;
        case 8: return bk_ui->automode_AutoFermentation1TimeHourTxt;
        case 9: return bk_ui->automode_AutoFermentation1TimeMinTxt;
        case 10: return bk_ui->automode_AutoFermentation2TempTxt;
        case 11: return bk_ui->automode_AutoFermentation2HumidityTxt;
        case 12: return bk_ui->automode_AutoFermentation2TimeHourTxt;
        case 13: return bk_ui->automode_AutoFermentation2TimeMinTxt;
        case 14: return bk_ui->automode_AutoModeCompleteYear;
        case 15: return bk_ui->automode_AutoModeCompleteMonth;
        case 16: return bk_ui->automode_AutoModeCompleteDay;
        case 17: return bk_ui->automode_AutoModeCompleteHour;
        case 18: return bk_ui->automode_AutoModeCompleteMin;
        default: return NULL;
    }
}

static void _underbar_all_hide_automode(bk_lv_ui_t *bk_ui)
{
    /* Guard: underbars are lazily created on first keypad open; may be NULL here */
    if (!bk_ui->automode_AutoModeFreezeTempUnderBarIm) return;
    lv_obj_add_flag(bk_ui->automode_AutoModeFreezeTempUnderBarIm, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(bk_ui->automode_AutoModeDefrostTempUnderBarIm, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(bk_ui->automode_AutoModeDefrostTimeHourUnderBarIm, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(bk_ui->automode_AutoModeDefrostTimeMinUnderBarIm, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(bk_ui->automode_AutoModeFermentation1TempUnderBarIm, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(bk_ui->automode_AutoModeFermentation1HumidityUnderBarIm, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(bk_ui->automode_AutoModeFermentation1TimeHourUnderBarIm, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(bk_ui->automode_AutoModeFermentation1TimeMinUnderBarIm, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(bk_ui->automode_AutoModeFermentation2TempUnderBarIm, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(bk_ui->automode_AutoModeFermentation2HumidityUnderBarIm, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(bk_ui->automode_AutoModeFermentation2TimeHourUnderBarIm, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(bk_ui->automode_AutoModeFermentation2TimeMinUnderBarIm, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(bk_ui->automode_AutoModeCompleteYearUnderBarIm, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(bk_ui->automode_AutoModeCompleteMonthUnderBarIm, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(bk_ui->automode_AutoModeCompleteDayUnderBarIm, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(bk_ui->automode_AutoModeCompleteHourUnderBarIm, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(bk_ui->automode_AutoModeCompleteMinUnderBarIm, LV_OBJ_FLAG_HIDDEN);
}

static void _underbar_show_automode(bk_lv_ui_t *bk_ui)
{
    switch (s_tci_automode) {
        case 1:  lv_obj_clear_flag(bk_ui->automode_AutoModeFreezeTempUnderBarIm,             LV_OBJ_FLAG_HIDDEN); break;
        case 2:  lv_obj_clear_flag(bk_ui->automode_AutoModeDefrostTempUnderBarIm,            LV_OBJ_FLAG_HIDDEN); break;
        case 4:  lv_obj_clear_flag(bk_ui->automode_AutoModeDefrostTimeHourUnderBarIm,        LV_OBJ_FLAG_HIDDEN); break;
        case 5:  lv_obj_clear_flag(bk_ui->automode_AutoModeDefrostTimeMinUnderBarIm,         LV_OBJ_FLAG_HIDDEN); break;
        case 6: {
            /* 발효1 온도: 섭씨(15~40)=2자리(x160,w56), 화씨(59~104)=3자리(x140,w75).
             * 텍스트(x135,w80, 우측정렬) 우측 끝(215)에 항상 맞춰 폭만 바뀌도록 x도 함께 조정. */
            int _is_f = (strcmp(settings_get_str("Degree"), "\xc2\xb0""F") == 0);
            lv_obj_set_size(bk_ui->automode_AutoModeFermentation1TempUnderBarIm, _is_f ? 75 : 56, 7);
            lv_obj_set_pos(bk_ui->automode_AutoModeFermentation1TempUnderBarIm, _is_f ? 140 : 160, 397);
            lv_obj_clear_flag(bk_ui->automode_AutoModeFermentation1TempUnderBarIm, LV_OBJ_FLAG_HIDDEN);
            break;
        }
        case 7:  lv_obj_clear_flag(bk_ui->automode_AutoModeFermentation1HumidityUnderBarIm,  LV_OBJ_FLAG_HIDDEN); break;
        case 8:  lv_obj_clear_flag(bk_ui->automode_AutoModeFermentation1TimeHourUnderBarIm,  LV_OBJ_FLAG_HIDDEN); break;
        case 9:  lv_obj_clear_flag(bk_ui->automode_AutoModeFermentation1TimeMinUnderBarIm,   LV_OBJ_FLAG_HIDDEN); break;
        case 10: {
            /* 발효2 온도: 섭씨(15~40)=2자리(x662,w56), 화씨(59~104)=3자리(x643,w75).
             * 텍스트(x638,w80, 우측정렬) 우측 끝(718)에 항상 맞춰 폭만 바뀌도록 x도 함께 조정. */
            int _is_f = (strcmp(settings_get_str("Degree"), "\xc2\xb0""F") == 0);
            lv_obj_set_size(bk_ui->automode_AutoModeFermentation2TempUnderBarIm, _is_f ? 75 : 56, 7);
            lv_obj_set_pos(bk_ui->automode_AutoModeFermentation2TempUnderBarIm, _is_f ? 643 : 662, 397);
            lv_obj_clear_flag(bk_ui->automode_AutoModeFermentation2TempUnderBarIm, LV_OBJ_FLAG_HIDDEN);
            break;
        }
        case 11: lv_obj_clear_flag(bk_ui->automode_AutoModeFermentation2HumidityUnderBarIm,  LV_OBJ_FLAG_HIDDEN); break;
        case 12: lv_obj_clear_flag(bk_ui->automode_AutoModeFermentation2TimeHourUnderBarIm,  LV_OBJ_FLAG_HIDDEN); break;
        case 13: lv_obj_clear_flag(bk_ui->automode_AutoModeFermentation2TimeMinUnderBarIm,   LV_OBJ_FLAG_HIDDEN); break;
        case 14: lv_obj_clear_flag(bk_ui->automode_AutoModeCompleteYearUnderBarIm,           LV_OBJ_FLAG_HIDDEN); break;
        case 15: lv_obj_clear_flag(bk_ui->automode_AutoModeCompleteMonthUnderBarIm,          LV_OBJ_FLAG_HIDDEN); break;
        case 16: lv_obj_clear_flag(bk_ui->automode_AutoModeCompleteDayUnderBarIm,            LV_OBJ_FLAG_HIDDEN); break;
        case 17: lv_obj_clear_flag(bk_ui->automode_AutoModeCompleteHourUnderBarIm,           LV_OBJ_FLAG_HIDDEN); break;
        case 18: lv_obj_clear_flag(bk_ui->automode_AutoModeCompleteMinUnderBarIm,            LV_OBJ_FLAG_HIDDEN); break;
        default: break;
    }
}



static void _keypad_on_automode(bk_lv_ui_t *bk_ui)
{
    printf("keypad on ");
    if (!bk_ui->automode_keypadbaseim) {
        bk_ui->automode_keypadbaseim = lv_image_create(bk_ui->automode);
        lv_obj_add_flag(bk_ui->automode_keypadbaseim, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_pos(bk_ui->automode_keypadbaseim, 0, 430);
        lv_obj_set_size(bk_ui->automode_keypadbaseim, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    }
    int _lang = settings_get_int("LANGUAGE");
    const char *_lsuf = (_lang == 1) ? "_china" : (_lang == 2) ? "_english" : "";
    {
        char _kp[64];
        snprintf(_kp, sizeof(_kp), "/images/keypadn%s.jpg", _lsuf);
        _img_set_src_timed(bk_ui->automode_keypadbaseim, _kp);
    }
    if (!bk_ui->automode_KeyPadBt[0]) {
        init_keypad_group(bk_ui);
    }
    if (!bk_ui->automode_keypadhide_im) {
        bk_ui->automode_keypadhide_im = lv_image_create(bk_ui->automode);
        lv_obj_set_pos(bk_ui->automode_keypadhide_im, 884, 453);
        lv_obj_set_size(bk_ui->automode_keypadhide_im, 120, 75);
        lv_obj_remove_flag(bk_ui->automode_keypadhide_im, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_flag(bk_ui->automode_keypadhide_im, LV_OBJ_FLAG_HIDDEN);
    }
    {
        char _kc[64];
        snprintf(_kc, sizeof(_kc), "/images/keypadback_close%s.png", _lsuf);
        _img_set_src_timed(bk_ui->automode_keypadhide_im, _kc);
    }
    /* 키패드 버튼 활성화 */
    for (int i = 0; i < 12; i++) {
        if (bk_ui->automode_KeyPadBt[i])
            lv_obj_add_flag(bk_ui->automode_KeyPadBt[i], LV_OBJ_FLAG_CLICKABLE);
    }
    lv_obj_clear_flag(bk_ui->automode_keypadhide, LV_OBJ_FLAG_HIDDEN);
    /* keypadhide_im: 항상 보이지 않고 눌렀을 때만 표시(press feedback) — event_cb에서 처리 */
    if (bk_ui->automode_startbt)
        lv_obj_clear_flag(bk_ui->automode_startbt, LV_OBJ_FLAG_CLICKABLE);
    ui_keypad_slide_on(bk_ui->automode_keypadbaseim);
}

static void _keypad_off_automode(bk_lv_ui_t *bk_ui)
{
    /* 키패드 버튼 비활성화 — manualmodestart와 동일한 방식 */
    for (int i = 0; i < 12; i++) {
        if (bk_ui->automode_KeyPadBt[i])
            lv_obj_clear_flag(bk_ui->automode_KeyPadBt[i], LV_OBJ_FLAG_CLICKABLE);
        if (bk_ui->automode_KeyPadIm[i])
            lv_obj_add_flag(bk_ui->automode_KeyPadIm[i], LV_OBJ_FLAG_HIDDEN);
    }
    lv_obj_add_flag(bk_ui->automode_keypadhide, LV_OBJ_FLAG_HIDDEN);
    if (bk_ui->automode_keypadhide_im)
        lv_obj_add_flag(bk_ui->automode_keypadhide_im, LV_OBJ_FLAG_HIDDEN);
    ui_keypad_slide_off(bk_ui->automode_keypadbaseim);
    if (bk_ui->automode_startbt)
        lv_obj_add_flag(bk_ui->automode_startbt, LV_OBJ_FLAG_CLICKABLE);
}


// void MaxMinRange(bk_lv_ui_t *bk_ui) {
//     if (bk_ui == NULL) return;

//     // 전역 설정에서 섭씨/화씨 여부 확인 (예시 변수: is_celsius)
//     bool is_celsius = true; // 실제 프로젝트의 설정값 참조 필요

//     switch (s_tci_automode) {
//         case 1: // AutoFreezeTemp
//             if (is_celsius) check_range_and_set(bk_ui->automode_AutoFreezeTempTxt, FreezeTempMin, FreezeTempMax);
//             else check_range_and_set(bk_ui->automode_AutoFreezeTempTxt, FreezeTempMinF, FreezeTempMaxF);
//             break;

//         case 2: // AutoDefrostTemp
//             if (is_celsius) check_range_and_set(bk_ui->automode_AutoDefrostTempTxt, DefrostTempMin, DefrostTempMax);
//             else check_range_and_set(bk_ui->automode_AutoDefrostTempTxt, DefrostTempMinF, DefrostTempMaxF);
//             break;

//         case 4: // AutoDefrostTimeHour
//         case 5: // AutoDefrostTimeMin
//         case 8: // AutoFermentation1TimeHour
//         case 9: // AutoFermentation1TimeMin
//         case 12: // AutoFermentation2TimeHour
//         case 13: // AutoFermentation2TimeMin
//         {
//             lv_obj_t *target = (s_tci_automode == 4) ? bk_ui->automode_AutoDefrostTimeHourTxt :
//                                (s_tci_automode == 5) ? bk_ui->automode_AutoDefrostTimeMinTxt :
//                                (s_tci_automode == 8) ? bk_ui->automode_AutoFermentation1TimeHourTxt :
//                                (s_tci_automode == 9) ? bk_ui->automode_AutoFermentation1TimeMinTxt :
//                                (s_tci_automode == 12) ? bk_ui->automode_AutoFermentation2TimeHourTxt : 
//                                                         bk_ui->automode_AutoFermentation2TimeMinTxt;
//             char buf[10];
//             zero_add(buf, lv_label_get_text(target));
//             lv_label_set_text(target, buf);
//             break;
//         }

//         case 6: // Fermentation1Temp
//             if (is_celsius) check_range_and_set(bk_ui->automode_AutoFermentation1TempTxt, Fermentation1TempMin, Fermentation1TempMax);
//             else check_range_and_set(bk_ui->automode_AutoFermentation1TempTxt, Fermentation1TempMinF, Fermentation1TempMaxF);
//             break;

//         case 7: // Fermentation1Humidity
//             check_range_and_set(bk_ui->automode_AutoFermentation1HumidityTxt, Fermentation1HumidityMin, Fermentation1HumidityMax);
//             break;

//         case 10: // Fermentation2Temp
//             if (is_celsius) check_range_and_set(bk_ui->automode_AutoFermentation2TempTxt, Fermentation2TempMin, Fermentation2TempMax);
//             else check_range_and_set(bk_ui->automode_AutoFermentation2TempTxt, Fermentation2TempMinF, Fermentation2TempMaxF);
//             break;

//         case 11: // Fermentation2Humidity
//             check_range_and_set(bk_ui->automode_AutoFermentation2HumidityTxt, Fermentation2HumidityMin, Fermentation2HumidityMax);
//             break;

//         case 15: // Complete Month
//         case 16: // Complete Day
//         case 17: // Complete Hour
//         case 18: // Complete Min
//         {
//             lv_obj_t *target = (s_tci_automode == 15) ? bk_ui->automode_AutoModeCompleteMonth :
//                                (s_tci_automode == 16) ? bk_ui->automode_AutoModeCompleteDay :
//                                (s_tci_automode == 17) ? bk_ui->automode_AutoModeCompleteHour :
//                                                         bk_ui->automode_AutoModeCompleteMin;
//             char buf[10];
//             zero_add(buf, lv_label_get_text(target));
//             lv_label_set_text(target, buf);
//             break;
//         }
        
//         default:
//             break;
//     }
// }


// /**
//  * @brief 입력값이 비어있거나 "-"일 경우 이전 데이터로 복원
//  */
// void SaveData(bk_lv_ui_t *bk_ui) {
//     if (bk_ui == NULL) return;

//     // 타겟 오브젝트 포인터 결정
//     lv_obj_t *target_obj = NULL;

//     switch (s_tci_automode) {
//         case 1:  target_obj = bk_ui->automode_AutoFreezeTempTxt; break;
//         case 2:  target_obj = bk_ui->automode_AutoDefrostTempTxt; break;
//         case 4:  target_obj = bk_ui->automode_AutoDefrostTimeHourTxt; break;
//         case 5:  target_obj = bk_ui->automode_AutoDefrostTimeMinTxt; break;
//         case 6:  target_obj = bk_ui->automode_AutoFermentation1TempTxt; break;
//         case 7:  target_obj = bk_ui->automode_AutoFermentation1HumidityTxt; break;
//         case 8:  target_obj = bk_ui->automode_AutoFermentation1TimeHourTxt; break;
//         case 9:  target_obj = bk_ui->automode_AutoFermentation1TimeMinTxt; break;
//         case 10: target_obj = bk_ui->automode_AutoFermentation2TempTxt; break;
//         case 11: target_obj = bk_ui->automode_AutoFermentation2HumidityTxt; break;
//         case 12: target_obj = bk_ui->automode_AutoFermentation2TimeHourTxt; break;
//         case 13: target_obj = bk_ui->automode_AutoFermentation2TimeMinTxt; break;
//         case 14: target_obj = bk_ui->automode_AutoModeCompleteYear; break;
//         case 15: target_obj = bk_ui->automode_AutoModeCompleteMonth; break;
//         case 16: target_obj = bk_ui->automode_AutoModeCompleteDay; break;
//         case 17: target_obj = bk_ui->automode_AutoModeCompleteHour; break;
//         case 18: target_obj = bk_ui->automode_AutoModeCompleteMin; break;
//         default: break;
//     }

//     if (target_obj != NULL) {
//         const char *current_txt = lv_label_get_text(target_obj);

//         // 자바의 .equals("") || .equals("-") 체크
//         if (current_txt == NULL || strlen(current_txt) == 0 || strcmp(current_txt, "-") == 0) {
//             char buf[16];
//             // SaveDataText(이전 값)를 가져와서 0 패딩 후 설정
//             // SaveDataText는 외부 전역 변수로 선언되어 있어야 합니다.
//             int val = atoi(SaveDataText); 
//             sprintf(buf, "%02d", val);
            
//             lv_label_set_text(target_obj, buf);
//         }
//     }
// }

static void _maxmin_automode(bk_lv_ui_t *bk_ui)
{
    int is_f = (strcmp(settings_get_str("Degree"), "\xc2\xb0""F") == 0);
    char _buf[16];
    int _v;
    _v = atoi(lv_label_get_text(bk_ui->automode_AutoFreezeTempTxt));
    if (is_f) { if (_v < 5)   _v = 5;   if (_v > 32)  _v = 32; }   /* F: 5~32 */
    else       { if (_v < -15) _v = -15; if (_v > 0)   _v = 0;  }   /* C:-15~0 */
    snprintf(_buf, sizeof(_buf), "%02d", _v);
    lv_label_set_text(bk_ui->automode_AutoFreezeTempTxt, _buf);
    _v = atoi(lv_label_get_text(bk_ui->automode_AutoDefrostTempTxt));
    if (is_f) { if (_v < 32) _v = 32; if (_v > 73) _v = 73; }      /* F: 32~73 */
    else       { if (_v < 0)  _v = 0;  if (_v > 23) _v = 23; }      /* C:  0~23 */
    snprintf(_buf, sizeof(_buf), "%02d", _v);
    lv_label_set_text(bk_ui->automode_AutoDefrostTempTxt, _buf);
    _v = atoi(lv_label_get_text(bk_ui->automode_AutoDefrostTimeHourTxt));
    if (_v < 0) _v = 0;
    if (_v > 99) _v = 99;
    snprintf(_buf, sizeof(_buf), "%02d", _v);
    lv_label_set_text(bk_ui->automode_AutoDefrostTimeHourTxt, _buf);
    _v = atoi(lv_label_get_text(bk_ui->automode_AutoDefrostTimeMinTxt));
    if (_v < 0) _v = 0;
    if (_v > 59) _v = 59;
    snprintf(_buf, sizeof(_buf), "%02d", _v);
    lv_label_set_text(bk_ui->automode_AutoDefrostTimeMinTxt, _buf);
    _v = atoi(lv_label_get_text(bk_ui->automode_AutoFermentation1TempTxt));
    if (is_f) { if (_v < 59) _v = 59; if (_v > 104) _v = 104; }    /* F: 59~104 */
    else       { if (_v < 15) _v = 15; if (_v > 40)  _v = 40;  }    /* C: 15~40 */
    snprintf(_buf, sizeof(_buf), "%02d", _v);
    lv_label_set_text(bk_ui->automode_AutoFermentation1TempTxt, _buf);
    _v = atoi(lv_label_get_text(bk_ui->automode_AutoFermentation1HumidityTxt));
    if (_v < 30) _v = 30;
    if (_v > 90) _v = 90;
    snprintf(_buf, sizeof(_buf), "%02d", _v);
    lv_label_set_text(bk_ui->automode_AutoFermentation1HumidityTxt, _buf);
    _v = atoi(lv_label_get_text(bk_ui->automode_AutoFermentation1TimeHourTxt));
    if (_v < 0) _v = 0;
    if (_v > 99) _v = 99;
    snprintf(_buf, sizeof(_buf), "%02d", _v);
    lv_label_set_text(bk_ui->automode_AutoFermentation1TimeHourTxt, _buf);
    _v = atoi(lv_label_get_text(bk_ui->automode_AutoFermentation1TimeMinTxt));
    if (_v < 0) _v = 0;
    if (_v > 59) _v = 59;
    snprintf(_buf, sizeof(_buf), "%02d", _v);
    lv_label_set_text(bk_ui->automode_AutoFermentation1TimeMinTxt, _buf);
    _v = atoi(lv_label_get_text(bk_ui->automode_AutoFermentation2TempTxt));
    if (is_f) { if (_v < 59) _v = 59; if (_v > 104) _v = 104; }    /* F: 59~104 */
    else       { if (_v < 15) _v = 15; if (_v > 40)  _v = 40;  }    /* C: 15~40 */
    snprintf(_buf, sizeof(_buf), "%02d", _v);
    lv_label_set_text(bk_ui->automode_AutoFermentation2TempTxt, _buf);
    _v = atoi(lv_label_get_text(bk_ui->automode_AutoFermentation2HumidityTxt));
    if (_v < 30) _v = 30;
    if (_v > 90) _v = 90;
    snprintf(_buf, sizeof(_buf), "%02d", _v);
    lv_label_set_text(bk_ui->automode_AutoFermentation2HumidityTxt, _buf);
    _v = atoi(lv_label_get_text(bk_ui->automode_AutoFermentation2TimeHourTxt));
    if (_v < 0) _v = 0;
    if (_v > 99) _v = 99;
    snprintf(_buf, sizeof(_buf), "%02d", _v);
    lv_label_set_text(bk_ui->automode_AutoFermentation2TimeHourTxt, _buf);
    _v = atoi(lv_label_get_text(bk_ui->automode_AutoFermentation2TimeMinTxt));
    if (_v < 0) _v = 0;
    if (_v > 59) _v = 59;
    snprintf(_buf, sizeof(_buf), "%02d", _v);
    lv_label_set_text(bk_ui->automode_AutoFermentation2TimeMinTxt, _buf);
    _v = atoi(lv_label_get_text(bk_ui->automode_AutoModeCompleteYear));
    if (_v < 2024) _v = 2024;
    if (_v > 2099) _v = 2099;
    snprintf(_buf, sizeof(_buf), "%04d", _v);
    lv_label_set_text(bk_ui->automode_AutoModeCompleteYear, _buf);
    _v = atoi(lv_label_get_text(bk_ui->automode_AutoModeCompleteMonth));
    if (_v < 1) _v = 1;
    if (_v > 12) _v = 12;
    snprintf(_buf, sizeof(_buf), "%02d", _v);
    lv_label_set_text(bk_ui->automode_AutoModeCompleteMonth, _buf);
    _v = atoi(lv_label_get_text(bk_ui->automode_AutoModeCompleteDay));
    if (_v < 1) _v = 1;
    if (_v > 31) _v = 31;
    snprintf(_buf, sizeof(_buf), "%02d", _v);
    lv_label_set_text(bk_ui->automode_AutoModeCompleteDay, _buf);
    _v = atoi(lv_label_get_text(bk_ui->automode_AutoModeCompleteHour));
    if (_v < 0)  _v = 0;
    if (_v > 23) _v = 23;
    snprintf(_buf, sizeof(_buf), "%02d", _v);
    lv_label_set_text(bk_ui->automode_AutoModeCompleteHour, _buf);
    _v = atoi(lv_label_get_text(bk_ui->automode_AutoModeCompleteMin));
    if (_v < 0) _v = 0;
    if (_v > 59) _v = 59;
    snprintf(_buf, sizeof(_buf), "%02d", _v);
    lv_label_set_text(bk_ui->automode_AutoModeCompleteMin, _buf);
    /* reset all label colors to normal after clamping */
    lv_obj_set_style_text_color(bk_ui->automode_AutoFreezeTempTxt,             lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_color(bk_ui->automode_AutoDefrostTempTxt,            lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_color(bk_ui->automode_AutoDefrostTimeHourTxt,        lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_color(bk_ui->automode_AutoDefrostTimeMinTxt,         lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_color(bk_ui->automode_AutoFermentation1TempTxt,      lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_color(bk_ui->automode_AutoFermentation1HumidityTxt,  lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_color(bk_ui->automode_AutoFermentation1TimeHourTxt,  lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_color(bk_ui->automode_AutoFermentation1TimeMinTxt,   lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_color(bk_ui->automode_AutoFermentation2TempTxt,      lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_color(bk_ui->automode_AutoFermentation2HumidityTxt,  lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_color(bk_ui->automode_AutoFermentation2TimeHourTxt,  lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_color(bk_ui->automode_AutoFermentation2TimeMinTxt,   lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_color(bk_ui->automode_AutoModeCompleteYear,          lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_color(bk_ui->automode_AutoModeCompleteMonth,         lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_color(bk_ui->automode_AutoModeCompleteDay,           lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_color(bk_ui->automode_AutoModeCompleteHour,          lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_color(bk_ui->automode_AutoModeCompleteMin,           lv_color_hex(0x3C3A3D), 0);
}

/* minutes since 2000-01-01 00:00 (approximate, ignores sub-minute DST) */
static long long _datetime_to_mins(int y, int m, int d, int h, int mn)
{
    static const int _md[12] = {31,28,31,30,31,30,31,31,30,31,30,31};
    long long days = (long long)(y - 2000) * 365LL + (long long)((y - 2000 + 3) / 4);
    for (int i = 0; i < m - 1; i++) days += _md[i];
    if (m > 2 && (y % 4 == 0 && (y % 100 != 0 || y % 400 == 0))) days++;
    days += d - 1;
    return days * 1440LL + (long long)h * 60LL + mn;
}

/* Set all 5 completion-time label colors: red by default, black if datetime is valid.
 * Valid = completion > RTC_now + (defrost + ferm1 + ferm2 total minutes).
 * Only acts when s_tci_automode is 14-18 (completion time editing). */
static void _rclr_automode(bk_lv_ui_t *bk_ui)
{
    /* 완료시간 유효성 검사 → 5개 레이블 색상 결정.
     * Android 동일 조건 (ButtonBroadcastReceiver.java:921):
     *   빨간: !(CompleteTime > DeadlineTime) || CurrentTime+10일 < CompleteTime
     *   검정: CompleteTime > DeadlineTime  AND  CompleteTime ≤ CurrentTime+10일
     * 편집 중 여부와 무관하게 항상 검사 (Android는 UART 수신마다 갱신). */
    int cy  = atoi(lv_label_get_text(bk_ui->automode_AutoModeCompleteYear));
    int cm  = atoi(lv_label_get_text(bk_ui->automode_AutoModeCompleteMonth));
    int cd  = atoi(lv_label_get_text(bk_ui->automode_AutoModeCompleteDay));
    int ch  = atoi(lv_label_get_text(bk_ui->automode_AutoModeCompleteHour));
    int cmn = atoi(lv_label_get_text(bk_ui->automode_AutoModeCompleteMin));

    /* 초기 empty state(날짜 미입력): 색상 변경 없음 */
    if (cy < 2020 || cy > 2099 || cm < 1 || cm > 12 || cd < 1 || cd > 31) return;

    /* 현재 RTC 시각 */
    int ry, rm_r, rd_r, rh_r, rmn_r, rs_r;
    hal_rtc_get(&ry, &rm_r, &rd_r, &rh_r, &rmn_r, &rs_r);
    if (ry < 2020) { ry = 2026; rm_r = 1; rd_r = 1; rh_r = 0; rmn_r = 0; }

    /* 당일완료(해동 AUTO TIME) 판정: Android ButtonBroadcastReceiver.java:864-866 동일 조건
     * (현재 Y/M/D 가 모두 완료시각 Y/M/D 이상 → 당일 완료로 간주, 느슨한 비교를 그대로 이식) */
    bool _same_day = (ry >= cy && rm_r >= cm && rd_r >= cd);
    s_auto_time_active_automode = _same_day;

    if (_same_day) {
        lv_obj_clear_flag(bk_ui->automode_AutoModeDefrostAutoTime, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(bk_ui->automode_AutoDefrostTimeHourBt, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(bk_ui->automode_AutoDefrostTimeMinBt,  LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(bk_ui->automode_AutoDefrostTimeHourTxt, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(bk_ui->automode_AutoDefrostTimeMinTxt,  LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(bk_ui->automode_AutoModeDefrostAutoTime, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(bk_ui->automode_AutoDefrostTimeHourBt, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(bk_ui->automode_AutoDefrostTimeMinBt,  LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(bk_ui->automode_AutoDefrostTimeHourTxt, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(bk_ui->automode_AutoDefrostTimeMinTxt,  LV_OBJ_FLAG_HIDDEN);
    }

    /* 최소 필요 작업시간: 당일완료면 해동 제외(발효1+발효2), 아니면 해동+발효1+발효2
     * (Android ButtonBroadcastReceiver.java:882-910 CompleteDeatLineTime 계산 동일) */
    int op_min = _same_day
        ? (atoi(lv_label_get_text(bk_ui->automode_AutoFermentation1TimeHourTxt)) +
           atoi(lv_label_get_text(bk_ui->automode_AutoFermentation2TimeHourTxt))) * 60 +
           atoi(lv_label_get_text(bk_ui->automode_AutoFermentation1TimeMinTxt))  +
           atoi(lv_label_get_text(bk_ui->automode_AutoFermentation2TimeMinTxt))
        : (atoi(lv_label_get_text(bk_ui->automode_AutoDefrostTimeHourTxt))       +
           atoi(lv_label_get_text(bk_ui->automode_AutoFermentation1TimeHourTxt)) +
           atoi(lv_label_get_text(bk_ui->automode_AutoFermentation2TimeHourTxt))) * 60 +
           atoi(lv_label_get_text(bk_ui->automode_AutoDefrostTimeMinTxt))        +
           atoi(lv_label_get_text(bk_ui->automode_AutoFermentation1TimeMinTxt))  +
           atoi(lv_label_get_text(bk_ui->automode_AutoFermentation2TimeMinTxt));

    long long rtc_mins  = _datetime_to_mins(ry, rm_r, rd_r, rh_r, rmn_r);
    long long comp_mins = _datetime_to_mins(cy, cm, cd, ch, cmn);
    long long max_mins  = rtc_mins + (long long)(10 * 24 * 60);  /* 10일 상한 */

    /* valid: 데드라인 이후 AND 10일 이내 */
    bool _valid = (comp_mins > rtc_mins + (long long)op_min) && (comp_mins <= max_mins);
    lv_color_t _clr = _valid ? lv_color_hex(0x3C3A3D) : lv_color_hex(0xFF0000);
    lv_obj_set_style_text_color(bk_ui->automode_AutoModeCompleteYear,  _clr, 0);
    lv_obj_set_style_text_color(bk_ui->automode_AutoModeCompleteMonth, _clr, 0);
    lv_obj_set_style_text_color(bk_ui->automode_AutoModeCompleteDay,   _clr, 0);
    lv_obj_set_style_text_color(bk_ui->automode_AutoModeCompleteHour,  _clr, 0);
    lv_obj_set_style_text_color(bk_ui->automode_AutoModeCompleteMin,   _clr, 0);
}

static lv_obj_t *_make_underbar(lv_obj_t *parent, lv_color_t color,
                                 int x, int y, int w, int h)
{
    lv_obj_t *obj = lv_obj_create(parent);
    lv_obj_set_pos(obj, x, y);
    lv_obj_set_size(obj, w, h);
    lv_obj_set_style_bg_color(obj, color, 0);
    lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(obj, 0, 0);
    lv_obj_set_style_radius(obj, 0, 0);
    lv_obj_set_style_pad_all(obj, 0, 0);
    lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN);
    return obj;
}

static void _ensure_underbars_created(bk_lv_ui_t *bk_ui)
{
    if (bk_ui->automode_AutoModeFreezeTempUnderBarIm != NULL) return;
    lv_obj_t *scr = bk_ui->automode;
    bk_ui->automode_AutoModeCompleteYearUnderBarIm      = _make_underbar(scr, lv_color_hex(0x000000), 191, 146, 80, 6);
    bk_ui->automode_AutoModeCompleteMonthUnderBarIm     = _make_underbar(scr, lv_color_hex(0x000000), 289, 146, 40, 6);
    bk_ui->automode_AutoModeCompleteDayUnderBarIm       = _make_underbar(scr, lv_color_hex(0x000000), 346, 146, 40, 6);
    bk_ui->automode_AutoModeCompleteHourUnderBarIm      = _make_underbar(scr, lv_color_hex(0x000000), 409, 146, 40, 6);
    bk_ui->automode_AutoModeCompleteMinUnderBarIm       = _make_underbar(scr, lv_color_hex(0x000000), 475, 146, 40, 6);
    bk_ui->automode_AutoModeFreezeTempUnderBarIm        = _make_underbar(scr, lv_color_hex(0x1F3FA0), 143, 265, 75, 7);
    bk_ui->automode_AutoModeDefrostTempUnderBarIm       = _make_underbar(scr, lv_color_hex(0x55B5D8), 664, 265, 56, 7);
    bk_ui->automode_AutoModeDefrostTimeHourUnderBarIm   = _make_underbar(scr, lv_color_hex(0x55B5D8), 809, 265, 56, 7);
    bk_ui->automode_AutoModeDefrostTimeMinUnderBarIm    = _make_underbar(scr, lv_color_hex(0x55B5D8), 886, 265, 56, 7);
    bk_ui->automode_AutoModeFermentation1TempUnderBarIm     = _make_underbar(scr, lv_color_hex(0xD49020), 160, 397, 56, 7);
    bk_ui->automode_AutoModeFermentation1HumidityUnderBarIm = _make_underbar(scr, lv_color_hex(0xD49020), 260, 397, 56, 7);
    bk_ui->automode_AutoModeFermentation1TimeHourUnderBarIm = _make_underbar(scr, lv_color_hex(0xD49020), 354, 397, 56, 7);
    bk_ui->automode_AutoModeFermentation1TimeMinUnderBarIm  = _make_underbar(scr, lv_color_hex(0xD49020), 431, 397, 56, 7);
    bk_ui->automode_AutoModeFermentation2TempUnderBarIm     = _make_underbar(scr, lv_color_hex(0xD82020), 643, 397, 75, 7);
    bk_ui->automode_AutoModeFermentation2HumidityUnderBarIm = _make_underbar(scr, lv_color_hex(0xD82020), 764, 397, 56, 7);
    bk_ui->automode_AutoModeFermentation2TimeHourUnderBarIm = _make_underbar(scr, lv_color_hex(0xD82020), 858, 397, 56, 7);
    bk_ui->automode_AutoModeFermentation2TimeMinUnderBarIm  = _make_underbar(scr, lv_color_hex(0xD82020), 935, 397, 56, 7);
}

static void _common_click_automode(bk_lv_ui_t *bk_ui)
{
    if (lv_tick_elaps(s_last_click_automode) < 250) return;
    s_last_click_automode = lv_tick_get();
    hal_buzzer_beep();
    lv_obj_t *_lbl = _get_target_label_automode(bk_ui);
    if (_lbl) {
        strncpy(s_save_automode, lv_label_get_text(_lbl), sizeof(s_save_automode) - 1);
        s_save_automode[sizeof(s_save_automode) - 1] = '\0';
    }
    s_edit_buf_automode[0] = '\0';
    testcontrol = true;  /* 첫 번째 숫자 입력 시 기존값 지우고 새 입력으로 교체 */
    _ensure_underbars_created(bk_ui);
    _underbar_all_hide_automode(bk_ui);
    _underbar_show_automode(bk_ui);
    _keypad_on_automode(bk_ui);
    _maxmin_automode(bk_ui);
    /* 원래값유지 상태: 레이블에 s_save_automode를 그대로 유지, underbar만 표시.
     * 입력 시작 시 _keypad_input_automode()가 s_edit_buf_automode로 교체한다. */
    /* 완료시간 색상: 강제 빨간 대신 유효성 검사(_rclr_automode)로 결정
     * 어느 필드를 선택하든 완료시간 유효성이 즉시 반영된다 (Android 동일). */
    // _rclr_automode(bk_ui);
}

static void _keypad_input_automode(bk_lv_ui_t *bk_ui, char digit)
{
    /* 첫 번째 숫자 입력(edit_buf 비어있음): 필드 선택 직후 바로 입력 허용 (디바운스 skip)
     * 두 번째 이후: 250ms 디바운스 적용 (더블탭 방지) */
    bool _first = (s_edit_buf_automode[0] == '\0');
    if (!_first && lv_tick_elaps(s_last_click_automode) < 250) return;
    s_last_click_automode = lv_tick_get();
    if (s_tci_automode == 0) return;
    int _max = 2;
    switch (s_tci_automode) {
        case 1: _max = 3; break;
        case 2: _max = 3; break;
        case 6: _max = 3; break;
        case 10: _max = 3; break;
        case 14: _max = 4; break;
        default: _max = 2; break;
    }
    /* 냉동 온도(case 1, °C에서 -15~0 범위) 등 '-' 부호가 자릿수와 별개로 앞에
     * 붙는 필드는, 부호를 자릿수 제한에서 제외해야 한다 — 안 그러면 예를 들어
     * "-15" 상태(이미 max=3 도달)에서 숫자를 더 누를 때 맨 앞 문자(부호)가
     * 밀려나 "-15"가 "150" 같은 양수로 바뀌는 버그가 생긴다
     * (manualmodestart_cb.c의 동일 버그를 여기도 이식). */
    int _has_sign = (s_edit_buf_automode[0] == '-');
    if (_has_sign) _max += 1;
    size_t _len = strlen(s_edit_buf_automode);
    if ((int)_len >= _max) {
        int _start = _has_sign ? 1 : 0;
        memmove(s_edit_buf_automode + _start, s_edit_buf_automode + _start + 1, _len - _start - 1);
        s_edit_buf_automode[_len - 1] = digit;
        s_edit_buf_automode[_len]     = '\0';
    } else {
        s_edit_buf_automode[_len]     = digit;
        s_edit_buf_automode[_len + 1] = '\0';
    }
    lv_obj_t *_lbl = _get_target_label_automode(bk_ui);
    if (_lbl) lv_label_set_text(_lbl, s_edit_buf_automode);
    _rclr_automode(bk_ui);
}

static void _keypad_minor_automode(bk_lv_ui_t *bk_ui)
{
    if (!(s_tci_automode == 1 || s_tci_automode == 2 || s_tci_automode == 6 || s_tci_automode == 10)) return;
    if (lv_tick_elaps(s_last_click_automode) < 250) return;
    s_last_click_automode = lv_tick_get();
    size_t _len = strlen(s_edit_buf_automode);
    if (strchr(s_edit_buf_automode, '.') == NULL && _len < sizeof(s_edit_buf_automode) - 2) {
        s_edit_buf_automode[_len]     = '.';
        s_edit_buf_automode[_len + 1] = '\0';
        lv_obj_t *_lbl = _get_target_label_automode(bk_ui);
        if (_lbl) lv_label_set_text(_lbl, s_edit_buf_automode);
        _rclr_automode(bk_ui);
    }
}

static void _keypad_backspace_automode(bk_lv_ui_t *bk_ui)
{
    if (lv_tick_elaps(s_last_click_automode) < 250) return;
    s_last_click_automode = lv_tick_get();
    if (s_tci_automode == 0) return;
    size_t _len = strlen(s_edit_buf_automode);
    if (_len > 0) s_edit_buf_automode[_len - 1] = '\0';
    lv_obj_t *_lbl = _get_target_label_automode(bk_ui);
    if (_lbl) lv_label_set_text(_lbl,
        s_edit_buf_automode[0] != '\0' ? s_edit_buf_automode : s_save_automode);
    _rclr_automode(bk_ui);
}

static void _keypad_hide_automode(bk_lv_ui_t *bk_ui)
{
    if (lv_tick_elaps(s_last_click_hide_am) < 250) return;
    s_last_click_hide_am = lv_tick_get();
    printf("_keypad_hide_automode\n");

    _maxmin_automode(bk_ui);
    {
        /* F→C 역변환: label은 F값, settings는 항상 C로 저장 */
        int _is_f = (strcmp(settings_get_str("Degree"), "\xc2\xb0""F") == 0);
#define _ST(key, obj) do { \
    const char *_sv = lv_label_get_text(obj); \
    if (_is_f && _sv && _sv[0]) { \
        char _cb[16]; snprintf(_cb, sizeof(_cb), "%d", (atoi(_sv) - 32) * 5 / 9); \
        settings_set_str(key, _cb); \
    } else settings_set_str(key, _sv); \
} while(0)
        _ST("CurrentSaveFreezeTemp",       bk_ui->automode_AutoFreezeTempTxt);
        _ST("CurrentSaveDefreezeTemp",     bk_ui->automode_AutoDefrostTempTxt);
        settings_set_str("CurrentSaveDefreezeTimeHour", lv_label_get_text(bk_ui->automode_AutoDefrostTimeHourTxt));
        settings_set_str("CurrentSaveDefreezeTimeMin",  lv_label_get_text(bk_ui->automode_AutoDefrostTimeMinTxt));
        _ST("CurrentSaveFermentation1Temp",   bk_ui->automode_AutoFermentation1TempTxt);
        settings_set_str("CurrentSaveFermentation1Humidity",  lv_label_get_text(bk_ui->automode_AutoFermentation1HumidityTxt));
        settings_set_str("CurrentSaveFermentation1TimeHour",  lv_label_get_text(bk_ui->automode_AutoFermentation1TimeHourTxt));
        settings_set_str("CurrentSaveFermentation1TimeMin",   lv_label_get_text(bk_ui->automode_AutoFermentation1TimeMinTxt));
        _ST("CurrentSaveFermentation2Temp",   bk_ui->automode_AutoFermentation2TempTxt);
        settings_set_str("CurrentSaveFermentation2Humidity",  lv_label_get_text(bk_ui->automode_AutoFermentation2HumidityTxt));
        settings_set_str("CurrentSaveFermentation2TimeHour",  lv_label_get_text(bk_ui->automode_AutoFermentation2TimeHourTxt));
        settings_set_str("CurrentSaveFermentation2TimeMin",   lv_label_get_text(bk_ui->automode_AutoFermentation2TimeMinTxt));
#undef _ST
    }
    settings_set_str("CurrentCompleteYear", lv_label_get_text(bk_ui->automode_AutoModeCompleteYear));
    settings_set_str("CurrentCompleteMonth", lv_label_get_text(bk_ui->automode_AutoModeCompleteMonth));
    settings_set_str("CurrentCompleteDay", lv_label_get_text(bk_ui->automode_AutoModeCompleteDay));
    settings_set_str("CurrentCompleteHour", lv_label_get_text(bk_ui->automode_AutoModeCompleteHour));
    settings_set_str("CurrentCompleteMin", lv_label_get_text(bk_ui->automode_AutoModeCompleteMin));
    uart_comm_trigger_change_setting();
    settings_set_str("BasicCurrentSaveFreezeTemp",            settings_get_str("CurrentSaveFreezeTemp"));
    settings_set_str("BasicCurrentSaveDefreezeTemp",          settings_get_str("CurrentSaveDefreezeTemp"));
    /* 당일완료(AUTO TIME) 상태에서는 해동시간이 역산된 임시값이므로 Basic*에 반영하지 않는다.
     * 그래야 다음에 정상(비당일) 설정으로 돌아왔을 때 마지막 실제 입력값이 남아있다
     * (Android AutoModeFragment.java:1006 if (AutoDefrostAutoTime not visible) 저장 동일). */
    if (!s_auto_time_active_automode) {
        settings_set_str("BasicCurrentSaveDefreezeTimeHour",  settings_get_str("CurrentSaveDefreezeTimeHour"));
        settings_set_str("BasicCurrentSaveDefreezeTimeMin",   settings_get_str("CurrentSaveDefreezeTimeMin"));
    }
    settings_set_str("BasicCurrentSaveFermentation1Temp",     settings_get_str("CurrentSaveFermentation1Temp"));
    settings_set_str("BasicCurrentSaveFermentation1Humidity", settings_get_str("CurrentSaveFermentation1Humidity"));
    settings_set_str("BasicCurrentSaveFermentation1TimeHour", settings_get_str("CurrentSaveFermentation1TimeHour"));
    settings_set_str("BasicCurrentSaveFermentation1TimeMin",  settings_get_str("CurrentSaveFermentation1TimeMin"));
    settings_set_str("BasicCurrentSaveFermentation2Temp",     settings_get_str("CurrentSaveFermentation2Temp"));
    settings_set_str("BasicCurrentSaveFermentation2Humidity", settings_get_str("CurrentSaveFermentation2Humidity"));
    settings_set_str("BasicCurrentSaveFermentation2TimeHour", settings_get_str("CurrentSaveFermentation2TimeHour"));
    settings_set_str("BasicCurrentSaveFermentation2TimeMin",  settings_get_str("CurrentSaveFermentation2TimeMin"));
    _underbar_all_hide_automode(bk_ui);
    _keypad_off_automode(bk_ui);
    settings_save_dirty();
    _rclr_automode(bk_ui);   /* 완료시간 편집이었으면 최종 유효성 색상 반영 */
    s_tci_automode = 0;
    s_edit_buf_automode[0] = '\0';
}

void automode_backbt_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    if (lv_tick_elaps(s_last_click_backbt) < 250) return;
    s_last_click_backbt = lv_tick_get();
    hal_buzzer_beep();

    /* 키패드가 열려있으면 강제 닫기 */
    if (s_tci_automode != 0) {
        _maxmin_automode(bk_ui);
        _underbar_all_hide_automode(bk_ui);
        _keypad_off_automode(bk_ui);
        s_tci_automode = 0;
        s_edit_buf_automode[0] = '\0';
    }

    settings_save_dirty();
    init_page_main(bk_ui);
    lv_scr_load(preRenderRoot);
    lv_refr_now(NULL);
    settings_set_str("saveChecking", "0");
    settings_save_dirty();
}

void automode_startbt_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    device_state_t *state = &g_device_state;

    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    if (lv_tick_elaps(s_last_click_automode) < 250) return;
    s_last_click_automode = lv_tick_get();
    hal_buzzer_beep();
    printf("startbt_event_cb\n");

    /* first_freeze/first_defrost 플래그만 선행 설정. uart_comm_trigger_first_start()는
     * 모든 send_*_hour/min 파라미터 확정 후 settings_save_dirty() 뒤에서 호출한다.
     * 이전 코드는 trigger를 이 위치에서 호출해 UART 태스크가 send_* 설정 전에
     * 0x31을 전송하는 레이스가 발생 → 전 파라미터 0h0m → MCU 256h 버그. */
    state->first_freeze  = true;
    state->first_defrost = false;

    settings_set_str("BasicCurrentSaveFreezeTemp", settings_get_str("CurrentSaveFreezeTemp"));
    settings_set_str("BasicCurrentSaveDefreezeTemp", settings_get_str("CurrentSaveDefreezeTemp"));
    settings_set_str("BasicCurrentSaveFermentation1Temp", settings_get_str("CurrentSaveFermentation1Temp"));
    settings_set_str("BasicCurrentSaveFermentation1Humidity", settings_get_str("CurrentSaveFermentation1Humidity"));
    settings_set_str("BasicCurrentSaveFermentation1TimeHour", settings_get_str("CurrentSaveFermentation1TimeHour"));
    settings_set_str("BasicCurrentSaveFermentation1TimeMin", settings_get_str("CurrentSaveFermentation1TimeMin"));
    settings_set_str("BasicCurrentSaveFermentation2Temp", settings_get_str("CurrentSaveFermentation2Temp"));
    settings_set_str("BasicCurrentSaveFermentation2Humidity", settings_get_str("CurrentSaveFermentation2Humidity"));
    settings_set_str("BasicCurrentSaveFermentation2TimeHour", settings_get_str("CurrentSaveFermentation2TimeHour"));
    settings_set_str("BasicCurrentSaveFermentation2TimeMin", settings_get_str("CurrentSaveFermentation2TimeMin"));
    settings_set_str("BasicCurrentSaveFreezeTemp", settings_get_str("CurrentSaveFreezeTemp"));
    settings_set_str("BasicCurrentSaveDefreezeTemp", settings_get_str("CurrentSaveDefreezeTemp"));
    settings_set_str("BasicCurrentSaveFermentation1Temp", settings_get_str("CurrentSaveFermentation1Temp"));
    settings_set_str("BasicCurrentSaveFermentation1Humidity", settings_get_str("CurrentSaveFermentation1Humidity"));
    settings_set_str("BasicCurrentSaveFermentation1TimeHour", settings_get_str("CurrentSaveFermentation1TimeHour"));
    settings_set_str("BasicCurrentSaveFermentation1TimeMin", settings_get_str("CurrentSaveFermentation1TimeMin"));
    settings_set_str("BasicCurrentSaveFermentation2Temp", settings_get_str("CurrentSaveFermentation2Temp"));
    settings_set_str("BasicCurrentSaveFermentation2Humidity", settings_get_str("CurrentSaveFermentation2Humidity"));
    settings_set_str("BasicCurrentSaveFermentation2TimeHour", settings_get_str("CurrentSaveFermentation2TimeHour"));
    settings_set_str("BasicCurrentSaveFermentation2TimeMin", settings_get_str("CurrentSaveFermentation2TimeMin"));
    settings_set_str("BasicCurrentSaveDefreezeTimeHour", settings_get_str("CurrentSaveDefreezeTimeHour"));
    settings_set_str("BasicCurrentSaveDefreezeTimeMin", settings_get_str("CurrentSaveDefreezeTimeMin"));
    settings_save_dirty();
    state->send_freeze_temp     = atoi(settings_get_str("CurrentSaveFreezeTemp"));
    state->send_freeze_hour     = atoi(settings_get_str("CurrentSaveFreezeTimeHour"));
    state->send_freeze_min      = atoi(settings_get_str("CurrentSaveFreezeTimeMin"));
    state->send_defreeze_temp   = atoi(settings_get_str("CurrentSaveDefreezeTemp"));
    state->send_defreeze_hour   = atoi(settings_get_str("CurrentSaveDefreezeTimeHour"));
    state->send_defreeze_min    = atoi(settings_get_str("CurrentSaveDefreezeTimeMin"));
    state->send_ferm1_temp      = atoi(settings_get_str("CurrentSaveFermentation1Temp"));
    state->send_ferm1_humidity  = atoi(settings_get_str("CurrentSaveFermentation1Humidity"));
    state->send_ferm1_hour      = atoi(settings_get_str("CurrentSaveFermentation1TimeHour"));
    state->send_ferm1_min       = atoi(settings_get_str("CurrentSaveFermentation1TimeMin"));
    state->send_ferm2_temp      = atoi(settings_get_str("CurrentSaveFermentation2Temp"));
    state->send_ferm2_humidity  = atoi(settings_get_str("CurrentSaveFermentation2Humidity"));
    state->send_ferm2_hour      = atoi(settings_get_str("CurrentSaveFermentation2TimeHour"));
    state->send_ferm2_min       = atoi(settings_get_str("CurrentSaveFermentation2TimeMin"));

    /* ① 완료시간: 사용자 입력 유효 시 우선 사용, 아니면 익일 08:00 기본값
     * ② 냉동시간 역산 */
    {
        int sy, sm, sd, sh, smn, ss;
        hal_rtc_get(&sy, &sm, &sd, &sh, &smn, &ss);
        if (sy < 2020) { sy = 2026; sm = 1; sd = 1; sh = 0; smn = 0; }

        /* 현재 시각 보존 */
        int ry = sy, rm = sm, rd = sd, rh = sh, rmn = smn;

        /* 사용자가 입력한 완료시간 읽기 */
        int cy  = atoi(lv_label_get_text(bk_ui->automode_AutoModeCompleteYear));
        int cm  = atoi(lv_label_get_text(bk_ui->automode_AutoModeCompleteMonth));
        int cd  = atoi(lv_label_get_text(bk_ui->automode_AutoModeCompleteDay));
        int ch  = atoi(lv_label_get_text(bk_ui->automode_AutoModeCompleteHour));
        int cmn = atoi(lv_label_get_text(bk_ui->automode_AutoModeCompleteMin));

        /* 유효성 검사: 날짜 범위 + 현재시각보다 미래인지 */
        bool user_valid = (cy >= 2020 && cy <= 2099 &&
                           cm >= 1    && cm <= 12   &&
                           cd >= 1    && cd <= 31   &&
                           ch >= 0    && ch <= 23   &&
                           cmn >= 0   && cmn <= 59  &&
                           _datetime_to_mins(cy, cm, cd, ch, cmn) >
                               _datetime_to_mins(ry, rm, rd, rh, rmn));

        if (user_valid) {
            sy = cy; sm = cm; sd = cd; sh = ch; smn = cmn;
            printf("[COMPLETE] %04d-%02d-%02d %02d:%02d (user input)\n", sy, sm, sd, sh, smn);
        } else {
            /* 기본값: 익일 08:00 */
            sd++;
            if (sd > _days_in_month(sm, sy)) { sd = 1; if (++sm > 12) { sm = 1; sy++; } }
            sh = 8; smn = 0;
            printf("[COMPLETE] %04d-%02d-%02d %02d:%02d (default next-day 08:00)\n", sy, sm, sd, sh, smn);
        }

        char buf[8];
        snprintf(buf, sizeof(buf), "%04d", sy); settings_set_str("CurrentCompleteYear",  buf);
        snprintf(buf, sizeof(buf), "%02d", sm); settings_set_str("CurrentCompleteMonth", buf);
        snprintf(buf, sizeof(buf), "%02d", sd); settings_set_str("CurrentCompleteDay",   buf);
        snprintf(buf, sizeof(buf), "%02d", sh); settings_set_str("CurrentCompleteHour",  buf);
        snprintf(buf, sizeof(buf), "%02d", smn);settings_set_str("CurrentCompleteMin",   buf);
        state->send_complete_year  = sy;
        state->send_complete_month = sm;
        state->send_complete_day   = sd;
        state->send_complete_hour  = sh;
        state->send_complete_min   = smn;

        /* 냉동시간 역산: 완료시간 - 현재시간 - (해동+발효1+발효2)
         * settings가 stale할 수 있으므로 UI 라벨에서 직접 읽는다 */
        long long comp_min  = _datetime_to_mins(sy, sm, sd, sh, smn);
        long long rtc_min   = _datetime_to_mins(ry, rm, rd, rh, rmn);
        long long other_min =
            (long long)(atoi(lv_label_get_text(bk_ui->automode_AutoDefrostTimeHourTxt))
                      + atoi(lv_label_get_text(bk_ui->automode_AutoFermentation1TimeHourTxt))
                      + atoi(lv_label_get_text(bk_ui->automode_AutoFermentation2TimeHourTxt))) * 60
          + atoi(lv_label_get_text(bk_ui->automode_AutoDefrostTimeMinTxt))
          + atoi(lv_label_get_text(bk_ui->automode_AutoFermentation1TimeMinTxt))
          + atoi(lv_label_get_text(bk_ui->automode_AutoFermentation2TimeMinTxt));
        /* state 동기 */
        state->send_defreeze_hour = atoi(lv_label_get_text(bk_ui->automode_AutoDefrostTimeHourTxt));
        state->send_defreeze_min  = atoi(lv_label_get_text(bk_ui->automode_AutoDefrostTimeMinTxt));
        state->send_ferm1_hour    = atoi(lv_label_get_text(bk_ui->automode_AutoFermentation1TimeHourTxt));
        state->send_ferm1_min     = atoi(lv_label_get_text(bk_ui->automode_AutoFermentation1TimeMinTxt));
        state->send_ferm2_hour    = atoi(lv_label_get_text(bk_ui->automode_AutoFermentation2TimeHourTxt));
        state->send_ferm2_min     = atoi(lv_label_get_text(bk_ui->automode_AutoFermentation2TimeMinTxt));
        long long total_min  = comp_min - rtc_min;
        long long freeze_min = total_min - other_min;

        /* day_period: Android AutoModeStartFragment.java의 GregorianCalendar 기반
         * day-crossing 카운트(diffdays, 자정을 넘는지 실제 날짜 필드로 재확인)와
         * 동일한 값을, 시각을 무시한 순수 "달력 날짜 차이"로 직접 계산한다.
         * 기존 total_min/1440(시간差 기반) 방식은 완료 시각이 현재 시각보다 이른
         * 경우(예: 지금 22:47 → 완료 08:00, 33h13m) 정수 나눗셈이 자정을 한 번
         * 더 넘는 나머지를 놓쳐 실제보다 하루 적게 계산했다 — 예: 이 경우 (33h13m
         * /1440)+1=2 였지만 Android/실제 날짜差는 2일(8/3→8/5)이라 dayPeriod=3이어야
         * 함. 이 하루 차이가 MCU에 보내는 payload[1](day_period)에 그대로 실려
         * 냉동시간이 실제보다 24시간 부족하게(MCU가 day_period를 이용해 총
         * 운전시간을 재구성할 경우) 계산되는 원인이었다. */
        bool _same_day = (sy == ry && sm == rm && sd == rd);
        {
            long long comp_date_min = _datetime_to_mins(sy, sm, sd, 0, 0);
            long long rtc_date_min  = _datetime_to_mins(ry, rm, rd, 0, 0);
            int date_diff = (int)((comp_date_min - rtc_date_min) / 1440);
            state->day_period = _same_day ? 1 : (date_diff + 1);
        }

        /* 당일 완료(달력 기준) 또는 냉동 시간 부족(freeze_min≤0) → 냉동 건너뛰고 해동부터 시작
         * Android AutoModeStartFragment.java dayPeriod==1 분기 동일 로직 */
        if (_same_day || freeze_min <= 0) {
            state->send_freeze_hour = 0;
            state->send_freeze_min  = 0;
            state->first_freeze     = false;
            state->first_defrost    = true;
            /* 당일: 해동시간 = 전체가용시간 - (발효1 + 발효2) */
            long long ferm_only_min = (long long)(state->send_ferm1_hour + state->send_ferm2_hour) * 60
                                    + state->send_ferm1_min + state->send_ferm2_min;
            long long new_defrost = total_min - ferm_only_min;
            if (new_defrost < 0) new_defrost = 0;
            state->send_defreeze_hour = (int)(new_defrost / 60);
            state->send_defreeze_min  = (int)(new_defrost % 60);
            snprintf(buf, sizeof(buf), "%02d", state->send_defreeze_hour);
            settings_set_str("BasicCurrentSaveDefreezeTimeHour", buf);
            settings_set_str("CurrentSaveDefreezeTimeHour",      buf);
            snprintf(buf, sizeof(buf), "%02d", state->send_defreeze_min);
            settings_set_str("BasicCurrentSaveDefreezeTimeMin",  buf);
            settings_set_str("CurrentSaveDefreezeTimeMin",       buf);
            settings_set_str("CurrentSaveFreezeTimeHour", "00");
            settings_set_str("CurrentSaveFreezeTimeMin",  "00");
            printf("[SKIP FREEZE] day_period=%d freeze_min=%lld → DEFROST %dh%dm\n",
                   state->day_period, freeze_min,
                   state->send_defreeze_hour, state->send_defreeze_min);
        } else {
            if (freeze_min < 1) freeze_min = 1;
            state->send_freeze_hour = (int)(freeze_min / 60);
            state->send_freeze_min  = (int)(freeze_min % 60);
            snprintf(buf, sizeof(buf), "%02d", state->send_freeze_hour);
            settings_set_str("CurrentSaveFreezeTimeHour", buf);
            snprintf(buf, sizeof(buf), "%02d", state->send_freeze_min);
            settings_set_str("CurrentSaveFreezeTimeMin",  buf);
            printf("[FREEZE] calc %dh %dm (comp=%lld rtc=%lld other=%lld)\n",
                   state->send_freeze_hour, state->send_freeze_min, comp_min, rtc_min, other_min);
        }

        /* 정전 복구 대비: automode 시작 시각을 origin* 에 저장.
         * saveChecking==1 로 복구 시 _record_save_slot0이 origin* 를 사용하므로
         * 여기서 현재 RTC(ry/rm/rd/rh/rmn)를 기록하지 않으면 기본값(3/29 7:00)이 표시됨.
         * manualmodestart_cb.c 와 동일한 패턴. */
        snprintf(buf, sizeof(buf), "%04d", ry); settings_set_str("originYear",  buf);
        snprintf(buf, sizeof(buf), "%02d", rm); settings_set_str("originMonth", buf);
        snprintf(buf, sizeof(buf), "%02d", rd); settings_set_str("originDay",   buf);
        snprintf(buf, sizeof(buf), "%02d", rh); settings_set_str("originHour",  buf);
        snprintf(buf, sizeof(buf), "%02d", rmn);settings_set_str("originMin",   buf);
        state->send_start_year  = ry;
        state->send_start_month = rm;
        state->send_start_day   = rd;
        state->send_start_hour  = rh;
        state->send_start_min   = rmn;
    }

#if AUTO_MODE_TEST
    state->send_freeze_hour   = 0; state->send_freeze_min   = AUTO_MODE_TEST_MIN;
    state->send_defreeze_hour = 0; state->send_defreeze_min = AUTO_MODE_TEST_MIN;
    state->send_ferm1_hour    = 0; state->send_ferm1_min    = AUTO_MODE_TEST_MIN;
    state->send_ferm2_hour    = 0; state->send_ferm2_min    = AUTO_MODE_TEST_MIN;
    printf("[TEST] auto mode: each stage forced to %d min\n", AUTO_MODE_TEST_MIN);
#endif

    /* arc 초기값: 첫 행정 잔여시간 (냉동 건너뛰면 해동 잔여시간) */
    if (state->first_defrost) {
        state->current_op_mode = OP_MODE_DEFROST;
        state->remain_hour     = state->send_defreeze_hour;
        state->remain_min      = state->send_defreeze_min;
    } else {
        state->current_op_mode = OP_MODE_FREEZE;
        state->remain_hour     = state->send_freeze_hour;
        state->remain_min      = state->send_freeze_min;
    }

    /* 정전 복구 대비: 전체 운전 총 시간을 저장
     * _blackout_recovery()는 saveRemainHour/Min을 원래 총 운전 시간(냉동+해동+발효1+발효2)으로
     * 가정하여 정전 시점별 잔여시간을 역산한다. 여기서 한 번만 저장하면 된다. */
    {
        int total_op_min = (state->send_freeze_hour  + state->send_defreeze_hour +
                            state->send_ferm1_hour   + state->send_ferm2_hour) * 60 +
                           (state->send_freeze_min   + state->send_defreeze_min +
                            state->send_ferm1_min    + state->send_ferm2_min);
        settings_set_int("saveRemainHour", total_op_min / 60);
        settings_set_int("saveRemainMin",  total_op_min % 60);
    }

    settings_set_int("saveDayPeriod",         state->day_period);         /* 정전 복구 시 자동/수동 분기 식별 */
    settings_set_int("saveOperationTemp",     state->current_op_mode);    /* 첫 행정 (냉동 or 해동) */
    settings_set_int("saveCurrentRemainHour", state->remain_hour);        /* MCU 첫 STATUS 전 정전 시 0:00 방지 */
    settings_set_int("saveCurrentRemainMin",  state->remain_min);
    settings_set_str("saveChecking", "1");
    /* EasyFlash GC가 uart_comm_trigger_first_start() 이후에 발생하면
     * UART RX 태스크가 블로킹되어 MCU 응답이 유실되고 시퀀스가 망가진다.
     * GC를 UART 핸드셰이크 이전에 완료시키기 위해 여기서 flush 한다. */
    settings_save_dirty();

    /* 모든 send_* 파라미터 확정 + GC flush 완료 후 UART 시퀀스 시작.
     * trigger를 여기서 호출하면 UART 태스크가 올바른 파라미터로 0x31을 전송한다. */
    uart_comm_trigger_first_start();   /* first_start=true, start_run=false */
    state->auto_mode_start = true;
    state->operation       = true;
    settings_set_str("SaveWriting", "0");

    /* Navigate — all settings ready before screen load event fires */
    init_page_automodestart(bk_ui);
    lv_scr_load(bk_ui->automodestart);
    destroy_page_automode(bk_ui);

}

void automode_AutoModeCompleteYearBt_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    s_tci_automode = 14;
    _common_click_automode(bk_ui);
}

void automode_AutoModeCompleteMonthBt_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    s_tci_automode = 15;
    _common_click_automode(bk_ui);
}

void automode_AutoModeCompleteDayBt_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    s_tci_automode = 16;
    _common_click_automode(bk_ui);
}

void automode_AutoModeCompleteHourBt_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    s_tci_automode = 17;
    _common_click_automode(bk_ui);
}

void automode_AutoModeCompleteMinBt_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    s_tci_automode = 18;
    _common_click_automode(bk_ui);
}

void automode_loadbt_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    device_state_t *state = &g_device_state;
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    if (lv_tick_elaps(s_last_click_automode) < 250) return;
    s_last_click_automode = lv_tick_get();
    hal_buzzer_beep();

    state->memory_mode_check   = MEMORY_MODE_LOAD;
    state->memory_slot_page    = 0;
    state->memory_slot_checking = 0;   /* no initial selection */
    if (bk_ui->memorymode == NULL || !lv_obj_is_valid(bk_ui->memorymode))
        init_page_memorymode(bk_ui);
    lv_scr_load(bk_ui->memorymode);
}

// #include <time.h>
// #include <stdlib.h>
// #include <stdio.h>
// #include <string.h>
// // mainActivity_Lock

// void process_auto_mode_save(bk_lv_ui_t *ui) {
//     // 1. 초기 조건 체크 (Lock 및 DriveCheck 상태 확인)
//     // mainActivity.Lock 및 DriveCheck는 프로젝트의 전역 변수 혹은 구조체 멤버라고 가정합니다.
//     // if (mainActivity_Lock || !mainActivity_Lock) return;

//     // 2. 시간 계산을 위한 구조체 설정
//     time_t now_sec = time(NULL);
//     struct tm *current_tm = localtime(&now_sec);
    
//     struct tm complete_tm = {0};
//     complete_tm.tm_year = atoi(lv_label_get_text(ui->automode_AutoModeCompleteYear)) - 1900;
//     complete_tm.tm_mon  = atoi(lv_label_get_text(ui->automode_AutoModeCompleteMonth)) - 1;
//     complete_tm.tm_mday = atoi(lv_label_get_text(ui->automode_AutoModeCompleteDay));
//     complete_tm.tm_hour = atoi(lv_label_get_text(ui->automode_AutoModeCompleteHour));
//     complete_tm.tm_min  = atoi(lv_label_get_text(ui->automode_AutoModeCompleteMin));
//     complete_tm.tm_isdst = -1;

//     time_t complete_sec = mktime(&complete_tm);

//     // 3. 시간 차이 계산 (초 단위 -> 밀리초 단위 변환)
//     long long diff_ms = (long long)(difftime(complete_sec, now_sec) * 1000);
    
//     // 전체 분(min) 계산
//     int all_min = (int)(diff_ms / 60000);
    
//     // dayperiod 계산 (Java 로직 기반: 초 차이를 일 단위로 변환 후 보정)
//     long diff_sec = (long)difftime(complete_sec, now_sec);
//     long diff_days = diff_sec / 86400;
    
//     // Java의 remaincalendar 보정 로직을 단순화하여 일수 계산
//     int dayperiod = (int)diff_days + 1;
    
//     // 4. 메모리 저장 (MemoryAutoSave 배열에 저장)
//     // mainActivity_MemoryModeCheck = 1;
//     sprintf(mainActivity_MemoryAutoSave[0], "%d", dayperiod);

//     // 5. 온도 단위 처리 (°C/°F)
//     const char* degree_unit = settings_get_str("Degree");
//     if (strcmp(degree_unit, "°C") == 0) {
//         strcpy(mainActivity_MemoryAutoSave[1], lv_label_get_text(ui->AutoFreezeTempTxt));
//         strcpy(mainActivity_MemoryAutoSave[2], lv_label_get_text(ui->AutoDefrostTempTxt));
//     } else {
//         // 화씨 변환 로직 (기존 Java의 reversedegreebasicchange 대응)
//         int f_freeze = (int)reversedegreebasicchange(atof(lv_label_get_text(ui->AutoFreezeTempTxt)));
//         int f_defrost = (int)reversedegreebasicchange(atof(lv_label_get_text(ui->AutoDefrostTempTxt)));
//         sprintf(mainActivity_MemoryAutoSave[1], "%d", f_freeze);
//         sprintf(mainActivity_MemoryAutoSave[2], "%d", f_defrost);
//     }

//     // 6. 당일 운전 여부에 따른 해동 시간 계산
//     if (dayperiod == 1) {
//         int fer1_h = atoi(lv_label_get_text(ui->AutoFermentation1TimeHourTxt));
//         int fer2_h = atoi(lv_label_get_text(ui->AutoFermentation2TimeHourTxt));
//         int fer1_m = atoi(lv_label_get_text(ui->AutoFermentation1TimeMinTxt));
//         int fer2_m = atoi(lv_label_get_text(ui->AutoFermentation2TimeMinTxt));

//         int defrost_minor_min = (fer1_h + fer2_h) * 60 + (fer1_m + fer2_m);
//         int remain_time_min = all_min - defrost_minor_min;

//         int rth = (remain_time_min > 60) ? (remain_time_min / 60) : 0;
//         int rtm = (remain_time_min > 60) ? (remain_time_min % 60) : remain_time_min;

//         sprintf(mainActivity_MemoryAutoSave[3], "%d", rth);
//         sprintf(mainActivity_MemoryAutoSave[4], "%d", rtm);
//     } else {
//         strcpy(mainActivity_MemoryAutoSave[3], lv_label_get_text(ui->AutoDefrostTimeHourTxt));
//         strcpy(mainActivity_MemoryAutoSave[4], lv_label_get_text(ui->AutoDefrostTimeMinTxt));
//     }

//     // 7. 발효 설정 저장
//     if (strcmp(degree_unit, "°C") == 0) {
//         strcpy(mainActivity_MemoryAutoSave[5], lv_label_get_text(ui->AutoFermentation1TempTxt));
//         strcpy(mainActivity_MemoryAutoSave[9], lv_label_get_text(ui->AutoFermentation2TempTxt));
//     } else {
//         sprintf(mainActivity_MemoryAutoSave[5], "%d", (int)reversedegreebasicchange(atof(lv_label_get_text(ui->AutoFermentation1TempTxt))));
//         sprintf(mainActivity_MemoryAutoSave[9], "%d", (int)reversedegreebasicchange(atof(lv_label_get_text(ui->AutoFermentation2TempTxt))));
//     }

//     strcpy(mainActivity_MemoryAutoSave[6], lv_label_get_text(ui->AutoFermentation1HumidityTxt));
//     strcpy(mainActivity_MemoryAutoSave[7], lv_label_get_text(ui->AutoFermentation1TimeHourTxt));
//     strcpy(mainActivity_MemoryAutoSave[8], lv_label_get_text(ui->AutoFermentation1TimeMinTxt));

//     strcpy(mainActivity_MemoryAutoSave[10], lv_label_get_text(ui->AutoFermentation2HumidityTxt));
//     strcpy(mainActivity_MemoryAutoSave[11], lv_label_get_text(ui->AutoFermentation2TimeHourTxt));
//     strcpy(mainActivity_MemoryAutoSave[12], lv_label_get_text(ui->AutoFermentation2TimeMinTxt));

//     // 8. 완료 시각 저장
//     strcpy(mainActivity_MemoryAutoSave[13], lv_label_get_text(ui->automode_AutoModeCompleteHour));
//     strcpy(mainActivity_MemoryAutoSave[14], lv_label_get_text(ui->automode_AutoModeCompleteMin));

//     // 9. 화면 전환 (Fragment 교체 대응)
//     // 예: load_memory_mode_screen();
// }

// // #include <time.h>
// // #include <stdlib.h>
// // #include <stdio.h>
// // #include <string.h>
// #include <math.h>

void process_auto_mode_save(bk_lv_ui_t *ui) {
    device_state_t *state = &g_device_state;

    // 1. 초기 조건 체크 (g_device_state 참조)
    // if (state->lock || !state->drive_check) return;
    // process_auto_mode_save

    // 2. 시간 계산을 위한 구조체 설정
    struct timeval _tv_now;
    bk_rtc_gettimeofday(&_tv_now, NULL);
    time_t now_sec = _tv_now.tv_sec;

    struct tm complete_tm = {0};
    complete_tm.tm_year = atoi(lv_label_get_text(ui->automode_AutoModeCompleteYear)) - 1900;
    complete_tm.tm_mon  = atoi(lv_label_get_text(ui->automode_AutoModeCompleteMonth)) - 1;
    complete_tm.tm_mday = atoi(lv_label_get_text(ui->automode_AutoModeCompleteDay));
    complete_tm.tm_hour = atoi(lv_label_get_text(ui->automode_AutoModeCompleteHour));
    complete_tm.tm_min  = atoi(lv_label_get_text(ui->automode_AutoModeCompleteMin));
    complete_tm.tm_isdst = -1;

    time_t complete_sec = mktime(&complete_tm);

    // 3. 달력 날짜 차이 계산 (자정 기준 — 초 단위 나눗셈 대신 날짜 경계 비교)
    struct tm *now_tm_p = localtime(&now_sec);
    struct tm now_midnight = *now_tm_p;
    now_midnight.tm_hour = now_midnight.tm_min = now_midnight.tm_sec = 0;
    now_midnight.tm_isdst = -1;
    time_t now_midnight_t = mktime(&now_midnight);

    struct tm comp_midnight = complete_tm;
    comp_midnight.tm_hour = comp_midnight.tm_min = comp_midnight.tm_sec = 0;
    comp_midnight.tm_isdst = -1;
    time_t comp_midnight_t = mktime(&comp_midnight);

    // 같은날=0, 다음날=1, 그다음날=2 ...
    int day_period = (int)(difftime(comp_midnight_t, now_midnight_t) / 86400.0 + 0.5);
    
    // 4. 상태 및 기간 저장
    state->memory_mode_check = 1; // 1=저장
    state->day_period = day_period;

    snprintf(g_device_state.memory_auto_save[0], 16, "%d", day_period);

    // 5. 온도 단위 처리 (°C/°F)
    const char* degree_unit = settings_get_str("Degree");
    bool is_celsius = (strcmp(degree_unit, "°C") == 0);

    if (is_celsius) {
        strncpy(g_device_state.memory_auto_save[1], lv_label_get_text(ui->automode_AutoFreezeTempTxt), 15);
        // strncpy(g_device_state.memory_auto_save[1], lv_label_get_text(ui->obj), 15); // 정상
        strncpy(g_device_state.memory_auto_save[2], lv_label_get_text(ui->automode_AutoDefrostTempTxt), 15);
    } else {
        // 화씨인 경우 역변환 함수 호출 (함수명이 프로젝트에 정의되어 있어야 함)
        int f_freeze = (int)reverse_degree_basic_change(atof(lv_label_get_text(ui->automode_AutoFreezeTempTxt)));
        int f_defrost = (int)reverse_degree_basic_change(atof(lv_label_get_text(ui->automode_AutoDefrostTempTxt)));
        snprintf(g_device_state.memory_auto_save[1], 16, "%d", f_freeze);
        snprintf(g_device_state.memory_auto_save[2], 16, "%d", f_defrost);
    }

    // 6. 해동 시간 저장 — day_period와 무관하게 레이블 값을 직접 저장.
    // day_period==1 분기에서 "완료시각까지 남은 시간 - 발효시간"을 계산해 저장하던
    // 잘못된 로직(예: 13:55)을 제거. MemoryDefrostHour/Min은 항상 설정된 해동시간이어야 함.
    strncpy(g_device_state.memory_auto_save[3], lv_label_get_text(ui->automode_AutoDefrostTimeHourTxt), 15);
    strncpy(g_device_state.memory_auto_save[4], lv_label_get_text(ui->automode_AutoDefrostTimeMinTxt), 15);
    g_device_state.memory_auto_save[3][15] = '\0';
    g_device_state.memory_auto_save[4][15] = '\0';

    // 7. 발효 설정 저장
    if (is_celsius) {
        strncpy(g_device_state.memory_auto_save[5], lv_label_get_text(ui->automode_AutoFermentation1TempTxt), 15);
        strncpy(g_device_state.memory_auto_save[9], lv_label_get_text(ui->automode_AutoFermentation2TempTxt), 15);
    } else {
        int f_fer1 = (int)reverse_degree_basic_change(atof(lv_label_get_text(ui->automode_AutoFermentation1TempTxt)));
        int f_fer2 = (int)reverse_degree_basic_change(atof(lv_label_get_text(ui->automode_AutoFermentation2TempTxt)));
        snprintf(g_device_state.memory_auto_save[5], 16, "%d", f_fer1);
        snprintf(g_device_state.memory_auto_save[9], 16, "%d", f_fer2);
    }

    strncpy(g_device_state.memory_auto_save[6], lv_label_get_text(ui->automode_AutoFermentation1HumidityTxt), 15);
    strncpy(g_device_state.memory_auto_save[7], lv_label_get_text(ui->automode_AutoFermentation1TimeHourTxt), 15);
    strncpy(g_device_state.memory_auto_save[8], lv_label_get_text(ui->automode_AutoFermentation1TimeMinTxt), 15);

    strncpy(g_device_state.memory_auto_save[10], lv_label_get_text(ui->automode_AutoFermentation2HumidityTxt), 15);
    strncpy(g_device_state.memory_auto_save[11], lv_label_get_text(ui->automode_AutoFermentation2TimeHourTxt), 15);
    strncpy(g_device_state.memory_auto_save[12], lv_label_get_text(ui->automode_AutoFermentation2TimeMinTxt), 15);

    // 8. 완료 시각 저장
    strncpy(g_device_state.memory_auto_save[13], lv_label_get_text(ui->automode_AutoModeCompleteHour), 15);
    strncpy(g_device_state.memory_auto_save[14], lv_label_get_text(ui->automode_AutoModeCompleteMin), 15);
    // --- 디버그 출력 추가 ---
    printf("\n==============================================\n");
    printf("[DEBUG] Auto Mode Memory Save Results:\n");
    printf("----------------------------------------------\n");
    for (int i = 0; i < 15; i++) {
        // 인덱스별로 저장된 문자열 출력
        printf("Index [%02d]: %s\n", i, g_device_state .memory_auto_save[i]);
    }
    printf("==============================================\n\n");

    // 9. 화면 전환 로직 (필요 시 추가)
    // lv_scr_load(ui->MemoryMode_screen);
}

void automode_savebt_event_cb(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    if (lv_tick_elaps(s_last_click_automode) < 250) return;
    s_last_click_automode = lv_tick_get();
    hal_buzzer_beep();

    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    process_auto_mode_save(bk_ui);   /* memory_auto_save[] 채우기 */

    /* 슬롯 선택을 위해 메모리 모드로 이동 */
    g_device_state.memory_mode_check    = MEMORY_MODE_SAVE;
    g_device_state.memory_slot_page     = 0;
    g_device_state.memory_slot_checking = 1;
    printf("[SAVE] navigate to memorymode for slot selection\n");
    if (bk_ui->memorymode == NULL || !lv_obj_is_valid(bk_ui->memorymode))
        init_page_memorymode(bk_ui);
    lv_scr_load(bk_ui->memorymode);
}

void automode_AutoFreezeTempBt_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    s_tci_automode = 1;
    _common_click_automode(bk_ui);
}

void automode_AutoDefrostTempBt_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    s_tci_automode = 2;
    _common_click_automode(bk_ui);
}

void automode_AutoDefrostTimeHourBt_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    s_tci_automode = 4;
    _common_click_automode(bk_ui);
}

void automode_AutoDefrostTimeMinBt_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    s_tci_automode = 5;
    _common_click_automode(bk_ui);
}

void automode_AutoFermentation1TempBt_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    s_tci_automode = 6;
    _common_click_automode(bk_ui);
}

void automode_AutoFermentation1HumidityBt_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    s_tci_automode = 7;
    _common_click_automode(bk_ui);
}

void automode_AutoFermentation1TimeHourBt_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    s_tci_automode = 8;
    _common_click_automode(bk_ui);
}

void automode_AutoFermentation1TimeMinBt_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    s_tci_automode = 9;
    _common_click_automode(bk_ui);
}

void automode_AutoFermentation2TempBt_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    s_tci_automode = 10;
    _common_click_automode(bk_ui);
}

void automode_AutoFermentation2HumidityBt_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    s_tci_automode = 11;
    _common_click_automode(bk_ui);
}

void automode_AutoFermentation2TimeHourBt_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    s_tci_automode = 12;
    _common_click_automode(bk_ui);
}

void automode_AutoFermentation2TimeMinBt_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    s_tci_automode = 13;
    _common_click_automode(bk_ui);
}

void keypad_touch_event_cb0(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    int idx = (int)(intptr_t)lv_event_get_user_data(e);
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_PRESSED) {
        lv_obj_remove_flag(bk_ui->automode_KeyPadIm[idx], LV_OBJ_FLAG_HIDDEN);
        return;
    }
    if (code == LV_EVENT_RELEASED || code == LV_EVENT_PRESS_LOST) {
        lv_obj_add_flag(bk_ui->automode_KeyPadIm[idx], LV_OBJ_FLAG_HIDDEN);
    }
    if (code != LV_EVENT_CLICKED) return;

    if (idx >= 0 && idx <= 8) {
        _keypad_input_automode(bk_ui, '1' + idx);
    } else if (idx == 9) {
        _keypad_input_automode(bk_ui, '0');
    } else if (idx == 10) {
        _keypad_minor_automode(bk_ui);
    } else if (idx == 11) {
        _keypad_backspace_automode(bk_ui);
    }
}

/* [KEYPAD-AM] password 팝업과 동일한 형식의 진단 로그 — 715 대비 자동운전
 * 키패드도 느려졌는지 직접 비교하기 위함. */
static uint32_t s_am_kp_last_pressed_t  = 0;
static uint32_t s_am_kp_last_released_t = 0;

void keypad_touch_event_cb(lv_event_t * e) {
    bk_lv_ui_t * bk_ui = &bk_lv_tool_ui;
    lv_event_code_t code = lv_event_get_code(e);
    int index = (int)(uintptr_t)lv_event_get_user_data(e); // 버튼 인덱스 (0~11)

    // if (is_locked) return;

    // --- [PRESSED] 눌렀을 때: 피드백 이미지 표시 ---
    if (code == LV_EVENT_PRESSED) {
        uint32_t now = lv_tick_get();
        printf("[KEYPAD-AM] PRESSED   idx=%d  dt_since_last_pressed=%lums\n",
               index, (unsigned long)(now - s_am_kp_last_pressed_t));
        s_am_kp_last_pressed_t = now;
        lv_obj_set_flag(bk_ui->automode_KeyPadIm[index], LV_OBJ_FLAG_HIDDEN, false);
    }

    // --- [RELEASED] 뗐을 때: 입력 로직 실행 ---
    else if (code == LV_EVENT_RELEASED) {
        uint32_t now = lv_tick_get();
        printf("[KEYPAD-AM] RELEASED  idx=%d  dt_since_last_released=%lums\n",
               index, (unsigned long)(now - s_am_kp_last_released_t));
        s_am_kp_last_released_t = now;
        lv_obj_set_flag(bk_ui->automode_KeyPadIm[index], LV_OBJ_FLAG_HIDDEN, true);

        // 현재 수정 중인 타겟 라벨 가져오기
        lv_obj_t * target_label = _get_target_label_automode(bk_ui);
        if (target_label == NULL) return;
        hal_buzzer_beep();

        char changestr[16];
        strcpy(changestr, lv_label_get_text(target_label));

        // 처음 입력 시작 시 기존 내용 삭제 (testcontrol)
        if (testcontrol) {
            changestr[0] = '\0';
            testcontrol = false;
        }

        if (index < 10) { // 0 ~ 9 숫자 입력
            int len = strlen(changestr);
            bool is_minus = (changestr[0] == '-');
            int mapped_num = -1;

            // --- 숫자 매핑 로직 (0123456789 => 1234567890) ---
            if (index >= 0 && index <= 8) {
                mapped_num = (int)index + 1; // 인덱스 0~8 -> 숫자 1~9
            } else if (index == 9) {
                mapped_num = 0;              // 인덱스 9 -> 숫자 0
            }
            index=mapped_num;

            if (is_minus) {
                if (len < 3) sprintf(changestr + len, "%d", index); // -X 형태까지만
            } else {
                // 도메인별 입력 제한 로직
                if (s_tci_automode == 14) { // 년 (4자리)
                    if (len < 4) sprintf(changestr + len, "%d", index);
                } else if (s_tci_automode == 15) { // 월 (12까지)
                    if (len == 0) sprintf(changestr, "%d", index);
                    else if (len == 1 && (changestr[0] == '0' || (changestr[0] == '1' && index <= 2))) 
                        sprintf(changestr + len, "%d", index);
                } else if (s_tci_automode == 16) { // 일 (날짜 로직 생략, 월별 최대일 체크 필요)
                    if (len < 2) sprintf(changestr + len, "%d", index);
                } else if (s_tci_automode == 17 || s_tci_automode == 4 || s_tci_automode == 8 || s_tci_automode == 12) { // 시 (23까지)
                    if (len == 0) sprintf(changestr, "%d", index);
                    else if (len == 1) {
                        int first = changestr[0] - '0';
                        if (first < 2 || (first == 2 && index <= 3)) sprintf(changestr + len, "%d", index);
                    }
                } else if (s_tci_automode == 18 || s_tci_automode == 5 || s_tci_automode == 9 || s_tci_automode == 13) { // 분 (59까지)
                    if (len == 0) sprintf(changestr, "%d", index);
                    else if (len == 1 && (changestr[0] >= '0' && changestr[0] <= '5')) sprintf(changestr + len, "%d", index);
                } else if (s_tci_automode == 6 || s_tci_automode == 10) { // 발효 온도 (3자리까지)
                    if (len < 3) sprintf(changestr + len, "%d", index);
                } else { // 기타 (2자리)
                    if (len < 2) sprintf(changestr + len, "%d", index);
                }
            }
            spare = false;
        } 
        else if (index == 10) { // Minus (-) 버튼
            if (s_tci_automode == 1 && strlen(changestr) == 0) { // 냉동온도만 마이너스 허용
                strcpy(changestr, "-");
            }
            spare = (strlen(changestr) == 0);
        } 
        else if (index == 11) { // Backspace 버튼
            int len = strlen(changestr);
            if (len > 0) {
                changestr[len - 1] = '\0';
            }
            spare = (strlen(changestr) == 0);
        }

        // 결과 반영 및 저장
        lv_label_set_text(target_label, changestr);

        // 완료시각(시/분) 실시간 세팅 데이터 반영: 다음 자동모드 진입 시 기본값으로 사용
        // (Android AutoModeFragment.java:781,784 CurrentSaveHour/Min 동일)
        if (strlen(changestr) > 0 && strcmp(changestr, "-") != 0) {
            char _zb[8];
            if (s_tci_automode == 17) {
                snprintf(_zb, sizeof(_zb), "%02d", atoi(changestr));
                settings_set_str("CurrentSaveHour", _zb);
            } else if (s_tci_automode == 18) {
                snprintf(_zb, sizeof(_zb), "%02d", atoi(changestr));
                settings_set_str("CurrentSaveMin", _zb);
            }
        }

        /* 완료시간/해동/발효 시간 중 어느 필드를 편집하든 당일완료(AUTO TIME) 상태와
         * 유효성 색상을 즉시 재평가 (Android가 UART 수신마다 재평가하는 것과 동일 효과) */
        _rclr_automode(bk_ui);
    }
}

void automode_keypadhide_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_PRESSED) {
        if (bk_ui->automode_keypadhide_im) lv_obj_clear_flag(bk_ui->automode_keypadhide_im, LV_OBJ_FLAG_HIDDEN);
        return;
    }
    if (code == LV_EVENT_RELEASED || code == LV_EVENT_PRESS_LOST) {
        if (bk_ui->automode_keypadhide_im) lv_obj_add_flag(bk_ui->automode_keypadhide_im, LV_OBJ_FLAG_HIDDEN);
    }
    if (code != LV_EVENT_CLICKED) return;
    hal_buzzer_beep();
    _keypad_hide_automode(bk_ui);
}

// LVGL 화면 요소에 텍스트를 세팅하는 함수 (예시)
void update_lock_complete_time(bk_lv_ui_t *ui)
{
    // 1. 현재 시간 가져오기
    time_t now = time(NULL);
    struct tm *lock_time = localtime(&now);

    // 2. 하루 더하기 (Java의 LockComplelteTime.add(Calendar.DAY_OF_MONTH, 1)와 동일)
    lock_time->tm_mday += 1;
    mktime(lock_time); // 날짜 오버플로우(예: 31일->1일) 자동 보정

    // 3. 설정 데이터에서 시/분 가져오기 (문자열 -> 정수 변환)
    const char* hour_str = settings_get_str("CurrentSaveHour");
    const char* min_str = settings_get_str("CurrentSaveMin");

    lock_time->tm_hour = atoi(hour_str);
    lock_time->tm_min = atoi(min_str);
    lock_time->tm_sec = 0;
    
    // 최종 보정 (시/분 변경 후 날짜 유효성 재확인)
    mktime(lock_time);

    // 4. LVGL Label에 텍스트 설정 (Java의 zeroAdd 대응)
    // 연(YEAR): tm_year + 1900, 월(MONTH): tm_mon + 1
    char buf[8];

    sprintf(buf, "%04d", lock_time->tm_year + 1900);
    lv_label_set_text(ui->automode_AutoModeCompleteYear, buf);

    sprintf(buf, "%02d", lock_time->tm_mon + 1);
    lv_label_set_text(ui->automode_AutoModeCompleteMonth, buf);

    sprintf(buf, "%02d", lock_time->tm_mday);
    lv_label_set_text(ui->automode_AutoModeCompleteDay, buf);

    sprintf(buf, "%02d", lock_time->tm_hour);
    lv_label_set_text(ui->automode_AutoModeCompleteHour, buf);

    sprintf(buf, "%02d", lock_time->tm_min);
    lv_label_set_text(ui->automode_AutoModeCompleteMin, buf);
}

/* Days per month (non-leap). Index 1..12 */
static const int k_days_in_month[13] = {0,31,28,31,30,31,30,31,31,30,31,30,31};

static int _is_leap(int y) { return ((y%4==0 && y%100!=0) || y%400==0); }

static int _days_in_month(int m, int y)
{
    if (m == 2 && _is_leap(y)) return 29;
    if (m < 1 || m > 12) return 30;
    return k_days_in_month[m];
}

/* Calculate completion date: start (RTC) + add_hours + add_mins,
 * writes result into the five Complete labels and device_state */
static void _calc_completion_date(bk_lv_ui_t *bk_ui, bool from_mem)
{
    int sy, sm, sd, sh, smn, ss;
    hal_rtc_get(&sy, &sm, &sd, &sh, &smn, &ss);
    (void)ss;

    /* RTC not yet synced — fallback */
    if (sy < 2020) {
        sy = 2026; sm = 5; sd = 17;
    }

    /* Use stored completion time; fall back to 08:00 */
    {
        const char *_ch  = settings_get_str("CurrentCompleteHour");
        const char *_cmn = settings_get_str("CurrentCompleteMin");
        int _h  = (_ch  && _ch[0])  ? atoi(_ch)  : 8;
        int _mn = (_cmn && _cmn[0]) ? atoi(_cmn) : 0;
        sh  = (_h  >= 0 && _h  <= 23) ? _h  : 8;
        smn = (_mn >= 0 && _mn <= 59) ? _mn : 0;
    }

    /* saveDayPeriod: 같은날=0, 다음날=1, 그다음날=2 ...
     * days_to_add = day_period (직접 사용).
     * 메모리 불러오기(from_mem)는 저장된 값이 명시적 당일(0)일 수 있으므로 그대로 사용 —
     * 0을 1(내일)로 강제 fallback하면 "가동일자 0(당일운전)"이 "다음날 완료"로 잘못 표시된다.
     * 신규 진입(첫 설정 등)만 0/미설정을 1(내일)로 fallback한다. */
    {
        int _dp = settings_get_int("saveDayPeriod");
        int _days = from_mem ? ((_dp >= 0) ? _dp : 1) : ((_dp > 0) ? _dp : 1);
        for (int _i = 0; _i < _days; _i++) {
            sd++;
            if (sd > _days_in_month(sm, sy)) { sd = 1; if (++sm > 12) { sm = 1; sy++; } }
        }
    }

    char buf[8];
    snprintf(buf, sizeof(buf), "%04d", sy);
    lv_label_set_text(bk_ui->automode_AutoModeCompleteYear,  buf);
    settings_set_str("CurrentCompleteYear", buf);

    snprintf(buf, sizeof(buf), "%02d", sm);
    lv_label_set_text(bk_ui->automode_AutoModeCompleteMonth, buf);
    settings_set_str("CurrentCompleteMonth", buf);

    snprintf(buf, sizeof(buf), "%02d", sd);
    lv_label_set_text(bk_ui->automode_AutoModeCompleteDay,   buf);
    settings_set_str("CurrentCompleteDay", buf);

    snprintf(buf, sizeof(buf), "%02d", sh);
    lv_label_set_text(bk_ui->automode_AutoModeCompleteHour,  buf);
    settings_set_str("CurrentCompleteHour", buf);

    snprintf(buf, sizeof(buf), "%02d", smn);
    lv_label_set_text(bk_ui->automode_AutoModeCompleteMin,   buf);
    settings_set_str("CurrentCompleteMin", buf);

    g_device_state.send_complete_year  = sy;
    g_device_state.send_complete_month = sm;
    g_device_state.send_complete_day   = sd;
    g_device_state.send_complete_hour  = sh;
    g_device_state.send_complete_min   = smn;
}


void automode_load_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_SCREEN_UNLOAD_START) { _ams_pw_cancel(); automode_mm_prewarm_cancel(); return; }
    if (code == LV_EVENT_SCREEN_LOADED) {
        bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
        ui_title_anim(bk_ui->automode_title);
        _ams_pw_start();
        automode_mm_prewarm_start();
        return;
    }
    if (code != LV_EVENT_SCREEN_LOAD_START) return;

    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;

    /* 메모리 불러오기로 진입했는지 여부 — automode_startbt_event_cb에서 클리어하지 않으므로
     * 여기서 한 번만 읽고 즉시 클리어한다 (아래에서 반복 참조하기 위해 화면 로드 최상단으로 이동). */
    bool _from_mem_load = (g_device_state.memory_mode_check == MEMORY_MODE_LOAD);
    g_device_state.memory_mode_check = MEMORY_MODE_NONE;

    // lv_label_set_text(bk_ui->automode_AutoFreezeTempTxt, settings_get_str("BasicCurrentSaveFreezeTemp"));
    // lv_label_set_text(bk_ui->automode_AutoDefrostTempTxt, settings_get_str("BasicCurrentSaveDefreezeTemp"));
    // lv_label_set_text(bk_ui->automode_AutoDefrostTimeHourTxt, settings_get_str("BasicCurrentSaveDefreezeTimeHour"));
    // lv_label_set_text(bk_ui->automode_AutoDefrostTimeMinTxt, settings_get_str("BasicCurrentSaveDefreezeTimeMin"));
    // lv_label_set_text(bk_ui->automode_AutoFermentation1TempTxt, settings_get_str("BasicCurrentSaveFermentation1Temp"));
    // lv_label_set_text(bk_ui->automode_AutoFermentation1HumidityTxt, settings_get_str("BasicCurrentSaveFermentation1Humidity"));
    // lv_label_set_text(bk_ui->automode_AutoFermentation1TimeHourTxt, settings_get_str("BasicCurrentSaveFermentation1TimeHour"));
    // lv_label_set_text(bk_ui->automode_AutoFermentation1TimeMinTxt, settings_get_str("BasicCurrentSaveFermentation1TimeMin"));
    // lv_label_set_text(bk_ui->automode_AutoFermentation2TempTxt, settings_get_str("BasicCurrentSaveFermentation2Temp"));
    // lv_label_set_text(bk_ui->automode_AutoFermentation2HumidityTxt, settings_get_str("BasicCurrentSaveFermentation2Humidity"));
    // lv_label_set_text(bk_ui->automode_AutoFermentation2TimeHourTxt, settings_get_str("BasicCurrentSaveFermentation2TimeHour"));
    // lv_label_set_text(bk_ui->automode_AutoFermentation2TimeMinTxt, settings_get_str("BasicCurrentSaveFermentation2TimeMin"));
    // lv_label_set_text(bk_ui->automode_AutoDefrostTimeHourTxt, settings_get_str("BasicCurrentSaveDefreezeTimeHour"));
    // lv_label_set_text(bk_ui->automode_AutoDefrostTimeMinTxt, settings_get_str("BasicCurrentSaveDefreezeTimeMin"));
    // lv_label_set_text(bk_ui->automode_AutoFermentation1HumidityTxt, settings_get_str("BasicCurrentSaveFermentation1Humidity"));
    // lv_label_set_text(bk_ui->automode_AutoFermentation1TimeHourTxt, settings_get_str("BasicCurrentSaveFermentation1TimeHour"));
    // lv_label_set_text(bk_ui->automode_AutoFermentation1TimeMinTxt, settings_get_str("BasicCurrentSaveFermentation1TimeMin"));
    // lv_label_set_text(bk_ui->automode_AutoFermentation2HumidityTxt, settings_get_str("BasicCurrentSaveFermentation2Humidity"));
    // lv_label_set_text(bk_ui->automode_AutoFermentation2TimeHourTxt, settings_get_str("BasicCurrentSaveFermentation2TimeHour"));
    // lv_label_set_text(bk_ui->automode_AutoFermentation2TimeMinTxt, settings_get_str("BasicCurrentSaveFermentation2TimeMin"));
    
    /* settings는 항상 C 저장, F 모드이면 C→F 변환 후 표시 */
    /* 이전 버전에서 _maxmin_automode가 C값을 F범위로 클램핑해 저장한 stale 데이터 정규화:
     * C 범위를 벗어난 값(= stale F값)은 C 기본값으로 리셋 후 로드 */
    {
        static const struct { const char *key; int lo; int hi; const char *def; } _tbl[] = {
            { "CurrentSaveFreezeTemp",      -15, 0,  "-10" },
            { "CurrentSaveDefreezeTemp",    0,   23, "02"  },
            { "CurrentSaveFermentation1Temp", 15, 40, "20" },
            { "CurrentSaveFermentation2Temp", 15, 40, "33" },
        };
        for (int _i = 0; _i < 4; _i++) {
            const char *_sv = settings_get_str(_tbl[_i].key);
            if (_sv && _sv[0]) {
                int _v = atoi(_sv);
                if (_v < _tbl[_i].lo || _v > _tbl[_i].hi)
                    settings_set_str(_tbl[_i].key, _tbl[_i].def);
            }
        }
        int _is_f = (strcmp(settings_get_str("Degree"), "\xc2\xb0""F") == 0);
#define _LT(obj, key) do { \
    const char *_sv = settings_get_str(key); \
    if (_sv && _sv[0]) { \
        char _fb[16]; \
        int _tv = _is_f ? (atoi(_sv) * 9 / 5 + 32) : atoi(_sv); \
        snprintf(_fb, sizeof(_fb), "%02d", _tv); \
        lv_label_set_text(obj, _fb); \
    } else lv_label_set_text(obj, ""); \
} while(0)
        _LT(bk_ui->automode_AutoFreezeTempTxt,            "CurrentSaveFreezeTemp");
        _LT(bk_ui->automode_AutoDefrostTempTxt,           "CurrentSaveDefreezeTemp");
        /* 메모리 불러오기가 아니면 CurrentSaveDefreezeTimeHour/Min 대신 Basic* 사용: 당일완료
         * (AUTO TIME)로 운전 시작하면 CurrentSave*는 역산된 임시값(예: 1h30m)으로 덮어써지므로,
         * 다음 신규 진입 시 그 값을 그대로 보여주면 실제 해동시간이 사라진 것처럼 보인다. Basic*는
         * _keypad_hide_automode()가 당일완료 상태일 때 갱신을 건너뛰므로 마지막 실제 입력값을
         * 유지한다 (Android AutoModeFragment.java:505-506,527-528 동일).
         * 메모리 불러오기일 때는 memory_load_from_slot()이 CurrentSave*만 갱신하고 Basic*는
         * 건드리지 않으므로, 이 경우는 CurrentSave*를 그대로 써야 불러온 슬롯 값이 보인다. */
        if (_from_mem_load) {
            lv_label_set_text(bk_ui->automode_AutoDefrostTimeHourTxt,   settings_get_str("CurrentSaveDefreezeTimeHour"));
            lv_label_set_text(bk_ui->automode_AutoDefrostTimeMinTxt,    settings_get_str("CurrentSaveDefreezeTimeMin"));
        } else {
            lv_label_set_text(bk_ui->automode_AutoDefrostTimeHourTxt,   settings_get_str("BasicCurrentSaveDefreezeTimeHour"));
            lv_label_set_text(bk_ui->automode_AutoDefrostTimeMinTxt,    settings_get_str("BasicCurrentSaveDefreezeTimeMin"));
        }
        _LT(bk_ui->automode_AutoFermentation1TempTxt,     "CurrentSaveFermentation1Temp");
        lv_label_set_text(bk_ui->automode_AutoFermentation1HumidityTxt, settings_get_str("CurrentSaveFermentation1Humidity"));
        lv_label_set_text(bk_ui->automode_AutoFermentation1TimeHourTxt, settings_get_str("CurrentSaveFermentation1TimeHour"));
        lv_label_set_text(bk_ui->automode_AutoFermentation1TimeMinTxt,  settings_get_str("CurrentSaveFermentation1TimeMin"));
        _LT(bk_ui->automode_AutoFermentation2TempTxt,     "CurrentSaveFermentation2Temp");
        lv_label_set_text(bk_ui->automode_AutoFermentation2HumidityTxt, settings_get_str("CurrentSaveFermentation2Humidity"));
        lv_label_set_text(bk_ui->automode_AutoFermentation2TimeHourTxt, settings_get_str("CurrentSaveFermentation2TimeHour"));
        lv_label_set_text(bk_ui->automode_AutoFermentation2TimeMinTxt,  settings_get_str("CurrentSaveFermentation2TimeMin"));
#undef _LT
    }
    if (!g_device_state.operation && !_from_mem_load) {
        /* 하드코딩된 08:00 대신 사용자가 마지막으로 입력한 완료시각(CurrentSaveHour/Min)을
         * 기본값으로 사용 (Android AutoModeFragment.java:552-561 LockComplelteTime 동일) */
        char _hb[8], _mb[8];
        const char *_sh = settings_get_str("CurrentSaveHour");
        const char *_sm = settings_get_str("CurrentSaveMin");
        snprintf(_hb, sizeof(_hb), "%02d", (_sh && _sh[0]) ? atoi(_sh) : 8);
        snprintf(_mb, sizeof(_mb), "%02d", (_sm && _sm[0]) ? atoi(_sm) : 0);
        settings_set_str("CurrentCompleteHour", _hb);
        settings_set_str("CurrentCompleteMin",  _mb);
        settings_set_int("saveDayPeriod", 1);
    }
    lv_label_set_text(bk_ui->automode_AutoModeCompleteHour,          settings_get_str("CurrentCompleteHour"));
    lv_label_set_text(bk_ui->automode_AutoModeCompleteMin,           settings_get_str("CurrentCompleteMin"));
    _calc_completion_date(bk_ui, _from_mem_load);
    _rclr_automode(bk_ui);  /* 저장된 완료시간 유효성 → 초기 색상 설정 */

    /* 화씨일 때만 °F 아이콘 lazy 생성 후 노출 */
    if (strcmp(settings_get_str("Degree"), "\xc2\xb0""F") == 0) {
        if (!bk_ui->automode_auto_f1) {
            lv_obj_t *scr = bk_ui->automode;
            bk_ui->automode_auto_f1 = lv_image_create(scr);
            _img_set_src_timed(bk_ui->automode_auto_f1, "/images/temp_f.png");
            lv_obj_set_pos(bk_ui->automode_auto_f1, 215, 215);
            lv_obj_set_size(bk_ui->automode_auto_f1, 24, 23);
            bk_ui->automode_auto_f2 = lv_image_create(scr);
            _img_set_src_timed(bk_ui->automode_auto_f2, "/images/temp_f.png");
            lv_obj_set_pos(bk_ui->automode_auto_f2, 719, 215);
            lv_obj_set_size(bk_ui->automode_auto_f2, 24, 23);
            bk_ui->automode_auto_f3 = lv_image_create(scr);
            _img_set_src_timed(bk_ui->automode_auto_f3, "/images/temp_f.png");
            lv_obj_set_pos(bk_ui->automode_auto_f3, 215, 347);
            lv_obj_set_size(bk_ui->automode_auto_f3, 24, 23);
            bk_ui->automode_auto_f4 = lv_image_create(scr);
            _img_set_src_timed(bk_ui->automode_auto_f4, "/images/temp_f.png");
            lv_obj_set_pos(bk_ui->automode_auto_f4, 719, 347);
            lv_obj_set_size(bk_ui->automode_auto_f4, 24, 23);
        }
        _img_ensure_src(bk_ui->automode_auto_f1);
        lv_obj_clear_flag(bk_ui->automode_auto_f1, LV_OBJ_FLAG_HIDDEN);
        _img_ensure_src(bk_ui->automode_auto_f2);
        lv_obj_clear_flag(bk_ui->automode_auto_f2, LV_OBJ_FLAG_HIDDEN);
        _img_ensure_src(bk_ui->automode_auto_f3);
        lv_obj_clear_flag(bk_ui->automode_auto_f3, LV_OBJ_FLAG_HIDDEN);
        _img_ensure_src(bk_ui->automode_auto_f4);
        lv_obj_clear_flag(bk_ui->automode_auto_f4, LV_OBJ_FLAG_HIDDEN);
    } else {
        if (bk_ui->automode_auto_f1) {
            lv_obj_add_flag(bk_ui->automode_auto_f1, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(bk_ui->automode_auto_f2, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(bk_ui->automode_auto_f3, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(bk_ui->automode_auto_f4, LV_OBJ_FLAG_HIDDEN);
        }
    }

    ui_lang_apply_automode(bk_ui);
}
