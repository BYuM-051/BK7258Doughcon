#include "lvgl.h"
#include "beken_ui.h"
#include "custom_func.h"
#include "event_runtime.h"
#include "ui_lang.h"
#include <stdio.h>
#include <string.h>

extern bk_lv_ui_t bk_lv_tool_ui;
extern void settingmodedetailsetting_backbt_event_cb(lv_event_t *e);
extern void settingmodedetailsetting_detail_temp_bt_event_cb(lv_event_t *e);
extern void settingmodedetailsetting_detail_humidity_bt_event_cb(lv_event_t *e);
extern void settingmodedetailsetting_detail_time_bt_event_cb(lv_event_t *e);
extern void settingmodedetailsetting_detail_damper_bt_event_cb(lv_event_t *e);
extern void settingmodedetailsetting_detail_defrost_bt_event_cb(lv_event_t *e);
extern void settingmodedetailsetting_detail_reset_bt_event_cb(lv_event_t *e);
extern void settingmodedetailsetting_load_event_cb(lv_event_t *e);

void destroy_page_settingmodedetailsetting(bk_lv_ui_t *bk_ui)
{
    if (bk_ui == NULL) {
        return;
    }
    if (bk_ui->settingmodedetailsetting != NULL) {
        lv_obj_del(bk_ui->settingmodedetailsetting);
        bk_ui->settingmodedetailsetting = NULL;
    }
}

void init_page_settingmodedetailsetting(bk_lv_ui_t * bk_ui) {
    if (bk_ui->settingmodedetailsetting != NULL && lv_obj_is_valid(bk_ui->settingmodedetailsetting)) {
        destroy_page_settingmodedetailsetting(bk_ui);
    }

    ui_lang_reset_settingmodedetailsetting_cache();
    bk_ui->settingmodedetailsetting = lv_obj_create(NULL);
    lv_obj_set_size(bk_ui->settingmodedetailsetting, 1024, 600);
    lv_obj_set_scrollbar_mode(bk_ui->settingmodedetailsetting, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_event_cb(bk_ui->settingmodedetailsetting, settingmodedetailsetting_load_event_cb, LV_EVENT_ALL, NULL);
    bk_ui->settingmodedetailsetting_bg = lv_image_create(bk_ui->settingmodedetailsetting);
#if !UI_SETTINGMODEDETAILSETTING_COMBINED_BG_ENABLE
    _bg_set(bk_ui->settingmodedetailsetting_bg);
#endif
    lv_obj_set_pos(bk_ui->settingmodedetailsetting_bg, 0, 0);

    // ImageView: title
#if !UI_SETTINGMODEDETAILSETTING_COMBINED_BG_ENABLE
    bk_ui->settingmodedetailsetting_title = lv_image_create(bk_ui->settingmodedetailsetting);
    _img_set_src_timed(bk_ui->settingmodedetailsetting_title, "/images/detail_title.png");
    lv_obj_set_pos(bk_ui->settingmodedetailsetting_title, 0, 10);
    lv_obj_set_size(bk_ui->settingmodedetailsetting_title, 380, 80);
    lv_image_set_inner_align(bk_ui->settingmodedetailsetting_title, LV_IMAGE_ALIGN_TOP_LEFT);
#endif

    // Button: backbt
    bk_ui->settingmodedetailsetting_backbt = lv_button_create(bk_ui->settingmodedetailsetting);
    lv_obj_add_flag(bk_ui->settingmodedetailsetting_backbt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->settingmodedetailsetting_backbt, settingmodedetailsetting_backbt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->settingmodedetailsetting_backbt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->settingmodedetailsetting_backbt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->settingmodedetailsetting_backbt, 0, 0);
    lv_obj_set_pos(bk_ui->settingmodedetailsetting_backbt, 825, 13);
    lv_obj_set_size(bk_ui->settingmodedetailsetting_backbt, 179, 74);

    // ImageView: imageview3
#if !UI_SETTINGMODEDETAILSETTING_COMBINED_BG_ENABLE
    bk_ui->settingmodedetailsetting_imageview3 = lv_image_create(bk_ui->settingmodedetailsetting);
    _img_set_src_timed(bk_ui->settingmodedetailsetting_imageview3, "/images/exit_bt.png");
    lv_obj_set_pos(bk_ui->settingmodedetailsetting_imageview3, 825, 13);
    lv_obj_set_size(bk_ui->settingmodedetailsetting_imageview3, 179, 74);
#endif

    // ImageView: imageview4
#if !UI_SETTINGMODEDETAILSETTING_COMBINED_BG_ENABLE
    bk_ui->settingmodedetailsetting_imageview4 = lv_image_create(bk_ui->settingmodedetailsetting);
    _img_set_src_timed(bk_ui->settingmodedetailsetting_imageview4, "/images/detail_temp_bt.png");
    lv_obj_set_pos(bk_ui->settingmodedetailsetting_imageview4, 16, 175);
    lv_obj_set_size(bk_ui->settingmodedetailsetting_imageview4, 176, 231);
#endif

    // Button: detail_temp_bt
    bk_ui->settingmodedetailsetting_detail_temp_bt = lv_button_create(bk_ui->settingmodedetailsetting);
    lv_obj_add_flag(bk_ui->settingmodedetailsetting_detail_temp_bt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->settingmodedetailsetting_detail_temp_bt, settingmodedetailsetting_detail_temp_bt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->settingmodedetailsetting_detail_temp_bt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->settingmodedetailsetting_detail_temp_bt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->settingmodedetailsetting_detail_temp_bt, 0, 0);
    lv_obj_set_pos(bk_ui->settingmodedetailsetting_detail_temp_bt, 16, 175);
    lv_obj_set_size(bk_ui->settingmodedetailsetting_detail_temp_bt, 176, 231);

    // ImageView: imageview6
#if !UI_SETTINGMODEDETAILSETTING_COMBINED_BG_ENABLE
    bk_ui->settingmodedetailsetting_imageview6 = lv_image_create(bk_ui->settingmodedetailsetting);
    _img_set_src_timed(bk_ui->settingmodedetailsetting_imageview6, "/images/detail_humidity_bt.png");
    lv_obj_set_pos(bk_ui->settingmodedetailsetting_imageview6, 220, 175);
    lv_obj_set_size(bk_ui->settingmodedetailsetting_imageview6, 176, 231);
#endif

    // Button: detail_humidity_bt
    bk_ui->settingmodedetailsetting_detail_humidity_bt = lv_button_create(bk_ui->settingmodedetailsetting);
    lv_obj_add_flag(bk_ui->settingmodedetailsetting_detail_humidity_bt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->settingmodedetailsetting_detail_humidity_bt, settingmodedetailsetting_detail_humidity_bt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->settingmodedetailsetting_detail_humidity_bt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->settingmodedetailsetting_detail_humidity_bt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->settingmodedetailsetting_detail_humidity_bt, 0, 0);
    lv_obj_set_pos(bk_ui->settingmodedetailsetting_detail_humidity_bt, 220, 175);
    lv_obj_set_size(bk_ui->settingmodedetailsetting_detail_humidity_bt, 176, 231);

    // ImageView: imageview8
#if !UI_SETTINGMODEDETAILSETTING_COMBINED_BG_ENABLE
    bk_ui->settingmodedetailsetting_imageview8 = lv_image_create(bk_ui->settingmodedetailsetting);
    _img_set_src_timed(bk_ui->settingmodedetailsetting_imageview8, "/images/detail_time_bt.png");
    lv_obj_set_pos(bk_ui->settingmodedetailsetting_imageview8, 424, 175);
    lv_obj_set_size(bk_ui->settingmodedetailsetting_imageview8, 176, 231);
#endif

    // Button: detail_time_bt
    bk_ui->settingmodedetailsetting_detail_time_bt = lv_button_create(bk_ui->settingmodedetailsetting);
    lv_obj_add_flag(bk_ui->settingmodedetailsetting_detail_time_bt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->settingmodedetailsetting_detail_time_bt, settingmodedetailsetting_detail_time_bt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->settingmodedetailsetting_detail_time_bt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->settingmodedetailsetting_detail_time_bt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->settingmodedetailsetting_detail_time_bt, 0, 0);
    lv_obj_set_pos(bk_ui->settingmodedetailsetting_detail_time_bt, 424, 175);
    lv_obj_set_size(bk_ui->settingmodedetailsetting_detail_time_bt, 176, 231);

    // ImageView: imageview10
#if !UI_SETTINGMODEDETAILSETTING_COMBINED_BG_ENABLE
    bk_ui->settingmodedetailsetting_imageview10 = lv_image_create(bk_ui->settingmodedetailsetting);
    _img_set_src_timed(bk_ui->settingmodedetailsetting_imageview10, "/images/detail_damper_bt.png");
    lv_obj_set_pos(bk_ui->settingmodedetailsetting_imageview10, 628, 175);
    lv_obj_set_size(bk_ui->settingmodedetailsetting_imageview10, 176, 231);
#endif

    // Button: detail_damper_bt
    bk_ui->settingmodedetailsetting_detail_damper_bt = lv_button_create(bk_ui->settingmodedetailsetting);
    lv_obj_add_flag(bk_ui->settingmodedetailsetting_detail_damper_bt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->settingmodedetailsetting_detail_damper_bt, settingmodedetailsetting_detail_damper_bt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->settingmodedetailsetting_detail_damper_bt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->settingmodedetailsetting_detail_damper_bt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->settingmodedetailsetting_detail_damper_bt, 0, 0);
    lv_obj_set_pos(bk_ui->settingmodedetailsetting_detail_damper_bt, 628, 175);
    lv_obj_set_size(bk_ui->settingmodedetailsetting_detail_damper_bt, 176, 231);

    // ImageView: imageview12
#if !UI_SETTINGMODEDETAILSETTING_COMBINED_BG_ENABLE
    bk_ui->settingmodedetailsetting_imageview12 = lv_image_create(bk_ui->settingmodedetailsetting);
    _img_set_src_timed(bk_ui->settingmodedetailsetting_imageview12, "/images/detail_defrost_bt.png");
    lv_obj_set_pos(bk_ui->settingmodedetailsetting_imageview12, 832, 175);
    lv_obj_set_size(bk_ui->settingmodedetailsetting_imageview12, 176, 231);
#endif

    // Button: detail_defrost_bt
    bk_ui->settingmodedetailsetting_detail_defrost_bt = lv_button_create(bk_ui->settingmodedetailsetting);
    lv_obj_add_flag(bk_ui->settingmodedetailsetting_detail_defrost_bt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->settingmodedetailsetting_detail_defrost_bt, settingmodedetailsetting_detail_defrost_bt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->settingmodedetailsetting_detail_defrost_bt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->settingmodedetailsetting_detail_defrost_bt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->settingmodedetailsetting_detail_defrost_bt, 0, 0);
    lv_obj_set_pos(bk_ui->settingmodedetailsetting_detail_defrost_bt, 832, 175);
    lv_obj_set_size(bk_ui->settingmodedetailsetting_detail_defrost_bt, 176, 231);

    // ImageView: imageview14
#if !UI_SETTINGMODEDETAILSETTING_COMBINED_BG_ENABLE
    bk_ui->settingmodedetailsetting_imageview14 = lv_image_create(bk_ui->settingmodedetailsetting);
    _img_set_src_timed(bk_ui->settingmodedetailsetting_imageview14, "/images/detail_reset_bt.png");
    lv_obj_set_pos(bk_ui->settingmodedetailsetting_imageview14, 825, 431);
    lv_obj_set_size(bk_ui->settingmodedetailsetting_imageview14, 179, 74);
#endif

    // Button: detail_reset_bt
    bk_ui->settingmodedetailsetting_detail_reset_bt = lv_button_create(bk_ui->settingmodedetailsetting);
    lv_obj_add_flag(bk_ui->settingmodedetailsetting_detail_reset_bt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->settingmodedetailsetting_detail_reset_bt, settingmodedetailsetting_detail_reset_bt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->settingmodedetailsetting_detail_reset_bt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->settingmodedetailsetting_detail_reset_bt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->settingmodedetailsetting_detail_reset_bt, 0, 0);
    lv_obj_set_pos(bk_ui->settingmodedetailsetting_detail_reset_bt, 825, 431);
    lv_obj_set_size(bk_ui->settingmodedetailsetting_detail_reset_bt, 179, 74);

}
