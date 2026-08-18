#include "lvgl.h"
#include "beken_ui.h"
#include "custom_func.h"
#include "event_runtime.h"
#include "ui_lang.h"
#include <stdio.h>
#include <string.h>

extern bk_lv_ui_t bk_lv_tool_ui;
extern void detailsettingdefrost_backbt_event_cb(lv_event_t *e);
extern void detailsettingdefrost_settingbt1_event_cb(lv_event_t *e);
extern void detailsettingdefrost_settingbt2_event_cb(lv_event_t *e);
extern void detailsettingdefrost_settingbt3_event_cb(lv_event_t *e);
extern void detailsettingdefrost_changebt_event_cb(lv_event_t *e);
extern void detailsettingdefrost_roller_event_cb(lv_event_t *e);
extern void detailsettingdefrost_load_event_cb(lv_event_t *e);

void destroy_page_detailsettingdefrost(bk_lv_ui_t *bk_ui)
{
    if (bk_ui == NULL) {
        return;
    }
    if (bk_ui->detailsettingdefrost != NULL) {
        lv_obj_del(bk_ui->detailsettingdefrost);
        bk_ui->detailsettingdefrost = NULL;
    }
}

void init_page_detailsettingdefrost(bk_lv_ui_t * bk_ui) {
    if (bk_ui->detailsettingdefrost != NULL && lv_obj_is_valid(bk_ui->detailsettingdefrost)) {
        destroy_page_detailsettingdefrost(bk_ui);
    }

    ui_lang_reset_detailsettingdefrost_cache();
    bk_ui->detailsettingdefrost = lv_obj_create(NULL);
    lv_obj_set_size(bk_ui->detailsettingdefrost, 1024, 600);
    lv_obj_set_scrollbar_mode(bk_ui->detailsettingdefrost, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_event_cb(bk_ui->detailsettingdefrost, detailsettingdefrost_load_event_cb, LV_EVENT_SCREEN_LOAD_START, NULL);
    lv_obj_add_event_cb(bk_ui->detailsettingdefrost, detailsettingdefrost_load_event_cb, LV_EVENT_SCREEN_LOADED,     NULL);
    bk_ui->detailsettingdefrost_bg = lv_image_create(bk_ui->detailsettingdefrost);
    lv_obj_add_flag(bk_ui->detailsettingdefrost_bg, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_style_bg_color(bk_ui->detailsettingdefrost, lv_color_hex(0xd9d9d9), 0);
    lv_obj_set_style_bg_opa(bk_ui->detailsettingdefrost, LV_OPA_COVER, 0);
    lv_obj_set_pos(bk_ui->detailsettingdefrost_bg, 0, 0);

    // ImageView: title
    bk_ui->detailsettingdefrost_title = lv_image_create(bk_ui->detailsettingdefrost);
    lv_obj_set_pos(bk_ui->detailsettingdefrost_title, 0, 10);
    lv_obj_set_size(bk_ui->detailsettingdefrost_title, 380, 80);
    lv_image_set_inner_align(bk_ui->detailsettingdefrost_title, LV_IMAGE_ALIGN_TOP_LEFT);

    // Button: backbt
    bk_ui->detailsettingdefrost_backbt = lv_button_create(bk_ui->detailsettingdefrost);
    lv_obj_add_flag(bk_ui->detailsettingdefrost_backbt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->detailsettingdefrost_backbt, detailsettingdefrost_backbt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->detailsettingdefrost_backbt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->detailsettingdefrost_backbt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->detailsettingdefrost_backbt, 0, 0);
    lv_obj_set_pos(bk_ui->detailsettingdefrost_backbt, 825, 13);
    lv_obj_set_size(bk_ui->detailsettingdefrost_backbt, 179, 74);

    // ImageView: imageview3
    bk_ui->detailsettingdefrost_imageview3 = lv_image_create(bk_ui->detailsettingdefrost);
    lv_obj_set_pos(bk_ui->detailsettingdefrost_imageview3, 825, 13);
    lv_obj_set_size(bk_ui->detailsettingdefrost_imageview3, 179, 74);

    // ImageView: settingim1
    bk_ui->detailsettingdefrost_settingim1 = lv_image_create(bk_ui->detailsettingdefrost);
    lv_obj_set_pos(bk_ui->detailsettingdefrost_settingim1, 23, 120);
    lv_obj_set_size(bk_ui->detailsettingdefrost_settingim1, 538, 66);

    // TextView: settingtxt1
    bk_ui->detailsettingdefrost_settingtxt1 = lv_label_create(bk_ui->detailsettingdefrost);
    lv_label_set_text(bk_ui->detailsettingdefrost_settingtxt1, "");
    lv_obj_set_style_bg_opa(bk_ui->detailsettingdefrost_settingtxt1, 0, 0);
    lv_obj_set_style_text_color(bk_ui->detailsettingdefrost_settingtxt1, lv_color_hex(0xA6A6A6), 0);
    lv_obj_set_style_text_font(bk_ui->detailsettingdefrost_settingtxt1, &lv_font_scdream_regular_32, 0);
    lv_obj_set_pos(bk_ui->detailsettingdefrost_settingtxt1, 270, 135);
    lv_obj_set_size(bk_ui->detailsettingdefrost_settingtxt1, 200, 42);
    lv_obj_set_style_text_align(bk_ui->detailsettingdefrost_settingtxt1, LV_TEXT_ALIGN_RIGHT, 0);

    // Button: settingbt1
    bk_ui->detailsettingdefrost_settingbt1 = lv_button_create(bk_ui->detailsettingdefrost);
    lv_obj_add_flag(bk_ui->detailsettingdefrost_settingbt1, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->detailsettingdefrost_settingbt1, detailsettingdefrost_settingbt1_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->detailsettingdefrost_settingbt1, 0, 0);
    lv_obj_set_style_border_width(bk_ui->detailsettingdefrost_settingbt1, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->detailsettingdefrost_settingbt1, 0, 0);
    lv_obj_set_pos(bk_ui->detailsettingdefrost_settingbt1, 23, 120);
    lv_obj_set_size(bk_ui->detailsettingdefrost_settingbt1, 538, 66);

    // ImageView: settingim2
    bk_ui->detailsettingdefrost_settingim2 = lv_image_create(bk_ui->detailsettingdefrost);
    lv_obj_set_pos(bk_ui->detailsettingdefrost_settingim2, 23, 198);
    lv_obj_set_size(bk_ui->detailsettingdefrost_settingim2, 538, 66);

    // TextView: settingtxt2
    bk_ui->detailsettingdefrost_settingtxt2 = lv_label_create(bk_ui->detailsettingdefrost);
    lv_label_set_text(bk_ui->detailsettingdefrost_settingtxt2, "");
    lv_obj_set_style_bg_opa(bk_ui->detailsettingdefrost_settingtxt2, 0, 0);
    lv_obj_set_style_text_color(bk_ui->detailsettingdefrost_settingtxt2, lv_color_hex(0xA6A6A6), 0);
    lv_obj_set_style_text_font(bk_ui->detailsettingdefrost_settingtxt2, &lv_font_scdream_regular_32, 0);
    lv_obj_set_pos(bk_ui->detailsettingdefrost_settingtxt2, 400, 213);
    lv_obj_set_size(bk_ui->detailsettingdefrost_settingtxt2, 70, 42);
    lv_obj_set_style_text_align(bk_ui->detailsettingdefrost_settingtxt2, LV_TEXT_ALIGN_RIGHT, 0);

    // Button: settingbt2
    bk_ui->detailsettingdefrost_settingbt2 = lv_button_create(bk_ui->detailsettingdefrost);
    lv_obj_add_flag(bk_ui->detailsettingdefrost_settingbt2, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->detailsettingdefrost_settingbt2, detailsettingdefrost_settingbt2_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->detailsettingdefrost_settingbt2, 0, 0);
    lv_obj_set_style_border_width(bk_ui->detailsettingdefrost_settingbt2, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->detailsettingdefrost_settingbt2, 0, 0);
    lv_obj_set_pos(bk_ui->detailsettingdefrost_settingbt2, 23, 198);
    lv_obj_set_size(bk_ui->detailsettingdefrost_settingbt2, 538, 66);

    // ImageView: settingim3
    bk_ui->detailsettingdefrost_settingim3 = lv_image_create(bk_ui->detailsettingdefrost);
    lv_obj_set_pos(bk_ui->detailsettingdefrost_settingim3, 23, 276);
    lv_obj_set_size(bk_ui->detailsettingdefrost_settingim3, 538, 66);

    // TextView: settingtxt3
    bk_ui->detailsettingdefrost_settingtxt3 = lv_label_create(bk_ui->detailsettingdefrost);
    lv_label_set_text(bk_ui->detailsettingdefrost_settingtxt3, "");
    lv_obj_set_style_bg_opa(bk_ui->detailsettingdefrost_settingtxt3, 0, 0);
    lv_obj_set_style_text_color(bk_ui->detailsettingdefrost_settingtxt3, lv_color_hex(0xA6A6A6), 0);
    lv_obj_set_style_text_font(bk_ui->detailsettingdefrost_settingtxt3, &lv_font_scdream_regular_32, 0);
    lv_obj_set_pos(bk_ui->detailsettingdefrost_settingtxt3, 420, 291);
    lv_obj_set_size(bk_ui->detailsettingdefrost_settingtxt3, 50, 42);
    lv_obj_set_style_text_align(bk_ui->detailsettingdefrost_settingtxt3, LV_TEXT_ALIGN_RIGHT, 0);

    // Button: settingbt3
    bk_ui->detailsettingdefrost_settingbt3 = lv_button_create(bk_ui->detailsettingdefrost);
    lv_obj_add_flag(bk_ui->detailsettingdefrost_settingbt3, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->detailsettingdefrost_settingbt3, detailsettingdefrost_settingbt3_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->detailsettingdefrost_settingbt3, 0, 0);
    lv_obj_set_style_border_width(bk_ui->detailsettingdefrost_settingbt3, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->detailsettingdefrost_settingbt3, 0, 0);
    lv_obj_set_pos(bk_ui->detailsettingdefrost_settingbt3, 20, 276);
    lv_obj_set_size(bk_ui->detailsettingdefrost_settingbt3, 538, 66);

    // ImageView: pickerbox
    bk_ui->detailsettingdefrost_pickerbox = lv_image_create(bk_ui->detailsettingdefrost);
    ui_lang_apply_picker(bk_ui->detailsettingdefrost_pickerbox, 1);
    lv_obj_set_pos(bk_ui->detailsettingdefrost_pickerbox, 624, 120);
    lv_obj_set_size(bk_ui->detailsettingdefrost_pickerbox, 376, 376);
    lv_obj_add_flag(bk_ui->detailsettingdefrost_pickerbox, LV_OBJ_FLAG_HIDDEN);


    // NumberPicker: settemp_setn1 (lv_roller)
    bk_ui->detailsettingdefrost_settemp_setn1 = lv_roller_create(bk_ui->detailsettingdefrost);
    lv_roller_set_options(bk_ui->detailsettingdefrost_settemp_setn1, "0", LV_ROLLER_MODE_NORMAL);
    /* 폰트 스타일을 먼저 적용 — visible_row_count는 현재 폰트의 line height로 높이를
     * 계산하므로 폰트 적용 전에 호출하면 롤러 높이가 작게 계산된다 */
    lv_obj_set_style_bg_opa(bk_ui->detailsettingdefrost_settemp_setn1, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(bk_ui->detailsettingdefrost_settemp_setn1, 0, LV_PART_MAIN);
    lv_obj_set_style_text_color(bk_ui->detailsettingdefrost_settemp_setn1, lv_color_hex(0xCCCCCC), LV_PART_MAIN);
    lv_obj_set_style_text_font(bk_ui->detailsettingdefrost_settemp_setn1, &lv_font_scdream_regular_72, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(bk_ui->detailsettingdefrost_settemp_setn1, LV_OPA_TRANSP, LV_PART_SELECTED);
    lv_obj_set_style_text_color(bk_ui->detailsettingdefrost_settemp_setn1, lv_color_hex(0x333333), LV_PART_SELECTED);
    lv_obj_set_style_text_font(bk_ui->detailsettingdefrost_settemp_setn1, &lv_font_scdream_regular_90, LV_PART_SELECTED);
    lv_roller_set_visible_row_count(bk_ui->detailsettingdefrost_settemp_setn1, 3);
    lv_obj_add_event_cb(bk_ui->detailsettingdefrost_settemp_setn1, detailsettingdefrost_roller_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_add_flag(bk_ui->detailsettingdefrost_settemp_setn1, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_pos(bk_ui->detailsettingdefrost_settemp_setn1,  700-10, 136-5-2-1);
    lv_obj_set_width(bk_ui->detailsettingdefrost_settemp_setn1, 240);

    // Button: changebt
    bk_ui->detailsettingdefrost_changebt = lv_button_create(bk_ui->detailsettingdefrost);
    lv_obj_add_flag(bk_ui->detailsettingdefrost_changebt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->detailsettingdefrost_changebt, detailsettingdefrost_changebt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->detailsettingdefrost_changebt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->detailsettingdefrost_changebt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->detailsettingdefrost_changebt, 0, 0);
    lv_obj_set_pos(bk_ui->detailsettingdefrost_changebt, 590, 420);
    lv_obj_set_size(bk_ui->detailsettingdefrost_changebt, 410, 80);

    ui_lang_apply_detailsettingdefrost(bk_ui);
}
