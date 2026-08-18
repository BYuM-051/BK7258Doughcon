#include "lvgl.h"
#include "beken_ui.h"
#include "custom_func.h"
#include "event_runtime.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "ui_animations.h"
#include "ui_lang.h"
#include "settings.h"
#include "hardware_hal.h"
#include "rtc_sync.h"

extern bk_lv_ui_t bk_lv_tool_ui;
extern void lv_digital_clock_set_datetime(int yr, int mo, int day, int hr, int mn, int sc);

static uint32_t s_last_click_smt      = 0;
static uint32_t s_last_click_hide_smt = 0;

/* s_tci_smt: 0=닫힘 1=year 2=month 3=day 4=hour 5=min
 * s_mode_smt: 0=날짜 1=시간 */
static int  s_tci_smt      = 0;
static int  s_mode_smt     = 0;
static bool s_keypad_open  = false;
static char s_save_smt[16]     = {0};
static char s_edit_buf_smt[16] = {0};

/* ── 유틸 ────────────────────────────────────────────────────────────── */

static int _days_in_month(int y, int m)
{
    static const int dim[] = {0,31,28,31,30,31,30,31,31,30,31,30,31};
    if (m < 1 || m > 12) return 30;
    if (m == 2) { int l = (y%4==0 && y%100!=0)||(y%400==0); return l?29:28; }
    return dim[m];
}

static lv_obj_t *_target_label_smt(bk_lv_ui_t *bk_ui)
{
    switch (s_tci_smt) {
        case 1: return bk_ui->settingmodetime_year_txt;
        case 2: return bk_ui->settingmodetime_month_txt;
        case 3: return bk_ui->settingmodetime_day_txt;
        case 4: return bk_ui->settingmodetime_hour_txt;
        case 5: return bk_ui->settingmodetime_min_txt;
        default: return NULL;
    }
}

static int _max_digits_smt(void) { return (s_tci_smt == 1) ? 4 : 2; }

/* ── 언더바 ───────────────────────────────────────────────────────────── */

static void _underbar_all_hide_smt(bk_lv_ui_t *bk_ui)
{
#define _UBH(f) do { if (bk_ui->f) lv_obj_add_flag(bk_ui->f, LV_OBJ_FLAG_HIDDEN); } while(0)
    _UBH(settingmodetime_year_ub);  _UBH(settingmodetime_month_ub); _UBH(settingmodetime_day_ub);
    _UBH(settingmodetime_hour_ub);  _UBH(settingmodetime_min_ub);
#undef _UBH
}

static void _underbar_show_smt(bk_lv_ui_t *bk_ui)
{
    lv_obj_t *ub = NULL;
    switch (s_tci_smt) {
        case 1: ub = bk_ui->settingmodetime_year_ub;  break;
        case 2: ub = bk_ui->settingmodetime_month_ub; break;
        case 3: ub = bk_ui->settingmodetime_day_ub;   break;
        case 4: ub = bk_ui->settingmodetime_hour_ub;  break;
        case 5: ub = bk_ui->settingmodetime_min_ub;   break;
        default: break;
    }
    if (ub) {
        lv_obj_set_style_bg_color(ub, lv_color_hex(0x000000), 0);
        lv_obj_clear_flag(ub, LV_OBJ_FLAG_HIDDEN);
    }
}

/* ── 색상 유효성 검증 ─────────────────────────────────────────────────── */

static void _update_field_color(bk_lv_ui_t *bk_ui)
{
    lv_obj_t *lbl = _target_label_smt(bk_ui);
    if (!lbl) return;
    const char *txt = lv_label_get_text(lbl);
    if (!txt || txt[0] == '\0') return;
    int v = atoi(txt);
    bool red = false;
    switch (s_tci_smt) {
        case 1: red = (v < 2020 || v > 2099); break;
        case 2: red = (v < 1 || v > 12); break;
        case 3: {
            int yr = atoi(lv_label_get_text(bk_ui->settingmodetime_year_txt));
            int mo = atoi(lv_label_get_text(bk_ui->settingmodetime_month_txt));
            red = (v < 1 || v > _days_in_month(yr, mo)); break;
        }
        case 4: red = (v < 0 || v > 23); break;
        case 5: red = (v < 0 || v > 59); break;
        default: break;
    }
    lv_obj_set_style_text_color(lbl, red ? lv_color_hex(0xFF2020) : lv_color_hex(0x3C3A3D), 0);
}

static void _reset_all_field_colors(bk_lv_ui_t *bk_ui)
{
    lv_color_t c = lv_color_hex(0x3C3A3D);
#define _RC(f) do { if (bk_ui->f) lv_obj_set_style_text_color(bk_ui->f, c, 0); } while(0)
    _RC(settingmodetime_year_txt); _RC(settingmodetime_month_txt); _RC(settingmodetime_day_txt);
    _RC(settingmodetime_hour_txt); _RC(settingmodetime_min_txt);
#undef _RC
}

/* ── AM/PM 라벨 갱신 ─────────────────────────────────────────────────── */

static void _update_ampm_label(bk_lv_ui_t *bk_ui)
{
    if (!bk_ui->settingmodetime_ampm_txt) return;
    if (!bk_ui->settingmodetime_hour_txt) return;
    int h = atoi(lv_label_get_text(bk_ui->settingmodetime_hour_txt));
    lv_label_set_text(bk_ui->settingmodetime_ampm_txt, h < 12 ? "AM" : "PM");
}

/* ── 키패드 lazy 생성 + on/off ───────────────────────────────────────── */

static void _keypad_on_smt(bk_lv_ui_t *bk_ui)
{
    if (!bk_ui->settingmodetime_keypadbaseim) {
        bk_ui->settingmodetime_keypadbaseim = lv_image_create(bk_ui->settingmodetime);
        lv_obj_add_flag(bk_ui->settingmodetime_keypadbaseim, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_pos(bk_ui->settingmodetime_keypadbaseim, 0, 430);
    }
    /* 다른 화면(automode 등)과 동일하게 언어별 경로로 매번 갱신 —
     * 기존엔 "keypadn.jpg" 고정이라 언어 변경이 반영되지 않았음. */
    {
        int _lang = settings_get_int("LANGUAGE");
        const char *_lsuf = (_lang == 1) ? "_china" : (_lang == 2) ? "_english" : "";
        char _kp[64];
        snprintf(_kp, sizeof(_kp), "/images/keypadn%s.jpg", _lsuf);
        _img_set_src_timed(bk_ui->settingmodetime_keypadbaseim, _kp);
    }
    if (!bk_ui->settingmodetime_KeyPadBt[0]) {
        static const char *kp[12] = {
            "keypad1","keypad2","keypad3","keypad4",
            "keypad5","keypad6","keypad7","keypad8",
            "keypad9","keypad0","keypadminor","keypadback"
        };
        char buf[64];
        for (int i = 0; i < 12; i++) {
            bk_ui->settingmodetime_KeyPadBt[i] = lv_button_create(bk_ui->settingmodetime);
            lv_obj_set_style_bg_opa(bk_ui->settingmodetime_KeyPadBt[i], 0, 0);
            lv_obj_set_style_border_width(bk_ui->settingmodetime_KeyPadBt[i], 0, 0);
            lv_obj_set_style_shadow_width(bk_ui->settingmodetime_KeyPadBt[i], 0, 0);
            lv_obj_set_pos(bk_ui->settingmodetime_KeyPadBt[i], 20 + i * 72, 453);
            lv_obj_set_size(bk_ui->settingmodetime_KeyPadBt[i], 65, 75);
            lv_obj_add_event_cb(bk_ui->settingmodetime_KeyPadBt[i],
                                settingmodetime_keypad_event_cb,
                                LV_EVENT_ALL, (void *)(intptr_t)i);
            snprintf(buf, sizeof(buf), "/images/%s.png", kp[i]);
            bk_ui->settingmodetime_KeyPadIm[i] = lv_image_create(bk_ui->settingmodetime);
            _img_set_src_timed(bk_ui->settingmodetime_KeyPadIm[i], buf);
            lv_obj_set_pos(bk_ui->settingmodetime_KeyPadIm[i], 20 + i * 72, 453);
            lv_obj_set_size(bk_ui->settingmodetime_KeyPadIm[i], 65, 75);
            lv_obj_add_flag(bk_ui->settingmodetime_KeyPadIm[i], LV_OBJ_FLAG_HIDDEN);
            lv_obj_remove_flag(bk_ui->settingmodetime_KeyPadIm[i], LV_OBJ_FLAG_CLICKABLE);
        }
        bk_ui->settingmodetime_keypadhide = lv_button_create(bk_ui->settingmodetime);
        lv_obj_set_style_bg_opa(bk_ui->settingmodetime_keypadhide, 0, 0);
        lv_obj_set_style_border_width(bk_ui->settingmodetime_keypadhide, 0, 0);
        lv_obj_set_style_shadow_width(bk_ui->settingmodetime_keypadhide, 0, 0);
        lv_obj_set_pos(bk_ui->settingmodetime_keypadhide, 884, 453);
        lv_obj_set_size(bk_ui->settingmodetime_keypadhide, 120, 75);
        lv_obj_add_event_cb(bk_ui->settingmodetime_keypadhide,
                            settingmodetime_keypadhide_event_cb, LV_EVENT_ALL, NULL);
        lv_obj_add_flag(bk_ui->settingmodetime_keypadhide, LV_OBJ_FLAG_HIDDEN);

        bk_ui->settingmodetime_keypadhide_im = lv_image_create(bk_ui->settingmodetime);
        lv_obj_set_pos(bk_ui->settingmodetime_keypadhide_im, 884, 453);
        lv_obj_set_size(bk_ui->settingmodetime_keypadhide_im, 120, 75);
        lv_obj_remove_flag(bk_ui->settingmodetime_keypadhide_im, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_flag(bk_ui->settingmodetime_keypadhide_im, LV_OBJ_FLAG_HIDDEN);
    }
    {
        int _lang = settings_get_int("LANGUAGE");
        const char *_lsuf = (_lang == 1) ? "_china" : (_lang == 2) ? "_english" : "";
        char _kc[64];
        snprintf(_kc, sizeof(_kc), "/images/keypadback_close%s.png", _lsuf);
        _img_set_src_timed(bk_ui->settingmodetime_keypadhide_im, _kc);
    }
    for (int i = 0; i < 12; i++)
        lv_obj_add_flag(bk_ui->settingmodetime_KeyPadBt[i], LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(bk_ui->settingmodetime_keypadhide, LV_OBJ_FLAG_HIDDEN);
    /* keypadhide_im: 항상 보이지 않고 눌렀을 때만 표시(press feedback) — event_cb에서 처리 */

    if (!s_keypad_open) {
        ui_keypad_slide_on(bk_ui->settingmodetime_keypadbaseim);
        s_keypad_open = true;
    }
}

static void _keypad_off_smt(bk_lv_ui_t *bk_ui)
{
    if (!bk_ui->settingmodetime_KeyPadBt[0]) return;
    for (int i = 0; i < 12; i++) {
        lv_obj_clear_flag(bk_ui->settingmodetime_KeyPadBt[i], LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_flag(bk_ui->settingmodetime_KeyPadIm[i],   LV_OBJ_FLAG_HIDDEN);
    }
    lv_obj_add_flag(bk_ui->settingmodetime_keypadhide, LV_OBJ_FLAG_HIDDEN);
    if (bk_ui->settingmodetime_keypadhide_im)
        lv_obj_add_flag(bk_ui->settingmodetime_keypadhide_im, LV_OBJ_FLAG_HIDDEN);
    if (bk_ui->settingmodetime_keypadbaseim && s_keypad_open)
        ui_keypad_slide_off(bk_ui->settingmodetime_keypadbaseim);
    s_keypad_open = false;
}

/* ── 범위 검증 + 보정 ────────────────────────────────────────────────── */

static void _maxmin_smt(bk_lv_ui_t *bk_ui)
{
    char buf[8];
    int v;
#define _CLAMP(lbl, lo, hi, fmt) do { \
    v = atoi(lv_label_get_text(bk_ui->lbl)); \
    if (v < (lo)) v = (lo); if (v > (hi)) v = (hi); \
    snprintf(buf, sizeof(buf), fmt, v); \
    lv_label_set_text(bk_ui->lbl, buf); \
} while(0)

    _CLAMP(settingmodetime_year_txt,  2020, 2099, "%04d");
    _CLAMP(settingmodetime_month_txt, 1,    12,   "%02d");
    {
        int yr = atoi(lv_label_get_text(bk_ui->settingmodetime_year_txt));
        int mo = atoi(lv_label_get_text(bk_ui->settingmodetime_month_txt));
        v = atoi(lv_label_get_text(bk_ui->settingmodetime_day_txt));
        int mx = _days_in_month(yr, mo);
        if (v < 1) v = 1;
        if (v > mx) v = mx;
        snprintf(buf, sizeof(buf), "%02d", v);
        lv_label_set_text(bk_ui->settingmodetime_day_txt, buf);
    }
    _CLAMP(settingmodetime_hour_txt, 0, 23, "%02d");
    _CLAMP(settingmodetime_min_txt,  0, 59, "%02d");
#undef _CLAMP

    _reset_all_field_colors(bk_ui);
    _update_ampm_label(bk_ui);
}

/* ── 필드 이동 ───────────────────────────────────────────────────────── */

static void _switch_field_smt(bk_lv_ui_t *bk_ui, int new_tci)
{
    /* 빈 버퍼 → 이전 저장값 복원, 색 리셋 */
    lv_obj_t *lbl = _target_label_smt(bk_ui);
    if (lbl) {
        if (s_edit_buf_smt[0] == '\0')
            lv_label_set_text(lbl, s_save_smt);
        lv_obj_set_style_text_color(lbl, lv_color_hex(0x3C3A3D), 0);
    }

    s_tci_smt = new_tci;
    s_edit_buf_smt[0] = '\0';

    lbl = _target_label_smt(bk_ui);
    if (lbl) {
        strncpy(s_save_smt, lv_label_get_text(lbl), sizeof(s_save_smt) - 1);
        s_save_smt[sizeof(s_save_smt)-1] = '\0';
    }
    _underbar_all_hide_smt(bk_ui);
    _underbar_show_smt(bk_ui);
}

/* ── 키패드 입력 ─────────────────────────────────────────────────────── */

static void _keypad_input_smt(bk_lv_ui_t *bk_ui, char digit)
{
    if (s_tci_smt == 0) return;
    int _max = _max_digits_smt();
    size_t _len = strlen(s_edit_buf_smt);
    if ((int)_len >= _max) {
        memmove(s_edit_buf_smt, s_edit_buf_smt + 1, _len - 1);
        s_edit_buf_smt[_len - 1] = digit;
        s_edit_buf_smt[_len]     = '\0';
    } else {
        s_edit_buf_smt[_len]     = digit;
        s_edit_buf_smt[_len + 1] = '\0';
    }
    lv_obj_t *lbl = _target_label_smt(bk_ui);
    if (lbl) lv_label_set_text(lbl, s_edit_buf_smt);

    _update_field_color(bk_ui);
    if (s_tci_smt == 4) _update_ampm_label(bk_ui);  /* 시 입력 시 AM/PM 즉시 반영 */

    /* 자리수 완성 → 다음 필드 자동 이동 */
    if ((int)strlen(s_edit_buf_smt) == _max) {
        int next = 0;
        if      (s_tci_smt == 1) next = 2;
        else if (s_tci_smt == 2) next = 3;
        else if (s_tci_smt == 4) next = 5;
        if (next) _switch_field_smt(bk_ui, next);
    }
}

static void _keypad_backspace_smt(bk_lv_ui_t *bk_ui)
{
    if (s_tci_smt == 0) return;
    size_t _len = strlen(s_edit_buf_smt);
    if (_len > 0) s_edit_buf_smt[_len - 1] = '\0';
    lv_obj_t *lbl = _target_label_smt(bk_ui);
    if (lbl) lv_label_set_text(lbl,
        s_edit_buf_smt[0] != '\0' ? s_edit_buf_smt : s_save_smt);
    _update_field_color(bk_ui);
    if (s_tci_smt == 4) _update_ampm_label(bk_ui);
}

/* ── RTC 저장 ────────────────────────────────────────────────────────── */

static void _commit_datetime_smt(bk_lv_ui_t *bk_ui)
{
    int y  = atoi(lv_label_get_text(bk_ui->settingmodetime_year_txt));
    int mo = atoi(lv_label_get_text(bk_ui->settingmodetime_month_txt));
    int d  = atoi(lv_label_get_text(bk_ui->settingmodetime_day_txt));
    int h  = atoi(lv_label_get_text(bk_ui->settingmodetime_hour_txt));
    int mi = atoi(lv_label_get_text(bk_ui->settingmodetime_min_txt));

    hal_rtc_set(y, mo, d, h, mi, 0);
    rtc_sync_save_now();
    lv_digital_clock_set_datetime(y, mo, d, h, mi, 0);

    char date_buf[20], time_buf[24];
    snprintf(date_buf, sizeof(date_buf), "%d.%02d.%02d", y, mo, d);
    int hour12 = h % 12;
    snprintf(time_buf, sizeof(time_buf), "%s.  %02d : %02d",
             h < 12 ? "AM" : "PM", hour12 == 0 ? 12 : hour12, mi);

    lv_label_set_text(bk_ui->settingmodetime_setting_time_setdate, date_buf);
    lv_label_set_text(bk_ui->settingmodetime_setting_time_settime, time_buf);
    settings_set_str("settingday",  date_buf);
    settings_set_str("settingtime", time_buf);
    printf("[TIME] RTC %d.%02d.%02d %02d:%02d\n", y, mo, d, h, mi);
}

/* ── RTC 값으로 입력 필드 초기화 ────────────────────────────────────── */

static void _init_input_from_rtc(bk_lv_ui_t *bk_ui)
{
    int y, mo, d, h, mi, sc;
    hal_rtc_get(&y, &mo, &d, &h, &mi, &sc);
    if (y < 2020 || y > 2099) {
        const char *sd = settings_get_str("settingday");
        if (!sd || sscanf(sd, "%d.%d.%d", &y, &mo, &d) != 3)
            { y = 2026; mo = 1; d = 1; }
        h = 0; mi = 0;
    }
    char buf[8];
    snprintf(buf, sizeof(buf), "%04d", y);  lv_label_set_text(bk_ui->settingmodetime_year_txt,  buf);
    snprintf(buf, sizeof(buf), "%02d", mo); lv_label_set_text(bk_ui->settingmodetime_month_txt, buf);
    snprintf(buf, sizeof(buf), "%02d", d);  lv_label_set_text(bk_ui->settingmodetime_day_txt,   buf);
    snprintf(buf, sizeof(buf), "%02d", h);  lv_label_set_text(bk_ui->settingmodetime_hour_txt,  buf);
    snprintf(buf, sizeof(buf), "%02d", mi); lv_label_set_text(bk_ui->settingmodetime_min_txt,   buf);
}

/* ── keypad hide (확인) ──────────────────────────────────────────────── */

static void _keypad_hide_smt(bk_lv_ui_t *bk_ui)
{
    _maxmin_smt(bk_ui);
    _commit_datetime_smt(bk_ui);

    _underbar_all_hide_smt(bk_ui);
    lv_obj_add_flag(bk_ui->settingmodetime_ampm_bt, LV_OBJ_FLAG_HIDDEN);
    _keypad_off_smt(bk_ui);
    /* 필드 라벨(연/월/일, 시/분, AM-PM)은 항상 표시되므로 숨기거나 복원할 필요 없음 */

    s_tci_smt = 0;
    s_edit_buf_smt[0] = '\0';
}

/* ── 편집 모드 열기 ──────────────────────────────────────────────────── */

static void _open_date_edit(bk_lv_ui_t *bk_ui)
{
    /* 이미 날짜 편집 중이면 무시 */
    if (s_tci_smt != 0 && s_mode_smt == 0) return;

    /* 시간 모드로 이미 열려있다면 silent commit 후 전환 */
    if (s_tci_smt != 0 && s_mode_smt == 1) {
        _maxmin_smt(bk_ui);
        _commit_datetime_smt(bk_ui);
        _underbar_all_hide_smt(bk_ui);
        lv_obj_add_flag(bk_ui->settingmodetime_ampm_bt, LV_OBJ_FLAG_HIDDEN);
        s_tci_smt = 0; s_edit_buf_smt[0] = '\0';
    }

    _init_input_from_rtc(bk_ui);
    _reset_all_field_colors(bk_ui);
    s_mode_smt = 0;
    /* 필드 라벨은 항상 표시 상태이므로 별도로 보이기/숨기기 불필요 */

    /* year 필드부터 시작 */
    s_tci_smt = 1;
    s_edit_buf_smt[0] = '\0';
    strncpy(s_save_smt, lv_label_get_text(bk_ui->settingmodetime_year_txt),
            sizeof(s_save_smt) - 1);
    s_save_smt[sizeof(s_save_smt)-1] = '\0';

    _underbar_all_hide_smt(bk_ui);
    _underbar_show_smt(bk_ui);
    _keypad_on_smt(bk_ui);
}

static void _open_time_edit(bk_lv_ui_t *bk_ui)
{
    /* 이미 시간 편집 중이면 무시 */
    if (s_tci_smt != 0 && s_mode_smt == 1) return;

    /* 날짜 모드로 이미 열려있다면 silent commit 후 전환 */
    if (s_tci_smt != 0 && s_mode_smt == 0) {
        _maxmin_smt(bk_ui);
        _commit_datetime_smt(bk_ui);
        _underbar_all_hide_smt(bk_ui);
        s_tci_smt = 0; s_edit_buf_smt[0] = '\0';
    }

    _init_input_from_rtc(bk_ui);
    _reset_all_field_colors(bk_ui);
    s_mode_smt = 1;
    /* 필드 라벨은 항상 표시 상태이므로 별도로 보이기/숨기기 불필요.
     * ampm_bt(탭 영역)만 시간 편집 중에 활성화. */
    lv_obj_clear_flag(bk_ui->settingmodetime_ampm_bt, LV_OBJ_FLAG_HIDDEN);
    _update_ampm_label(bk_ui);

    /* hour 필드부터 시작 */
    s_tci_smt = 4;
    s_edit_buf_smt[0] = '\0';
    strncpy(s_save_smt, lv_label_get_text(bk_ui->settingmodetime_hour_txt),
            sizeof(s_save_smt) - 1);
    s_save_smt[sizeof(s_save_smt)-1] = '\0';

    _underbar_all_hide_smt(bk_ui);
    _underbar_show_smt(bk_ui);
    _keypad_on_smt(bk_ui);
}

/* ── 공개 콜백 ───────────────────────────────────────────────────────── */

void settingmodetime_backbt_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    if (lv_tick_elaps(s_last_click_smt) < 250) return;
    s_last_click_smt = lv_tick_get();
    hal_buzzer_beep();

    if (s_tci_smt != 0) {
        _maxmin_smt(bk_ui);
        _commit_datetime_smt(bk_ui);
        _underbar_all_hide_smt(bk_ui);
        lv_obj_add_flag(bk_ui->settingmodetime_ampm_bt, LV_OBJ_FLAG_HIDDEN);
        _keypad_off_smt(bk_ui);
        s_tci_smt = 0; s_edit_buf_smt[0] = '\0';
    }
    if (bk_ui->settingmode == NULL || !lv_obj_is_valid(bk_ui->settingmode))
        init_page_settingmode(bk_ui);
    lv_scr_load(bk_ui->settingmode);
}

/* 입력 필드 직접 탭 → 해당 필드로 포커스 이동 */
void settingmodetime_field_tap_event_cb(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    if (lv_tick_elaps(s_last_click_smt) < 250) return;
    s_last_click_smt = lv_tick_get();
    hal_buzzer_beep();

    int field = (int)(intptr_t)lv_event_get_user_data(e);
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    bool date_field = (field >= 1 && field <= 3);

    /* 필드 라벨이 항상 보이므로, 편집 중이 아니거나 다른 그룹을 편집 중이면
     * 해당 그룹 편집을 새로 연다 (같은 그룹이면 _open_*_edit이 안전하게 no-op). */
    if (date_field) _open_date_edit(bk_ui);
    else            _open_time_edit(bk_ui);

    if (field != s_tci_smt) _switch_field_smt(bk_ui, field);
}

void settingmodetime_setting_time_setdatebt_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    if (lv_tick_elaps(s_last_click_smt) < 250) return;
    s_last_click_smt = lv_tick_get();
    hal_buzzer_beep();
    _open_date_edit(bk_ui);
}

void settingmodetime_setting_time_settimebt_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    if (lv_tick_elaps(s_last_click_smt) < 250) return;
    s_last_click_smt = lv_tick_get();
    hal_buzzer_beep();
    _open_time_edit(bk_ui);
}

/* AM/PM 토글: 시 ± 12 */
void settingmodetime_ampm_bt_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    if (lv_tick_elaps(s_last_click_smt) < 250) return;
    s_last_click_smt = lv_tick_get();
    hal_buzzer_beep();

    if (!bk_ui->settingmodetime_hour_txt) return;
    int h = atoi(lv_label_get_text(bk_ui->settingmodetime_hour_txt));
    h = (h < 12) ? h + 12 : h - 12;
    if (h < 0) h = 0;
    if (h > 23) h = 23;

    char buf[4];
    snprintf(buf, sizeof(buf), "%02d", h);
    lv_label_set_text(bk_ui->settingmodetime_hour_txt, buf);
    /* 편집 버퍼도 동기화 */
    if (s_tci_smt == 4) {
        strncpy(s_edit_buf_smt, buf, sizeof(s_edit_buf_smt) - 1);
        strncpy(s_save_smt, buf, sizeof(s_save_smt) - 1);
    }
    _update_ampm_label(bk_ui);
    lv_obj_set_style_text_color(bk_ui->settingmodetime_hour_txt, lv_color_hex(0x3C3A3D), 0);
}

void settingmodetime_keypad_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    int idx = (int)(intptr_t)lv_event_get_user_data(e);
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;

    /* 이미지 오버레이: 누르는 동안만 표시 */
    if (idx >= 0 && idx < 12 && bk_ui->settingmodetime_KeyPadIm[idx]) {
        if (code == LV_EVENT_PRESSED)
            lv_obj_clear_flag(bk_ui->settingmodetime_KeyPadIm[idx], LV_OBJ_FLAG_HIDDEN);
        else if (code == LV_EVENT_RELEASED || code == LV_EVENT_PRESS_LOST)
            lv_obj_add_flag(bk_ui->settingmodetime_KeyPadIm[idx], LV_OBJ_FLAG_HIDDEN);
    }

    if (code != LV_EVENT_CLICKED) return;
    if (lv_tick_elaps(s_last_click_smt) < 250) return;
    s_last_click_smt = lv_tick_get();
    hal_buzzer_beep();

    if      (idx < 9)  _keypad_input_smt(bk_ui, (char)('1' + idx));
    else if (idx == 9) _keypad_input_smt(bk_ui, '0');
    else if (idx == 10) { /* 소수점 — 정수 입력, skip */ }
    else if (idx == 11) _keypad_backspace_smt(bk_ui);
}

void settingmodetime_keypadhide_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_PRESSED) {
        if (bk_ui->settingmodetime_keypadhide_im) lv_obj_clear_flag(bk_ui->settingmodetime_keypadhide_im, LV_OBJ_FLAG_HIDDEN);
        return;
    }
    if (code == LV_EVENT_RELEASED || code == LV_EVENT_PRESS_LOST) {
        if (bk_ui->settingmodetime_keypadhide_im) lv_obj_add_flag(bk_ui->settingmodetime_keypadhide_im, LV_OBJ_FLAG_HIDDEN);
    }
    if (code != LV_EVENT_CLICKED) return;
    if (lv_tick_elaps(s_last_click_hide_smt) < 250) return;
    s_last_click_hide_smt = lv_tick_get();
    hal_buzzer_beep();
    _keypad_hide_smt(bk_ui);
}

void settingmodetime_load_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;

    if (code == LV_EVENT_SCREEN_UNLOAD_START) {
        if (s_tci_smt != 0) {
            /* 확정 없이 화면을 벗어남 — 기존과 동일하게 편집 내용은 커밋하지 않고
             * 키패드/언더바만 정리 (필드 라벨은 항상 표시이므로 별도 처리 불필요) */
            _keypad_off_smt(bk_ui);
            _underbar_all_hide_smt(bk_ui);
            lv_obj_add_flag(bk_ui->settingmodetime_ampm_bt, LV_OBJ_FLAG_HIDDEN);
            s_tci_smt = 0; s_edit_buf_smt[0] = '\0';
        }
        return;
    }

    if (code == LV_EVENT_SCREEN_LOAD_START) {
        ui_lang_apply_settingmodetime(bk_ui);
        /* 진입 시부터 필드 분리 표시(연/월/일, 시/분, AM-PM)를 현재 RTC 값으로 채움 */
        _init_input_from_rtc(bk_ui);
        _reset_all_field_colors(bk_ui);
        _update_ampm_label(bk_ui);
        return;
    }

    if (code == LV_EVENT_SCREEN_LOADED) {
        ui_title_anim(bk_ui->settingmodetime_title);
        return;
    }
}
