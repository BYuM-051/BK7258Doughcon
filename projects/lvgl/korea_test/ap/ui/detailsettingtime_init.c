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
extern void detailsettingtime_backbt_event_cb(lv_event_t *e);
extern void detailsettingtime_settingbt1_event_cb(lv_event_t *e);
extern void detailsettingtime_settingbt2_event_cb(lv_event_t *e);
extern void detailsettingtime_settingbt3_event_cb(lv_event_t *e);
extern void detailsettingtime_settingbt4_event_cb(lv_event_t *e);
extern void detailsettingtime_settingbt5_event_cb(lv_event_t *e);
extern void detailsettingtime_changebt_event_cb(lv_event_t *e);
extern void detailsettingtime_roller_event_cb(lv_event_t *e);
extern void detailsettingtime_load_start_event_cb(lv_event_t *e);
extern void detailsettingtime_loaded_event_cb(lv_event_t *e);
extern void detailsettingtime_unload_start_event_cb(lv_event_t *e);
extern void detailsettingtime_unloaded_event_cb(lv_event_t *e);

void destroy_page_detailsettingtime(bk_lv_ui_t *bk_ui)
{
    if (bk_ui == NULL) {
        return;
    }
    if (bk_ui->detailsettingtime != NULL) {
        lv_obj_del(bk_ui->detailsettingtime);
        bk_ui->detailsettingtime = NULL;
    }
}

void init_page_detailsettingtime(bk_lv_ui_t * bk_ui) {
    if (bk_ui->detailsettingtime != NULL && lv_obj_is_valid(bk_ui->detailsettingtime)) {
        destroy_page_detailsettingtime(bk_ui);
    }

    ui_lang_reset_detailsettingtime_cache();

#if UI_PRENDERING_ENABLE
    bk_ui->detailsettingtime = lv_obj_create(preRenderRoot);
    lv_obj_remove_style_all(bk_ui->detailsettingtime);
    lv_obj_set_size(bk_ui->detailsettingtime, 1024, 600);
    lv_obj_set_pos(bk_ui->detailsettingtime, 0, 0);
    lv_obj_set_style_radius(bk_ui->detailsettingtime, 0, LV_PART_MAIN);
    lv_obj_set_scrollbar_mode(bk_ui->detailsettingtime, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_event_cb(bk_ui->detailsettingtime, detailsettingtime_load_start_event_cb, UI_EVENT_PAGE_SHOW_START, NULL);
    lv_obj_add_event_cb(bk_ui->detailsettingtime, detailsettingtime_loaded_event_cb, UI_EVENT_PAGE_SHOWN,     NULL);
    lv_obj_add_event_cb(bk_ui->detailsettingtime, detailsettingtime_unload_start_event_cb, UI_EVENT_PAGE_HIDE_START, NULL);
    lv_obj_add_event_cb(bk_ui->detailsettingtime, detailsettingtime_unloaded_event_cb, UI_EVENT_PAGE_HIDDEN,     NULL);
#else
    bk_ui->detailsettingtime = lv_obj_create(NULL);
    lv_obj_set_size(bk_ui->detailsettingtime, 1024, 600);
    lv_obj_set_scrollbar_mode(bk_ui->detailsettingtime, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_event_cb(bk_ui->detailsettingtime, detailsettingtime_load_start_event_cb, LV_EVENT_SCREEN_LOAD_START, NULL);
    lv_obj_add_event_cb(bk_ui->detailsettingtime, detailsettingtime_loaded_event_cb, LV_EVENT_SCREEN_LOADED,     NULL);
    lv_obj_add_event_cb(bk_ui->detailsettingtime, detailsettingtime_unload_start_event_cb, LV_EVENT_SCREEN_UNLOAD_START, NULL);
    lv_obj_add_event_cb(bk_ui->detailsettingtime, detailsettingtime_unloaded_event_cb, LV_EVENT_SCREEN_UNLOADED,     NULL);
#endif /* UI_PRENDERING_ENABLE */
    bk_ui->detailsettingtime_bg = lv_image_create(bk_ui->detailsettingtime);
    lv_obj_add_flag(bk_ui->detailsettingtime_bg, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_style_bg_color(bk_ui->detailsettingtime, lv_color_hex(0xd9d9d9), 0);
    lv_obj_set_style_bg_opa(bk_ui->detailsettingtime, LV_OPA_COVER, 0);
    lv_obj_set_pos(bk_ui->detailsettingtime_bg, 0, 0);

    // ImageView: title
    bk_ui->detailsettingtime_title = lv_image_create(bk_ui->detailsettingtime);
    lv_obj_set_pos(bk_ui->detailsettingtime_title, 0, 10);
    lv_obj_set_size(bk_ui->detailsettingtime_title, 380, 80);
    lv_image_set_inner_align(bk_ui->detailsettingtime_title, LV_IMAGE_ALIGN_TOP_LEFT);

    // Button: backbt
    bk_ui->detailsettingtime_backbt = lv_button_create(bk_ui->detailsettingtime);
    lv_obj_add_flag(bk_ui->detailsettingtime_backbt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->detailsettingtime_backbt, detailsettingtime_backbt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->detailsettingtime_backbt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->detailsettingtime_backbt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->detailsettingtime_backbt, 0, 0);
    lv_obj_set_pos(bk_ui->detailsettingtime_backbt, 825, 13);
    lv_obj_set_size(bk_ui->detailsettingtime_backbt, 179, 74);

    // ImageView: imageview3
    bk_ui->detailsettingtime_imageview3 = lv_image_create(bk_ui->detailsettingtime);
    lv_obj_set_pos(bk_ui->detailsettingtime_imageview3, 825, 13);
    lv_obj_set_size(bk_ui->detailsettingtime_imageview3, 179, 74);

    // ImageView: settingim1
    bk_ui->detailsettingtime_settingim1 = lv_image_create(bk_ui->detailsettingtime);
    lv_obj_set_pos(bk_ui->detailsettingtime_settingim1, 23, 120);
    lv_obj_set_size(bk_ui->detailsettingtime_settingim1, 538, 66);

    // TextView: settingtxt1_1
    bk_ui->detailsettingtime_settingtxt1_1 = lv_label_create(bk_ui->detailsettingtime);
    lv_label_set_text(bk_ui->detailsettingtime_settingtxt1_1, "");
    lv_obj_set_style_bg_opa(bk_ui->detailsettingtime_settingtxt1_1, 0, 0);
    lv_obj_set_style_text_color(bk_ui->detailsettingtime_settingtxt1_1, lv_color_hex(0xA6A6A6), 0);
    lv_obj_set_style_text_font(bk_ui->detailsettingtime_settingtxt1_1, &lv_font_scdream_regular_32, 0);
    lv_obj_set_pos(bk_ui->detailsettingtime_settingtxt1_1, 355, 135);
    lv_obj_set_size(bk_ui->detailsettingtime_settingtxt1_1, 50, 42);
    lv_obj_set_style_text_align(bk_ui->detailsettingtime_settingtxt1_1, LV_TEXT_ALIGN_RIGHT, 0);

    // TextView: settingtxt1_2
    bk_ui->detailsettingtime_settingtxt1_2 = lv_label_create(bk_ui->detailsettingtime);
    lv_label_set_text(bk_ui->detailsettingtime_settingtxt1_2, "");
    lv_obj_set_style_bg_opa(bk_ui->detailsettingtime_settingtxt1_2, 0, 0);
    lv_obj_set_style_text_color(bk_ui->detailsettingtime_settingtxt1_2, lv_color_hex(0xA6A6A6), 0);
    lv_obj_set_style_text_font(bk_ui->detailsettingtime_settingtxt1_2, &lv_font_scdream_regular_32, 0);
    lv_obj_set_pos(bk_ui->detailsettingtime_settingtxt1_2, 420, 135);
    lv_obj_set_size(bk_ui->detailsettingtime_settingtxt1_2, 50, 42);
    lv_obj_set_style_text_align(bk_ui->detailsettingtime_settingtxt1_2, LV_TEXT_ALIGN_RIGHT, 0);

    // Button: settingbt1
    bk_ui->detailsettingtime_settingbt1 = lv_button_create(bk_ui->detailsettingtime);
    lv_obj_add_flag(bk_ui->detailsettingtime_settingbt1, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->detailsettingtime_settingbt1, detailsettingtime_settingbt1_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->detailsettingtime_settingbt1, 0, 0);
    lv_obj_set_style_border_width(bk_ui->detailsettingtime_settingbt1, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->detailsettingtime_settingbt1, 0, 0);
    lv_obj_set_pos(bk_ui->detailsettingtime_settingbt1, 20, 120);
    lv_obj_set_size(bk_ui->detailsettingtime_settingbt1, 538, 66);

    // ImageView: settingim2
    bk_ui->detailsettingtime_settingim2 = lv_image_create(bk_ui->detailsettingtime);
    lv_obj_set_pos(bk_ui->detailsettingtime_settingim2, 23, 198);
    lv_obj_set_size(bk_ui->detailsettingtime_settingim2, 538, 66);

    // TextView: settingtxt2_1
    bk_ui->detailsettingtime_settingtxt2_1 = lv_label_create(bk_ui->detailsettingtime);
    lv_label_set_text(bk_ui->detailsettingtime_settingtxt2_1, "");
    lv_obj_set_style_bg_opa(bk_ui->detailsettingtime_settingtxt2_1, 0, 0);
    lv_obj_set_style_text_color(bk_ui->detailsettingtime_settingtxt2_1, lv_color_hex(0xA6A6A6), 0);
    lv_obj_set_style_text_font(bk_ui->detailsettingtime_settingtxt2_1, &lv_font_scdream_regular_32, 0);
    lv_obj_set_pos(bk_ui->detailsettingtime_settingtxt2_1, 310, 213);
    lv_obj_set_size(bk_ui->detailsettingtime_settingtxt2_1, 50, 42);
    lv_obj_set_style_text_align(bk_ui->detailsettingtime_settingtxt2_1, LV_TEXT_ALIGN_RIGHT, 0);

    // TextView: settingtxt2_2
    bk_ui->detailsettingtime_settingtxt2_2 = lv_label_create(bk_ui->detailsettingtime);
    lv_label_set_text(bk_ui->detailsettingtime_settingtxt2_2, "");
    lv_obj_set_style_bg_opa(bk_ui->detailsettingtime_settingtxt2_2, 0, 0);
    lv_obj_set_style_text_color(bk_ui->detailsettingtime_settingtxt2_2, lv_color_hex(0xA6A6A6), 0);
    lv_obj_set_style_text_font(bk_ui->detailsettingtime_settingtxt2_2, &lv_font_scdream_regular_32, 0);
    lv_obj_set_pos(bk_ui->detailsettingtime_settingtxt2_2, 420, 213);
    lv_obj_set_size(bk_ui->detailsettingtime_settingtxt2_2, 50, 42);
    lv_obj_set_style_text_align(bk_ui->detailsettingtime_settingtxt2_2, LV_TEXT_ALIGN_RIGHT, 0);

    // Button: settingbt2
    bk_ui->detailsettingtime_settingbt2 = lv_button_create(bk_ui->detailsettingtime);
    lv_obj_add_flag(bk_ui->detailsettingtime_settingbt2, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->detailsettingtime_settingbt2, detailsettingtime_settingbt2_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->detailsettingtime_settingbt2, 0, 0);
    lv_obj_set_style_border_width(bk_ui->detailsettingtime_settingbt2, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->detailsettingtime_settingbt2, 0, 0);
    lv_obj_set_pos(bk_ui->detailsettingtime_settingbt2, 20, 198);
    lv_obj_set_size(bk_ui->detailsettingtime_settingbt2, 538, 66);

    // ImageView: settingim3
    bk_ui->detailsettingtime_settingim3 = lv_image_create(bk_ui->detailsettingtime);
    lv_obj_set_pos(bk_ui->detailsettingtime_settingim3, 23, 276);
    lv_obj_set_size(bk_ui->detailsettingtime_settingim3, 538, 66);

    // TextView: settingtxt3
    bk_ui->detailsettingtime_settingtxt3 = lv_label_create(bk_ui->detailsettingtime);
    lv_label_set_text(bk_ui->detailsettingtime_settingtxt3, "");
    lv_obj_set_style_bg_opa(bk_ui->detailsettingtime_settingtxt3, 0, 0);
    lv_obj_set_style_text_color(bk_ui->detailsettingtime_settingtxt3, lv_color_hex(0xA6A6A6), 0);
    lv_obj_set_style_text_font(bk_ui->detailsettingtime_settingtxt3, &lv_font_scdream_regular_32, 0);
    lv_obj_set_pos(bk_ui->detailsettingtime_settingtxt3, 410, 291);
    lv_obj_set_size(bk_ui->detailsettingtime_settingtxt3, 60, 42);
    lv_obj_set_style_text_align(bk_ui->detailsettingtime_settingtxt3, LV_TEXT_ALIGN_RIGHT, 0);

    // Button: settingbt3
    bk_ui->detailsettingtime_settingbt3 = lv_button_create(bk_ui->detailsettingtime);
    lv_obj_add_flag(bk_ui->detailsettingtime_settingbt3, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->detailsettingtime_settingbt3, detailsettingtime_settingbt3_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->detailsettingtime_settingbt3, 0, 0);
    lv_obj_set_style_border_width(bk_ui->detailsettingtime_settingbt3, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->detailsettingtime_settingbt3, 0, 0);
    lv_obj_set_pos(bk_ui->detailsettingtime_settingbt3, 20, 276);
    lv_obj_set_size(bk_ui->detailsettingtime_settingbt3, 538, 66);

    // ImageView: settingim4
    bk_ui->detailsettingtime_settingim4 = lv_image_create(bk_ui->detailsettingtime);
    lv_obj_set_pos(bk_ui->detailsettingtime_settingim4, 23, 354);
    lv_obj_set_size(bk_ui->detailsettingtime_settingim4, 538, 66);

    // TextView: settingtxt4
    bk_ui->detailsettingtime_settingtxt4 = lv_label_create(bk_ui->detailsettingtime);
    lv_label_set_text(bk_ui->detailsettingtime_settingtxt4, "");
    lv_obj_set_style_bg_opa(bk_ui->detailsettingtime_settingtxt4, 0, 0);
    lv_obj_set_style_text_color(bk_ui->detailsettingtime_settingtxt4, lv_color_hex(0xA6A6A6), 0);
    lv_obj_set_style_text_font(bk_ui->detailsettingtime_settingtxt4, &lv_font_scdream_regular_32, 0);
    /* "OFF" 폭(~58px)이 박스 폭(60px)에 거의 딱 맞아 우측 정렬 시 시작부분(좌측) 몇
     * 픽셀이 잘려 보이던 문제 — 박스를 우측 고정, 좌측으로 확장 */
    lv_label_set_long_mode(bk_ui->detailsettingtime_settingtxt4, LV_LABEL_LONG_CLIP);
    lv_obj_set_pos(bk_ui->detailsettingtime_settingtxt4, 410-20, 369);
    lv_obj_set_size(bk_ui->detailsettingtime_settingtxt4, 80, 42);
    lv_obj_set_style_text_align(bk_ui->detailsettingtime_settingtxt4, LV_TEXT_ALIGN_RIGHT, 0);

    // Button: settingbt4
    bk_ui->detailsettingtime_settingbt4 = lv_button_create(bk_ui->detailsettingtime);
    lv_obj_add_flag(bk_ui->detailsettingtime_settingbt4, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->detailsettingtime_settingbt4, detailsettingtime_settingbt4_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->detailsettingtime_settingbt4, 0, 0);
    lv_obj_set_style_border_width(bk_ui->detailsettingtime_settingbt4, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->detailsettingtime_settingbt4, 0, 0);
    lv_obj_set_pos(bk_ui->detailsettingtime_settingbt4, 23, 354);
    lv_obj_set_size(bk_ui->detailsettingtime_settingbt4, 538, 66);

    // ImageView: settingim5
    bk_ui->detailsettingtime_settingim5 = lv_image_create(bk_ui->detailsettingtime);
    lv_obj_set_pos(bk_ui->detailsettingtime_settingim5, 23, 432);
    lv_obj_set_size(bk_ui->detailsettingtime_settingim5, 538, 66);

    // TextView: settingtxt5
    bk_ui->detailsettingtime_settingtxt5 = lv_label_create(bk_ui->detailsettingtime);
    lv_label_set_text(bk_ui->detailsettingtime_settingtxt5, "");
    lv_obj_set_style_bg_opa(bk_ui->detailsettingtime_settingtxt5, 0, 0);
    lv_obj_set_style_text_color(bk_ui->detailsettingtime_settingtxt5, lv_color_hex(0xA6A6A6), 0);
    lv_obj_set_style_text_font(bk_ui->detailsettingtime_settingtxt5, &lv_font_scdream_regular_32, 0);
    lv_obj_set_pos(bk_ui->detailsettingtime_settingtxt5, 420, 447);
    lv_obj_set_size(bk_ui->detailsettingtime_settingtxt5, 50, 42);
    lv_obj_set_style_text_align(bk_ui->detailsettingtime_settingtxt5, LV_TEXT_ALIGN_RIGHT, 0);

    // Button: settingbt5
    bk_ui->detailsettingtime_settingbt5 = lv_button_create(bk_ui->detailsettingtime);
    lv_obj_add_flag(bk_ui->detailsettingtime_settingbt5, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->detailsettingtime_settingbt5, detailsettingtime_settingbt5_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->detailsettingtime_settingbt5, 0, 0);
    lv_obj_set_style_border_width(bk_ui->detailsettingtime_settingbt5, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->detailsettingtime_settingbt5, 0, 0);
    lv_obj_set_pos(bk_ui->detailsettingtime_settingbt5, 23, 432);
    lv_obj_set_size(bk_ui->detailsettingtime_settingbt5, 538, 66);

    // ImageView: pickerbox (src changed dynamically: picker_1/2.png)
    bk_ui->detailsettingtime_pickerbox = lv_image_create(bk_ui->detailsettingtime);
    ui_lang_apply_picker(bk_ui->detailsettingtime_pickerbox, 3);
    lv_obj_set_pos(bk_ui->detailsettingtime_pickerbox, 624, 120);
    lv_obj_set_size(bk_ui->detailsettingtime_pickerbox, 376, 376);
    lv_obj_add_flag(bk_ui->detailsettingtime_pickerbox, LV_OBJ_FLAG_HIDDEN);


    // ImageView: pickerbox2 (for bt2 4-roller WaterInterval)
    bk_ui->detailsettingtime_pickerbox2 = lv_image_create(bk_ui->detailsettingtime);
    ui_lang_apply_picker(bk_ui->detailsettingtime_pickerbox2, 4);
    lv_obj_add_flag(bk_ui->detailsettingtime_pickerbox2, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_pos(bk_ui->detailsettingtime_pickerbox2, 590, 120);
    lv_obj_set_size(bk_ui->detailsettingtime_pickerbox2, 410, 376);

#define _TIME_ROLLER_STYLE(obj) \
    lv_obj_set_style_bg_opa((obj), LV_OPA_TRANSP, LV_PART_MAIN); \
    lv_obj_set_style_border_width((obj), 0, LV_PART_MAIN); \
    lv_obj_set_style_text_color((obj), lv_color_hex(0xCCCCCC), LV_PART_MAIN); \
    lv_obj_set_style_text_font((obj), &lv_font_scdream_regular_72, LV_PART_MAIN); \
    lv_obj_set_style_bg_opa((obj), LV_OPA_TRANSP, LV_PART_SELECTED); \
    lv_obj_set_style_text_color((obj), lv_color_hex(0x333333), LV_PART_SELECTED); \
    lv_obj_set_style_text_font((obj), &lv_font_scdream_regular_90, LV_PART_SELECTED)

    // setn1: left roller in 3-roller groups (bt0=tens-sec, bt2=hundreds-heater)
    bk_ui->detailsettingtime_settemp_setn1 = lv_roller_create(bk_ui->detailsettingtime);
    lv_roller_set_options(bk_ui->detailsettingtime_settemp_setn1, "0", LV_ROLLER_MODE_NORMAL);
    _TIME_ROLLER_STYLE(bk_ui->detailsettingtime_settemp_setn1);
    lv_roller_set_visible_row_count(bk_ui->detailsettingtime_settemp_setn1, 3);
    lv_obj_add_event_cb(bk_ui->detailsettingtime_settemp_setn1, detailsettingtime_roller_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_add_flag(bk_ui->detailsettingtime_settemp_setn1, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_pos(bk_ui->detailsettingtime_settemp_setn1, 652-1, 136-2-1-5);
    lv_obj_set_width(bk_ui->detailsettingtime_settemp_setn1, 90);

    // setn2: center roller in 3-roller groups; also single roller for bt3/bt4
    bk_ui->detailsettingtime_settemp_setn2 = lv_roller_create(bk_ui->detailsettingtime);
    lv_roller_set_options(bk_ui->detailsettingtime_settemp_setn2, "0", LV_ROLLER_MODE_NORMAL);
    _TIME_ROLLER_STYLE(bk_ui->detailsettingtime_settemp_setn2);
    lv_roller_set_visible_row_count(bk_ui->detailsettingtime_settemp_setn2, 3);
    lv_obj_add_event_cb(bk_ui->detailsettingtime_settemp_setn2, detailsettingtime_roller_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_add_flag(bk_ui->detailsettingtime_settemp_setn2, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_pos(bk_ui->detailsettingtime_settemp_setn2, 767-1, 136-2-1-5);
    lv_obj_set_width(bk_ui->detailsettingtime_settemp_setn2, 90);

    // setn3: right roller in 3-roller groups
    bk_ui->detailsettingtime_settemp_setn3 = lv_roller_create(bk_ui->detailsettingtime);
    lv_roller_set_options(bk_ui->detailsettingtime_settemp_setn3, "0", LV_ROLLER_MODE_NORMAL);
    _TIME_ROLLER_STYLE(bk_ui->detailsettingtime_settemp_setn3);
    lv_roller_set_visible_row_count(bk_ui->detailsettingtime_settemp_setn3, 3);
    lv_obj_add_event_cb(bk_ui->detailsettingtime_settemp_setn3, detailsettingtime_roller_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_add_flag(bk_ui->detailsettingtime_settemp_setn3, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_pos(bk_ui->detailsettingtime_settemp_setn3, 882-1, 136-2-1-5);
    lv_obj_set_width(bk_ui->detailsettingtime_settemp_setn3, 90);

    // setn4: leftmost in 4-roller group (bt1: tens-of-minutes)
    bk_ui->detailsettingtime_settemp_setn4 = lv_roller_create(bk_ui->detailsettingtime);
    lv_roller_set_options(bk_ui->detailsettingtime_settemp_setn4, "0", LV_ROLLER_MODE_NORMAL);
    _TIME_ROLLER_STYLE(bk_ui->detailsettingtime_settemp_setn4);
    lv_roller_set_visible_row_count(bk_ui->detailsettingtime_settemp_setn4, 3);
    lv_obj_add_event_cb(bk_ui->detailsettingtime_settemp_setn4, detailsettingtime_roller_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_add_flag(bk_ui->detailsettingtime_settemp_setn4, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_pos(bk_ui->detailsettingtime_settemp_setn4, 613-1, 136-2-1-5);
    lv_obj_set_width(bk_ui->detailsettingtime_settemp_setn4, 80);

    // setn5: second in 4-roller group (units-of-minutes)
    bk_ui->detailsettingtime_settemp_setn5 = lv_roller_create(bk_ui->detailsettingtime);
    lv_roller_set_options(bk_ui->detailsettingtime_settemp_setn5, "0", LV_ROLLER_MODE_NORMAL);
    _TIME_ROLLER_STYLE(bk_ui->detailsettingtime_settemp_setn5);
    lv_roller_set_visible_row_count(bk_ui->detailsettingtime_settemp_setn5, 3);
    lv_obj_add_event_cb(bk_ui->detailsettingtime_settemp_setn5, detailsettingtime_roller_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_add_flag(bk_ui->detailsettingtime_settemp_setn5, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_pos(bk_ui->detailsettingtime_settemp_setn5, 708-1, 136-2-1-5);
    lv_obj_set_width(bk_ui->detailsettingtime_settemp_setn5, 80);

    // setn6: third in 4-roller group (tens-of-seconds Water2)
    bk_ui->detailsettingtime_settemp_setn6 = lv_roller_create(bk_ui->detailsettingtime);
    lv_roller_set_options(bk_ui->detailsettingtime_settemp_setn6, "0", LV_ROLLER_MODE_NORMAL);
    _TIME_ROLLER_STYLE(bk_ui->detailsettingtime_settemp_setn6);
    lv_roller_set_visible_row_count(bk_ui->detailsettingtime_settemp_setn6, 3);
    lv_obj_add_event_cb(bk_ui->detailsettingtime_settemp_setn6, detailsettingtime_roller_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_add_flag(bk_ui->detailsettingtime_settemp_setn6, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_pos(bk_ui->detailsettingtime_settemp_setn6, 803-1, 136-2-1-5);
    lv_obj_set_width(bk_ui->detailsettingtime_settemp_setn6, 80);

    // setn7: rightmost in 4-roller group (units-of-seconds Water3)
    bk_ui->detailsettingtime_settemp_setn7 = lv_roller_create(bk_ui->detailsettingtime);
    lv_roller_set_options(bk_ui->detailsettingtime_settemp_setn7, "0", LV_ROLLER_MODE_NORMAL);
    _TIME_ROLLER_STYLE(bk_ui->detailsettingtime_settemp_setn7);
    lv_roller_set_visible_row_count(bk_ui->detailsettingtime_settemp_setn7, 3);
    lv_obj_add_event_cb(bk_ui->detailsettingtime_settemp_setn7, detailsettingtime_roller_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_add_flag(bk_ui->detailsettingtime_settemp_setn7, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_pos(bk_ui->detailsettingtime_settemp_setn7, 898-1, 136-2-1-5);
    lv_obj_set_width(bk_ui->detailsettingtime_settemp_setn7, 80);

#undef _TIME_ROLLER_STYLE

    // Button: changebt
    bk_ui->detailsettingtime_changebt = lv_button_create(bk_ui->detailsettingtime);
    lv_obj_add_flag(bk_ui->detailsettingtime_changebt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->detailsettingtime_changebt, detailsettingtime_changebt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->detailsettingtime_changebt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->detailsettingtime_changebt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->detailsettingtime_changebt, 0, 0);
    lv_obj_set_pos(bk_ui->detailsettingtime_changebt, 590, 420);
    lv_obj_set_size(bk_ui->detailsettingtime_changebt, 410, 80);

    ui_lang_apply_detailsettingtime(bk_ui);
}

void init_page_detailsettingtime_with_step(bk_lv_ui_t *bk_ui)
{
    return;
}
