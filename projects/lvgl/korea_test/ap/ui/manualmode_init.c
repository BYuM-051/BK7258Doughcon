#include "lvgl.h"
#include "beken_ui.h"
#include "custom_func.h"
#include "event_runtime.h"
#include "ui_lang.h"
#include <stdio.h>
#include <string.h>

extern bk_lv_ui_t bk_lv_tool_ui;
extern lv_obj_t *preRenderRoot;
extern void manualmode_backbt_event_cb(lv_event_t *e);
extern void manualmode_manual_freezebt_event_cb(lv_event_t *e);
extern void manualmode_manual_defrostbt_event_cb(lv_event_t *e);
extern void manualmode_manual_fermentationbt_event_cb(lv_event_t *e);
extern void manualmode_load_event_cb(lv_event_t *e);

void destroy_page_manualmode(bk_lv_ui_t *bk_ui)
{
    if (bk_ui == NULL) {
        return;
    }
    if (bk_ui->manualmode != NULL) {
        lv_obj_del(bk_ui->manualmode);
        bk_ui->manualmode = NULL;
    }
}

void init_page_manualmode(bk_lv_ui_t * bk_ui) {
    if (bk_ui->manualmode != NULL && lv_obj_is_valid(bk_ui->manualmode)) 
    {
        lv_obj_move_to_index(bk_ui->manualmode, -1);
        return;
        // destroy_page_manualmode(bk_ui);
    }

    ui_lang_reset_manualmode_cache();
    bk_ui->manualmode = lv_obj_create(preRenderRoot);
    lv_obj_set_size(bk_ui->manualmode, 1024, 600);
    lv_obj_set_scrollbar_mode(bk_ui->manualmode, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_event_cb(bk_ui->manualmode, manualmode_load_event_cb, LV_EVENT_SCREEN_LOAD_START, NULL);
    lv_obj_add_event_cb(bk_ui->manualmode, manualmode_load_event_cb, LV_EVENT_SCREEN_LOADED,     NULL);
    lv_obj_set_style_bg_color(bk_ui->manualmode, lv_color_hex(0xD9D9D9), 0);
    lv_obj_set_style_radius(bk_ui->manualmode, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(bk_ui->manualmode, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(bk_ui->manualmode, LV_OPA_COVER, LV_PART_MAIN);

    // bk_ui->manualmode_bg = lv_image_create(bk_ui->manualmode);
    // _img_set_src_timed(bk_ui->manualmode_bg, "/images/manual_bg.jpg");
    // lv_obj_set_pos(bk_ui->manualmode_bg, 0, 0);

    // ImageView: title
    bk_ui->manualmode_title = lv_image_create(bk_ui->manualmode);
    _img_set_src_timed(bk_ui->manualmode_title, "/images/manualmode_title.png");
    lv_obj_set_pos(bk_ui->manualmode_title, 0, 10);
    lv_obj_set_size(bk_ui->manualmode_title, 380, 80);
    lv_image_set_inner_align(bk_ui->manualmode_title, LV_IMAGE_ALIGN_TOP_LEFT);

    // Button: backbt
    bk_ui->manualmode_backbt = lv_button_create(bk_ui->manualmode);
    lv_obj_add_flag(bk_ui->manualmode_backbt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->manualmode_backbt, manualmode_backbt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->manualmode_backbt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->manualmode_backbt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->manualmode_backbt, 0, 0);
    lv_obj_set_pos(bk_ui->manualmode_backbt, 13, 445);
    lv_obj_set_size(bk_ui->manualmode_backbt, 179, 74);

    // ImageView: imageview3
    bk_ui->manualmode_imageview3 = lv_image_create(bk_ui->manualmode);
    _img_set_src_timed(bk_ui->manualmode_imageview3, "/images/exit_bt.png");
    lv_obj_set_pos(bk_ui->manualmode_imageview3, 13, 445);
    lv_obj_set_size(bk_ui->manualmode_imageview3, 179, 74);

    // ImageView: imageview4
    bk_ui->manualmode_imageview4 = lv_image_create(bk_ui->manualmode);
    _img_set_src_timed(bk_ui->manualmode_imageview4, "/images/manual_menu.jpg");
    /* manual_menu.jpg 원본(1024x340, 상하 30px 투명)에서 상하 28px씩 크롭 후
     * 잔여 2px 투명 여백은 화면 배경색(0xD9D9D9)으로 flatten하여 1024x284로 재저장 —
     * 색상 블록 위치 유지 위해 y를 28px 내림 */
    lv_obj_set_pos(bk_ui->manualmode_imageview4, 0, 109+28);
    lv_obj_set_size(bk_ui->manualmode_imageview4, 1024, 284);

    // Button: manual_freezebt
    bk_ui->manualmode_manual_freezebt = lv_button_create(bk_ui->manualmode);
    lv_obj_add_flag(bk_ui->manualmode_manual_freezebt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->manualmode_manual_freezebt, manualmode_manual_freezebt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->manualmode_manual_freezebt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->manualmode_manual_freezebt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->manualmode_manual_freezebt, 0, 0);
    lv_obj_set_pos(bk_ui->manualmode_manual_freezebt, 0, 128);
    lv_obj_set_size(bk_ui->manualmode_manual_freezebt, 334, 290);

    // Button: manual_defrostbt
    bk_ui->manualmode_manual_defrostbt = lv_button_create(bk_ui->manualmode);
    lv_obj_add_flag(bk_ui->manualmode_manual_defrostbt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->manualmode_manual_defrostbt, manualmode_manual_defrostbt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->manualmode_manual_defrostbt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->manualmode_manual_defrostbt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->manualmode_manual_defrostbt, 0, 0);
    lv_obj_set_pos(bk_ui->manualmode_manual_defrostbt, 342, 128);
    lv_obj_set_size(bk_ui->manualmode_manual_defrostbt, 334, 290);

    // Button: manual_fermentationbt
    bk_ui->manualmode_manual_fermentationbt = lv_button_create(bk_ui->manualmode);
    lv_obj_add_flag(bk_ui->manualmode_manual_fermentationbt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->manualmode_manual_fermentationbt, manualmode_manual_fermentationbt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->manualmode_manual_fermentationbt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->manualmode_manual_fermentationbt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->manualmode_manual_fermentationbt, 0, 0);
    lv_obj_set_pos(bk_ui->manualmode_manual_fermentationbt, 685, 128);
    lv_obj_set_size(bk_ui->manualmode_manual_fermentationbt, 334, 290);

}
