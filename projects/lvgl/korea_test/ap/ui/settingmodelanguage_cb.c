#include "lvgl.h"
#include "beken_ui.h"
#include "custom_func.h"
#include "event_runtime.h"
#include <stdio.h>
#include <string.h>

#include "beken_ui.h"
#include "settings.h"
#include "device_state.h"
#include "ui_animations.h"
#include "ui_lang.h"
#include "hardware_hal.h"
#include "pageManager.h"

extern bk_lv_ui_t bk_lv_tool_ui;

static uint32_t last_click_time = 0;

void settingmodelanguage_backbt_event_cb(lv_event_t *e);
void settingmodelanguage_koreanbt_event_cb(lv_event_t *e);
void settingmodelanguage_englishbt_event_cb(lv_event_t *e);
void settingmodelanguage_chinabt_event_cb(lv_event_t *e);
void settingmodelanguage_load_start_event_cb(lv_event_t *e);
void settingmodelanguage_loaded_event_cb(lv_event_t *e);
void settingmodelanguage_unload_start_event_cb(lv_event_t *e);
void settingmodelanguage_unloaded_event_cb(lv_event_t *e);

void settingmodelanguage_backbt_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    if (lv_tick_elaps(last_click_time) < 250) return;
    last_click_time = lv_tick_get();
    hal_buzzer_beep();
    ui_page_change(PAGE_SETTINGMODE);
}

extern void main_settingmode_prewarm_reset(void);
extern void settingmode_testmode_prewarm_reset(void);
extern void settingmode_popuppassword_prewarm_reset(void);
extern void automode_ams_prewarm_reset(void);

static void _apply_language_change(bk_lv_ui_t *bk_ui)
{
    volatile int language = g_device_state.language;
    
    char fullPath[128];
    getImageFullPath("/images/language_title", true, false, ".png", fullPath, sizeof(fullPath));
    lv_image_set_src(bk_ui->settingmodelanguage_title, fullPath);

    getImageFullPath("/images/exit_bt", true, false, ".png", fullPath, sizeof(fullPath));
    lv_image_set_src(bk_ui->settingmodelanguage_exitim, fullPath);

    lv_image_set_src(bk_ui->settingmodelanguage_koreanim, "/images/language_korean_off.png");
    lv_image_set_src(bk_ui->settingmodelanguage_chinaim, "/images/language_china_off.png");
    lv_image_set_src(bk_ui->settingmodelanguage_englishim, "/images/language_english_off.png");

    // 선택된 언어만 On 이미지로 변경 및 타이틀/종료버튼 언어 교체
    if (g_device_state.language == 0) 
    {
        lv_image_set_src(bk_ui->settingmodelanguage_koreanim, "/images/language_korean_on.png");
    } 
    else if (g_device_state.language == 1) 
    {
        lv_image_set_src(bk_ui->settingmodelanguage_chinaim, "/images/language_china_on.png");
    } 
    else if (g_device_state.language == 2) 
    {
        lv_image_set_src(bk_ui->settingmodelanguage_englishim, "/images/language_english_on.png");
    }

    getImageFullPath("/images/timebar_timebar", true, false, ".png", fullPath, sizeof(fullPath));
    lv_image_set_src(bk_ui->timebar_timebar_bg, fullPath);
    getImageFullPath("/images/timebar_error_on", true, false, ".png", fullPath, sizeof(fullPath));
    lv_image_set_src(bk_ui->timebar_timebar_error_checkim, fullPath);
    getImageFullPath("/images/timebar_comp_on", true, false, ".png", fullPath, sizeof(fullPath));
    lv_image_set_src(bk_ui->timebar_comp_checkim, fullPath);
    getImageFullPath("/images/timebar_watevalve_on", true, false, ".png", fullPath, sizeof(fullPath));
    lv_image_set_src(bk_ui->timebar_watervalve_checkim, fullPath);
    getImageFullPath("/images/timebar_roomfan_on", true, false, ".png", fullPath, sizeof(fullPath));
    lv_image_set_src(bk_ui->timebar_roomfan_checkim, fullPath);
    getImageFullPath("/images/timebar_damper_on", true, false, ".png", fullPath, sizeof(fullPath));
    lv_image_set_src(bk_ui->timebar_damper_checkim, fullPath);
    getImageFullPath("/images/timebar_humidityheater_on", true, false, ".png", fullPath, sizeof(fullPath));
    lv_image_set_src(bk_ui->timebar_humid_checkim, fullPath);
    getImageFullPath("/images/timebar_fireheater_on", true, false, ".png", fullPath, sizeof(fullPath));
    lv_image_set_src(bk_ui->timebar_heat_checkim, fullPath);
    getImageFullPath("/images/timebar_defrostheater_on", true, false, ".png", fullPath, sizeof(fullPath));
    lv_image_set_src(bk_ui->timebar_defrost_checkim, fullPath);

    lv_lock();
    lv_obj_invalidate(bk_ui->settingmodelanguage);
    lv_refr_now(NULL);
    lv_unlock();

    ui_lang_invalidate_cached_screens(bk_ui);
    /* 현재 화면(언어선택)의 타이틀·나가기 버튼을 새 언어로 즉시 갱신 */
}

void settingmodelanguage_koreanbt_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    device_state_t *state = &g_device_state;
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    if (lv_tick_elaps(last_click_time) < 250) return;
    last_click_time = lv_tick_get();

    if (!settings_is_loaded()) return;
    state->language = 0;
    settings_set_str("LANGUAGE", "0");
    settings_save_dirty();
    hal_buzzer_beep();
    _apply_language_change(bk_ui);
}

void settingmodelanguage_englishbt_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    device_state_t *state = &g_device_state;
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    if (lv_tick_elaps(last_click_time) < 250) return;
    last_click_time = lv_tick_get();

    if (!settings_is_loaded()) return;
    state->language = 2;
    settings_set_str("LANGUAGE", "2");
    settings_save_dirty();
    hal_buzzer_beep();
    _apply_language_change(bk_ui);
}

void settingmodelanguage_chinabt_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    device_state_t *state = &g_device_state;
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    if (lv_tick_elaps(last_click_time) < 250) return;
    last_click_time = lv_tick_get();

    if (!settings_is_loaded()) return;
    state->language = 1;
    settings_set_str("LANGUAGE", "1");
    settings_save_dirty();
    hal_buzzer_beep();
    _apply_language_change(bk_ui);
}



void settingmodelanguage_loaded_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    ui_title_anim(bk_ui->settingmodelanguage_title);
}

void settingmodelanguage_unload_start_event_cb(lv_event_t *e)
{
    (void)e;
}

void settingmodelanguage_unloaded_event_cb(lv_event_t *e)
{
    (void)e;
}

void settingmodelanguage_load_start_event_cb(lv_event_t *e)
{
}
