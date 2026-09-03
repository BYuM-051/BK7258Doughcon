#include "lvgl.h"
#include "beken_ui.h"
#include "custom_func.h"
#include "event_runtime.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "settings.h"

#include "beken_ui.h"
#include "ui_animations.h"
#include "ui_lang.h"
#include "settings.h"
#include "device_state.h"
#include "hardware_hal.h"
#include "pageManager.h"

extern bk_lv_ui_t bk_lv_tool_ui;
extern double degree_basic_change(double ch);
extern double reverse_degree_basic_change(double ch);

static uint32_t last_click_time = 0;

/* CurrentSave* 온도값을 단위 전환에 맞춰 일괄 변환 후 settings에 재저장 */
static void _convert_current_temps(int to_fahrenheit)
{
    static const char * const keys[] = {
        "CurrentSaveFreezeTemp",
        "CurrentSaveDefreezeTemp",
        "CurrentSaveFermentation1Temp",
        "CurrentSaveFermentation2Temp",
        NULL
    };
    char buf[16];
    for (int i = 0; keys[i] != NULL; i++) {
        const char *v = settings_get_str(keys[i]);
        if (!v || v[0] == '\0') continue;
        double converted = to_fahrenheit
            ? degree_basic_change(atof(v))
            : reverse_degree_basic_change(atof(v));
        snprintf(buf, sizeof(buf), "%.0f", converted);
        settings_set_str(keys[i], buf);
    }
}

void settingmodedegree_backbt_event_cb(lv_event_t *e);
void settingmodedegree_degree_c_bt_event_cb(lv_event_t *e);
void settingmodedegree_degree_f_bt_event_cb(lv_event_t *e);
void settingmodedegree_load_start_event_cb(lv_event_t *e);
void settingmodedegree_loaded_event_cb(lv_event_t *e);
void settingmodedegree_unload_start_event_cb(lv_event_t *e);
void settingmodedegree_unloaded_event_cb(lv_event_t *e);

#define DEGREE_F "\xc2\xb0""F"
#define DEGREE_C "\xc2\xb0""C"

static void _update_images(bk_lv_ui_t *bk_ui, int is_f)
{
    if (is_f) {
        _img_set_src_timed(bk_ui->settingmodedegree_degree_c_im, "/images/degree_bt_c_off.png");
        _img_set_src_timed(bk_ui->settingmodedegree_degree_f_im, "/images/degree_bt_f_on.png");
    } else {
        _img_set_src_timed(bk_ui->settingmodedegree_degree_c_im, "/images/degree_bt_c_on.png");
        _img_set_src_timed(bk_ui->settingmodedegree_degree_f_im, "/images/degree_bt_f_off.png");
    }
}

void settingmodedegree_loaded_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    ui_title_anim(bk_ui->settingmodedegree_title);
}

void settingmodedegree_unload_start_event_cb(lv_event_t *e)
{
    (void)e;
}

void settingmodedegree_unloaded_event_cb(lv_event_t *e)
{
    (void)e;
}

void settingmodedegree_load_start_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    ui_lang_apply_settingmodedegree(bk_ui);
    _update_images(bk_ui, strcmp(settings_get_str("Degree"), DEGREE_F) == 0);
}

void settingmodedegree_backbt_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    device_state_t *state = &g_device_state;
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    if (lv_tick_elaps(last_click_time) < 250) return;
    last_click_time = lv_tick_get();

    if (state->lock) return;
    hal_buzzer_beep();
#if UI_PRENDERING_ENABLE
    ui_page_change(PAGE_SETTINGMODE);
#else
    if (bk_ui->settingmode == NULL || !lv_obj_is_valid(bk_ui->settingmode))
        init_page_settingmode(bk_ui);
    lv_scr_load(bk_ui->settingmode);
    destroy_page_settingmodedegree(bk_ui);
#endif /* UI_PRENDERING_ENABLE */
}

void settingmodedegree_degree_c_bt_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    device_state_t *state = &g_device_state;
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    if (lv_tick_elaps(last_click_time) < 250) return;
    last_click_time = lv_tick_get();

    if (state->lock) return;
    if (!settings_is_loaded()) return;   /* blob 로드 전엔 온도변환 기준값이 틀림 */
    hal_buzzer_beep();

    if (strcmp(settings_get_str("Degree"), DEGREE_F) == 0)
        _convert_current_temps(0);   /* °F → °C */
    settings_set_str("Degree", DEGREE_C);
    settings_save_dirty();
    _update_images(bk_ui, 0);
    ui_lang_invalidate_cached_screens(bk_ui);
}

void settingmodedegree_degree_f_bt_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    device_state_t *state = &g_device_state;
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    if (lv_tick_elaps(last_click_time) < 250) return;
    last_click_time = lv_tick_get();

    if (state->lock) return;
    if (!settings_is_loaded()) return;   /* blob 로드 전엔 온도변환 기준값이 틀림 */
    hal_buzzer_beep();

    if (strcmp(settings_get_str("Degree"), DEGREE_C) == 0)
        _convert_current_temps(1);   /* °C → °F */
    settings_set_str("Degree", DEGREE_F);
    settings_save_dirty();
    _update_images(bk_ui, 1);
    ui_lang_invalidate_cached_screens(bk_ui);
}
