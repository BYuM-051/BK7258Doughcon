#include "lvgl.h"
#include "beken_ui.h"
#include "custom_func.h"
#include "event_runtime.h"
#include "ui_lang.h"
#include <stdio.h>
#include <string.h>

#include "ui_config.h"
#include "preRenderer.h"

extern bk_lv_ui_t bk_lv_tool_ui;
extern lv_obj_t *preRenderRoot;
extern void settingmode_setting_detailsettingbt_event_cb(lv_event_t *e);
extern void settingmode_setting_degreebt_event_cb(lv_event_t *e);
extern void settingmode_setting_recordbt_event_cb(lv_event_t *e);
extern void settingmode_setting_testbt_event_cb(lv_event_t *e);
extern void settingmode_setting_timebt_event_cb(lv_event_t *e);
extern void settingmode_setting_languagebt_event_cb(lv_event_t *e);
extern void settingmode_backbt_event_cb(lv_event_t *e);
extern void settingmode_loaded_event_cb(lv_event_t *e);
extern void settingmode_unload_start_event_cb(lv_event_t *e);
extern void settingmode_unloaded_event_cb(lv_event_t *e);
extern void settingmode_load_start_event_cb(lv_event_t *e);


void destroy_page_settingmode(bk_lv_ui_t *bk_ui)
{
    if (bk_ui == NULL) {
        return;
    }
    if (bk_ui->settingmode != NULL) {
        lv_obj_del(bk_ui->settingmode);
        bk_ui->settingmode = NULL;
    }
}

void init_page_settingmode(bk_lv_ui_t * bk_ui) {
    if (bk_ui->settingmode != NULL && lv_obj_is_valid(bk_ui->settingmode)) {
        destroy_page_settingmode(bk_ui);
    }

    ui_lang_reset_settingmode_cache();

#if UI_PRENDERING_ENABLE
    bk_ui->settingmode = lv_obj_create(preRenderRoot);
    lv_obj_remove_style_all(bk_ui->settingmode);
    lv_obj_set_size(bk_ui->settingmode, 1024, 600);
    lv_obj_set_pos(bk_ui->settingmode, 0, 0);
    lv_obj_set_style_radius(bk_ui->settingmode, 0, LV_PART_MAIN);
    lv_obj_set_scrollbar_mode(bk_ui->settingmode, LV_SCROLLBAR_MODE_OFF);
    // FIXME : 안나눠놨네. 나눠야죠. initcallback인데.
    lv_obj_add_event_cb(bk_ui->settingmode, settingmode_load_start_event_cb, UI_EVENT_PAGE_SHOW_START,   NULL);
    lv_obj_add_event_cb(bk_ui->settingmode, settingmode_loaded_event_cb,     UI_EVENT_PAGE_SHOWN,       NULL);
    lv_obj_add_event_cb(bk_ui->settingmode, settingmode_unload_start_event_cb, UI_EVENT_PAGE_HIDE_START, NULL);
    lv_obj_add_event_cb(bk_ui->settingmode, settingmode_unloaded_event_cb,   UI_EVENT_PAGE_HIDDEN,     NULL);
#else
    bk_ui->settingmode = lv_obj_create(NULL);
    lv_obj_set_size(bk_ui->settingmode, 1024, 600);
    lv_obj_set_scrollbar_mode(bk_ui->settingmode, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_event_cb(bk_ui->settingmode, settingmode_load_start_event_cb, LV_EVENT_SCREEN_LOAD_START,   NULL);
    lv_obj_add_event_cb(bk_ui->settingmode, settingmode_loaded_event_cb,     LV_EVENT_SCREEN_LOADED,       NULL);
    lv_obj_add_event_cb(bk_ui->settingmode, settingmode_unload_start_event_cb, LV_EVENT_SCREEN_UNLOAD_START, NULL);
    lv_obj_add_event_cb(bk_ui->settingmode, settingmode_unloaded_event_cb,   LV_EVENT_SCREEN_UNLOADED,     NULL);
#endif /* UI_PRENDERING_ENABLE */
    bk_ui->settingmode_bg = lv_image_create(bk_ui->settingmode);
#if !UI_SETTINGMODE_COMBINED_BG_ENABLE
    _bg_set(bk_ui->settingmode_bg);
#endif
    lv_obj_set_pos(bk_ui->settingmode_bg, 0, 0);

    // ImageView: title
#if !UI_SETTINGMODE_COMBINED_BG_ENABLE
    bk_ui->settingmode_title = lv_image_create(bk_ui->settingmode);
    ui_page_build_set_image_src(bk_ui->settingmode_title, "/images/setting_title.png");
    lv_obj_set_pos(bk_ui->settingmode_title, 0, 10);
    lv_obj_set_size(bk_ui->settingmode_title, 380, 80);
    lv_image_set_inner_align(bk_ui->settingmode_title, LV_IMAGE_ALIGN_TOP_LEFT);
#endif

    // Button: setting_detailsettingbt
    bk_ui->settingmode_setting_detailsettingbt = lv_button_create(bk_ui->settingmode);
    lv_obj_add_flag(bk_ui->settingmode_setting_detailsettingbt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->settingmode_setting_detailsettingbt, settingmode_setting_detailsettingbt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->settingmode_setting_detailsettingbt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->settingmode_setting_detailsettingbt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->settingmode_setting_detailsettingbt, 0, 0);
    lv_obj_set_pos(bk_ui->settingmode_setting_detailsettingbt, 408, 320);
    lv_obj_set_size(bk_ui->settingmode_setting_detailsettingbt, 207, 182);

    // ImageView: imageview3
#if !UI_SETTINGMODE_COMBINED_BG_ENABLE
    bk_ui->settingmode_imageview3 = lv_image_create(bk_ui->settingmode);
    ui_page_build_set_image_src(bk_ui->settingmode_imageview3, "/images/setting_mode_detailsetting.png");
    lv_obj_set_pos(bk_ui->settingmode_imageview3, 408, 320);
    lv_obj_set_size(bk_ui->settingmode_imageview3, 207, 182);
#endif

    // Button: setting_degreebt
    bk_ui->settingmode_setting_degreebt = lv_button_create(bk_ui->settingmode);
    lv_obj_add_flag(bk_ui->settingmode_setting_degreebt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->settingmode_setting_degreebt, settingmode_setting_degreebt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->settingmode_setting_degreebt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->settingmode_setting_degreebt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->settingmode_setting_degreebt, 0, 0);
    lv_obj_set_pos(bk_ui->settingmode_setting_degreebt, 703, 120);
    lv_obj_set_size(bk_ui->settingmode_setting_degreebt, 207, 182);

    // ImageView: imageview5
#if !UI_SETTINGMODE_COMBINED_BG_ENABLE
    bk_ui->settingmode_imageview5 = lv_image_create(bk_ui->settingmode);
    ui_page_build_set_image_src(bk_ui->settingmode_imageview5, "/images/setting_mode_degree.png");
    lv_obj_set_pos(bk_ui->settingmode_imageview5, 703, 120);
    lv_obj_set_size(bk_ui->settingmode_imageview5, 207, 182);
#endif

    // Button: setting_recordbt
    bk_ui->settingmode_setting_recordbt = lv_button_create(bk_ui->settingmode);
    lv_obj_add_flag(bk_ui->settingmode_setting_recordbt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->settingmode_setting_recordbt, settingmode_setting_recordbt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->settingmode_setting_recordbt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->settingmode_setting_recordbt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->settingmode_setting_recordbt, 0, 0);
    lv_obj_set_pos(bk_ui->settingmode_setting_recordbt, 703, 320);
    lv_obj_set_size(bk_ui->settingmode_setting_recordbt, 207, 182);

    // ImageView: imageview7
#if !UI_SETTINGMODE_COMBINED_BG_ENABLE
    bk_ui->settingmode_imageview7 = lv_image_create(bk_ui->settingmode);
    ui_page_build_set_image_src(bk_ui->settingmode_imageview7, "/images/setting_mode_record.png");
    lv_obj_set_pos(bk_ui->settingmode_imageview7, 703, 320);
    lv_obj_set_size(bk_ui->settingmode_imageview7, 207, 182);
#endif

    // Button: setting_testbt
    bk_ui->settingmode_setting_testbt = lv_button_create(bk_ui->settingmode);
    lv_obj_add_flag(bk_ui->settingmode_setting_testbt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->settingmode_setting_testbt, settingmode_setting_testbt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->settingmode_setting_testbt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->settingmode_setting_testbt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->settingmode_setting_testbt, 0, 0);
    lv_obj_set_pos(bk_ui->settingmode_setting_testbt, 113, 320);
    lv_obj_set_size(bk_ui->settingmode_setting_testbt, 207, 182);

    // ImageView: imageview9
#if !UI_SETTINGMODE_COMBINED_BG_ENABLE
    bk_ui->settingmode_imageview9 = lv_image_create(bk_ui->settingmode);
    ui_page_build_set_image_src(bk_ui->settingmode_imageview9, "/images/setting_mode_test.png");
    lv_obj_set_pos(bk_ui->settingmode_imageview9, 113, 320);
    lv_obj_set_size(bk_ui->settingmode_imageview9, 207, 182);
#endif

    // Button: setting_timebt
    bk_ui->settingmode_setting_timebt = lv_button_create(bk_ui->settingmode);
    lv_obj_add_flag(bk_ui->settingmode_setting_timebt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->settingmode_setting_timebt, settingmode_setting_timebt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->settingmode_setting_timebt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->settingmode_setting_timebt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->settingmode_setting_timebt, 0, 0);
    lv_obj_set_pos(bk_ui->settingmode_setting_timebt, 113, 120);
    lv_obj_set_size(bk_ui->settingmode_setting_timebt, 207, 182);

    // ImageView: imageview11
#if !UI_SETTINGMODE_COMBINED_BG_ENABLE
    bk_ui->settingmode_imageview11 = lv_image_create(bk_ui->settingmode);
    ui_page_build_set_image_src(bk_ui->settingmode_imageview11, "/images/setting_mode_time.png");
    lv_obj_set_pos(bk_ui->settingmode_imageview11, 113, 120);
    lv_obj_set_size(bk_ui->settingmode_imageview11, 207, 182);
#endif

    // Button: setting_languagebt
    bk_ui->settingmode_setting_languagebt = lv_button_create(bk_ui->settingmode);
    lv_obj_add_flag(bk_ui->settingmode_setting_languagebt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->settingmode_setting_languagebt, settingmode_setting_languagebt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->settingmode_setting_languagebt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->settingmode_setting_languagebt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->settingmode_setting_languagebt, 0, 0);
    lv_obj_set_pos(bk_ui->settingmode_setting_languagebt, 408, 120);
    lv_obj_set_size(bk_ui->settingmode_setting_languagebt, 207, 182);

    // ImageView: imageview13
#if !UI_SETTINGMODE_COMBINED_BG_ENABLE
    bk_ui->settingmode_imageview13 = lv_image_create(bk_ui->settingmode);
    ui_page_build_set_image_src(bk_ui->settingmode_imageview13, "/images/setting_mode_language.png");
    lv_obj_set_pos(bk_ui->settingmode_imageview13, 408, 120);
    lv_obj_set_size(bk_ui->settingmode_imageview13, 207, 182);
#endif

    // Button: backbt
    bk_ui->settingmode_backbt = lv_button_create(bk_ui->settingmode);
    lv_obj_add_flag(bk_ui->settingmode_backbt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->settingmode_backbt, settingmode_backbt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->settingmode_backbt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->settingmode_backbt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->settingmode_backbt, 0, 0);
    lv_obj_set_pos(bk_ui->settingmode_backbt, 825, 13);
    lv_obj_set_size(bk_ui->settingmode_backbt, 179, 74);

    // ImageView: imageview15
#if !UI_SETTINGMODE_COMBINED_BG_ENABLE
    bk_ui->settingmode_imageview15 = lv_image_create(bk_ui->settingmode);
    ui_page_build_set_image_src(bk_ui->settingmode_imageview15, "/images/exit_bt.png");
    lv_obj_set_pos(bk_ui->settingmode_imageview15, 825, 13);
    lv_obj_set_size(bk_ui->settingmode_imageview15, 179, 74);
#endif

}
