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
/* 이미지 상태 업데이트 함수 (alloffim 역할) */
void _update_language_ui(void)
{
    // 전체 체크 해제 상태로 초기화 (Off 이미지)
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    device_state_t *state = &g_device_state;

    _img_set_src_timed(bk_ui->settingmodelanguage_koreanim, "/images/language_korean_off.png");
    _img_set_src_timed(bk_ui->settingmodelanguage_chinaim, "/images/language_china_off.png");
    _img_set_src_timed(bk_ui->settingmodelanguage_englishim, "/images/language_english_off.png");

    // 선택된 언어만 On 이미지로 변경 및 타이틀/종료버튼 언어 교체
    if (state->language == 0) {
        _img_set_src_timed(bk_ui->settingmodelanguage_koreanim, "/images/language_korean_on.png");
        // _img_set_src_timed(bk_ui->settingmodelanguage_chinaim, "/images/language_china_off.png");
        // _img_set_src_timed(bk_ui->settingmodelanguage_englishim, "/images/language_english_off.png");
    } else if (state->language == 1) {
        _img_set_src_timed(bk_ui->settingmodelanguage_chinaim, "/images/language_china_on.png");
        // _img_set_src_timed(s_title_img, "/images/language_title_china.png");
        // _img_set_src_timed(s_exit_img, "/images/exit_bt_china.png");
    } else if (state->language == 2) {
        _img_set_src_timed(bk_ui->settingmodelanguage_englishim, "/images/language_english_on.png");
        // _img_set_src_timed(s_title_img, "/images/language_title_english.png");
        // _img_set_src_timed(s_exit_img, "/images/exit_bt_english.png");
    }
}


void settingmodelanguage_backbt_event_cb(lv_event_t *e)
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

extern void main_settingmode_prewarm_reset(void);
extern void settingmode_testmode_prewarm_reset(void);
extern void settingmode_popuppassword_prewarm_reset(void);
extern void automode_ams_prewarm_reset(void);

static void _apply_language_change(bk_lv_ui_t *bk_ui)
{
    _update_language_ui();
    main_settingmode_prewarm_reset();
    settingmode_testmode_prewarm_reset();
    settingmode_popuppassword_prewarm_reset();
    automode_ams_prewarm_reset();
    /* Destroy cached screens instead of re-decoding images on all of them.
     * Each screen will be re-created with the new language on next visit. */
    ui_lang_invalidate_cached_screens(bk_ui);
    /* 현재 화면(언어선택)의 타이틀·나가기 버튼을 새 언어로 즉시 갱신 */
    ui_lang_apply_settingmodelanguage(bk_ui);
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
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    ui_lang_apply_settingmodelanguage(bk_ui);
    _update_language_ui();
}
