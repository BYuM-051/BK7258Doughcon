#include "lvgl.h"
#include "beken_ui.h"
#include "custom_func.h"
#include "event_runtime.h"
#include <stdio.h>
#include <string.h>

#include "beken_ui.h"
#include "ui_animations.h"
#include "ui_lang.h"
#include "hardware_hal.h"

extern bk_lv_ui_t bk_lv_tool_ui;

static uint32_t last_click_time = 0;

void settingmodedefrost_backbt_event_cb(lv_event_t *e);
void settingmodedefrost_load_event_cb(lv_event_t *e);

void settingmodedefrost_backbt_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    if (lv_tick_elaps(last_click_time) < 250) return;
    last_click_time = lv_tick_get();
    hal_buzzer_beep();
    if (bk_ui->settingmode == NULL || !lv_obj_is_valid(bk_ui->settingmode))
        init_page_settingmode(bk_ui);
    lv_scr_load(bk_ui->settingmode);
}

void settingmodedefrost_load_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    if (code == LV_EVENT_SCREEN_LOADED) {
        ui_title_anim(bk_ui->settingmodedefrost_title);
        return;
    }
    if (code != LV_EVENT_SCREEN_LOAD_START) return;

    ui_lang_apply_settingmodedefrost(bk_ui);
}
