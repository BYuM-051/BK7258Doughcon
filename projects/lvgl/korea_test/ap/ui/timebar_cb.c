#include "lvgl.h"
#include "beken_ui.h"
#include "custom_func.h"
#include "event_runtime.h"
#include <time.h>
#include <stdio.h>
#include <string.h>
#include "beken_ui.h"
#include "settings.h"
#include "device_state.h"
#include "ui_lang.h"
#include "hardware_hal.h"

extern bk_lv_ui_t bk_lv_tool_ui;
extern void lv_digital_clock_register(lv_obj_t *label, int show_second, int use_ampm, int hour, int minute, int second);
extern void lv_digital_date_register(lv_obj_t *label);
extern void popuperror_tick(bk_lv_ui_t *bk_ui);
extern void popuperror_dismiss(bk_lv_ui_t *bk_ui);
extern void popuperror_toggle(bk_lv_ui_t *bk_ui);
extern void popupconnectionerror_tick(bk_lv_ui_t *bk_ui);

static uint32_t last_click_time = 0;
static lv_timer_t *s_timebar_icon_timer = NULL;

/* 아이콘 표시/숨김 헬퍼 — 상태 변경 시에만 flag 변경(중복 호출 → lv_obj_invalidate 방지)
 * 불필요한 dirty area 생성 시 반투명 popuptime 피드백 루프로 배경 깨짐이 발생하므로
 * 현재 HIDDEN 상태와 원하는 상태가 다를 때만 변경한다 */
#define _ICON(obj, cond) \
    do { \
        bool _want_vis = (bool)(cond); \
        bool _is_hidden = lv_obj_has_flag((obj), LV_OBJ_FLAG_HIDDEN); \
        if (_want_vis && _is_hidden)  { _img_ensure_src((obj)); lv_obj_clear_flag((obj), LV_OBJ_FLAG_HIDDEN); } \
        else if (!_want_vis && !_is_hidden) lv_obj_add_flag((obj), LV_OBJ_FLAG_HIDDEN); \
    } while(0)

static void _update_sound_icon(bk_lv_ui_t *bk_ui)
{
    static int s_last_mute = -1;
    int cur = g_device_state.mute ? 1 : 0;
    if (cur == s_last_mute) return;
    s_last_mute = cur;
    _img_set_src_timed(bk_ui->timebar_sound_checkim,
        cur ? "/images/timebar_volume_off.png"
            : "/images/timebar_volume_on.png");
}

static void _timebar_icon_update(bk_lv_ui_t *bk_ui)
{
    device_state_t *state = &g_device_state;
    /* op_* flags come from MCU 0x43 packets and are never cleared on stop.
     * Gate them on the running state so stale values don't show after completion. */
    bool running = state->operation || state->auto_mode_start || state->auto_dry_mode_start;
    _ICON(bk_ui->timebar_timebar_error_checkim, state->op_error);
    _ICON(bk_ui->timebar_comp_checkim,          running && state->op_comp);
    _ICON(bk_ui->timebar_heat_checkim,          running && state->op_fire_heater);
    _ICON(bk_ui->timebar_humid_checkim,         running && state->op_humid_heater);
    _ICON(bk_ui->timebar_watervalve_checkim,    running && state->op_water_pump);
    _ICON(bk_ui->timebar_roomfan_checkim,       running && state->op_fan);
    _ICON(bk_ui->timebar_defrost_checkim,       running && state->op_frozen_heater);
    _ICON(bk_ui->timebar_damper_checkim,        running && state->op_damper);
    _update_sound_icon(bk_ui);
}

static void _timebar_icon_timer_cb(lv_timer_t *timer)
{
    (void)timer;
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    _timebar_icon_update(bk_ui);
    popuperror_tick(bk_ui);
    popupconnectionerror_tick(bk_ui);
}

void timebar_timebar_error_checkbt_event_cb(lv_event_t *e);
void timebar_sound_checkbt_event_cb(lv_event_t *e);
void timebar_load_event_cb(lv_event_t *e);

void timebar_timebar_error_checkbt_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    if (!g_device_state.op_error) return;
    if (lv_tick_elaps(last_click_time) < 250) return;
    last_click_time = lv_tick_get();
    hal_buzzer_beep();
    popuperror_toggle(bk_ui);
}

void timebar_sound_checkbt_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    device_state_t *state = &g_device_state;
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    if (lv_tick_elaps(last_click_time) < 250) return;
    last_click_time = lv_tick_get();

    state->mute = !state->mute;
    settings_set_str("Mute", state->mute ? "1" : "0");
    _update_sound_icon(bk_ui);
    /* 음소거 ON/OFF 모두 토글 확인음 발생 — hal_buzzer_beep()는 mute일 때
     * 스스로 무음 처리하므로, mute를 켜는 순간에도 들리도록 저수준 함수 직접 호출 */
    hal_buzzer_start(3000, 50);
}

void timebar_start_icon_timer(void)
{
    if (s_timebar_icon_timer) return;
    s_timebar_icon_timer = lv_timer_create(_timebar_icon_timer_cb, 300, NULL);
}

void timebar_load_event_cb(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_SCREEN_LOAD_START) return;
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    ui_lang_apply_timebar(bk_ui);
}
