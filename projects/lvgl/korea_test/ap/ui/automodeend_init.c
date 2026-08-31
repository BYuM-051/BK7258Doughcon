#include "lvgl.h"
#include "beken_ui.h"
#include "custom_func.h"
#include "event_runtime.h"
#include "ui_lang.h"
#include <stdio.h>
#include <string.h>

#include "ui_config.h"
#include "preRenderer.h"

#define TAG "[automode_init.c] "
#include "preRenderer.h"
#define bk_printf(fmt, ...) do {if(0) printf(fmt, ##__VA_ARGS__); } while(0) // disable printf
extern bk_lv_ui_t bk_lv_tool_ui;
extern lv_obj_t *preRenderRoot;

extern void automodeend_stopbt_event_cb(lv_event_t *e);
extern void automodeend_load_start_event_cb(lv_event_t *e);
extern void automodeend_loaded_event_cb(lv_event_t *e);
extern void automodeend_unload_start_event_cb(lv_event_t *e);
extern void automodeend_unloaded_event_cb(lv_event_t *e);

void destroy_page_automodeend(bk_lv_ui_t *bk_ui)
{
    if (bk_ui == NULL) {
        return;
    }
    if (bk_ui->automodeend != NULL) {
        lv_obj_del(bk_ui->automodeend);
        bk_ui->automodeend = NULL;
    }
}

void init_page_automodeend(bk_lv_ui_t * bk_ui) {
    if (bk_ui->automodeend != NULL && lv_obj_is_valid(bk_ui->automodeend)) {
        destroy_page_automodeend(bk_ui);
    }

    ui_lang_reset_automodeend_cache();

#if UI_PRENDERING_ENABLE
    bk_ui->automodeend = lv_obj_create(preRenderRoot);
    lv_obj_remove_style_all(bk_ui->automodeend);
    lv_obj_set_size(bk_ui->automodeend, 1024, 600);
    lv_obj_set_pos(bk_ui->automodeend, 0, 0);
    lv_obj_set_style_radius(bk_ui->automodeend, 0, LV_PART_MAIN);
    lv_obj_set_scrollbar_mode(bk_ui->automodeend, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_event_cb(bk_ui->automodeend, automodeend_load_start_event_cb, UI_EVENT_PAGE_SHOW_START, NULL);
    lv_obj_add_event_cb(bk_ui->automodeend, automodeend_loaded_event_cb, UI_EVENT_PAGE_SHOWN,     NULL);
    lv_obj_add_event_cb(bk_ui->automodeend, automodeend_unload_start_event_cb, UI_EVENT_PAGE_HIDE_START, NULL);
    lv_obj_add_event_cb(bk_ui->automodeend, automodeend_unloaded_event_cb, UI_EVENT_PAGE_HIDDEN,     NULL);

    lv_obj_set_style_bg_color(bk_ui->automodeend, lv_color_hex(0xD9D9D9), 0);
    lv_obj_set_style_bg_opa(bk_ui->automodeend, LV_OPA_COVER, 0);

#else
    bk_ui->automodeend = lv_obj_create(NULL);
    lv_obj_set_size(bk_ui->automodeend, 1024, 600);
    lv_obj_set_scrollbar_mode(bk_ui->automodeend, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_event_cb(bk_ui->automodeend, automodeend_load_start_event_cb, LV_EVENT_SCREEN_LOAD_START, NULL);
    lv_obj_add_event_cb(bk_ui->automodeend, automodeend_loaded_event_cb, LV_EVENT_SCREEN_LOADED,     NULL);
    lv_obj_add_event_cb(bk_ui->automodeend, automodeend_unload_start_event_cb, LV_EVENT_SCREEN_UNLOAD_START, NULL);
    lv_obj_add_event_cb(bk_ui->automodeend, automodeend_unloaded_event_cb, LV_EVENT_SCREEN_UNLOADED,     NULL);
#endif /* UI_PRENDERING_ENABLE */

    // ImageView: auto_mode_end_bg
    bk_ui->automodeend_auto_mode_end_bg = lv_image_create(bk_ui->automodeend);
    // _img_set_src_timed(bk_ui->automodeend_auto_mode_end_bg, "/images/auto_mode_end_bgi.jpg");
    lv_obj_set_pos(bk_ui->automodeend_auto_mode_end_bg, 0, 0);
    lv_obj_set_size(bk_ui->automodeend_auto_mode_end_bg, 1024, 540);


    // TextView: auto_mode_end_overtime
    bk_ui->automodeend_auto_mode_end_overtime = lv_label_create(bk_ui->automodeend);
    lv_label_set_text(bk_ui->automodeend_auto_mode_end_overtime, "");
    lv_obj_set_style_bg_opa(bk_ui->automodeend_auto_mode_end_overtime, 0, 0);
    lv_obj_set_style_text_color(bk_ui->automodeend_auto_mode_end_overtime, lv_color_hex(0x000000), 0);
    lv_obj_set_style_text_font(bk_ui->automodeend_auto_mode_end_overtime, &lv_font_scdream_regular_32, 0);


    // lv_obj_set_style_text_font(bk_ui->automodeend_auto_mode_end_overtime, &lv_font_scdream_bold_32, 0);

    lv_obj_set_style_text_align(bk_ui->automodeend_auto_mode_end_overtime, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_set_pos(bk_ui->automodeend_auto_mode_end_overtime, 277, 470+5+5);
    lv_obj_set_size(bk_ui->automodeend_auto_mode_end_overtime, 40, 40);

    // ImageView: title
    bk_ui->automodeend_title = lv_image_create(bk_ui->automodeend);
    _img_set_src_timed(bk_ui->automodeend_title, "/images/automode_title.png");
    lv_obj_set_pos(bk_ui->automodeend_title, 0, 10);
    lv_obj_set_size(bk_ui->automodeend_title, 380, 80);
    lv_image_set_inner_align(bk_ui->automodeend_title, LV_IMAGE_ALIGN_TOP_LEFT);

    // Button: stopbt
    bk_ui->automodeend_stopbt = lv_button_create(bk_ui->automodeend);
    lv_obj_add_flag(bk_ui->automodeend_stopbt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->automodeend_stopbt, automodeend_stopbt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->automodeend_stopbt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->automodeend_stopbt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->automodeend_stopbt, 0, 0);
    lv_obj_set_pos(bk_ui->automodeend_stopbt, 847, 445);
    lv_obj_set_size(bk_ui->automodeend_stopbt, 164, 74);

    // ImageView: imageview5
    bk_ui->automodeend_imageview5 = lv_image_create(bk_ui->automodeend);
    _img_set_src_timed(bk_ui->automodeend_imageview5, "/images/stop_bt.png");
    lv_obj_set_pos(bk_ui->automodeend_imageview5, 847, 445);
    lv_obj_set_size(bk_ui->automodeend_imageview5, 164, 74);

    // TextView: AutoModeCompleteYear
    bk_ui->automodeend_AutoModeCompleteYear = lv_label_create(bk_ui->automodeend);
    lv_label_set_text(bk_ui->automodeend_AutoModeCompleteYear, "");
    lv_obj_set_style_bg_opa(bk_ui->automodeend_AutoModeCompleteYear, 0, 0);
    lv_obj_set_style_text_color(bk_ui->automodeend_AutoModeCompleteYear, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->automodeend_AutoModeCompleteYear, &lv_font_scdream_regular_40, 0);
    lv_obj_set_pos(bk_ui->automodeend_AutoModeCompleteYear, 419, 356+4);
    lv_obj_set_size(bk_ui->automodeend_AutoModeCompleteYear, 100, 64);

    // TextView: AutoModeCompleteMonth
    bk_ui->automodeend_AutoModeCompleteMonth = lv_label_create(bk_ui->automodeend);
    lv_label_set_text(bk_ui->automodeend_AutoModeCompleteMonth, "");
    lv_obj_set_style_bg_opa(bk_ui->automodeend_AutoModeCompleteMonth, 0, 0);
    lv_obj_set_style_text_color(bk_ui->automodeend_AutoModeCompleteMonth, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->automodeend_AutoModeCompleteMonth, &lv_font_scdream_regular_40, 0);
    lv_obj_set_pos(bk_ui->automodeend_AutoModeCompleteMonth, 535, 356+4);
    lv_obj_set_size(bk_ui->automodeend_AutoModeCompleteMonth, 60, 64);

    // TextView: AutoModeCompleteDay
    bk_ui->automodeend_AutoModeCompleteDay = lv_label_create(bk_ui->automodeend);
    lv_label_set_text(bk_ui->automodeend_AutoModeCompleteDay, "");
    lv_obj_set_style_bg_opa(bk_ui->automodeend_AutoModeCompleteDay, 0, 0);
    lv_obj_set_style_text_color(bk_ui->automodeend_AutoModeCompleteDay, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->automodeend_AutoModeCompleteDay, &lv_font_scdream_regular_40, 0);
    lv_obj_set_pos(bk_ui->automodeend_AutoModeCompleteDay, 603, 356+4);
    lv_obj_set_size(bk_ui->automodeend_AutoModeCompleteDay, 60, 64);

    // TextView: AutoModeCompleteHour
    bk_ui->automodeend_AutoModeCompleteHour = lv_label_create(bk_ui->automodeend);
    lv_label_set_text(bk_ui->automodeend_AutoModeCompleteHour, "");
    lv_obj_set_style_bg_opa(bk_ui->automodeend_AutoModeCompleteHour, 0, 0);
    lv_obj_set_style_text_color(bk_ui->automodeend_AutoModeCompleteHour, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->automodeend_AutoModeCompleteHour, &lv_font_scdream_regular_40, 0);
    lv_obj_set_pos(bk_ui->automodeend_AutoModeCompleteHour, 687, 356+4);
    lv_obj_set_size(bk_ui->automodeend_AutoModeCompleteHour, 60, 64);

    // TextView: AutoModeCompleteMin
    bk_ui->automodeend_AutoModeCompleteMin = lv_label_create(bk_ui->automodeend);
    lv_label_set_text(bk_ui->automodeend_AutoModeCompleteMin, "");
    lv_obj_set_style_bg_opa(bk_ui->automodeend_AutoModeCompleteMin, 0, 0);
    lv_obj_set_style_text_color(bk_ui->automodeend_AutoModeCompleteMin, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->automodeend_AutoModeCompleteMin, &lv_font_scdream_regular_40, 0);
    lv_obj_set_pos(bk_ui->automodeend_AutoModeCompleteMin, 751, 356+4);
    lv_obj_set_size(bk_ui->automodeend_AutoModeCompleteMin, 60, 64);

    // ImageView: blackout
    bk_ui->automodeend_blackout = lv_image_create(bk_ui->automodeend);
    _img_set_src_deferred(bk_ui->automodeend_blackout, "/images/blackout.png");
    lv_obj_add_flag(bk_ui->automodeend_blackout, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_pos(bk_ui->automodeend_blackout, 841, 384);
    // lv_obj_set_size(bk_ui->automodeend_blackout, 0, 0);

}

rendererFuncStatus_t init_page_automodeend_with_step(bk_lv_ui_t *bk_ui)
{
    return RENDERER_FUNC_FAILED;
}