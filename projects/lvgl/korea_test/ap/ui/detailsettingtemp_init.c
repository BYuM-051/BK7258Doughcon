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
extern void detailsettingtemp_backbt_event_cb(lv_event_t *e);
extern void detailsettingtemp_settingbt1_event_cb(lv_event_t *e);
extern void detailsettingtemp_settingbt2_event_cb(lv_event_t *e);
extern void detailsettingtemp_settingbt3_event_cb(lv_event_t *e);
extern void detailsettingtemp_settingbt4_event_cb(lv_event_t *e);
extern void detailsettingtemp_leftbt_event_cb(lv_event_t *e);
extern void detailsettingtemp_rightbt_event_cb(lv_event_t *e);
extern void detailsettingtemp_changebt_event_cb(lv_event_t *e);
extern void detailsettingtemp_roller_event_cb(lv_event_t *e);
extern void detailsettingtemp_load_start_event_cb(lv_event_t *e);
extern void detailsettingtemp_loaded_event_cb(lv_event_t *e);
extern void detailsettingtemp_unload_start_event_cb(lv_event_t *e);
extern void detailsettingtemp_unloaded_event_cb(lv_event_t *e);

void destroy_page_detailsettingtemp(bk_lv_ui_t *bk_ui)
{
    if (bk_ui == NULL) {
        return;
    }
    if (bk_ui->detailsettingtemp != NULL) {
        lv_obj_del(bk_ui->detailsettingtemp);
        bk_ui->detailsettingtemp = NULL;
    }
}

void init_page_detailsettingtemp(bk_lv_ui_t * bk_ui) {
    if (bk_ui->detailsettingtemp != NULL && lv_obj_is_valid(bk_ui->detailsettingtemp)) {
        destroy_page_detailsettingtemp(bk_ui);
    }

    ui_lang_reset_detailsettingtemp_cache();

#if UI_PRENDERING_ENABLE
    bk_ui->detailsettingtemp = lv_obj_create(preRenderRoot);
    lv_obj_remove_style_all(bk_ui->detailsettingtemp);
    lv_obj_set_size(bk_ui->detailsettingtemp, 1024, 600);
    lv_obj_set_pos(bk_ui->detailsettingtemp, 0, 0);
    lv_obj_set_style_radius(bk_ui->detailsettingtemp, 0, LV_PART_MAIN);
    lv_obj_set_scrollbar_mode(bk_ui->detailsettingtemp, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_event_cb(bk_ui->detailsettingtemp, detailsettingtemp_load_start_event_cb, UI_EVENT_PAGE_SHOW_START, NULL);
    lv_obj_add_event_cb(bk_ui->detailsettingtemp, detailsettingtemp_loaded_event_cb, UI_EVENT_PAGE_SHOWN,     NULL);
    lv_obj_add_event_cb(bk_ui->detailsettingtemp, detailsettingtemp_unload_start_event_cb, UI_EVENT_PAGE_HIDE_START, NULL);
    lv_obj_add_event_cb(bk_ui->detailsettingtemp, detailsettingtemp_unloaded_event_cb, UI_EVENT_PAGE_HIDDEN,     NULL);
#else
    bk_ui->detailsettingtemp = lv_obj_create(NULL);
    lv_obj_set_size(bk_ui->detailsettingtemp, 1024, 600);
    lv_obj_set_scrollbar_mode(bk_ui->detailsettingtemp, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_event_cb(bk_ui->detailsettingtemp, detailsettingtemp_load_start_event_cb, LV_EVENT_SCREEN_LOAD_START, NULL);
    lv_obj_add_event_cb(bk_ui->detailsettingtemp, detailsettingtemp_loaded_event_cb, LV_EVENT_SCREEN_LOADED,     NULL);
    lv_obj_add_event_cb(bk_ui->detailsettingtemp, detailsettingtemp_unload_start_event_cb, LV_EVENT_SCREEN_UNLOAD_START, NULL);
    lv_obj_add_event_cb(bk_ui->detailsettingtemp, detailsettingtemp_unloaded_event_cb, LV_EVENT_SCREEN_UNLOADED,     NULL);
#endif /* UI_PRENDERING_ENABLE */
    bk_ui->detailsettingtemp_bg = lv_image_create(bk_ui->detailsettingtemp);
    lv_obj_add_flag(bk_ui->detailsettingtemp_bg, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_style_bg_color(bk_ui->detailsettingtemp, lv_color_hex(0xd9d9d9), 0);
    lv_obj_set_style_bg_opa(bk_ui->detailsettingtemp, LV_OPA_COVER, 0);
    lv_obj_set_pos(bk_ui->detailsettingtemp_bg, 0, 0);

    // ImageView: title
    bk_ui->detailsettingtemp_title = lv_image_create(bk_ui->detailsettingtemp);
    lv_obj_set_pos(bk_ui->detailsettingtemp_title, 0, 10);
    lv_obj_set_size(bk_ui->detailsettingtemp_title, 380, 80);
    lv_image_set_inner_align(bk_ui->detailsettingtemp_title, LV_IMAGE_ALIGN_TOP_LEFT);

    // Button: backbt
    bk_ui->detailsettingtemp_backbt = lv_button_create(bk_ui->detailsettingtemp);
    lv_obj_add_flag(bk_ui->detailsettingtemp_backbt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->detailsettingtemp_backbt, detailsettingtemp_backbt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->detailsettingtemp_backbt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->detailsettingtemp_backbt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->detailsettingtemp_backbt, 0, 0);
    lv_obj_set_pos(bk_ui->detailsettingtemp_backbt, 825, 13);
    lv_obj_set_size(bk_ui->detailsettingtemp_backbt, 179, 74);

    // ImageView: imageview3
    bk_ui->detailsettingtemp_imageview3 = lv_image_create(bk_ui->detailsettingtemp);
    lv_obj_set_pos(bk_ui->detailsettingtemp_imageview3, 825, 13);
    lv_obj_set_size(bk_ui->detailsettingtemp_imageview3, 179, 74);

    // ImageView: settingim1
    bk_ui->detailsettingtemp_settingim1 = lv_image_create(bk_ui->detailsettingtemp);
    lv_obj_set_pos(bk_ui->detailsettingtemp_settingim1, 23, 120);
    lv_obj_set_size(bk_ui->detailsettingtemp_settingim1, 538, 66);

    // TextView: settingtxt1
    bk_ui->detailsettingtemp_settingtxt1 = lv_label_create(bk_ui->detailsettingtemp);
    lv_label_set_text(bk_ui->detailsettingtemp_settingtxt1, "");
    lv_obj_set_style_bg_opa(bk_ui->detailsettingtemp_settingtxt1, 0, 0);
    lv_obj_set_style_text_color(bk_ui->detailsettingtemp_settingtxt1, lv_color_hex(0xA6A6A6), 0);
    lv_obj_set_style_text_font(bk_ui->detailsettingtemp_settingtxt1, &lv_font_scdream_regular_32, 0);
    lv_obj_set_pos(bk_ui->detailsettingtemp_settingtxt1, 400, 135);
    lv_obj_set_size(bk_ui->detailsettingtemp_settingtxt1,70, 42);
    lv_obj_set_style_text_align(bk_ui->detailsettingtemp_settingtxt1, LV_TEXT_ALIGN_RIGHT, 0);

    // Button: settingbt1
    bk_ui->detailsettingtemp_settingbt1 = lv_button_create(bk_ui->detailsettingtemp);
    lv_obj_add_flag(bk_ui->detailsettingtemp_settingbt1, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->detailsettingtemp_settingbt1, detailsettingtemp_settingbt1_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->detailsettingtemp_settingbt1, 0, 0);
    lv_obj_set_style_border_width(bk_ui->detailsettingtemp_settingbt1, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->detailsettingtemp_settingbt1, 0, 0);
    lv_obj_set_pos(bk_ui->detailsettingtemp_settingbt1, 23, 120);
    lv_obj_set_size(bk_ui->detailsettingtemp_settingbt1, 538, 66);

    // ImageView: settingim2
    bk_ui->detailsettingtemp_settingim2 = lv_image_create(bk_ui->detailsettingtemp);
    lv_obj_set_pos(bk_ui->detailsettingtemp_settingim2, 23, 198);
    lv_obj_set_size(bk_ui->detailsettingtemp_settingim2, 538, 66);

    // TextView: settingtxt2
    bk_ui->detailsettingtemp_settingtxt2 = lv_label_create(bk_ui->detailsettingtemp);
    lv_label_set_text(bk_ui->detailsettingtemp_settingtxt2, "");
    lv_obj_set_style_bg_opa(bk_ui->detailsettingtemp_settingtxt2, 0, 0);
    lv_obj_set_style_text_color(bk_ui->detailsettingtemp_settingtxt2, lv_color_hex(0xA6A6A6), 0);
    lv_obj_set_style_text_font(bk_ui->detailsettingtemp_settingtxt2, &lv_font_scdream_regular_32, 0);
    lv_obj_set_pos(bk_ui->detailsettingtemp_settingtxt2, 400, 213);
    lv_obj_set_size(bk_ui->detailsettingtemp_settingtxt2, 70, 42);
    lv_obj_set_style_text_align(bk_ui->detailsettingtemp_settingtxt2, LV_TEXT_ALIGN_RIGHT, 0);

    // Button: settingbt2
    bk_ui->detailsettingtemp_settingbt2 = lv_button_create(bk_ui->detailsettingtemp);
    lv_obj_add_flag(bk_ui->detailsettingtemp_settingbt2, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->detailsettingtemp_settingbt2, detailsettingtemp_settingbt2_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->detailsettingtemp_settingbt2, 0, 0);
    lv_obj_set_style_border_width(bk_ui->detailsettingtemp_settingbt2, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->detailsettingtemp_settingbt2, 0, 0);
    lv_obj_set_pos(bk_ui->detailsettingtemp_settingbt2, 23, 198);
    lv_obj_set_size(bk_ui->detailsettingtemp_settingbt2, 538, 66);

    // ImageView: settingim3
    bk_ui->detailsettingtemp_settingim3 = lv_image_create(bk_ui->detailsettingtemp);
    lv_obj_set_pos(bk_ui->detailsettingtemp_settingim3, 23, 276);
    lv_obj_set_size(bk_ui->detailsettingtemp_settingim3, 538, 66);

    // TextView: settingtxt3
    bk_ui->detailsettingtemp_settingtxt3 = lv_label_create(bk_ui->detailsettingtemp);
    lv_label_set_text(bk_ui->detailsettingtemp_settingtxt3, "");
    lv_obj_set_style_bg_opa(bk_ui->detailsettingtemp_settingtxt3, 0, 0);
    lv_obj_set_style_text_color(bk_ui->detailsettingtemp_settingtxt3, lv_color_hex(0xA6A6A6), 0);
    lv_obj_set_style_text_font(bk_ui->detailsettingtemp_settingtxt3, &lv_font_scdream_regular_32, 0);
    lv_obj_set_pos(bk_ui->detailsettingtemp_settingtxt3, 400, 291);
    lv_obj_set_size(bk_ui->detailsettingtemp_settingtxt3, 70, 42);
    lv_obj_set_style_text_align(bk_ui->detailsettingtemp_settingtxt3, LV_TEXT_ALIGN_RIGHT, 0);

    // Button: settingbt3
    bk_ui->detailsettingtemp_settingbt3 = lv_button_create(bk_ui->detailsettingtemp);
    lv_obj_add_flag(bk_ui->detailsettingtemp_settingbt3, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->detailsettingtemp_settingbt3, detailsettingtemp_settingbt3_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->detailsettingtemp_settingbt3, 0, 0);
    lv_obj_set_style_border_width(bk_ui->detailsettingtemp_settingbt3, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->detailsettingtemp_settingbt3, 0, 0);
    lv_obj_set_pos(bk_ui->detailsettingtemp_settingbt3, 23, 276);
    lv_obj_set_size(bk_ui->detailsettingtemp_settingbt3, 538, 66);

    // ImageView: settingim4
    bk_ui->detailsettingtemp_settingim4 = lv_image_create(bk_ui->detailsettingtemp);
    lv_obj_set_pos(bk_ui->detailsettingtemp_settingim4, 23, 354);
    lv_obj_set_size(bk_ui->detailsettingtemp_settingim4, 538, 66);

    // TextView: settingtxt4
    bk_ui->detailsettingtemp_settingtxt4 = lv_label_create(bk_ui->detailsettingtemp);
    lv_label_set_text(bk_ui->detailsettingtemp_settingtxt4, "");
    lv_obj_set_style_bg_opa(bk_ui->detailsettingtemp_settingtxt4, 0, 0);
    lv_obj_set_style_text_color(bk_ui->detailsettingtemp_settingtxt4, lv_color_hex(0xA6A6A6), 0);
    lv_obj_set_style_text_font(bk_ui->detailsettingtemp_settingtxt4, &lv_font_scdream_regular_32, 0);
    lv_obj_set_pos(bk_ui->detailsettingtemp_settingtxt4, 400, 369);
    lv_obj_set_size(bk_ui->detailsettingtemp_settingtxt4, 70, 42);
    lv_obj_set_style_text_align(bk_ui->detailsettingtemp_settingtxt4, LV_TEXT_ALIGN_RIGHT, 0);

    // Button: settingbt4
    bk_ui->detailsettingtemp_settingbt4 = lv_button_create(bk_ui->detailsettingtemp);
    lv_obj_add_flag(bk_ui->detailsettingtemp_settingbt4, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->detailsettingtemp_settingbt4, detailsettingtemp_settingbt4_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->detailsettingtemp_settingbt4, 0, 0);
    lv_obj_set_style_border_width(bk_ui->detailsettingtemp_settingbt4, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->detailsettingtemp_settingbt4, 0, 0);
    lv_obj_set_pos(bk_ui->detailsettingtemp_settingbt4, 23, 354);
    lv_obj_set_size(bk_ui->detailsettingtemp_settingbt4, 538, 66);

    // ImageView: imageview16
    bk_ui->detailsettingtemp_imageview16 = lv_image_create(bk_ui->detailsettingtemp);
    _img_set_src_timed(bk_ui->detailsettingtemp_imageview16, "/images/left_bt.png");
    lv_obj_set_pos(bk_ui->detailsettingtemp_imageview16, 19, 429);
    lv_obj_set_size(bk_ui->detailsettingtemp_imageview16, 106, 72);

    // Button: leftbt
    bk_ui->detailsettingtemp_leftbt = lv_button_create(bk_ui->detailsettingtemp);
    lv_obj_add_flag(bk_ui->detailsettingtemp_leftbt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->detailsettingtemp_leftbt, detailsettingtemp_leftbt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->detailsettingtemp_leftbt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->detailsettingtemp_leftbt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->detailsettingtemp_leftbt, 0, 0);
    lv_obj_set_pos(bk_ui->detailsettingtemp_leftbt, 19, 429);
    lv_obj_set_size(bk_ui->detailsettingtemp_leftbt, 106, 72);

    // ImageView: currentpage (page indicator image)
    bk_ui->detailsettingtemp_currentpage = lv_image_create(bk_ui->detailsettingtemp);
    ui_lang_apply_next_bt(bk_ui->detailsettingtemp_currentpage, 1);
    lv_obj_set_pos(bk_ui->detailsettingtemp_currentpage, 167, 429);
    lv_obj_set_size(bk_ui->detailsettingtemp_currentpage, 250, 72);

    // ImageView: imageview19
    bk_ui->detailsettingtemp_imageview19 = lv_image_create(bk_ui->detailsettingtemp);
    _img_set_src_timed(bk_ui->detailsettingtemp_imageview19, "/images/right_bt.png");
    lv_obj_set_pos(bk_ui->detailsettingtemp_imageview19, 457, 429);
    lv_obj_set_size(bk_ui->detailsettingtemp_imageview19, 106, 72);

    // Button: rightbt
    bk_ui->detailsettingtemp_rightbt = lv_button_create(bk_ui->detailsettingtemp);
    lv_obj_add_flag(bk_ui->detailsettingtemp_rightbt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->detailsettingtemp_rightbt, detailsettingtemp_rightbt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->detailsettingtemp_rightbt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->detailsettingtemp_rightbt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->detailsettingtemp_rightbt, 0, 0);
    lv_obj_set_pos(bk_ui->detailsettingtemp_rightbt, 457, 429);
    lv_obj_set_size(bk_ui->detailsettingtemp_rightbt, 106, 72);

    // ImageView: pickerbox
    bk_ui->detailsettingtemp_pickerbox = lv_image_create(bk_ui->detailsettingtemp);
    ui_lang_apply_picker(bk_ui->detailsettingtemp_pickerbox, 1);
    lv_obj_set_pos(bk_ui->detailsettingtemp_pickerbox, 624, 120);
    lv_obj_set_size(bk_ui->detailsettingtemp_pickerbox, 376, 376);
    lv_obj_add_flag(bk_ui->detailsettingtemp_pickerbox, LV_OBJ_FLAG_HIDDEN);


    // NumberPicker: settemp_setn1 (lv_roller)
    bk_ui->detailsettingtemp_settemp_setn1 = lv_roller_create(bk_ui->detailsettingtemp);
    lv_roller_set_options(bk_ui->detailsettingtemp_settemp_setn1, "0", LV_ROLLER_MODE_NORMAL);
    lv_roller_set_visible_row_count(bk_ui->detailsettingtemp_settemp_setn1, 3);
    lv_obj_set_style_bg_opa(bk_ui->detailsettingtemp_settemp_setn1, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(bk_ui->detailsettingtemp_settemp_setn1, 0, LV_PART_MAIN);
    lv_obj_set_style_text_color(bk_ui->detailsettingtemp_settemp_setn1, lv_color_hex(0xCCCCCC), LV_PART_MAIN);
    lv_obj_set_style_text_font(bk_ui->detailsettingtemp_settemp_setn1, &lv_font_scdream_regular_72, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(bk_ui->detailsettingtemp_settemp_setn1, LV_OPA_TRANSP, LV_PART_SELECTED);
    lv_obj_set_style_text_color(bk_ui->detailsettingtemp_settemp_setn1, lv_color_hex(0x333333), LV_PART_SELECTED);
    lv_obj_set_style_text_font(bk_ui->detailsettingtemp_settemp_setn1, &lv_font_scdream_regular_90, LV_PART_SELECTED);
    lv_obj_add_event_cb(bk_ui->detailsettingtemp_settemp_setn1, detailsettingtemp_roller_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_add_flag(bk_ui->detailsettingtemp_settemp_setn1, LV_OBJ_FLAG_HIDDEN);
    // lv_obj_set_style_text_line_space(bk_ui->detailsettingtemp_settemp_setn1, 20, LV_PART_MAIN);
    lv_obj_set_pos(bk_ui->detailsettingtemp_settemp_setn1, 700-10, 136-5-2-1);
    lv_obj_set_width(bk_ui->detailsettingtemp_settemp_setn1, 240);

    // Button: changebt
    bk_ui->detailsettingtemp_changebt = lv_button_create(bk_ui->detailsettingtemp);
    lv_obj_add_flag(bk_ui->detailsettingtemp_changebt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->detailsettingtemp_changebt, detailsettingtemp_changebt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->detailsettingtemp_changebt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->detailsettingtemp_changebt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->detailsettingtemp_changebt, 0, 0);
    lv_obj_set_pos(bk_ui->detailsettingtemp_changebt, 590, 420);
    lv_obj_set_size(bk_ui->detailsettingtemp_changebt, 410, 80);

    ui_lang_apply_detailsettingtemp(bk_ui);
}

void init_page_detailsettingtemp_with_step(bk_lv_ui_t *bk_ui)
{
    return;
}