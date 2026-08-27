#include "lvgl.h"
#include "beken_ui.h"
#include "custom_func.h"
#include "event_runtime.h"
#include <stdio.h>
#include <string.h>

#include "beken_ui.h"
#include "settings.h"
#include "hardware_hal.h"
#include "preRenderer.h"

extern bk_lv_ui_t bk_lv_tool_ui;

static uint32_t last_click_time = 0;

void popupreset_yesbt_event_cb(lv_event_t *e);
void popupreset_nobt_event_cb(lv_event_t *e);

void popupreset_yesbt_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    if (lv_tick_elaps(last_click_time) < 250) return;
    last_click_time = lv_tick_get();

    hal_buzzer_beep();
    destroy_page_popupreset(bk_ui);   /* 팝업 먼저 닫기 */
    settings_factory_reset();

    /* 초기화 완료 → settingmode 화면으로 복귀 */
#if UI_PRENDERING_ENABLE
    ui_page_change(PAGE_SETTINGMODE);
#else
    if (bk_ui->settingmode == NULL || !lv_obj_is_valid(bk_ui->settingmode))
        init_page_settingmode(bk_ui);
    lv_scr_load(bk_ui->settingmode);
#endif /* UI_PRENDERING_ENABLE */
}

void popupreset_nobt_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    if (lv_tick_elaps(last_click_time) < 250) return;
    last_click_time = lv_tick_get();

    hal_buzzer_beep();
    /* 취소 — 팝업만 닫고 배경 화면(settingmodedetailsetting) 그대로 유지 */
    destroy_page_popupreset(bk_ui);
}
