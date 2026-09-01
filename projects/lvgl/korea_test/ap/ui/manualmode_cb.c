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

#define TAG "[manualmode_cb.c] "
#include "preRenderer.h"
#define bk_printf(fmt, ...) do {if(0) printf(fmt, ##__VA_ARGS__); } while(0) // disable printf
extern bk_lv_ui_t bk_lv_tool_ui;
extern lv_obj_t *preRenderRoot;

static uint32_t last_click_time = 0;

void manualmode_backbt_event_cb(lv_event_t *e);
void manualmode_manual_freezebt_event_cb(lv_event_t *e);
void manualmode_manual_defrostbt_event_cb(lv_event_t *e);
void manualmode_manual_fermentationbt_event_cb(lv_event_t *e);
void manualmode_load_start_event_cb(lv_event_t *e);
void manualmode_loaded_event_cb(lv_event_t *e);
void manualmode_unload_start_event_cb(lv_event_t *e);
void manualmode_unloaded_event_cb(lv_event_t *e);

void manualmode_backbt_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    device_state_t *state = &g_device_state;
    if (lv_event_get_code(e) != LV_EVENT_PRESSED) return;
    if (lv_tick_elaps(last_click_time) < 250) return;
    last_click_time = lv_tick_get();
    hal_buzzer_beep();

#if UI_PRENDERING_ENABLE
    ui_page_change(PAGE_MAIN);
#else
    init_page_main(bk_ui);
    lv_scr_load(bk_ui->main);
#endif /* UI_PRENDERING_ENABLE */
    state->manual_current_mode = 0;
}

void manualmode_manual_freezebt_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    device_state_t *state = &g_device_state;
    if (lv_event_get_code(e) != LV_EVENT_PRESSED) return;
    if (lv_tick_elaps(last_click_time) < 250) return;
    last_click_time = lv_tick_get();
    hal_buzzer_beep();

    state->manual_current_mode = 1;

#if UI_PRENDERING_ENABLE
    ui_page_change(PAGE_MANUALMODESTART);
#else
    init_page_manualmodestart(bk_ui);
    lv_scr_load(bk_ui->manualmodestart);
#endif /* UI_PRENDERING_ENABLE */
}

void manualmode_manual_defrostbt_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    device_state_t *state = &g_device_state;
    if (lv_event_get_code(e) != LV_EVENT_PRESSED) return;
    if (lv_tick_elaps(last_click_time) < 250) return;
    last_click_time = lv_tick_get();
    hal_buzzer_beep();

    state->manual_current_mode = 2;

#if UI_PRENDERING_ENABLE
    ui_page_change(PAGE_MANUALMODESTART);
#else
    init_page_manualmodestart(bk_ui);
    lv_scr_load(bk_ui->manualmodestart);
#endif /* UI_PRENDERING_ENABLE */
}

void manualmode_manual_fermentationbt_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    device_state_t *state = &g_device_state;
    if (lv_event_get_code(e) != LV_EVENT_PRESSED) return;
    if (lv_tick_elaps(last_click_time) < 250) return;
    last_click_time = lv_tick_get();
    hal_buzzer_beep();

    state->manual_current_mode = 3;

#if UI_PRENDERING_ENABLE
    ui_page_change(PAGE_MANUALMODESTART);
#else
    init_page_manualmodestart(bk_ui);
    lv_scr_load(bk_ui->manualmodestart);
#endif /* UI_PRENDERING_ENABLE */
}

void manualmode_loaded_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    ui_title_anim(bk_ui->manualmode_title);
}

void manualmode_unload_start_event_cb(lv_event_t *e)
{
    (void)e;
}

void manualmode_unloaded_event_cb(lv_event_t *e)
{
    (void)e;
}

void manualmode_load_start_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    ui_lang_apply_manualmode(bk_ui);
}
