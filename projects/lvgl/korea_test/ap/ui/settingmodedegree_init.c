#include "lvgl.h"
#include "beken_ui.h"
#include "custom_func.h"
#include "event_runtime.h"
#include "ui_lang.h"
#include <stdio.h>
#include <string.h>

extern bk_lv_ui_t bk_lv_tool_ui;
extern void settingmodedegree_backbt_event_cb(lv_event_t *e);
extern void settingmodedegree_degree_c_bt_event_cb(lv_event_t *e);
extern void settingmodedegree_degree_f_bt_event_cb(lv_event_t *e);
extern void settingmodedegree_load_event_cb(lv_event_t *e);

void destroy_page_settingmodedegree(bk_lv_ui_t *bk_ui)
{
    if (bk_ui == NULL) {
        return;
    }
    if (bk_ui->settingmodedegree != NULL) {
        lv_obj_del(bk_ui->settingmodedegree);
        bk_ui->settingmodedegree = NULL;
    }
}

void init_page_settingmodedegree(bk_lv_ui_t * bk_ui) {
    if (bk_ui->settingmodedegree != NULL && lv_obj_is_valid(bk_ui->settingmodedegree)) {
        destroy_page_settingmodedegree(bk_ui);
    }

    ui_lang_reset_settingmodedegree_cache();
    bk_ui->settingmodedegree = lv_obj_create(NULL);
    lv_obj_set_size(bk_ui->settingmodedegree, 1024, 600);
    lv_obj_set_scrollbar_mode(bk_ui->settingmodedegree, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_event_cb(bk_ui->settingmodedegree, settingmodedegree_load_event_cb, LV_EVENT_SCREEN_LOAD_START, NULL);
    lv_obj_add_event_cb(bk_ui->settingmodedegree, settingmodedegree_load_event_cb, LV_EVENT_SCREEN_LOADED,     NULL);
    /* 배경 — bg.jpg 대신 단색(0xd9d9d9) */
    bk_ui->settingmodedegree_bg = lv_image_create(bk_ui->settingmodedegree);
    lv_obj_add_flag(bk_ui->settingmodedegree_bg, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_style_bg_color(bk_ui->settingmodedegree, lv_color_hex(0xd9d9d9), 0);
    lv_obj_set_style_bg_opa(bk_ui->settingmodedegree, LV_OPA_COVER, 0);
    lv_obj_set_pos(bk_ui->settingmodedegree_bg, 0, 0);

    // ImageView: title
    bk_ui->settingmodedegree_title = lv_image_create(bk_ui->settingmodedegree);
    _img_set_src_timed(bk_ui->settingmodedegree_title, "/images/symbol_title.png");
    lv_obj_set_pos(bk_ui->settingmodedegree_title, 0, 10);
    lv_obj_set_size(bk_ui->settingmodedegree_title, 380, 80);
    lv_image_set_inner_align(bk_ui->settingmodedegree_title, LV_IMAGE_ALIGN_TOP_LEFT);

    // Button: backbt
    bk_ui->settingmodedegree_backbt = lv_button_create(bk_ui->settingmodedegree);
    lv_obj_add_flag(bk_ui->settingmodedegree_backbt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->settingmodedegree_backbt, settingmodedegree_backbt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->settingmodedegree_backbt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->settingmodedegree_backbt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->settingmodedegree_backbt, 0, 0);
    lv_obj_set_pos(bk_ui->settingmodedegree_backbt, 825, 13);
    lv_obj_set_size(bk_ui->settingmodedegree_backbt, 179, 74);

    // ImageView: imageview3
    bk_ui->settingmodedegree_imageview3 = lv_image_create(bk_ui->settingmodedegree);
    _img_set_src_timed(bk_ui->settingmodedegree_imageview3, "/images/exit_bt.png");
    lv_obj_set_pos(bk_ui->settingmodedegree_imageview3, 825, 13);
    lv_obj_set_size(bk_ui->settingmodedegree_imageview3, 179, 74);

    // Button: degree_c_bt
    bk_ui->settingmodedegree_degree_c_bt = lv_button_create(bk_ui->settingmodedegree);
    lv_obj_add_flag(bk_ui->settingmodedegree_degree_c_bt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->settingmodedegree_degree_c_bt, settingmodedegree_degree_c_bt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->settingmodedegree_degree_c_bt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->settingmodedegree_degree_c_bt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->settingmodedegree_degree_c_bt, 0, 0);
    lv_obj_set_pos(bk_ui->settingmodedegree_degree_c_bt, 148, 192);
    lv_obj_set_size(bk_ui->settingmodedegree_degree_c_bt, 296, 156);

    // ImageView: degree_c_im
    bk_ui->settingmodedegree_degree_c_im = lv_image_create(bk_ui->settingmodedegree);
    _img_set_src_timed(bk_ui->settingmodedegree_degree_c_im, "/images/degree_bt_c_off.png");
    lv_obj_set_pos(bk_ui->settingmodedegree_degree_c_im, 148, 192);
    lv_obj_set_size(bk_ui->settingmodedegree_degree_c_im, 296, 156);

    // Button: degree_f_bt
    bk_ui->settingmodedegree_degree_f_bt = lv_button_create(bk_ui->settingmodedegree);
    lv_obj_add_flag(bk_ui->settingmodedegree_degree_f_bt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->settingmodedegree_degree_f_bt, settingmodedegree_degree_f_bt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->settingmodedegree_degree_f_bt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->settingmodedegree_degree_f_bt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->settingmodedegree_degree_f_bt, 0, 0);
    lv_obj_set_pos(bk_ui->settingmodedegree_degree_f_bt, 580, 192);
    lv_obj_set_size(bk_ui->settingmodedegree_degree_f_bt, 296, 156);

    // ImageView: degree_f_im
    bk_ui->settingmodedegree_degree_f_im = lv_image_create(bk_ui->settingmodedegree);
    _img_set_src_timed(bk_ui->settingmodedegree_degree_f_im, "/images/degree_bt_f_off.png");
    lv_obj_set_pos(bk_ui->settingmodedegree_degree_f_im, 580, 192);
    lv_obj_set_size(bk_ui->settingmodedegree_degree_f_im, 296, 156);

}
