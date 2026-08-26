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
#include "device_state.h"
#include "hardware_hal.h"
#include "uart_comm.h"

#define TAG "[detailsettingdefrost_cb.c] "
#define bk_printf(fmt, ...) do {if(0) printf(fmt, ##__VA_ARGS__); } while(0) // disable printf

extern bk_lv_ui_t bk_lv_tool_ui;

static uint32_t last_click_time = 0;
static int s_edit_field = -1;

static const char * const s_onOff[] = {"ON","OFF"}; /* 2 */

static const char * const s_defrostTemp[] = {
    "0","1","2","3","4","5","6","7","8","9","10",
    "11","12","13","14","15","16","17","18","19","20"
}; /* 21 */

/* 제상 복귀온도는 절대온도(센서가 도달해야 하는 실제 온도)이므로 Android 원본의 차등 변환
 * (×1.8, 0~36)이 아닌 절대온도 변환(×1.8+32, 32~68)을 사용 — LVGL에서만 수정.
 * (Android DetailSettingDefrostFragment.java는 degreechange()(×1.8)를 쓰지만, 이 필드는
 * FreezeTemp/DefrostTemp 등 다른 절대온도 설정과 마찬가지로 degreebasicchange()(×1.8+32)가
 * 물리적으로 맞다고 판단하여 LVGL 포팅에서 의도적으로 다르게 구현함.) */
static const char * const s_defrostTempF[] = {
    "32.0","33.8","35.6","37.4","39.2","41.0","42.8","44.6","46.4","48.2",
    "50.0","51.8","53.6","55.4","57.2","59.0","60.8","62.6","64.4","66.2","68.0"
}; /* 21 */
// static const char * const s_defrostTempF[] = {
//     "0.0","1.8","3.6","5.4","7.2","9.0","10.8","12.6","14.4","16.2",
//     "18.0","19.8","21.6","23.4","25.2","27.0","28.8","30.6","32.4","34.2","36.0"
// }; /* 21 */
static const char * const s_defrostTime[] = {
    "4","5","6","7","8","9","10","11","12"
}; /* 9 */

static const char * const s_keys[3] = {
    "DetailDefrostOnOff", "DetailDefrostReturnTemp", "DetailDefrostTime"
};

typedef struct { const char * const *arr; int cnt; } _picker_opt_t;

static const _picker_opt_t s_picker_opts[3] = {
    {s_onOff,        2},
    {s_defrostTemp,  21},
    {s_defrostTime,  9},
};
static const _picker_opt_t s_picker_optsF[3] = {
    {s_onOff,        2},
    {s_defrostTempF, 21},
    {s_defrostTime,  9},
};
static _picker_opt_t _picker_opt(int field)
{
    int is_f = (strcmp(settings_get_str("Degree"), "\xc2\xb0""F") == 0);
    return is_f ? s_picker_optsF[field] : s_picker_opts[field];
}

static char s_off_imgs[3][128];
static char s_on_imgs[3][128];
static int  s_roller_field = -1;

static void _build_img_arrays(void)
{
    int lang = settings_get_int("LANGUAGE");
    int is_f = (strcmp(settings_get_str("Degree"), "\xc2\xb0""F") == 0);
    const char *lsuf = (lang == 1) ? "_china" : (lang == 2) ? "_english" : "";
    /* item 0,2: china and english variants exist */
    const char *lsuf13 = (lang == 1) ? "_china" : (lang == 2) ? "_english" : "";
    /* item 1 (defrost_2): full _f + language variants */
    const char *fsuf2 = is_f ? "_f" : "";
    snprintf(s_off_imgs[0], 128, "/images/detail_defrost_1_off%s.png", lsuf13);
    snprintf(s_on_imgs[0],  128, "/images/detail_defrost_1_on%s.png",  lsuf13);
    snprintf(s_off_imgs[1], 128, "/images/detail_defrost_2_off%s%s.png", fsuf2, lsuf);
    snprintf(s_on_imgs[1],  128, "/images/detail_defrost_2_on%s%s.png",  fsuf2, lsuf);
    snprintf(s_off_imgs[2], 128, "/images/detail_defrost_3_off%s.png", lsuf13);
    snprintf(s_on_imgs[2],  128, "/images/detail_defrost_3_on%s.png",  lsuf13);
}

static void _build_img_arrays_if_changed(void)
{
    static int s_last_lang = -1;
    static int s_last_is_f = -1;
    int lang = settings_get_int("LANGUAGE");
    int is_f = (strcmp(settings_get_str("Degree"), "\xc2\xb0""F") == 0);
    if (lang == s_last_lang && is_f == s_last_is_f) return;
    s_last_lang = lang;
    s_last_is_f = is_f;
    _build_img_arrays();
    s_roller_field = -1;
}

void detailsettingdefrost_backbt_event_cb(lv_event_t *e);
void detailsettingdefrost_settingbt1_event_cb(lv_event_t *e);
void detailsettingdefrost_settingbt2_event_cb(lv_event_t *e);
void detailsettingdefrost_settingbt3_event_cb(lv_event_t *e);
void detailsettingdefrost_changebt_event_cb(lv_event_t *e);
void detailsettingdefrost_roller_event_cb(lv_event_t *e);
void detailsettingdefrost_load_start_event_cb(lv_event_t *e);
void detailsettingdefrost_loaded_event_cb(lv_event_t *e);
void detailsettingdefrost_unload_start_event_cb(lv_event_t *e);
void detailsettingdefrost_unloaded_event_cb(lv_event_t *e);

static void _get_row_objs(bk_lv_ui_t *bk_ui, lv_obj_t *ims[3], lv_obj_t *txts[3])
{
    ims[0]  = bk_ui->detailsettingdefrost_settingim1;
    ims[1]  = bk_ui->detailsettingdefrost_settingim2;
    ims[2]  = bk_ui->detailsettingdefrost_settingim3;
    txts[0] = bk_ui->detailsettingdefrost_settingtxt1;
    txts[1] = bk_ui->detailsettingdefrost_settingtxt2;
    txts[2] = bk_ui->detailsettingdefrost_settingtxt3;
}

/* 활성화(편집 중)된 항목은 검정, 나머지는 기존 회색 유지 */
static void _update_txt_colors(bk_lv_ui_t *bk_ui, int active_field)
{
    lv_obj_t *ims[3], *txts[3];
    _get_row_objs(bk_ui, ims, txts);
    for (int i = 0; i < 3; i++) {
        lv_obj_set_style_text_color(txts[i],
            (i == active_field) ? lv_color_hex(0x3C3A3D) : lv_color_hex(0xA6A6A6), 0);
    }
}

static void _settingoff(bk_lv_ui_t *bk_ui)
{
    lv_obj_t *ims[3], *txts[3];
    _get_row_objs(bk_ui, ims, txts);
    for (int i = 0; i < 3; i++)
        _img_set_src_timed(ims[i], s_off_imgs[i]);
    lv_obj_add_flag(bk_ui->detailsettingdefrost_settemp_setn1, LV_OBJ_FLAG_HIDDEN);
    s_edit_field = -1;
    _update_txt_colors(bk_ui, -1);
}

static const char *_display_val(int field, int is_f)
{
    const char *stored = settings_get_str(s_keys[field]);
    if (field != 1) return stored;
    if (!is_f) {
        /* C mode: stored might be an F value — map back to C */
        for (int i = 0; i < 21; i++) {
            if (strcmp(s_defrostTempF[i], stored) == 0) return s_defrostTemp[i];
        }
        return stored;
    } else {
        /* F mode: already an F value? show directly */
        for (int i = 0; i < 21; i++) {
            if (strcmp(s_defrostTempF[i], stored) == 0) return stored;
        }
        /* stored is a C value — map to F */
        for (int i = 0; i < 21; i++) {
            if (strcmp(s_defrostTemp[i], stored) == 0) return s_defrostTempF[i];
        }
        return stored;
    }
}

static void _refresh(bk_lv_ui_t *bk_ui)
{
    lv_obj_t *ims[3], *txts[3];
    _get_row_objs(bk_ui, ims, txts);
    int is_f = (strcmp(settings_get_str("Degree"), "\xc2\xb0""F") == 0);
    for (int i = 0; i < 3; i++) {
        _img_set_src_timed(ims[i], s_off_imgs[i]);
        if (i == 0) {   /* DefrostOnOff: ON/OFF → 중국어는 开/关 폰트 */
            ui_lang_set_onoff_display(txts[i], _display_val(i, is_f));
        } else {
            lv_label_set_text(txts[i], _display_val(i, is_f));
        }
    }
}

static void _show_picker(bk_lv_ui_t *bk_ui, int field)
{
    _settingoff(bk_ui);
    s_edit_field = field;
    _update_txt_colors(bk_ui, field);

    lv_obj_t *ims[3], *txts[3];
    _get_row_objs(bk_ui, ims, txts);
    _img_set_src_timed(ims[field], s_on_imgs[field]);
    _img_ensure_src(bk_ui->detailsettingdefrost_pickerbox);
    lv_obj_clear_flag(bk_ui->detailsettingdefrost_pickerbox, LV_OBJ_FLAG_HIDDEN);


    _picker_opt_t opt = _picker_opt(field);
    const char * const *arr = opt.arr;
    int cnt = opt.cnt;

    /* settemp_setn1은 필드 0~2가 전부 공유하는 roller — field 0(중국어 开/关)에서
     * 남긴 전용 폰트가 다른 숫자 필드(1,2)에도 남으면 숫자 글리프가 없어 깨진다.
     * 매번 진입 시 기본 폰트로 되돌린 뒤, field 0에서만 필요시 다시 교체한다. */
    lv_obj_set_style_text_font(bk_ui->detailsettingdefrost_settemp_setn1, &lv_font_scdream_regular_72, LV_PART_MAIN);
    lv_obj_set_style_text_font(bk_ui->detailsettingdefrost_settemp_setn1, &lv_font_scdream_regular_90, LV_PART_SELECTED);
    bool cn_onoff = (field == 0 && settings_get_int("LANGUAGE") == 1);

    if (s_roller_field != field) {
        if (cn_onoff) {
            /* "开\n关" (U+5F00, U+5173) — s_onOff 순서(ON,OFF)와 동일하게 매핑 */
            lv_roller_set_options(bk_ui->detailsettingdefrost_settemp_setn1,
                                   "\xe5\xbc\x80\n\xe5\x85\xb3", LV_ROLLER_MODE_NORMAL);
        } else {
            static char opts[512];
            char *p = opts;
            char * const end = opts + sizeof(opts) - 1;
            for (int i = 0; i < cnt && p < end; i++) {
                if (i > 0 && p < end) *p++ = '\n';
                size_t len = strlen(arr[i]);
                if (p + len >= end) break;
                memcpy(p, arr[i], len);
                p += len;
            }
            *p = '\0';
            lv_roller_set_options(bk_ui->detailsettingdefrost_settemp_setn1, opts, cnt <= 2 ? LV_ROLLER_MODE_NORMAL : LV_ROLLER_MODE_INFINITE);
        }
        lv_roller_set_visible_row_count(bk_ui->detailsettingdefrost_settemp_setn1, 3);
        s_roller_field = field;
    }
    if (cn_onoff) {
        lv_obj_set_style_text_font(bk_ui->detailsettingdefrost_settemp_setn1, &lv_font_onoff_cn_72, LV_PART_MAIN);
        lv_obj_set_style_text_font(bk_ui->detailsettingdefrost_settemp_setn1, &lv_font_onoff_cn_90, LV_PART_SELECTED);
    }

    const char *cur = settings_get_str(s_keys[field]);
    int sel = 0;
    int found = 0;
    for (int i = 0; i < cnt; i++) {
        if (strcmp(arr[i], cur) == 0) { sel = i; found = 1; break; }
    }
    if (!found && field == 1) {
        /* stored value is in opposite unit — convert and search.
         * 절대온도 변환(×1.8+32) 사용 — settings에는 항상 C 정수가 저장되므로
         * F 모드에서는 이 분기가 매번 실행된다. */
        int is_f = (strcmp(settings_get_str("Degree"), "\xc2\xb0""F") == 0);
        double v = atof(cur);
        char cbuf[16];

        // if (is_f)
        //     snprintf(cbuf, sizeof(cbuf), "%.1f", v * 1.8);   /* C int → F */
        // else
        //     snprintf(cbuf, sizeof(cbuf), "%d", (int)(v / 1.8 + 0.5));  /* F → C int */
        if (is_f)
            snprintf(cbuf, sizeof(cbuf), "%.1f", v * 1.8 + 32.0);   /* C int → F */
        else
            snprintf(cbuf, sizeof(cbuf), "%d", (int)((v - 32.0) / 1.8 + 0.5));  /* F → C int */
        for (int i = 0; i < cnt; i++) {
            if (strcmp(arr[i], cbuf) == 0) { sel = i; break; }
        }
    }
    lv_roller_set_selected(bk_ui->detailsettingdefrost_settemp_setn1, (uint16_t)sel, LV_ANIM_OFF);
    _img_ensure_src(bk_ui->detailsettingdefrost_settemp_setn1);
    lv_obj_clear_flag(bk_ui->detailsettingdefrost_settemp_setn1, LV_OBJ_FLAG_HIDDEN);
}

void detailsettingdefrost_roller_event_cb(lv_event_t *e)
{
    (void)e;
}

void detailsettingdefrost_backbt_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    if (lv_tick_elaps(last_click_time) < 250) return;
    last_click_time = lv_tick_get();
    hal_buzzer_beep();

    settings_save_dirty();
    if (bk_ui->settingmodedetailsetting == NULL || !lv_obj_is_valid(bk_ui->settingmodedetailsetting))
        init_page_settingmodedetailsetting(bk_ui);
    lv_scr_load(bk_ui->settingmodedetailsetting);
}

static void _settingbt(int field)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    if (lv_tick_elaps(last_click_time) < 250) return;
    last_click_time = lv_tick_get();
    hal_buzzer_beep();
    _show_picker(bk_ui, field);
}

void detailsettingdefrost_settingbt1_event_cb(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    _settingbt(0);
}

void detailsettingdefrost_settingbt2_event_cb(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    _settingbt(1);
}

void detailsettingdefrost_settingbt3_event_cb(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    _settingbt(2);
}

void detailsettingdefrost_changebt_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    device_state_t *state = &g_device_state;
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    if (lv_tick_elaps(last_click_time) < 250) return;
    last_click_time = lv_tick_get();
    hal_buzzer_beep();

    if (s_edit_field < 0 || s_edit_field >= 3) return;

    char buf[32];
    lv_roller_get_selected_str(bk_ui->detailsettingdefrost_settemp_setn1, buf, sizeof(buf));

    if (s_edit_field == 0) {
        /* DefrostOnOff roller가 중국어(开/关)로 표시 중일 수 있어 텍스트 대신
         * 선택 인덱스로 판정한다(0=ON,1=OFF, s_onOff 순서 고정) — 텍스트를
         * 그대로 쓰면 설정에 开/关이 저장되어 uart_comm.c의 strcmp(...,"ON")
         * 판정이 항상 실패하고, 재진입 시 폰트 불일치로 빈칸처럼 보이는
         * 문제가 있었다. */
        snprintf(buf, sizeof(buf), "%s",
                 (lv_roller_get_selected(bk_ui->detailsettingdefrost_settemp_setn1) == 0) ? "ON" : "OFF");
    }

    int is_f = (strcmp(settings_get_str("Degree"), "\xc2\xb0""F") == 0);
    /* 항상 섭씨(C) 정수로 정규화해서 저장 — uart_comm.c의 _rebuild_send_save_value1()이
     * settings_get_int("DetailDefrostReturnTemp")로 그대로 읽어 MCU에 보내므로, F 모드에서
     * 고른 롤러 문자열("18.0" 등)을 그대로 저장하면 atoi()가 소수점에서 잘려 18로 보내져
     * 실제 의도한 10°C가 아닌 엉뚱한 값이 하드웨어로 전달된다. */
    if (s_edit_field == 1 && is_f) {
        for (int i = 0; i < 21; i++) {
            if (strcmp(s_defrostTempF[i], buf) == 0) {
                strncpy(buf, s_defrostTemp[i], sizeof(buf) - 1);
                buf[sizeof(buf) - 1] = '\0';
                break;
            }
        }
    }

    lv_obj_t *ims[3], *txts[3];
    _get_row_objs(bk_ui, ims, txts);
    settings_set_str(s_keys[s_edit_field], buf);
    if (s_edit_field == 0) {   /* DefrostOnOff: ON/OFF → 중국어는 开/关 폰트 */
        ui_lang_set_onoff_display(txts[0], _display_val(0, is_f));
    } else {
        lv_label_set_text(txts[s_edit_field], _display_val(s_edit_field, is_f));
    }
    settings_save_dirty();

    state->change_setting = true;
    state->start_run = false;
    uart_comm_trigger_change_setting();
}

void detailsettingdefrost_loaded_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    ui_title_anim(bk_ui->detailsettingdefrost_title);
}

void detailsettingdefrost_unload_start_event_cb(lv_event_t *e)
{
    (void)e;
}

void detailsettingdefrost_unloaded_event_cb(lv_event_t *e)
{
    (void)e;
}

void detailsettingdefrost_load_start_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    uint32_t t0 = lv_tick_get();
    bk_printf(TAG "[PERF] detaildefrost load_event start\n");

    _build_img_arrays();
    bk_printf(TAG "[PERF]   _build_img_arrays +%lu ms\n", (unsigned long)lv_tick_elaps(t0));

    ui_lang_apply_detailsettingdefrost(bk_ui);
    s_edit_field = -1;
    s_roller_field = -1;
    _refresh(bk_ui);
    _update_txt_colors(bk_ui, -1);   /* 재진입 시 이전 선택 색상(검정) 잔류 방지 */
    bk_printf(TAG "[PERF]   _refresh +%lu ms\n", (unsigned long)lv_tick_elaps(t0));

    lv_obj_add_flag(bk_ui->detailsettingdefrost_settemp_setn1, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(bk_ui->detailsettingdefrost_pickerbox,     LV_OBJ_FLAG_HIDDEN);
    bk_printf(TAG "[PERF] detaildefrost load_event end total=%lu ms\n", (unsigned long)lv_tick_elaps(t0));
}
