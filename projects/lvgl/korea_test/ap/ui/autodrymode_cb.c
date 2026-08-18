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
#include "hardware_hal.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

extern bk_lv_ui_t bk_lv_tool_ui;

static uint32_t s_last_click_autodrymode = 0;
static uint32_t s_last_click_hide_adm   = 0;
static int      s_tci_autodrymode        = 0;
static char     s_save_autodrymode[32]   = {0};
static char     s_edit_buf_autodrymode[32] = {0};
static lv_timer_t *s_ui_timer_adm     = NULL;
static int         s_dry_cfg_min        = 0;   /* 설정 건조시간(분) — 시간 갱신 활성화 플래그 (0=정지) */
static lv_timer_t *s_run_anim_timer_adm = NULL;
static int32_t     s_arc_angle_adm      = 0;
static int32_t     s_gif_angle_adm      = 0;
/* 조건부 갱신 캐시 — 동일값 반복 set 시 lv_malloc+lv_obj_invalidate 방지 */
static int         s_last_temp_adm      = 0x7FFFFFFF;
static int         s_last_hum_adm       = 0x7FFFFFFF;
static int         s_last_remain_adm    = -1;

void autodrymode_backbt_event_cb(lv_event_t *e);
void autodrymode_auto_dry_temp_bt_event_cb(lv_event_t *e);
void autodrymode_auto_dry_humidity_bt_event_cb(lv_event_t *e);
void autodrymode_auto_dry_hour_bt_event_cb(lv_event_t *e);
void autodrymode_auto_dry_min_bt_event_cb(lv_event_t *e);
void autodrymode_auto_dry_start_event_cb(lv_event_t *e);
void autodrymode_keypad_event_cb(lv_event_t *e);
void autodrymode_keypadhide_event_cb(lv_event_t *e);
void autodrymode_load_event_cb(lv_event_t *e);

static lv_obj_t *_get_target_label_autodrymode(bk_lv_ui_t *bk_ui)
{
    switch (s_tci_autodrymode) {
        case 1: return bk_ui->autodrymode_auto_dry_temp_txt;
        case 2: return bk_ui->autodrymode_auto_dry_humidity_txt;
        case 3: return bk_ui->autodrymode_auto_dry_hour_txt;
        case 4: return bk_ui->autodrymode_auto_dry_min_txt;
        default: return NULL;
    }
}

static lv_obj_t *_make_underbar_adm(lv_obj_t *parent, lv_color_t color,
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

static void _ensure_underbars_created_adm(bk_lv_ui_t *bk_ui)
{
    if (bk_ui->autodrymode_auto_dry_temp_underbar) return;
    lv_obj_t *scr = bk_ui->autodrymode;
    bk_ui->autodrymode_auto_dry_temp_underbar     = _make_underbar_adm(scr, lv_color_hex(0x38C010), 417, 325+3, 75, 7);
    bk_ui->autodrymode_auto_dry_humidity_underbar = _make_underbar_adm(scr, lv_color_hex(0x38C010), 528, 325+3, 60, 7);
    bk_ui->autodrymode_auto_dry_hour_underbar     = _make_underbar_adm(scr, lv_color_hex(0x000000), 875, 285, 40, 6);
    bk_ui->autodrymode_auto_dry_min_underbar      = _make_underbar_adm(scr, lv_color_hex(0x000000), 941, 285, 40, 6);
}

static void _underbar_all_hide_autodrymode(bk_lv_ui_t *bk_ui)
{
    if (bk_ui->autodrymode_auto_dry_temp_underbar)
        lv_obj_add_flag(bk_ui->autodrymode_auto_dry_temp_underbar,    LV_OBJ_FLAG_HIDDEN);
    if (bk_ui->autodrymode_auto_dry_humidity_underbar)
        lv_obj_add_flag(bk_ui->autodrymode_auto_dry_humidity_underbar, LV_OBJ_FLAG_HIDDEN);
    if (bk_ui->autodrymode_auto_dry_hour_underbar)
        lv_obj_add_flag(bk_ui->autodrymode_auto_dry_hour_underbar,     LV_OBJ_FLAG_HIDDEN);
    if (bk_ui->autodrymode_auto_dry_min_underbar)
        lv_obj_add_flag(bk_ui->autodrymode_auto_dry_min_underbar,      LV_OBJ_FLAG_HIDDEN);
}

static void _underbar_show_autodrymode(bk_lv_ui_t *bk_ui)
{
    switch (s_tci_autodrymode) {
        case 1: lv_obj_clear_flag(bk_ui->autodrymode_auto_dry_temp_underbar,     LV_OBJ_FLAG_HIDDEN); break;
        case 2: lv_obj_clear_flag(bk_ui->autodrymode_auto_dry_humidity_underbar, LV_OBJ_FLAG_HIDDEN); break;
        case 3: lv_obj_clear_flag(bk_ui->autodrymode_auto_dry_hour_underbar,     LV_OBJ_FLAG_HIDDEN); break;
        case 4: lv_obj_clear_flag(bk_ui->autodrymode_auto_dry_min_underbar,      LV_OBJ_FLAG_HIDDEN); break;
        default: break;
    }
}

static void _keypad_on_autodrymode(bk_lv_ui_t *bk_ui)
{
    if (!bk_ui->autodrymode_keypadbaseim) {
        bk_ui->autodrymode_keypadbaseim = lv_image_create(bk_ui->autodrymode);
        lv_obj_add_flag(bk_ui->autodrymode_keypadbaseim, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_pos(bk_ui->autodrymode_keypadbaseim, 0, 430);
    }
    int _lang = settings_get_int("LANGUAGE");
    const char *_lsuf = (_lang == 1) ? "_china" : (_lang == 2) ? "_english" : "";
    {
        char _kp[64];
        snprintf(_kp, sizeof(_kp), "/images/keypadn%s.jpg", _lsuf);
        _img_set_src_timed(bk_ui->autodrymode_keypadbaseim, _kp);
    }
    if (!bk_ui->autodrymode_KeyPadBt[0]) {
        static const char *dry_names[12] = {
            "dry_keypad1","dry_keypad2","dry_keypad3","dry_keypad4",
            "dry_keypad5","dry_keypad6","dry_keypad7","dry_keypad8",
            "dry_keypad9","dry_keypad0","dry_keypadminor","dry_keypadback"
        };
        char buf[64];
        for (int i = 0; i < 12; i++) {
            bk_ui->autodrymode_KeyPadBt[i] = lv_button_create(bk_ui->autodrymode);
            lv_obj_set_style_bg_opa(bk_ui->autodrymode_KeyPadBt[i], 0, 0);
            lv_obj_set_style_border_width(bk_ui->autodrymode_KeyPadBt[i], 0, 0);
            lv_obj_set_style_shadow_width(bk_ui->autodrymode_KeyPadBt[i], 0, 0);
            lv_obj_set_pos(bk_ui->autodrymode_KeyPadBt[i], 20 + i * 72, 453);
            lv_obj_set_size(bk_ui->autodrymode_KeyPadBt[i], 65, 75);
            lv_obj_add_event_cb(bk_ui->autodrymode_KeyPadBt[i], autodrymode_keypad_event_cb,
                                LV_EVENT_ALL, (void *)(intptr_t)i);

            snprintf(buf, sizeof(buf), "/images/%s.png", dry_names[i]);
            bk_ui->autodrymode_KeyPadIm[i] = lv_image_create(bk_ui->autodrymode);
            _img_set_src_timed(bk_ui->autodrymode_KeyPadIm[i], buf);
            lv_obj_set_pos(bk_ui->autodrymode_KeyPadIm[i], 20 + i * 72, 453);
            lv_obj_set_size(bk_ui->autodrymode_KeyPadIm[i], 65, 75);
            lv_obj_add_flag(bk_ui->autodrymode_KeyPadIm[i], LV_OBJ_FLAG_HIDDEN);
            lv_obj_remove_flag(bk_ui->autodrymode_KeyPadIm[i], LV_OBJ_FLAG_CLICKABLE);
        }
        bk_ui->autodrymode_keypadhide = lv_button_create(bk_ui->autodrymode);
        lv_obj_set_style_bg_opa(bk_ui->autodrymode_keypadhide, 0, 0);
        lv_obj_set_style_border_width(bk_ui->autodrymode_keypadhide, 0, 0);
        lv_obj_set_style_shadow_width(bk_ui->autodrymode_keypadhide, 0, 0);
        lv_obj_set_pos(bk_ui->autodrymode_keypadhide, 884, 453);
        lv_obj_set_size(bk_ui->autodrymode_keypadhide, 120, 75);
        lv_obj_add_event_cb(bk_ui->autodrymode_keypadhide, autodrymode_keypadhide_event_cb,
                            LV_EVENT_ALL, NULL);
        lv_obj_add_flag(bk_ui->autodrymode_keypadhide, LV_OBJ_FLAG_HIDDEN);

        bk_ui->autodrymode_keypadhide_im = lv_image_create(bk_ui->autodrymode);
        lv_obj_set_pos(bk_ui->autodrymode_keypadhide_im, 884, 453);
        lv_obj_set_size(bk_ui->autodrymode_keypadhide_im, 120, 75);
        lv_obj_remove_flag(bk_ui->autodrymode_keypadhide_im, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_flag(bk_ui->autodrymode_keypadhide_im, LV_OBJ_FLAG_HIDDEN);
    }
    {
        char _kc[64];
        snprintf(_kc, sizeof(_kc), "/images/keypadback_close%s.png", _lsuf);
        _img_set_src_timed(bk_ui->autodrymode_keypadhide_im, _kc);
    }
    for (int i = 0; i < 12; i++)
        lv_obj_add_flag(bk_ui->autodrymode_KeyPadBt[i], LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(bk_ui->autodrymode_keypadhide, LV_OBJ_FLAG_HIDDEN);
    /* keypadhide_im: 항상 보이지 않고 눌렀을 때만 표시(press feedback) — event_cb에서 처리 */
    ui_keypad_slide_on(bk_ui->autodrymode_keypadbaseim);
}

static void _keypad_off_autodrymode(bk_lv_ui_t *bk_ui)
{
    if (!bk_ui->autodrymode_KeyPadBt[0]) return;
    for (int i = 0; i < 12; i++) {
        lv_obj_clear_flag(bk_ui->autodrymode_KeyPadBt[i], LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_flag(bk_ui->autodrymode_KeyPadIm[i],  LV_OBJ_FLAG_HIDDEN);
    }
    lv_obj_add_flag(bk_ui->autodrymode_keypadhide, LV_OBJ_FLAG_HIDDEN);
    if (bk_ui->autodrymode_keypadhide_im)
        lv_obj_add_flag(bk_ui->autodrymode_keypadhide_im, LV_OBJ_FLAG_HIDDEN);
    if (bk_ui->autodrymode_keypadbaseim)
        ui_keypad_slide_off(bk_ui->autodrymode_keypadbaseim);
}

static void _maxmin_autodrymode(bk_lv_ui_t *bk_ui)
{
    int is_f = (strcmp(settings_get_str("Degree"), "\xc2\xb0""F") == 0);
    char _buf[16];
    int _v;
    _v = atoi(lv_label_get_text(bk_ui->autodrymode_auto_dry_temp_txt));
    if (is_f) { if (_v < 95) _v = 95; if (_v > 113) _v = 113; }   /* F: 95~113 */
    else       { if (_v < 35) _v = 35; if (_v > 45)  _v = 45;  }   /* C: 35~45 */
    snprintf(_buf, sizeof(_buf), "%02d", _v);
    lv_label_set_text(bk_ui->autodrymode_auto_dry_temp_txt, _buf);
    _v = atoi(lv_label_get_text(bk_ui->autodrymode_auto_dry_humidity_txt));
    if (_v < 20) _v = 20;
    if (_v > 30) _v = 30;
    snprintf(_buf, sizeof(_buf), "%02d", _v);
    lv_label_set_text(bk_ui->autodrymode_auto_dry_humidity_txt, _buf);
    _v = atoi(lv_label_get_text(bk_ui->autodrymode_auto_dry_hour_txt));
    if (_v < 0) _v = 0;
    if (_v > 10) _v = 10;
    snprintf(_buf, sizeof(_buf), "%02d", _v);
    lv_label_set_text(bk_ui->autodrymode_auto_dry_hour_txt, _buf);
    _v = atoi(lv_label_get_text(bk_ui->autodrymode_auto_dry_min_txt));
    if (_v < 0) _v = 0;
    if (_v > 59) _v = 59;
    snprintf(_buf, sizeof(_buf), "%02d", _v);
    lv_label_set_text(bk_ui->autodrymode_auto_dry_min_txt, _buf);
    /* 최소 설정시간: 10분 */
    {
        int _th = atoi(lv_label_get_text(bk_ui->autodrymode_auto_dry_hour_txt));
        int _tm = atoi(lv_label_get_text(bk_ui->autodrymode_auto_dry_min_txt));
        if (_th * 60 + _tm < 10) {
            lv_label_set_text(bk_ui->autodrymode_auto_dry_hour_txt, "00");
            lv_label_set_text(bk_ui->autodrymode_auto_dry_min_txt,  "10");
        }
    }
    /* reset all label colors to normal after clamping */
    lv_obj_set_style_text_color(bk_ui->autodrymode_auto_dry_temp_txt,     lv_color_hex(0x49B206), 0);
    lv_obj_set_style_text_color(bk_ui->autodrymode_auto_dry_humidity_txt, lv_color_hex(0x49B206), 0);
    lv_obj_set_style_text_color(bk_ui->autodrymode_auto_dry_hour_txt,     lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_color(bk_ui->autodrymode_auto_dry_min_txt,      lv_color_hex(0x3C3A3D), 0);
}

static void _common_click_autodrymode(bk_lv_ui_t *bk_ui)
{
    if (lv_tick_elaps(s_last_click_autodrymode) < 250) return;
    s_last_click_autodrymode = lv_tick_get();
    hal_buzzer_beep();
    lv_obj_t *_lbl = _get_target_label_autodrymode(bk_ui);
    if (_lbl) {
        strncpy(s_save_autodrymode, lv_label_get_text(_lbl), sizeof(s_save_autodrymode) - 1);
        s_save_autodrymode[sizeof(s_save_autodrymode) - 1] = '\0';
    }
    s_edit_buf_autodrymode[0] = '\0';
    _ensure_underbars_created_adm(bk_ui);
    _underbar_all_hide_autodrymode(bk_ui);
    _underbar_show_autodrymode(bk_ui);
    _keypad_on_autodrymode(bk_ui);
    _maxmin_autodrymode(bk_ui);
    if (_lbl && lv_label_get_text(_lbl)[0] == '\0')
        lv_label_set_text(_lbl, s_save_autodrymode);
}

static void _keypad_input_autodrymode(bk_lv_ui_t *bk_ui, char digit)
{
    if (lv_tick_elaps(s_last_click_autodrymode) < 250) return;
    s_last_click_autodrymode = lv_tick_get();
    if (s_tci_autodrymode == 0) return;
    int _max = 2;
    switch (s_tci_autodrymode) {
        case 1: _max = 3; break;
        default: _max = 2; break;
    }
    size_t _len = strlen(s_edit_buf_autodrymode);
    if ((int)_len >= _max) {
        memmove(s_edit_buf_autodrymode, s_edit_buf_autodrymode + 1, _len - 1);
        s_edit_buf_autodrymode[_len - 1] = digit;
        s_edit_buf_autodrymode[_len]     = '\0';
    } else {
        s_edit_buf_autodrymode[_len]     = digit;
        s_edit_buf_autodrymode[_len + 1] = '\0';
    }
    lv_obj_t *_lbl = _get_target_label_autodrymode(bk_ui);
    if (_lbl) lv_label_set_text(_lbl, s_edit_buf_autodrymode);
}

static void _keypad_minor_autodrymode(bk_lv_ui_t *bk_ui)
{
    /* 자동건조 온도(유일하게 이 키를 쓰던 필드)는 35~45도 정수 고정 범위라
     * 부호/소수점 입력이 필요 없음 — 키 자체를 무시하도록 비활성화. */
    (void)bk_ui;
    return;
}

static void _keypad_backspace_autodrymode(bk_lv_ui_t *bk_ui)
{
    if (lv_tick_elaps(s_last_click_autodrymode) < 250) return;
    s_last_click_autodrymode = lv_tick_get();
    if (s_tci_autodrymode == 0) return;
    size_t _len = strlen(s_edit_buf_autodrymode);
    if (_len > 0) s_edit_buf_autodrymode[_len - 1] = '\0';
    lv_obj_t *_lbl = _get_target_label_autodrymode(bk_ui);
    if (_lbl) lv_label_set_text(_lbl,
        s_edit_buf_autodrymode[0] != '\0' ? s_edit_buf_autodrymode : s_save_autodrymode);
}

static void _keypad_hide_autodrymode(bk_lv_ui_t *bk_ui)
{
    if (lv_tick_elaps(s_last_click_hide_adm) < 250) return;
    s_last_click_hide_adm = lv_tick_get();
    _maxmin_autodrymode(bk_ui);
    {
        int _is_f = (strcmp(settings_get_str("Degree"), "\xc2\xb0""F") == 0);
        const char *_sv = lv_label_get_text(bk_ui->autodrymode_auto_dry_temp_txt);
        if (_is_f && _sv && _sv[0]) {
            char _cb[16]; snprintf(_cb, sizeof(_cb), "%d", (atoi(_sv) - 32) * 5 / 9);
            settings_set_str("CurrentSaveDryTemp", _cb);
        } else settings_set_str("CurrentSaveDryTemp", _sv);
    }
    settings_set_str("CurrentSaveDryHumidity", lv_label_get_text(bk_ui->autodrymode_auto_dry_humidity_txt));
    settings_set_str("CurrentSaveDryTimeHour", lv_label_get_text(bk_ui->autodrymode_auto_dry_hour_txt));
    settings_set_str("CurrentSaveDryTimeMin",  lv_label_get_text(bk_ui->autodrymode_auto_dry_min_txt));
    uart_comm_trigger_change_setting();
    _underbar_all_hide_autodrymode(bk_ui);
    _keypad_off_autodrymode(bk_ui);
    s_tci_autodrymode = 0;
    s_edit_buf_autodrymode[0] = '\0';
}

/* ── lv_arc 기반 서클 ────────────────────────────────────────────────
 * 정지: 90° 세그먼트 고정 (12시 중앙)
 * 운전: 세그먼트 360° 무한 회전
 * ──────────────────────────────────────────────────────────────────── */
static void _arc_anim_cb_adm(void *obj, int32_t val)
{
    /* arc_rounded=false 시 LVGL의 partial invalidation이 arc 끝 픽셀을 놓쳐
     * 구 위치 찌꺼기가 남음 (manualmodestart_cb.c의 _arc_anim_cb와 동일 이슈)
     * → 각도 변경 전에 전체 영역 먼저 무효화. */
    lv_obj_invalidate((lv_obj_t *)obj);
    uint16_t start = (uint16_t)(val % 360);
    uint16_t end   = (uint16_t)((val + 90) % 360);
    lv_arc_set_angles((lv_obj_t *)obj, start, end);
}

static lv_obj_t *s_run_arc_inner_white_adm = NULL; /* 회색 배경 링 안쪽 7px 흰색 테두리 */

static lv_obj_t *_create_run_arc_adm(lv_obj_t *scr)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;

    /* 회색 배경 링 바깥쪽 5px를 흰색으로 — manualmodestart_cb.c의 _create_run_arc와 동일 구조.
     * 전체 원 크기(반지름150)는 그대로 두고, 회색 링만 5px 줄여(반지름145까지) 비운
     * 바깥쪽 5px(145~150)를 원래 크기 그대로인 흰색 링으로 채운다.
     * 회색 링: outer=145(size 290), width=25 → inner=120
     * 흰색 링: outer=150(size 300, 기존 전체 크기 그대로), width=5 → 145~150만 칠함 */
    lv_obj_t *arc = lv_arc_create(scr);
    lv_obj_set_size(arc, 290, 290);
    lv_obj_set_pos(arc, 367, 122);

    lv_arc_set_bg_angles(arc, 0, 360);
    lv_obj_set_style_arc_color(arc, lv_color_hex(0xD8D8D8), LV_PART_MAIN);
    lv_obj_set_style_arc_width(arc, 26, LV_PART_MAIN);
    lv_obj_set_style_arc_rounded(arc, false, LV_PART_MAIN);

    /* 인디케이터: 회색 링(120~145) 안쪽 경계에서 5px 여백(반지름 125~145) */
    lv_arc_set_angles(arc, 225, 315);
    lv_obj_set_style_arc_color(arc, lv_color_hex(0x4DA212), LV_PART_INDICATOR);
    lv_obj_set_style_arc_width(arc, 20, LV_PART_INDICATOR);
    lv_obj_set_style_arc_rounded(arc, false, LV_PART_INDICATOR);
    lv_obj_set_style_pad_all(arc, 0, LV_PART_INDICATOR);

    lv_obj_set_style_bg_opa(arc, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_opa(arc, LV_OPA_TRANSP, LV_PART_KNOB);
    lv_obj_set_style_pad_all(arc, 0, LV_PART_KNOB);

    lv_obj_remove_flag(arc, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(arc, LV_OBJ_FLAG_HIDDEN);
    lv_obj_move_background(arc);

    /* 바깥쪽 5px 흰색 링 — 원래 전체 크기(300x300, 반지름150) 그대로, 두께만 5 */
    bk_ui->autodrymode_run_arc_bg_white = lv_arc_create(scr);
    lv_obj_set_size(bk_ui->autodrymode_run_arc_bg_white, 300, 300);
    lv_obj_set_pos(bk_ui->autodrymode_run_arc_bg_white, 362, 117);
    lv_arc_set_bg_angles(bk_ui->autodrymode_run_arc_bg_white, 0, 360);
    lv_obj_set_style_arc_color(bk_ui->autodrymode_run_arc_bg_white, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_arc_width(bk_ui->autodrymode_run_arc_bg_white, 6, LV_PART_MAIN);
    lv_obj_set_style_arc_rounded(bk_ui->autodrymode_run_arc_bg_white, false, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(bk_ui->autodrymode_run_arc_bg_white, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_opa(bk_ui->autodrymode_run_arc_bg_white, LV_OPA_TRANSP, LV_PART_INDICATOR);
    lv_obj_set_style_opa(bk_ui->autodrymode_run_arc_bg_white, LV_OPA_TRANSP, LV_PART_KNOB);
    lv_obj_remove_flag(bk_ui->autodrymode_run_arc_bg_white, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(bk_ui->autodrymode_run_arc_bg_white, LV_OBJ_FLAG_HIDDEN);
    lv_obj_move_background(bk_ui->autodrymode_run_arc_bg_white);

    /* 안쪽 7px 흰색 링 — 회색 링 안쪽 경계(반지름120)에 붙여서, 그 안쪽으로
     * 7px(반지름 113~120)를 흰색으로 채운다. 중심은 위 두 arc와 동일. */
    s_run_arc_inner_white_adm = lv_arc_create(scr);
    lv_obj_set_size(s_run_arc_inner_white_adm, 240, 240);
    lv_obj_set_pos(s_run_arc_inner_white_adm, 392, 147);
    lv_arc_set_bg_angles(s_run_arc_inner_white_adm, 0, 360);
    lv_obj_set_style_arc_color(s_run_arc_inner_white_adm, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_arc_width(s_run_arc_inner_white_adm, 7, LV_PART_MAIN);
    lv_obj_set_style_arc_rounded(s_run_arc_inner_white_adm, false, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(s_run_arc_inner_white_adm, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_opa(s_run_arc_inner_white_adm, LV_OPA_TRANSP, LV_PART_INDICATOR);
    lv_obj_set_style_opa(s_run_arc_inner_white_adm, LV_OPA_TRANSP, LV_PART_KNOB);
    lv_obj_remove_flag(s_run_arc_inner_white_adm, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(s_run_arc_inner_white_adm, LV_OBJ_FLAG_HIDDEN);
    lv_obj_move_background(s_run_arc_inner_white_adm);

    return arc;
}

static void _arc_anim_start_adm(lv_obj_t *arc)
{
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, arc);
    lv_anim_set_exec_cb(&a, _arc_anim_cb_adm);
    lv_anim_set_values(&a, 0, 360);
    lv_anim_set_duration(&a, 4000);
    lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
    lv_anim_start(&a);
}

static void _arc_anim_stop_adm(lv_obj_t *arc)
{
    lv_anim_delete(arc, _arc_anim_cb_adm);
    lv_arc_set_angles(arc, 225, 315);
}

/* ── auto_dry_gif PNG 회전 ─────────────────────────────────────────── */
static void _gif_rotate_cb_adm(void *obj, int32_t val)
{
    lv_image_set_rotation((lv_obj_t *)obj, (uint16_t)(val % 3600));
}

static void _gif_anim_start_adm(bk_lv_ui_t *bk_ui)
{
    lv_obj_t *gif = bk_ui->autodrymode_auto_dry_gif;
    _img_ensure_src(gif);
    lv_obj_clear_flag(gif, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(bk_ui->autodrymode_auto_dry_gif_basic, LV_OBJ_FLAG_HIDDEN);

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, gif);
    lv_anim_set_exec_cb(&a, _gif_rotate_cb_adm);
    lv_anim_set_values(&a, 0, 3600);
    lv_anim_set_duration(&a, 4000);
    lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
    lv_anim_start(&a);
}

static void _gif_anim_stop_adm(bk_lv_ui_t *bk_ui)
{
    lv_obj_t *gif = bk_ui->autodrymode_auto_dry_gif;
    lv_anim_delete(gif, _gif_rotate_cb_adm);
    lv_image_set_rotation(gif, 0);
    lv_obj_add_flag(gif, LV_OBJ_FLAG_HIDDEN);
    _img_ensure_src(bk_ui->autodrymode_auto_dry_gif_basic);
    lv_obj_clear_flag(bk_ui->autodrymode_auto_dry_gif_basic, LV_OBJ_FLAG_HIDDEN);
}

/* ── 실시간 온도/습도 + 잔여시간 갱신 ─────────────────────────────── */
static void _refresh_running_ui_adm(bk_lv_ui_t *bk_ui)
{
    device_state_t *state = &g_device_state;
    char buf[16];

    if (state->current_temp != s_last_temp_adm) {
        s_last_temp_adm = state->current_temp;
        int _is_f = (strcmp(settings_get_str("Degree"), "\xc2\xb0""F") == 0);
        int _t = _is_f ? (state->current_temp * 9 / 5 + 32) : state->current_temp;
        snprintf(buf, sizeof(buf), "%d", _t);
        lv_label_set_text(bk_ui->autodrymode_tempbox_current_temp, buf);
    }

    if (state->current_humidity != s_last_hum_adm) {
        s_last_hum_adm = state->current_humidity;
        snprintf(buf, sizeof(buf), "%d", state->current_humidity);
        lv_label_set_text(bk_ui->autodrymode_tempbox_current_humidity, buf);
    }

    /* MCU remain(saveoperation[10/11]) 기반 잔여시간 표시.
     * DRY 모드 MCU는 saveoperation[12/13](elapsed)를 갱신하지 않고
     * saveoperation[10/11](remain)만 카운트다운하므로 remain 직접 사용.
     * s_dry_cfg_min=0이면 정지 상태 — 시간 라벨 갱신 skip. */
    if (s_dry_cfg_min > 0) {
        int remain = (int)state->remain_hour * 60 + (int)state->remain_min;
        if (remain < 0) remain = 0;
        if (remain != s_last_remain_adm) {
            s_last_remain_adm = remain;
            snprintf(buf, sizeof(buf), "%02d", remain / 60);
            lv_label_set_text(bk_ui->autodrymode_auto_dry_hour_txt, buf);
            snprintf(buf, sizeof(buf), "%02d", remain % 60);
            lv_label_set_text(bk_ui->autodrymode_auto_dry_min_txt, buf);
        }
    }
}

static void _ui_apply_stopped_adm(bk_lv_ui_t *bk_ui);  /* forward decl */

static void _adm_end(bk_lv_ui_t *bk_ui)
{
    device_state_t *state = &g_device_state;
    if (s_ui_timer_adm) { lv_timer_delete(s_ui_timer_adm); s_ui_timer_adm = NULL; }
    state->auto_dry_mode_start = false;
    state->auto_dry_complete   = false;
    state->operation           = false;
    state->black_out_checking  = false;
    settings_set_str("saveChecking", "0");
    settings_set_int("saveCurrentRemainHour", 0);
    settings_set_int("saveCurrentRemainMin",  0);
    settings_save_dirty();
    uart_comm_trigger_change_setting();
    hal_buzzer_complete();
    init_page_main(bk_ui);
    lv_scr_load(bk_ui->main);
    printf("[DRY END] complete buzzer started, → main\n");
}

static void _ui_timer_cb_adm(lv_timer_t *timer)
{
    (void)timer;
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    device_state_t *state = &g_device_state;
    if (!bk_ui->autodrymode || !lv_obj_is_valid(bk_ui->autodrymode)) return;
    if (lv_scr_act() != bk_ui->autodrymode) return;
    /* UART 신규 패킷 없으면 모든 처리 건너뜀 — g_device_state 불변 */
    static uint32_t s_last_rx_seq_adm = 0;
    uint32_t _cur_seq = g_uart_rx_seq;
    if (_cur_seq == s_last_rx_seq_adm) return;
    s_last_rx_seq_adm = _cur_seq;
    if (state->auto_dry_mode_start) {
        /* 완료 감지: MCU remain==0 또는 0x52(완료 코드) 수신
         * DRY MCU는 saveoperation[12/13](elapsed)를 갱신하지 않으므로
         * elapsed >= total 비교 불가 → remain_hour/min == 0 으로 완료 판단.
         * false-positive 방지: MCU가 0x51/0x52(운전중/완료)를 보고한 뒤에만 체크. */
        int _total_min = state->send_dry_hour * 60 + state->send_dry_min;
        uint8_t _mcu_op = (uint8_t)state->saveoperation[5];
        bool _mcu_running = (_mcu_op == 0x51 || _mcu_op == 0x52);
        bool _mcu_done = (_total_min > 0) && _mcu_running &&
                         (state->remain_hour == 0 && state->remain_min == 0);
        if (state->auto_dry_complete || _mcu_done) {
            printf("[DRY CHK] complete=%d mcu_done=%d op=0x%02X remain=%dh%dm total=%dmin\n",
                   state->auto_dry_complete, _mcu_done, _mcu_op,
                   state->remain_hour, state->remain_min, _total_min);
            _adm_end(bk_ui);
            return;
        }
    }

    /* 정지 중에도 온도/습도 갱신; 운전 중이면 시간도 갱신 (cfg_min=0이면 시간 skip) */
    _refresh_running_ui_adm(bk_ui);

    /* 정전 복구를 위해 현재 잔여시간을 flash에 기록 (MCU remain 직접 사용). */
    if (state->auto_dry_mode_start && s_dry_cfg_min > 0) {
        int remain = (int)state->remain_hour * 60 + (int)state->remain_min;
        if (remain < 0) remain = 0;
        settings_set_int("saveCurrentRemainHour", remain / 60);
        settings_set_int("saveCurrentRemainMin",  remain % 60);
    }
}

/* ── 운전 중 UI 적용 ── */
static void _ui_apply_running_adm(bk_lv_ui_t *bk_ui)
{
    int lang = settings_get_int("LANGUAGE");

    if (bk_ui->autodrymode_run_arc_bg_white)
        lv_obj_clear_flag(bk_ui->autodrymode_run_arc_bg_white, LV_OBJ_FLAG_HIDDEN);
    if (s_run_arc_inner_white_adm)
        lv_obj_clear_flag(s_run_arc_inner_white_adm, LV_OBJ_FLAG_HIDDEN);
    if (bk_ui->autodrymode_run_arc) {
        _img_ensure_src(bk_ui->autodrymode_run_arc);
        lv_obj_clear_flag(bk_ui->autodrymode_run_arc, LV_OBJ_FLAG_HIDDEN);
        _arc_anim_start_adm(bk_ui->autodrymode_run_arc);
    }
    /* circle_basic(링 baked-in PNG)은 운전 중엔 숨김 — arc가 그리는 링과
     * 겹쳐 보이던 문제(경계 색 잔상, 두께 불일치) 방지. 초기 화면에서만 표시. */
    if (bk_ui->autodrymode_auto_dry_circle_basic)
        lv_obj_add_flag(bk_ui->autodrymode_auto_dry_circle_basic, LV_OBJ_FLAG_HIDDEN);
    _gif_anim_start_adm(bk_ui);

    if      (lang == 1) _img_set_src_timed(bk_ui->autodrymode_auto_dry_startim, "/images/stop_bt_china.png");
    else if (lang == 2) _img_set_src_timed(bk_ui->autodrymode_auto_dry_startim, "/images/stop_bt_english.png");
    else                _img_set_src_timed(bk_ui->autodrymode_auto_dry_startim, "/images/stop_bt.png");

    lv_obj_add_flag(bk_ui->autodrymode_backim,  LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(bk_ui->autodrymode_backbt,  LV_OBJ_FLAG_HIDDEN);

    _img_ensure_src(bk_ui->autodrymode_auto_tempbox);
    lv_obj_clear_flag(bk_ui->autodrymode_auto_tempbox,             LV_OBJ_FLAG_HIDDEN);
    _img_ensure_src(bk_ui->autodrymode_tempbox_current_temp);
    lv_obj_clear_flag(bk_ui->autodrymode_tempbox_current_temp,     LV_OBJ_FLAG_HIDDEN);
    _img_ensure_src(bk_ui->autodrymode_tempbox_current_humidity);
    lv_obj_clear_flag(bk_ui->autodrymode_tempbox_current_humidity, LV_OBJ_FLAG_HIDDEN);

    _refresh_running_ui_adm(bk_ui);

    if (s_ui_timer_adm) { lv_timer_delete(s_ui_timer_adm); s_ui_timer_adm = NULL; }
    s_ui_timer_adm = lv_timer_create(_ui_timer_cb_adm, 1000, NULL);
}

/* ── 정지 UI 적용 ── */
static void _ui_apply_stopped_adm(bk_lv_ui_t *bk_ui)
{
    int lang = settings_get_int("LANGUAGE");

    if (bk_ui->autodrymode_run_arc) {
        _arc_anim_stop_adm(bk_ui->autodrymode_run_arc);
        lv_obj_add_flag(bk_ui->autodrymode_run_arc, LV_OBJ_FLAG_HIDDEN);
    }
    if (bk_ui->autodrymode_run_arc_bg_white)
        lv_obj_add_flag(bk_ui->autodrymode_run_arc_bg_white, LV_OBJ_FLAG_HIDDEN);
    if (s_run_arc_inner_white_adm)
        lv_obj_add_flag(s_run_arc_inner_white_adm, LV_OBJ_FLAG_HIDDEN);
    /* 초기(정지) 화면으로 복귀 — 링이 baked-in된 circle_basic 다시 표시 */
    if (bk_ui->autodrymode_auto_dry_circle_basic)
        lv_obj_clear_flag(bk_ui->autodrymode_auto_dry_circle_basic, LV_OBJ_FLAG_HIDDEN);
    _gif_anim_stop_adm(bk_ui);

    if      (lang == 1) _img_set_src_timed(bk_ui->autodrymode_auto_dry_startim, "/images/start_bt_china.png");
    else if (lang == 2) _img_set_src_timed(bk_ui->autodrymode_auto_dry_startim, "/images/start_bt_english.png");
    else                _img_set_src_timed(bk_ui->autodrymode_auto_dry_startim, "/images/start_bt.png");

    _img_ensure_src(bk_ui->autodrymode_backim);
    lv_obj_clear_flag(bk_ui->autodrymode_backim, LV_OBJ_FLAG_HIDDEN);
    _img_ensure_src(bk_ui->autodrymode_backbt);
    lv_obj_clear_flag(bk_ui->autodrymode_backbt, LV_OBJ_FLAG_HIDDEN);

    /* 온도/습도 박스 표시 (운전 전에도 현재 온도 확인 가능) */
    _img_ensure_src(bk_ui->autodrymode_auto_tempbox);
    lv_obj_clear_flag(bk_ui->autodrymode_auto_tempbox,             LV_OBJ_FLAG_HIDDEN);
    _img_ensure_src(bk_ui->autodrymode_tempbox_current_temp);
    lv_obj_clear_flag(bk_ui->autodrymode_tempbox_current_temp,     LV_OBJ_FLAG_HIDDEN);
    _img_ensure_src(bk_ui->autodrymode_tempbox_current_humidity);
    lv_obj_clear_flag(bk_ui->autodrymode_tempbox_current_humidity, LV_OBJ_FLAG_HIDDEN);

    /* 시간 라벨을 설정값으로 복원 */
    lv_label_set_text(bk_ui->autodrymode_auto_dry_hour_txt, settings_get_str("CurrentSaveDryTimeHour"));
    lv_label_set_text(bk_ui->autodrymode_auto_dry_min_txt,  settings_get_str("CurrentSaveDryTimeMin"));

    /* cfg_min 리셋: 타이머가 시간 라벨을 덮어쓰지 않도록 */
    s_dry_cfg_min = 0;
    _refresh_running_ui_adm(bk_ui);  /* 초기 온도/습도 즉시 갱신 (시간 라벨은 cfg_min=0이므로 skip) */

    /* 타이머 유지 (정지 중에도 온도 갱신) */
    if (!s_ui_timer_adm)
        s_ui_timer_adm = lv_timer_create(_ui_timer_cb_adm, 1000, NULL);
}

/* ─────────────────────────────────────────────────────────────────────────── */

void autodrymode_backbt_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    device_state_t *state = &g_device_state;
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    if (lv_tick_elaps(s_last_click_autodrymode) < 250) return;
    s_last_click_autodrymode = lv_tick_get();
    hal_buzzer_beep();

    if (state->auto_dry_mode_start) return; /* 운전 중 나가기 차단 */

    if (s_ui_timer_adm) { lv_timer_delete(s_ui_timer_adm); s_ui_timer_adm = NULL; }
    init_page_main(bk_ui);
    lv_scr_load(bk_ui->main);
}

void autodrymode_auto_dry_temp_bt_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    device_state_t *state = &g_device_state;
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    if (state->auto_dry_mode_start) return;
    s_tci_autodrymode = 1;
    _common_click_autodrymode(bk_ui);
}

void autodrymode_auto_dry_humidity_bt_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    device_state_t *state = &g_device_state;
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    if (state->auto_dry_mode_start) return;
    s_tci_autodrymode = 2;
    _common_click_autodrymode(bk_ui);
}

void autodrymode_auto_dry_hour_bt_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    device_state_t *state = &g_device_state;
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    if (state->auto_dry_mode_start) return;
    s_tci_autodrymode = 3;
    _common_click_autodrymode(bk_ui);
}

void autodrymode_auto_dry_min_bt_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    device_state_t *state = &g_device_state;
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    if (state->auto_dry_mode_start) return;
    s_tci_autodrymode = 4;
    _common_click_autodrymode(bk_ui);
}

void autodrymode_auto_dry_start_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    device_state_t *state = &g_device_state;
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    if (lv_tick_elaps(s_last_click_autodrymode) < 250) return;
    s_last_click_autodrymode = lv_tick_get();

    hal_buzzer_beep();

    if (!state->auto_dry_mode_start) {
        /* ── 운전 시작 ── */
        _maxmin_autodrymode(bk_ui);
        {
            int _is_f = (strcmp(settings_get_str("Degree"), "\xc2\xb0""F") == 0);
            const char *_sv = lv_label_get_text(bk_ui->autodrymode_auto_dry_temp_txt);
            if (_is_f && _sv && _sv[0]) {
                char _cb[16]; snprintf(_cb, sizeof(_cb), "%d", (atoi(_sv) - 32) * 5 / 9);
                settings_set_str("CurrentSaveDryTemp", _cb);
            } else settings_set_str("CurrentSaveDryTemp", _sv);
        }
        settings_set_str("CurrentSaveDryHumidity", lv_label_get_text(bk_ui->autodrymode_auto_dry_humidity_txt));
        settings_set_str("CurrentSaveDryTimeHour", lv_label_get_text(bk_ui->autodrymode_auto_dry_hour_txt));
        settings_set_str("CurrentSaveDryTimeMin",  lv_label_get_text(bk_ui->autodrymode_auto_dry_min_txt));

        int sy, sm, sd, sh, smn, ss;
        hal_rtc_get(&sy, &sm, &sd, &sh, &smn, &ss);
        char buf[8];
        snprintf(buf, sizeof(buf), "%04d", sy); settings_set_str("originYear",  buf);
        snprintf(buf, sizeof(buf), "%02d", sm); settings_set_str("originMonth", buf);
        snprintf(buf, sizeof(buf), "%02d", sd); settings_set_str("originDay",   buf);
        snprintf(buf, sizeof(buf), "%02d", sh); settings_set_str("originHour",  buf);
        snprintf(buf, sizeof(buf), "%02d", smn);settings_set_str("originMin",   buf);

        char dp[4];
        snprintf(dp, sizeof(dp), "%d", state->day_period);
        settings_set_str("saveDayPeriod", dp);

        /* 벽시계 카운트다운 초기화 — _refresh_running_ui_adm 호출 전에 설정 */
        {
            int h = atoi(lv_label_get_text(bk_ui->autodrymode_auto_dry_hour_txt));
            int m = atoi(lv_label_get_text(bk_ui->autodrymode_auto_dry_min_txt));
            s_dry_cfg_min = h * 60 + m;  /* 시간 갱신 활성화 플래그 */
        }

        /* FIRST_START(0x31) payload에 사용되는 send_dry_* 필드 설정.
         * 미설정 시 0h0m으로 전송 → MCU dry total=0h → 256h 버그. */
        state->send_dry_temp     = atoi(settings_get_str("CurrentSaveDryTemp"));
        state->send_dry_humidity = atoi(settings_get_str("CurrentSaveDryHumidity"));
        state->send_dry_hour     = s_dry_cfg_min / 60;
        state->send_dry_min      = s_dry_cfg_min % 60;

        state->remain_hour         = s_dry_cfg_min / 60;
        state->remain_min          = s_dry_cfg_min % 60;
        state->operation           = false;
        state->auto_dry_mode_start = true;
        uart_comm_trigger_first_start();   /* first_start=true + s_first_start_count 리셋 */
        settings_set_str("saveOperationTemp",  "4");                                     /* 정전 복구 시 건조 분기 식별 */
        settings_set_int("saveCurrentRemainHour", s_dry_cfg_min / 60);                  /* MCU 첫 STATUS 전 정전 시 0:00 방지 */
        settings_set_int("saveCurrentRemainMin",  s_dry_cfg_min % 60);
        settings_set_str("saveChecking", "1");
        uart_comm_trigger_change_setting();
        settings_save_dirty();

        _ui_apply_running_adm(bk_ui);

    } else {
        /* ── 운전 정지 ── */
        state->operation           = false;
        state->start_run           = true;
        state->first_start         = false;
        state->auto_dry_mode_start = false;
        state->auto_dry_complete   = false;
        state->black_out_checking  = false;
        settings_set_str("saveChecking", "0");
        uart_comm_trigger_change_setting();
        settings_save_dirty();

        _ui_apply_stopped_adm(bk_ui);
    }
}

void autodrymode_keypad_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    lv_event_code_t code = lv_event_get_code(e);
    intptr_t idx = (intptr_t)lv_event_get_user_data(e);
    static const char digits[12] = {'1','2','3','4','5','6','7','8','9','0',0,0};
    lv_obj_t *im = (idx >= 0 && idx < 12 && bk_ui->autodrymode_KeyPadIm[idx])
                   ? bk_ui->autodrymode_KeyPadIm[idx] : NULL;
    if (code == LV_EVENT_PRESSED && im) {
        _img_ensure_src(im);
        lv_obj_clear_flag(im, LV_OBJ_FLAG_HIDDEN);
    } else if ((code == LV_EVENT_RELEASED || code == LV_EVENT_PRESS_LOST) && im)
        lv_obj_add_flag(im, LV_OBJ_FLAG_HIDDEN);
    else if (code == LV_EVENT_CLICKED) {
        hal_buzzer_beep();
        if      (idx == 10) _keypad_minor_autodrymode(bk_ui);
        else if (idx == 11) _keypad_backspace_autodrymode(bk_ui);
        else if (idx <= 9)  _keypad_input_autodrymode(bk_ui, digits[idx]);
    }
}

void autodrymode_keypadhide_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_PRESSED) {
        if (bk_ui->autodrymode_keypadhide_im) lv_obj_clear_flag(bk_ui->autodrymode_keypadhide_im, LV_OBJ_FLAG_HIDDEN);
        return;
    }
    if (code == LV_EVENT_RELEASED || code == LV_EVENT_PRESS_LOST) {
        if (bk_ui->autodrymode_keypadhide_im) lv_obj_add_flag(bk_ui->autodrymode_keypadhide_im, LV_OBJ_FLAG_HIDDEN);
    }
    if (code != LV_EVENT_CLICKED) return;
    hal_buzzer_beep();
    _keypad_hide_autodrymode(bk_ui);
}

void autodrymode_load_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    device_state_t *state = &g_device_state;

    /* ── SCREEN_LOADED: PNG 디코드 + UI 상태 복원 — lv_task_handler (얕은 스택) ── */
    if (code == LV_EVENT_SCREEN_LOADED) {
        ui_title_anim(bk_ui->autodrymode_title);
        if (!lv_image_get_src(bk_ui->autodrymode_auto_dry_circle_basic)) {
            _img_set_src_timed(bk_ui->autodrymode_auto_dry_circle_basic, "/images/auto_dry_circle_basic.png");
        

            // _img_set_src_timed(bk_ui->autodrymode_auto_dry_circle_gif,   "/images/auto_dry_circle.png");
            /* auto_dry_txt_basic: SCREEN_LOAD_START 말미의 ui_lang_apply_autodrymode()가
             * 언어 suffix를 포함한 올바른 경로로 이미 설정했으므로 여기서 덮어쓰지 않음. */
            _img_set_src_timed(bk_ui->autodrymode_auto_dry_gif,          "/images/auto_dry_gif.png");
            _img_set_src_timed(bk_ui->autodrymode_auto_dry_gif_basic,    "/images/auto_dry_gif.png");
            lv_image_set_pivot(bk_ui->autodrymode_auto_dry_gif, 26, 26);
        }
        if (state->auto_dry_mode_start)
            _ui_apply_running_adm(bk_ui);
        else
            _ui_apply_stopped_adm(bk_ui);
        return;
    }

    if (code != LV_EVENT_SCREEN_LOAD_START) return;

    /* ── run_arc 레이어 생성 (최초 1회) ──
     * z-order (back→front): circle_gif(불투명 초록 링 PNG) → circle_basic(투명 영역으로 링이 비침) →
     * run_arc(회색+인디케이터+흰 테두리, 운전 중에만 표시되어 위를 덮음) → 버튼/레이블 */
    if (!bk_ui->autodrymode_run_arc) {
        bk_ui->autodrymode_run_arc = _create_run_arc_adm(bk_ui->autodrymode);
        /* move_background 역순 호출 — auto_dry_circle.png는 불투명이라 맨 뒤에 있어야 하고,
         * circle_basic(투명 영역 有)이 그 위에서 링 색이 비쳐 보이게 배치. */
        lv_obj_move_background(bk_ui->autodrymode_auto_dry_circle_basic);
        lv_obj_move_background(bk_ui->autodrymode_auto_dry_circle_gif);
    }

    /* Keypad buttons start non-clickable; enabled only when keypad is open */
    _keypad_off_autodrymode(bk_ui);
    s_tci_autodrymode = 0;
    s_edit_buf_autodrymode[0] = '\0';
    _underbar_all_hide_autodrymode(bk_ui);

    {
        int _is_f = (strcmp(settings_get_str("Degree"), "\xc2\xb0""F") == 0);
        const char *_sv = settings_get_str("CurrentSaveDryTemp");
        if (_is_f && _sv && _sv[0]) {
            char _fb[16]; snprintf(_fb, sizeof(_fb), "%d", atoi(_sv) * 9 / 5 + 32);
            lv_label_set_text(bk_ui->autodrymode_auto_dry_temp_txt, _fb);
        } else lv_label_set_text(bk_ui->autodrymode_auto_dry_temp_txt, _sv ? _sv : "");
    }
    lv_label_set_text(bk_ui->autodrymode_auto_dry_humidity_txt, settings_get_str("CurrentSaveDryHumidity"));
    lv_label_set_text(bk_ui->autodrymode_auto_dry_hour_txt, settings_get_str("CurrentSaveDryTimeHour"));
    lv_label_set_text(bk_ui->autodrymode_auto_dry_min_txt, settings_get_str("CurrentSaveDryTimeMin"));

    /* 정전 복구: s_dry_cfg_min 복원 (시간 갱신 활성화 플래그).
     * 잔여시간 표시는 MCU saveoperation[12/13] 기반이므로
     * 벽시계 tick 보정 불필요. MCU가 복구 후 실제 경과를 보고한다. */
    if (state->black_out_checking) {
        int h = atoi(settings_get_str("CurrentSaveDryTimeHour"));
        int m = atoi(settings_get_str("CurrentSaveDryTimeMin"));
        s_dry_cfg_min = h * 60 + m;
    }

    /* 정전 복구 아이콘 */
    if (state->black_out_checking) {
        _img_ensure_src(bk_ui->autodrymode_blackout);
        lv_obj_clear_flag(bk_ui->autodrymode_blackout, LV_OBJ_FLAG_HIDDEN);
    } else
        lv_obj_add_flag(bk_ui->autodrymode_blackout,   LV_OBJ_FLAG_HIDDEN);

    /* 화면 재생성(init_page_autodrymode)으로 빈 라벨 생성 → 캐시 리셋해서 강제 갱신 */
    s_last_temp_adm   = 0x7FFFFFFF;
    s_last_hum_adm    = 0x7FFFFFFF;
    s_last_remain_adm = -1;

    ui_lang_apply_autodrymode(bk_ui);
}
