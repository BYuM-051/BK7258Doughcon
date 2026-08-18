#include "lvgl.h"
#include "beken_ui.h"
#include "custom_func.h"
#include "event_runtime.h"
#include <stdio.h>
#include <string.h>

#include "beken_ui.h"
#include "hardware_hal.h"

extern bk_lv_ui_t bk_lv_tool_ui;

static uint32_t last_click_time = 0;

void popupcaution_dismissbt_event_cb(lv_event_t *e);

void popupcaution_dismissbt_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    if (lv_tick_elaps(last_click_time) < 250) return;
    last_click_time = lv_tick_get();
    hal_buzzer_beep();

    if (bk_ui->popupcaution && lv_obj_is_valid(bk_ui->popupcaution)) {
        lv_obj_del(bk_ui->popupcaution);
        bk_ui->popupcaution = NULL;
    }
}
