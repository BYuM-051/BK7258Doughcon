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
#include "preRenderer.h"

#define TAG "[detailsettingdamper_cb.c] "
// #define bk_printf(fmt, ...) do {if(0) printf(fmt, ##__VA_ARGS__); } while(0) // disable printf

extern bk_lv_ui_t bk_lv_tool_ui;

static uint32_t last_click_time = 0;
static int s_edit_field = -1;

/* bt1/bt2: single roller (setn4), picker_1 */
static const char * const s_damperFan[] = {
    "1","2","3","4","5","6","7","8","9","10",
    "11","12","13","14","15","16","17","18","19","20"
}; /* 20 */

/* bt3/bt4: triple roller (setn1=hundreds, setn2=tens, setn3=units), picker_2 */
static const char * const s_damperSec0[] = {"0","1"};               /* 2 */
static const char * const s_damperSec1[] = {"0","1","2","3","4","5","6","7","8","9"}; /* 10 */

static const char * const s_keys[4] = {
    "DetailDamperFanOn", "DetailDamperFanOff",
    "DetailDamperOnSol", "DetailDamperOffSol"
};

static char s_off_imgs[4][128];
static char s_on_imgs[4][128];
static bool s_fan_opts_set    = false;
static bool s_triple_opts_set = false;

static void _build_img_arrays(void)
{
    int lang = settings_get_int("LANGUAGE");
    const char *lsuf = (lang == 1) ? "_china" : "";
    for (int i = 0; i < 4; i++) {
        snprintf(s_off_imgs[i], 128, "/images/detail_damper_%d_off%s.png", i + 1, lsuf);
        snprintf(s_on_imgs[i],  128, "/images/detail_damper_%d_on%s.png",  i + 1, lsuf);
    }
}

static void _build_img_arrays_if_changed(void)
{
    static int  s_last_lang = -1;
    static char s_last_degree[8] = "";
    int lang = settings_get_int("LANGUAGE");
    const char *degree = settings_get_str("Degree");
    /* ui_lang_invalidate_cached_screens()는 LANGUAGE뿐 아니라 Degree(°F/°C)
     * 변경 시에도 이 화면 객체를 destroy한다(settingmodedegree_cb.c) — 그러면
     * 롤러 위젯이 init 시점 placeholder("0")로 새로 만들어지는데, 예전엔 이
     * 함수가 LANGUAGE만 감시해서 s_fan_opts_set/s_triple_opts_set이 리셋 안
     * 되고 "이미 세팅 완료"로 남아있었다 — _show_picker()가 실제 배열(1~20 등)
     * 재로딩을 건너뛰어 롤러가 "0"에 갇히는 버그(화씨 전환 후 댐퍼설정 피커
     * 먹통)의 원인이었다. Degree도 같이 감시하도록 수정. */
    bool lang_changed   = (lang != s_last_lang);
    bool degree_changed = (strcmp(degree, s_last_degree) != 0);
    if (!lang_changed && !degree_changed) return;
    s_last_lang = lang;
    strncpy(s_last_degree, degree, sizeof(s_last_degree) - 1);
    s_last_degree[sizeof(s_last_degree) - 1] = '\0';
    _build_img_arrays();
    s_fan_opts_set    = false;
    s_triple_opts_set = false;
}

void detailsettingdamper_backbt_event_cb(lv_event_t *e);
void detailsettingdamper_settingbt1_event_cb(lv_event_t *e);
void detailsettingdamper_settingbt2_event_cb(lv_event_t *e);
void detailsettingdamper_settingbt3_event_cb(lv_event_t *e);
void detailsettingdamper_settingbt4_event_cb(lv_event_t *e);
void detailsettingdamper_changebt_event_cb(lv_event_t *e);
void detailsettingdamper_roller_event_cb(lv_event_t *e);
void detailsettingdamper_load_start_event_cb(lv_event_t *e);
void detailsettingdamper_loaded_event_cb(lv_event_t *e);
void detailsettingdamper_unload_start_event_cb(lv_event_t *e);
void detailsettingdamper_unloaded_event_cb(lv_event_t *e);

static void _get_row_objs(bk_lv_ui_t *bk_ui, lv_obj_t *ims[4], lv_obj_t *txts[4])
{
    ims[0]  = bk_ui->detailsettingdamper_settingim1;
    ims[1]  = bk_ui->detailsettingdamper_settingim2;
    ims[2]  = bk_ui->detailsettingdamper_settingim3;
    ims[3]  = bk_ui->detailsettingdamper_settingim4;
    txts[0] = bk_ui->detailsettingdamper_settingtxt1;
    txts[1] = bk_ui->detailsettingdamper_settingtxt2;
    txts[2] = bk_ui->detailsettingdamper_settingtxt3;
    txts[3] = bk_ui->detailsettingdamper_settingtxt4;
}

/* 활성화(편집 중)된 항목은 검정, 나머지는 기존 회색 유지 */
static void _update_txt_colors(bk_lv_ui_t *bk_ui, int active_field)
{
    lv_obj_t *ims[4], *txts[4];
    _get_row_objs(bk_ui, ims, txts);
    for (int i = 0; i < 4; i++) {
        lv_obj_set_style_text_color(txts[i],
            (i == active_field) ? lv_color_hex(0x3C3A3D) : lv_color_hex(0xA6A6A6), 0);
    }
}

static void _settingoff(bk_lv_ui_t *bk_ui)
{
    lv_obj_t *ims[4], *txts[4];
    _get_row_objs(bk_ui, ims, txts);
    for (int i = 0; i < 4; i++)
        _img_set_src_timed(ims[i], s_off_imgs[i]);
    lv_obj_add_flag(bk_ui->detailsettingdamper_settemp_setn1, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(bk_ui->detailsettingdamper_settemp_setn2, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(bk_ui->detailsettingdamper_settemp_setn3, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(bk_ui->detailsettingdamper_settemp_setn4, LV_OBJ_FLAG_HIDDEN);
    s_edit_field = -1;
    _update_txt_colors(bk_ui, -1);
}

static void _refresh(bk_lv_ui_t *bk_ui)
{
    lv_obj_t *ims[4], *txts[4];
    _get_row_objs(bk_ui, ims, txts);
    for (int i = 0; i < 4; i++) {
        _img_set_src_timed(ims[i], s_off_imgs[i]);
        lv_label_set_text(txts[i], settings_get_str(s_keys[i]));
    }
}

/* Build roller options string from array */
static void _build_opts(const char * const *arr, int cnt, char *buf, size_t bufsz)
{
    char *p = buf;
    char * const end = buf + bufsz - 1;
    for (int i = 0; i < cnt && p < end; i++) {
        if (i > 0 && p < end) *p++ = '\n';
        size_t len = strlen(arr[i]);
        if (p + len >= end) break;
        memcpy(p, arr[i], len);
        p += len;
    }
    *p = '\0';
}

static int _find_idx(const char * const *arr, int cnt, const char *val)
{
    for (int i = 0; i < cnt; i++)
        if (strcmp(arr[i], val) == 0) return i;
    return 0;
}

static void _show_picker(bk_lv_ui_t *bk_ui, int field)
{
    _settingoff(bk_ui);
    s_edit_field = field;
    _update_txt_colors(bk_ui, field);

    lv_obj_t *ims[4], *txts[4];
    _get_row_objs(bk_ui, ims, txts);
    _img_set_src_timed(ims[field], s_on_imgs[field]);

    // _img_ensure_src(bk_ui->detailsettingdamper_pickerbox);
 lv_obj_clear_flag(bk_ui->detailsettingdamper_pickerbox, LV_OBJ_FLAG_HIDDEN);


    static char opts[256];

    if (field <= 1) {
        /* Single roller (setn4) with picker_1 */
        ui_lang_apply_picker(bk_ui->detailsettingdamper_pickerbox, 1);
        lv_obj_set_pos(bk_ui->detailsettingdamper_pickerbox, 624, 120);
        lv_obj_set_size(bk_ui->detailsettingdamper_pickerbox, 376, 376);
        _img_ensure_src(bk_ui->detailsettingdamper_pickerbox);
        lv_obj_clear_flag(bk_ui->detailsettingdamper_pickerbox, LV_OBJ_FLAG_HIDDEN);

        if (!s_fan_opts_set) {
            _build_opts(s_damperFan, 20, opts, sizeof(opts));
            lv_roller_set_options(bk_ui->detailsettingdamper_settemp_setn4, opts, LV_ROLLER_MODE_INFINITE);
            lv_roller_set_visible_row_count(bk_ui->detailsettingdamper_settemp_setn4, 3);
            s_fan_opts_set = true;
        }
        int sel = _find_idx(s_damperFan, 20, settings_get_str(s_keys[field]));
        lv_roller_set_selected(bk_ui->detailsettingdamper_settemp_setn4, (uint16_t)sel, LV_ANIM_OFF);
        _img_ensure_src(bk_ui->detailsettingdamper_settemp_setn4);
        lv_obj_clear_flag(bk_ui->detailsettingdamper_settemp_setn4, LV_OBJ_FLAG_HIDDEN);
    } else {
        /* Triple roller (setn1=hundreds, setn2=tens, setn3=units) with picker_2 */
        ui_lang_apply_picker(bk_ui->detailsettingdamper_pickerbox, 2);
        lv_obj_set_pos(bk_ui->detailsettingdamper_pickerbox, 624, 120);
        lv_obj_set_size(bk_ui->detailsettingdamper_pickerbox, 376, 376);
        _img_ensure_src(bk_ui->detailsettingdamper_pickerbox);
        lv_obj_clear_flag(bk_ui->detailsettingdamper_pickerbox, LV_OBJ_FLAG_HIDDEN);

        const char *cur = settings_get_str(s_keys[field]);
        int len = (int)strlen(cur);
        char c0 = '0', c1 = '0', c2 = '0';
        if (len >= 3) { c0 = cur[0]; c1 = cur[1]; c2 = cur[2]; }
        else if (len == 2) { c1 = cur[0]; c2 = cur[1]; }
        else if (len == 1) { c2 = cur[0]; }

        int sel0 = (c0 == '0') ? 0 : 1;
        int sel1 = c1 - '0';
        int sel2 = c2 - '0';

        if (!s_triple_opts_set) {
            _build_opts(s_damperSec0, 2, opts, sizeof(opts));
            lv_roller_set_options(bk_ui->detailsettingdamper_settemp_setn1, opts, LV_ROLLER_MODE_NORMAL);
            lv_roller_set_visible_row_count(bk_ui->detailsettingdamper_settemp_setn1, 3);
            _build_opts(s_damperSec1, 10, opts, sizeof(opts));
            lv_roller_set_options(bk_ui->detailsettingdamper_settemp_setn2, opts, LV_ROLLER_MODE_INFINITE);
            lv_roller_set_visible_row_count(bk_ui->detailsettingdamper_settemp_setn2, 3);
            lv_roller_set_options(bk_ui->detailsettingdamper_settemp_setn3, opts, LV_ROLLER_MODE_INFINITE);
            lv_roller_set_visible_row_count(bk_ui->detailsettingdamper_settemp_setn3, 3);
            s_triple_opts_set = true;
        }
        lv_roller_set_selected(bk_ui->detailsettingdamper_settemp_setn1, (uint16_t)sel0, LV_ANIM_OFF);
        lv_roller_set_selected(bk_ui->detailsettingdamper_settemp_setn2, (uint16_t)sel1, LV_ANIM_OFF);
        lv_roller_set_selected(bk_ui->detailsettingdamper_settemp_setn3, (uint16_t)sel2, LV_ANIM_OFF);

        _img_ensure_src(bk_ui->detailsettingdamper_settemp_setn1);
        lv_obj_clear_flag(bk_ui->detailsettingdamper_settemp_setn1, LV_OBJ_FLAG_HIDDEN);
        _img_ensure_src(bk_ui->detailsettingdamper_settemp_setn2);
        lv_obj_clear_flag(bk_ui->detailsettingdamper_settemp_setn2, LV_OBJ_FLAG_HIDDEN);
        _img_ensure_src(bk_ui->detailsettingdamper_settemp_setn3);
        lv_obj_clear_flag(bk_ui->detailsettingdamper_settemp_setn3, LV_OBJ_FLAG_HIDDEN);
    }
}

void detailsettingdamper_roller_event_cb(lv_event_t *e) { (void)e; }

void detailsettingdamper_backbt_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    if (lv_tick_elaps(last_click_time) < 250) return;
    last_click_time = lv_tick_get();
    hal_buzzer_beep();

    settings_save_dirty();
#if UI_PRENDERING_ENABLE
    ui_page_change(PAGE_SETTINGMODEDETAILSETTING);
#else
    if (bk_ui->settingmodedetailsetting == NULL || !lv_obj_is_valid(bk_ui->settingmodedetailsetting))
        init_page_settingmodedetailsetting(bk_ui);
    lv_scr_load(bk_ui->settingmodedetailsetting);
#endif /* UI_PRENDERING_ENABLE */
}

static void _settingbt(int field)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    if (lv_tick_elaps(last_click_time) < 250) return;
    last_click_time = lv_tick_get();
    hal_buzzer_beep();
    _show_picker(bk_ui, field);
}

void detailsettingdamper_settingbt1_event_cb(lv_event_t *e)
{ if (lv_event_get_code(e) != LV_EVENT_CLICKED) return; _settingbt(0); }

void detailsettingdamper_settingbt2_event_cb(lv_event_t *e)
{ if (lv_event_get_code(e) != LV_EVENT_CLICKED) return; _settingbt(1); }

void detailsettingdamper_settingbt3_event_cb(lv_event_t *e)
{ if (lv_event_get_code(e) != LV_EVENT_CLICKED) return; _settingbt(2); }

void detailsettingdamper_settingbt4_event_cb(lv_event_t *e)
{ if (lv_event_get_code(e) != LV_EVENT_CLICKED) return; _settingbt(3); }

void detailsettingdamper_changebt_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    device_state_t *state = &g_device_state;
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    if (lv_tick_elaps(last_click_time) < 250) return;
    last_click_time = lv_tick_get();
    hal_buzzer_beep();

    if (s_edit_field < 0 || s_edit_field >= 4) return;

    lv_obj_t *ims[4], *txts[4];
    _get_row_objs(bk_ui, ims, txts);

    char result[16];

    if (s_edit_field <= 1) {
        /* Single roller (setn4) */
        lv_roller_get_selected_str(bk_ui->detailsettingdamper_settemp_setn4, result, sizeof(result));
    } else {
        /* Triple roller: concatenate with leading zero strip, clamp to 180 */
        char s0[4], s1[4], s2[4];
        lv_roller_get_selected_str(bk_ui->detailsettingdamper_settemp_setn1, s0, sizeof(s0));
        lv_roller_get_selected_str(bk_ui->detailsettingdamper_settemp_setn2, s1, sizeof(s1));
        lv_roller_get_selected_str(bk_ui->detailsettingdamper_settemp_setn3, s2, sizeof(s2));

        if (strcmp(s0, "0") == 0) {
            if (strcmp(s1, "0") == 0)
                snprintf(result, sizeof(result), "%s", s2);
            else
                snprintf(result, sizeof(result), "%s%s", s1, s2);
        } else {
            snprintf(result, sizeof(result), "%s%s%s", s0, s1, s2);
        }
        int val = atoi(result);
        if (val > 180) val = 180;
        snprintf(result, sizeof(result), "%d", val);
    }

    lv_label_set_text(txts[s_edit_field], result);
    settings_set_str(s_keys[s_edit_field], result);
    settings_save_dirty();

    state->change_setting = true;
    state->start_run = false;
    uart_comm_trigger_change_setting();
}

void detailsettingdamper_loaded_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    ui_title_anim(bk_ui->detailsettingdamper_title);
}

void detailsettingdamper_unload_start_event_cb(lv_event_t *e)
{
    (void)e;
}

void detailsettingdamper_unloaded_event_cb(lv_event_t *e)
{
    (void)e;
}

void detailsettingdamper_load_start_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    uint32_t t0 = lv_tick_get();
    bk_printf(TAG "[PERF] detaildamper load_event start\n");

    _build_img_arrays_if_changed();
    bk_printf(TAG "[PERF]   _build_img_arrays +%lu ms\n", (unsigned long)lv_tick_elaps(t0));

    ui_lang_apply_detailsettingdamper(bk_ui);
    s_edit_field = -1;
    /* s_fan_opts_set / s_triple_opts_set: 언어 변경 시 _build_img_arrays_if_changed()에서 초기화됨.
     * 스크린 재진입마다 초기화하면 첫 탭 시 lv_roller_set_options 가 불필요하게 재호출됨. */
    _refresh(bk_ui);
    _update_txt_colors(bk_ui, -1);   /* 재진입 시 이전 선택 색상(검정) 잔류 방지 */
    bk_printf(TAG "[PERF]   _refresh +%lu ms\n", (unsigned long)lv_tick_elaps(t0));

    lv_obj_add_flag(bk_ui->detailsettingdamper_settemp_setn1, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(bk_ui->detailsettingdamper_settemp_setn2, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(bk_ui->detailsettingdamper_settemp_setn3, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(bk_ui->detailsettingdamper_settemp_setn4, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(bk_ui->detailsettingdamper_pickerbox,     LV_OBJ_FLAG_HIDDEN);
    bk_printf(TAG "[PERF] detaildamper load_event end total=%lu ms\n", (unsigned long)lv_tick_elaps(t0));
}
