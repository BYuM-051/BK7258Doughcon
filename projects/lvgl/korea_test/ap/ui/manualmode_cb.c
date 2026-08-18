#include "lvgl.h"
#include "beken_ui.h"
#include "custom_func.h"
#include "event_runtime.h"
#include <stdio.h>
#include <string.h>

#include "beken_ui.h"
#include "ui_animations.h"
#include "ui_lang.h"
#include "device_state.h"
#include "hardware_hal.h"

extern bk_lv_ui_t bk_lv_tool_ui;

static uint32_t last_click_time = 0;

void manualmode_backbt_event_cb(lv_event_t *e);
void manualmode_manual_freezebt_event_cb(lv_event_t *e);
void manualmode_manual_defrostbt_event_cb(lv_event_t *e);
void manualmode_manual_fermentationbt_event_cb(lv_event_t *e);
void manualmode_load_event_cb(lv_event_t *e);

void manualmode_backbt_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    device_state_t *state = &g_device_state;
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    if (lv_tick_elaps(last_click_time) < 250) return;
    last_click_time = lv_tick_get();
    hal_buzzer_beep();

    init_page_main(bk_ui);
    lv_scr_load(bk_ui->main);
    state->manual_current_mode = 0;
}

void manualmode_manual_freezebt_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    device_state_t *state = &g_device_state;
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    if (lv_tick_elaps(last_click_time) < 250) return;
    last_click_time = lv_tick_get();
    hal_buzzer_beep();

    state->manual_current_mode = 1;

    init_page_manualmodestart(bk_ui);
    lv_scr_load(bk_ui->manualmodestart);
}

void manualmode_manual_defrostbt_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    device_state_t *state = &g_device_state;
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    if (lv_tick_elaps(last_click_time) < 250) return;
    last_click_time = lv_tick_get();
    hal_buzzer_beep();

    state->manual_current_mode = 2;

    init_page_manualmodestart(bk_ui);
    lv_scr_load(bk_ui->manualmodestart);
}

void manualmode_manual_fermentationbt_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    device_state_t *state = &g_device_state;
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    if (lv_tick_elaps(last_click_time) < 250) return;
    last_click_time = lv_tick_get();
    hal_buzzer_beep();

    state->manual_current_mode = 3;

    init_page_manualmodestart(bk_ui);
    lv_scr_load(bk_ui->manualmodestart);
}

void manualmode_load_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    if (code == LV_EVENT_SCREEN_LOADED) {
        ui_title_anim(bk_ui->manualmode_title);
        return;
    }
    if (code != LV_EVENT_SCREEN_LOAD_START) return;

    ui_lang_apply_manualmode(bk_ui);
}
