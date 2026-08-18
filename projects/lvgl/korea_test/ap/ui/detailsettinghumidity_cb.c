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

extern bk_lv_ui_t bk_lv_tool_ui;

static uint32_t last_click_time = 0;
static int s_edit_field = -1;

static const char * const s_humidMinor[] = {
    "0","1","2","3","4","5","6","7","8","9","10",
    "-10","-9","-8","-7","-6","-5","-4","-3","-2","-1"
}; /* 21 */

static const char * const s_humidBasic[] = {
    "1","2","3","4","5","6","7","8","9","10"
}; /* 10 */

static const char * const s_keys[3] = {
    "DetailHumidityOff", "DetailHumidityOd", "DetailHumidityRevision"
};

typedef struct { const char * const *arr; int cnt; } _picker_opt_t;

static const _picker_opt_t s_picker_opts[3] = {
    {s_humidMinor, 21},
    {s_humidBasic, 10},
    {s_humidMinor, 21},
};

static char s_off_imgs[3][128];
static char s_on_imgs[3][128];
static int  s_roller_field = -1;  /* last field loaded into roller; -1 = none */

static void _build_img_arrays(void)
{
    int lang = settings_get_int("LANGUAGE");
    const char *lsuf = (lang == 1) ? "_china" : (lang == 2) ? "_english" : "";
    static const char * const bases[3] = {
        "detail_humidity_1", "detail_humidity_2", "detail_humidity_3"
    };
    for (int i = 0; i < 3; i++) {
        snprintf(s_off_imgs[i], 128, "/images/%s_off%s.png", bases[i], lsuf);
        snprintf(s_on_imgs[i],  128, "/images/%s_on%s.png",  bases[i], lsuf);
    }
}

static void _build_img_arrays_if_changed(void)
{
    static int s_last_lang = -1;
    int lang = settings_get_int("LANGUAGE");
    if (lang == s_last_lang) return;
    s_last_lang = lang;
    _build_img_arrays();
}

void detailsettinghumidity_backbt_event_cb(lv_event_t *e);
void detailsettinghumidity_settingbt1_event_cb(lv_event_t *e);
void detailsettinghumidity_settingbt2_event_cb(lv_event_t *e);
void detailsettinghumidity_settingbt3_event_cb(lv_event_t *e);
void detailsettinghumidity_changebt_event_cb(lv_event_t *e);
void detailsettinghumidity_roller_event_cb(lv_event_t *e);
void detailsettinghumidity_load_event_cb(lv_event_t *e);

static void _get_row_objs(bk_lv_ui_t *bk_ui, lv_obj_t *ims[3], lv_obj_t *txts[3])
{
    ims[0]  = bk_ui->detailsettinghumidity_settingim1;
    ims[1]  = bk_ui->detailsettinghumidity_settingim2;
    ims[2]  = bk_ui->detailsettinghumidity_settingim3;
    txts[0] = bk_ui->detailsettinghumidity_settingtxt1;
    txts[1] = bk_ui->detailsettinghumidity_settingtxt2;
    txts[2] = bk_ui->detailsettinghumidity_settingtxt3;
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
    lv_obj_add_flag(bk_ui->detailsettinghumidity_settemp_setn1, LV_OBJ_FLAG_HIDDEN);
    s_edit_field = -1;
    _update_txt_colors(bk_ui, -1);
}

static void _refresh(bk_lv_ui_t *bk_ui)
{
    lv_obj_t *ims[3], *txts[3];
    _get_row_objs(bk_ui, ims, txts);
    for (int i = 0; i < 3; i++) {
        _img_set_src_timed(ims[i], s_off_imgs[i]);
        lv_label_set_text(txts[i], settings_get_str(s_keys[i]));
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
    _img_ensure_src(bk_ui->detailsettinghumidity_pickerbox);
    lv_obj_clear_flag(bk_ui->detailsettinghumidity_pickerbox, LV_OBJ_FLAG_HIDDEN);


    const char * const *arr = s_picker_opts[field].arr;
    int cnt = s_picker_opts[field].cnt;

    /* roller option caching — skip re-layout when same field re-opened */
    if (s_roller_field != field) {
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
        lv_roller_set_options(bk_ui->detailsettinghumidity_settemp_setn1, opts, LV_ROLLER_MODE_INFINITE);
        lv_roller_set_visible_row_count(bk_ui->detailsettinghumidity_settemp_setn1, 3);
        s_roller_field = field;
    }

    const char *cur = settings_get_str(s_keys[field]);
    int sel = 0;
    for (int i = 0; i < cnt; i++) {
        if (strcmp(arr[i], cur) == 0) { sel = i; break; }
    }
    lv_roller_set_selected(bk_ui->detailsettinghumidity_settemp_setn1, (uint16_t)sel, LV_ANIM_OFF);
    _img_ensure_src(bk_ui->detailsettinghumidity_settemp_setn1);
    lv_obj_clear_flag(bk_ui->detailsettinghumidity_settemp_setn1, LV_OBJ_FLAG_HIDDEN);
}

void detailsettinghumidity_roller_event_cb(lv_event_t *e)
{
    (void)e;
}

void detailsettinghumidity_backbt_event_cb(lv_event_t *e)
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

void detailsettinghumidity_settingbt1_event_cb(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    _settingbt(0);
}

void detailsettinghumidity_settingbt2_event_cb(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    _settingbt(1);
}

void detailsettinghumidity_settingbt3_event_cb(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    _settingbt(2);
}

void detailsettinghumidity_changebt_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    device_state_t *state = &g_device_state;
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    if (lv_tick_elaps(last_click_time) < 250) return;
    last_click_time = lv_tick_get();
    hal_buzzer_beep();

    if (s_edit_field < 0 || s_edit_field >= 3) return;

    char buf[32];
    lv_roller_get_selected_str(bk_ui->detailsettinghumidity_settemp_setn1, buf, sizeof(buf));

    lv_obj_t *ims[3], *txts[3];
    _get_row_objs(bk_ui, ims, txts);
    lv_label_set_text(txts[s_edit_field], buf);
    settings_set_str(s_keys[s_edit_field], buf);
    settings_save_dirty();

    state->change_setting = true;
    state->start_run = false;
    uart_comm_trigger_change_setting();
}

void detailsettinghumidity_load_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    if (code == LV_EVENT_SCREEN_LOADED) {
        ui_title_anim(bk_ui->detailsettinghumidity_title);
        return;
    }
    if (code != LV_EVENT_SCREEN_LOAD_START) return;
    uint32_t t0 = lv_tick_get();
    printf("[PERF] detailhumidity load_event start\n");

    _build_img_arrays_if_changed();
    printf("[PERF]   _build_img_arrays +%lu ms\n", (unsigned long)lv_tick_elaps(t0));

    ui_lang_apply_detailsettinghumidity(bk_ui);
    s_edit_field = -1;
    /* s_roller_field: options 은 숫자 고정값(언어 무관)이므로 스크린 재진입 시 초기화 불필요.
     * LVGL 위젯이 살아있는 한 이전에 설정한 options 가 그대로 유지된다. */
    _refresh(bk_ui);
    _update_txt_colors(bk_ui, -1);   /* 재진입 시 이전 선택 색상(검정) 잔류 방지 */
    printf("[PERF]   _refresh +%lu ms\n", (unsigned long)lv_tick_elaps(t0));

    lv_obj_add_flag(bk_ui->detailsettinghumidity_settemp_setn1, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(bk_ui->detailsettinghumidity_pickerbox,     LV_OBJ_FLAG_HIDDEN);
    printf("[PERF] detailhumidity load_event end total=%lu ms\n", (unsigned long)lv_tick_elaps(t0));
}
