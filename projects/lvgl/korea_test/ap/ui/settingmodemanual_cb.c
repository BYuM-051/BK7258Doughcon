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
#include "hardware_hal.h"
#include "pageManager.h"

extern bk_lv_ui_t bk_lv_tool_ui;

static uint32_t last_click_time = 0;

void settingmodemanual_backbt_event_cb(lv_event_t *e);
void settingmodemanual_setting_manual_autobt_event_cb(lv_event_t *e);
void settingmodemanual_setting_manual_manualbt_event_cb(lv_event_t *e);
void settingmodemanual_setting_manual_drybt_event_cb(lv_event_t *e);
void settingmodemanual_setting_manual_memorybt_event_cb(lv_event_t *e);
void settingmodemanual_setting_manual_settingbt_event_cb(lv_event_t *e);
void settingmodemanual_load_start_event_cb(lv_event_t *e);
void settingmodemanual_loaded_event_cb(lv_event_t *e);
void settingmodemanual_unload_start_event_cb(lv_event_t *e);
void settingmodemanual_unloaded_event_cb(lv_event_t *e);

/* Show selection box with title and numeric value (1-5).
 * Font lv_font_scdream_regular_32 is ASCII-only: use English for all languages. */
static void _select_mode(bk_lv_ui_t *bk_ui,
                         const char *title_en, const char *val)
{
    _img_ensure_src(bk_ui->settingmodemanual_setting_manual_boxim);
    lv_obj_clear_flag(bk_ui->settingmodemanual_setting_manual_boxim,    LV_OBJ_FLAG_HIDDEN);
    _img_ensure_src(bk_ui->settingmodemanual_setting_manual_title);
    lv_obj_clear_flag(bk_ui->settingmodemanual_setting_manual_title,    LV_OBJ_FLAG_HIDDEN);
    _img_ensure_src(bk_ui->settingmodemanual_setting_manual_value);
    lv_obj_clear_flag(bk_ui->settingmodemanual_setting_manual_value,    LV_OBJ_FLAG_HIDDEN);
    lv_label_set_text(bk_ui->settingmodemanual_setting_manual_title, title_en);
    lv_label_set_text(bk_ui->settingmodemanual_setting_manual_value, val);
    settings_set_str("SaveManual", val);
    settings_save_dirty();
}

void settingmodemanual_backbt_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    if (lv_tick_elaps(last_click_time) < 250) return;
    last_click_time = lv_tick_get();
    hal_buzzer_beep();

#if UI_PRENDERING_ENABLE
    ui_page_change(PAGE_SETTINGMODE);
#else
    if (bk_ui->settingmode == NULL || !lv_obj_is_valid(bk_ui->settingmode))
        init_page_settingmode(bk_ui);
    lv_scr_load(bk_ui->settingmode);
#endif /* UI_PRENDERING_ENABLE */
}

void settingmodemanual_setting_manual_autobt_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    if (lv_tick_elaps(last_click_time) < 250) return;
    last_click_time = lv_tick_get();
    hal_buzzer_beep();
    _select_mode(bk_ui, "Auto Setting", "1");  /* 자동설정 */
}

void settingmodemanual_setting_manual_manualbt_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    if (lv_tick_elaps(last_click_time) < 250) return;
    last_click_time = lv_tick_get();
    hal_buzzer_beep();
    _select_mode(bk_ui, "Manual Setting", "2");  /* 수동설정 */
}

void settingmodemanual_setting_manual_drybt_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    if (lv_tick_elaps(last_click_time) < 250) return;
    last_click_time = lv_tick_get();
    hal_buzzer_beep();
    _select_mode(bk_ui, "Dry Mode", "3");  /* 건직모드 */
}

void settingmodemanual_setting_manual_memorybt_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    if (lv_tick_elaps(last_click_time) < 250) return;
    last_click_time = lv_tick_get();
    hal_buzzer_beep();
    _select_mode(bk_ui, "Memory Mode", "4");  /* 메모리모드 */
}

void settingmodemanual_setting_manual_settingbt_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    if (lv_tick_elaps(last_click_time) < 250) return;
    last_click_time = lv_tick_get();
    hal_buzzer_beep();
    _select_mode(bk_ui, "Function Setting", "5");  /* 기능설정 */
}

void settingmodemanual_loaded_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    ui_title_anim(bk_ui->settingmodemanual_title);
}

void settingmodemanual_unload_start_event_cb(lv_event_t *e)
{
    (void)e;
}

void settingmodemanual_unloaded_event_cb(lv_event_t *e)
{
    (void)e;
}

void settingmodemanual_load_start_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    ui_lang_apply_settingmodemanual(bk_ui);

    /* Hide selection box until user taps a mode button */
    lv_obj_add_flag(bk_ui->settingmodemanual_setting_manual_boxim,  LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(bk_ui->settingmodemanual_setting_manual_title,  LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(bk_ui->settingmodemanual_setting_manual_value,  LV_OBJ_FLAG_HIDDEN);
}
