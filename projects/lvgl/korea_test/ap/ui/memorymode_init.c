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
extern void memorymode_backbt_event_cb(lv_event_t *e);
extern void memorymode_memory_check1bt_event_cb(lv_event_t *e);
extern void memorymode_memory_check2bt_event_cb(lv_event_t *e);
extern void memorymode_memory_check3bt_event_cb(lv_event_t *e);
extern void memorymode_memory_check4bt_event_cb(lv_event_t *e);
extern void memorymode_memoryleftbt_event_cb(lv_event_t *e);
extern void memorymode_memory1bt_event_cb(lv_event_t *e);
extern void memorymode_memory2bt_event_cb(lv_event_t *e);
extern void memorymode_memory3bt_event_cb(lv_event_t *e);
extern void memorymode_memoryrightbt_event_cb(lv_event_t *e);
extern void memorymode_okbt_event_cb(lv_event_t *e);
extern void memorymode_deletebt_event_cb(lv_event_t *e);
extern void memorymode_load_start_event_cb(lv_event_t *e);
extern void memorymode_loaded_event_cb(lv_event_t *e);
extern void memorymode_unload_start_event_cb(lv_event_t *e);
extern void memorymode_unloaded_event_cb(lv_event_t *e);

void destroy_page_memorymode(bk_lv_ui_t *bk_ui)
{
    if (bk_ui == NULL) {
        return;
    }
    if (bk_ui->memorymode != NULL) {
        lv_obj_del(bk_ui->memorymode);
        bk_ui->memorymode = NULL;
    }
}

void init_page_memorymode(bk_lv_ui_t * bk_ui) {
    if (bk_ui->memorymode != NULL && lv_obj_is_valid(bk_ui->memorymode)) {
        return;  /* keep-alive: reuse existing screen, load_event_cb handles refresh */
    }

    /* 오브젝트를 새로 만드므로 ui_lang 캐시를 무효화 — 다음 ui_lang_apply_memorymode()가
     * 언어 변경 여부와 무관하게 반드시 새 이미지를 채우게 함 */
    ui_lang_reset_memorymode_cache();

#if UI_PRENDERING_ENABLE
    bk_ui->memorymode = lv_obj_create(preRenderRoot);
    lv_obj_remove_style_all(bk_ui->memorymode);
    lv_obj_set_size(bk_ui->memorymode, 1024, 600);
    lv_obj_set_pos(bk_ui->memorymode, 0, 0);
    lv_obj_set_style_radius(bk_ui->memorymode, 0, LV_PART_MAIN);
    lv_obj_set_scrollbar_mode(bk_ui->memorymode, LV_SCROLLBAR_MODE_OFF);
    // 원래 LV_EVENT_ALL로 등록되어 있었음
    lv_obj_add_event_cb(bk_ui->memorymode, memorymode_load_start_event_cb, UI_EVENT_PAGE_SHOW_START, NULL);
    lv_obj_add_event_cb(bk_ui->memorymode, memorymode_loaded_event_cb, UI_EVENT_PAGE_SHOWN,     NULL);
    lv_obj_add_event_cb(bk_ui->memorymode, memorymode_unload_start_event_cb, UI_EVENT_PAGE_HIDE_START, NULL);
    lv_obj_add_event_cb(bk_ui->memorymode, memorymode_unloaded_event_cb, UI_EVENT_PAGE_HIDDEN,     NULL);
#else
    bk_ui->memorymode = lv_obj_create(NULL);
    lv_obj_set_size(bk_ui->memorymode, 1024, 600);
    lv_obj_set_scrollbar_mode(bk_ui->memorymode, LV_SCROLLBAR_MODE_OFF);
    // 원래 LV_EVENT_ALL로 등록되어 있었음
    lv_obj_add_event_cb(bk_ui->memorymode, memorymode_load_start_event_cb, LV_EVENT_SCREEN_LOAD_START, NULL);
    lv_obj_add_event_cb(bk_ui->memorymode, memorymode_loaded_event_cb, LV_EVENT_SCREEN_LOADED,     NULL);
    lv_obj_add_event_cb(bk_ui->memorymode, memorymode_unload_start_event_cb, LV_EVENT_SCREEN_UNLOAD_START, NULL);
    lv_obj_add_event_cb(bk_ui->memorymode, memorymode_unloaded_event_cb, LV_EVENT_SCREEN_UNLOADED,     NULL);
#endif /* UI_PRENDERING_ENABLE */
    bk_ui->memorymode_bg = lv_image_create(bk_ui->memorymode);
    /* bg.jpg 대신 단색(0xd9d9d9)으로 채움 */
    lv_obj_add_flag(bk_ui->memorymode_bg, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_style_bg_color(bk_ui->memorymode, lv_color_hex(0xd9d9d9), 0);
    lv_obj_set_style_bg_opa(bk_ui->memorymode, LV_OPA_COVER, 0);
    lv_obj_set_pos(bk_ui->memorymode_bg, 0, 0);

    // ImageView: title
    bk_ui->memorymode_title = lv_image_create(bk_ui->memorymode);
    _img_set_src_deferred(bk_ui->memorymode_title, "/images/memorymode_title.png");
    lv_obj_set_pos(bk_ui->memorymode_title, 0, 10);
    lv_obj_set_size(bk_ui->memorymode_title, 380, 80);
    lv_image_set_inner_align(bk_ui->memorymode_title, LV_IMAGE_ALIGN_TOP_LEFT);

    // Button: backbt
    bk_ui->memorymode_backbt = lv_button_create(bk_ui->memorymode);
    lv_obj_add_flag(bk_ui->memorymode_backbt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->memorymode_backbt, memorymode_backbt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->memorymode_backbt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->memorymode_backbt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->memorymode_backbt, 0, 0);
    lv_obj_set_pos(bk_ui->memorymode_backbt, 13, 445);
    lv_obj_set_size(bk_ui->memorymode_backbt, 179, 74);

    // ImageView: imageview3
    bk_ui->memorymode_imageview3 = lv_image_create(bk_ui->memorymode);
    _img_set_src_deferred(bk_ui->memorymode_imageview3, "/images/exit_bt.png");
    lv_obj_set_pos(bk_ui->memorymode_imageview3, 13, 445);
    lv_obj_set_size(bk_ui->memorymode_imageview3, 179, 74);

    // ImageView: imageview4
    bk_ui->memorymode_imageview4 = lv_image_create(bk_ui->memorymode);
    _img_set_src_deferred(bk_ui->memorymode_imageview4, "/images/memory_title_line.png");
    lv_obj_set_pos(bk_ui->memorymode_imageview4, 12, 93);
    lv_obj_set_size(bk_ui->memorymode_imageview4, 1000, 66);

    // ImageView: memorybox0
    bk_ui->memorymode_memorybox0 = lv_image_create(bk_ui->memorymode);
    _img_set_src_deferred(bk_ui->memorymode_memorybox0, "/images/memory_box.png");
    lv_obj_set_pos(bk_ui->memorymode_memorybox0, 12, 159);
    lv_obj_set_size(bk_ui->memorymode_memorybox0, 1000, 70);

    // TextView: memory_number0
    bk_ui->memorymode_memory_number0 = lv_label_create(bk_ui->memorymode);
    lv_label_set_text(bk_ui->memorymode_memory_number0, "");
    lv_obj_set_style_bg_opa(bk_ui->memorymode_memory_number0, 0, 0);
    lv_obj_set_style_text_color(bk_ui->memorymode_memory_number0, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->memorymode_memory_number0, &lv_font_scdream_regular_26, 0);
    lv_obj_set_pos(bk_ui->memorymode_memory_number0, 16+2, 170+10);
    lv_obj_set_size(bk_ui->memorymode_memory_number0, 60, 60);
    lv_obj_set_style_text_align(bk_ui->memorymode_memory_number0, LV_TEXT_ALIGN_CENTER, 0);


    // TextView: memory_Day_Period0
    bk_ui->memorymode_memory_Day_Period0 = lv_label_create(bk_ui->memorymode);
    lv_label_set_text(bk_ui->memorymode_memory_Day_Period0, "");
    lv_obj_set_style_bg_opa(bk_ui->memorymode_memory_Day_Period0, 0, 0);
    lv_obj_set_style_text_color(bk_ui->memorymode_memory_Day_Period0, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->memorymode_memory_Day_Period0, &lv_font_scdream_regular_26, 0);
    lv_obj_set_style_text_align(bk_ui->memorymode_memory_Day_Period0, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_set_pos(bk_ui->memorymode_memory_Day_Period0, 95, 180);
    lv_obj_set_size(bk_ui->memorymode_memory_Day_Period0, 40, 40);
    lv_label_set_long_mode(bk_ui->memorymode_memory_Day_Period0, LV_LABEL_LONG_CLIP);

    // TextView: memory_freeze_temp0
    bk_ui->memorymode_memory_freeze_temp0 = lv_label_create(bk_ui->memorymode);
    lv_label_set_text(bk_ui->memorymode_memory_freeze_temp0, "");
    lv_obj_set_style_bg_opa(bk_ui->memorymode_memory_freeze_temp0, 0, 0);
    lv_obj_set_style_text_color(bk_ui->memorymode_memory_freeze_temp0, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->memorymode_memory_freeze_temp0, &lv_font_scdream_regular_26, 0);
    lv_obj_set_style_text_align(bk_ui->memorymode_memory_freeze_temp0, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_set_pos(bk_ui->memorymode_memory_freeze_temp0, 190, 180);
    lv_obj_set_size(bk_ui->memorymode_memory_freeze_temp0, 60, 40);

    // TextView: memory_defrost_temp0
    bk_ui->memorymode_memory_defrost_temp0 = lv_label_create(bk_ui->memorymode);
    lv_label_set_text(bk_ui->memorymode_memory_defrost_temp0, "");
    lv_obj_set_style_bg_opa(bk_ui->memorymode_memory_defrost_temp0, 0, 0);
    lv_obj_set_style_text_color(bk_ui->memorymode_memory_defrost_temp0, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->memorymode_memory_defrost_temp0, &lv_font_scdream_regular_26, 0);
    lv_obj_set_style_text_align(bk_ui->memorymode_memory_defrost_temp0, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_set_pos(bk_ui->memorymode_memory_defrost_temp0, 326, 164);
    lv_obj_set_size(bk_ui->memorymode_memory_defrost_temp0, 60, 40);

    // TextView: memory_defrost_hour0
    bk_ui->memorymode_memory_defrost_hour0 = lv_label_create(bk_ui->memorymode);
    lv_label_set_text(bk_ui->memorymode_memory_defrost_hour0, "");
    lv_obj_set_style_bg_opa(bk_ui->memorymode_memory_defrost_hour0, 0, 0);
    lv_obj_set_style_text_color(bk_ui->memorymode_memory_defrost_hour0, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->memorymode_memory_defrost_hour0, &lv_font_scdream_regular_26, 0);
    lv_obj_set_pos(bk_ui->memorymode_memory_defrost_hour0, 318, 195);
    lv_obj_set_size(bk_ui->memorymode_memory_defrost_hour0, 80, 40);

    // TextView: memory_defrost_min0
    bk_ui->memorymode_memory_defrost_min0 = lv_label_create(bk_ui->memorymode);
    lv_label_set_text(bk_ui->memorymode_memory_defrost_min0, "");
    lv_obj_set_style_bg_opa(bk_ui->memorymode_memory_defrost_min0, 0, 0);
    lv_obj_set_style_text_color(bk_ui->memorymode_memory_defrost_min0, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->memorymode_memory_defrost_min0, &lv_font_scdream_regular_26, 0);
    lv_obj_set_pos(bk_ui->memorymode_memory_defrost_min0, 382, 195);
    lv_obj_set_size(bk_ui->memorymode_memory_defrost_min0, 40, 40);

    // TextView: memory_fermentation1_temp0
    bk_ui->memorymode_memory_fermentation1_temp0 = lv_label_create(bk_ui->memorymode);
    lv_label_set_text(bk_ui->memorymode_memory_fermentation1_temp0, "");
    lv_obj_set_style_bg_opa(bk_ui->memorymode_memory_fermentation1_temp0, 0, 0);
    lv_obj_set_style_text_color(bk_ui->memorymode_memory_fermentation1_temp0, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->memorymode_memory_fermentation1_temp0, &lv_font_scdream_regular_26, 0);
    lv_obj_set_style_text_align(bk_ui->memorymode_memory_fermentation1_temp0, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_set_pos(bk_ui->memorymode_memory_fermentation1_temp0, 445, 164);
    lv_obj_set_size(bk_ui->memorymode_memory_fermentation1_temp0, 60, 40);

    // TextView: memory_fermentation1_humidity0
    bk_ui->memorymode_memory_fermentation1_humidity0 = lv_label_create(bk_ui->memorymode);
    lv_label_set_text(bk_ui->memorymode_memory_fermentation1_humidity0, "");
    lv_obj_set_style_bg_opa(bk_ui->memorymode_memory_fermentation1_humidity0, 0, 0);
    lv_obj_set_style_text_color(bk_ui->memorymode_memory_fermentation1_humidity0, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->memorymode_memory_fermentation1_humidity0, &lv_font_scdream_regular_26, 0);
    lv_obj_set_style_text_align(bk_ui->memorymode_memory_fermentation1_humidity0, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_set_pos(bk_ui->memorymode_memory_fermentation1_humidity0, 529, 164);
    lv_obj_set_size(bk_ui->memorymode_memory_fermentation1_humidity0, 40, 40);

    // TextView: memory_fermentation1_hour0
    bk_ui->memorymode_memory_fermentation1_hour0 = lv_label_create(bk_ui->memorymode);
    lv_label_set_text(bk_ui->memorymode_memory_fermentation1_hour0, "");
    lv_obj_set_style_bg_opa(bk_ui->memorymode_memory_fermentation1_hour0, 0, 0);
    lv_obj_set_style_text_color(bk_ui->memorymode_memory_fermentation1_hour0, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->memorymode_memory_fermentation1_hour0, &lv_font_scdream_regular_26, 0);
    lv_obj_set_pos(bk_ui->memorymode_memory_fermentation1_hour0, 473, 195);
    lv_obj_set_size(bk_ui->memorymode_memory_fermentation1_hour0, 40, 40);

    // TextView: memory_fermentation1_min0
    bk_ui->memorymode_memory_fermentation1_min0 = lv_label_create(bk_ui->memorymode);
    lv_label_set_text(bk_ui->memorymode_memory_fermentation1_min0, "");
    lv_obj_set_style_bg_opa(bk_ui->memorymode_memory_fermentation1_min0, 0, 0);
    lv_obj_set_style_text_color(bk_ui->memorymode_memory_fermentation1_min0, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->memorymode_memory_fermentation1_min0, &lv_font_scdream_regular_26, 0);
    lv_obj_set_pos(bk_ui->memorymode_memory_fermentation1_min0, 537, 195);
    lv_obj_set_size(bk_ui->memorymode_memory_fermentation1_min0, 40, 40);

    // TextView: memory_fermentation2_temp0
    bk_ui->memorymode_memory_fermentation2_temp0 = lv_label_create(bk_ui->memorymode);
    lv_label_set_text(bk_ui->memorymode_memory_fermentation2_temp0, "");
    lv_obj_set_style_bg_opa(bk_ui->memorymode_memory_fermentation2_temp0, 0, 0);
    lv_obj_set_style_text_color(bk_ui->memorymode_memory_fermentation2_temp0, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->memorymode_memory_fermentation2_temp0, &lv_font_scdream_regular_26, 0);
    lv_obj_set_style_text_align(bk_ui->memorymode_memory_fermentation2_temp0, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_set_pos(bk_ui->memorymode_memory_fermentation2_temp0, 615, 164);
    lv_obj_set_size(bk_ui->memorymode_memory_fermentation2_temp0, 60, 40);

    // TextView: memory_fermentation2_humidity0
    bk_ui->memorymode_memory_fermentation2_humidity0 = lv_label_create(bk_ui->memorymode);
    lv_label_set_text(bk_ui->memorymode_memory_fermentation2_humidity0, "");
    lv_obj_set_style_bg_opa(bk_ui->memorymode_memory_fermentation2_humidity0, 0, 0);
    lv_obj_set_style_text_color(bk_ui->memorymode_memory_fermentation2_humidity0, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->memorymode_memory_fermentation2_humidity0, &lv_font_scdream_regular_26, 0);
    lv_obj_set_style_text_align(bk_ui->memorymode_memory_fermentation2_humidity0, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_set_pos(bk_ui->memorymode_memory_fermentation2_humidity0, 699, 164);
    lv_obj_set_size(bk_ui->memorymode_memory_fermentation2_humidity0, 40, 40);

    // TextView: memory_fermentation2_hour0
    bk_ui->memorymode_memory_fermentation2_hour0 = lv_label_create(bk_ui->memorymode);
    lv_label_set_text(bk_ui->memorymode_memory_fermentation2_hour0, "");
    lv_obj_set_style_bg_opa(bk_ui->memorymode_memory_fermentation2_hour0, 0, 0);
    lv_obj_set_style_text_color(bk_ui->memorymode_memory_fermentation2_hour0, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->memorymode_memory_fermentation2_hour0, &lv_font_scdream_regular_26, 0);
    lv_obj_set_pos(bk_ui->memorymode_memory_fermentation2_hour0, 643, 195);
    lv_obj_set_size(bk_ui->memorymode_memory_fermentation2_hour0, 40, 40);

    // TextView: memory_fermentation2_min0
    bk_ui->memorymode_memory_fermentation2_min0 = lv_label_create(bk_ui->memorymode);
    lv_label_set_text(bk_ui->memorymode_memory_fermentation2_min0, "");
    lv_obj_set_style_bg_opa(bk_ui->memorymode_memory_fermentation2_min0, 0, 0);
    lv_obj_set_style_text_color(bk_ui->memorymode_memory_fermentation2_min0, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->memorymode_memory_fermentation2_min0, &lv_font_scdream_regular_26, 0);
    lv_obj_set_pos(bk_ui->memorymode_memory_fermentation2_min0, 707, 195);
    lv_obj_set_size(bk_ui->memorymode_memory_fermentation2_min0, 40, 40);

    // TextView: memory_completehour0
    bk_ui->memorymode_memory_completehour0 = lv_label_create(bk_ui->memorymode);
    lv_label_set_text(bk_ui->memorymode_memory_completehour0, "");
    lv_obj_set_style_bg_opa(bk_ui->memorymode_memory_completehour0, 0, 0);
    lv_obj_set_style_text_color(bk_ui->memorymode_memory_completehour0, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->memorymode_memory_completehour0, &lv_font_scdream_regular_26, 0);
    lv_obj_set_style_text_align(bk_ui->memorymode_memory_completehour0, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_set_pos(bk_ui->memorymode_memory_completehour0, 785, 179);
    lv_obj_set_size(bk_ui->memorymode_memory_completehour0, 40, 40);

    // TextView: memory_completemin0
    bk_ui->memorymode_memory_completemin0 = lv_label_create(bk_ui->memorymode);
    lv_label_set_text(bk_ui->memorymode_memory_completemin0, "");
    lv_obj_set_style_bg_opa(bk_ui->memorymode_memory_completemin0, 0, 0);
    lv_obj_set_style_text_color(bk_ui->memorymode_memory_completemin0, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->memorymode_memory_completemin0, &lv_font_scdream_regular_26, 0);
    lv_obj_set_pos(bk_ui->memorymode_memory_completemin0, 847, 179);
    lv_obj_set_size(bk_ui->memorymode_memory_completemin0, 40, 40);

    // ImageView: memory_check1im  (src set by _refresh_display in load_event_cb)
    bk_ui->memorymode_memory_check1im = lv_image_create(bk_ui->memorymode);
    lv_obj_set_pos(bk_ui->memorymode_memory_check1im, 925, 172);
    lv_obj_set_size(bk_ui->memorymode_memory_check1im, 40, 40);

    // Button: memory_check1bt
    bk_ui->memorymode_memory_check1bt = lv_button_create(bk_ui->memorymode);
    lv_obj_add_flag(bk_ui->memorymode_memory_check1bt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->memorymode_memory_check1bt, memorymode_memory_check1bt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->memorymode_memory_check1bt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->memorymode_memory_check1bt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->memorymode_memory_check1bt, 0, 0);
    lv_obj_set_pos(bk_ui->memorymode_memory_check1bt, 925, 172);
    lv_obj_set_size(bk_ui->memorymode_memory_check1bt, 40, 40);

    // ImageView: memorybox1  (src deferred to load_event_cb — cache hit after memorybox0)
    bk_ui->memorymode_memorybox1 = lv_image_create(bk_ui->memorymode);
    lv_obj_set_pos(bk_ui->memorymode_memorybox1, 12, 231);
    lv_obj_set_size(bk_ui->memorymode_memorybox1, 1000, 70);

    // TextView: memory_number1
    bk_ui->memorymode_memory_number1 = lv_label_create(bk_ui->memorymode);
    lv_label_set_text(bk_ui->memorymode_memory_number1, "");
    lv_obj_set_style_bg_opa(bk_ui->memorymode_memory_number1, 0, 0);
    lv_obj_set_style_text_color(bk_ui->memorymode_memory_number1, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->memorymode_memory_number1, &lv_font_scdream_regular_26, 0);
    lv_obj_set_pos(bk_ui->memorymode_memory_number1, 16+2, 242+10);
    lv_obj_set_size(bk_ui->memorymode_memory_number1, 60,60);
    lv_obj_set_style_text_align(bk_ui->memorymode_memory_number1, LV_TEXT_ALIGN_CENTER, 0);

    // TextView: memory_Day_Period1
    bk_ui->memorymode_memory_Day_Period1 = lv_label_create(bk_ui->memorymode);
    lv_label_set_text(bk_ui->memorymode_memory_Day_Period1, "");
    lv_obj_set_style_bg_opa(bk_ui->memorymode_memory_Day_Period1, 0, 0);
    lv_obj_set_style_text_color(bk_ui->memorymode_memory_Day_Period1, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->memorymode_memory_Day_Period1, &lv_font_scdream_regular_26, 0);
    lv_obj_set_style_text_align(bk_ui->memorymode_memory_Day_Period1, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_set_pos(bk_ui->memorymode_memory_Day_Period1, 95, 252);
    lv_obj_set_size(bk_ui->memorymode_memory_Day_Period1, 40, 40);
    lv_label_set_long_mode(bk_ui->memorymode_memory_Day_Period1, LV_LABEL_LONG_CLIP);

    // TextView: memory_freeze_temp1
    bk_ui->memorymode_memory_freeze_temp1 = lv_label_create(bk_ui->memorymode);
    lv_label_set_text(bk_ui->memorymode_memory_freeze_temp1, "");
    lv_obj_set_style_bg_opa(bk_ui->memorymode_memory_freeze_temp1, 0, 0);
    lv_obj_set_style_text_color(bk_ui->memorymode_memory_freeze_temp1, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->memorymode_memory_freeze_temp1, &lv_font_scdream_regular_26, 0);
    lv_obj_set_style_text_align(bk_ui->memorymode_memory_freeze_temp1, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_set_pos(bk_ui->memorymode_memory_freeze_temp1, 190, 252);
    lv_obj_set_size(bk_ui->memorymode_memory_freeze_temp1, 60, 44);

    // TextView: memory_defrost_temp1
    bk_ui->memorymode_memory_defrost_temp1 = lv_label_create(bk_ui->memorymode);
    lv_label_set_text(bk_ui->memorymode_memory_defrost_temp1, "");
    lv_obj_set_style_bg_opa(bk_ui->memorymode_memory_defrost_temp1, 0, 0);
    lv_obj_set_style_text_color(bk_ui->memorymode_memory_defrost_temp1, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->memorymode_memory_defrost_temp1, &lv_font_scdream_regular_26, 0);
    lv_obj_set_style_text_align(bk_ui->memorymode_memory_defrost_temp1, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_set_pos(bk_ui->memorymode_memory_defrost_temp1, 326, 236);
    lv_obj_set_size(bk_ui->memorymode_memory_defrost_temp1, 60, 40);

    // TextView: memory_defrost_hour1
    bk_ui->memorymode_memory_defrost_hour1 = lv_label_create(bk_ui->memorymode);
    lv_label_set_text(bk_ui->memorymode_memory_defrost_hour1, "");
    lv_obj_set_style_bg_opa(bk_ui->memorymode_memory_defrost_hour1, 0, 0);
    lv_obj_set_style_text_color(bk_ui->memorymode_memory_defrost_hour1, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->memorymode_memory_defrost_hour1, &lv_font_scdream_regular_26, 0);
    lv_obj_set_pos(bk_ui->memorymode_memory_defrost_hour1, 318, 267);
    lv_obj_set_size(bk_ui->memorymode_memory_defrost_hour1, 80, 40);

    // TextView: memory_defrost_min1
    bk_ui->memorymode_memory_defrost_min1 = lv_label_create(bk_ui->memorymode);
    lv_label_set_text(bk_ui->memorymode_memory_defrost_min1, "");
    lv_obj_set_style_bg_opa(bk_ui->memorymode_memory_defrost_min1, 0, 0);
    lv_obj_set_style_text_color(bk_ui->memorymode_memory_defrost_min1, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->memorymode_memory_defrost_min1, &lv_font_scdream_regular_26, 0);
    lv_obj_set_pos(bk_ui->memorymode_memory_defrost_min1, 382, 267);
    lv_obj_set_size(bk_ui->memorymode_memory_defrost_min1, 40, 40);

    // TextView: memory_fermentation1_temp1
    bk_ui->memorymode_memory_fermentation1_temp1 = lv_label_create(bk_ui->memorymode);
    lv_label_set_text(bk_ui->memorymode_memory_fermentation1_temp1, "");
    lv_obj_set_style_bg_opa(bk_ui->memorymode_memory_fermentation1_temp1, 0, 0);
    lv_obj_set_style_text_color(bk_ui->memorymode_memory_fermentation1_temp1, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->memorymode_memory_fermentation1_temp1, &lv_font_scdream_regular_26, 0);
    lv_obj_set_style_text_align(bk_ui->memorymode_memory_fermentation1_temp1, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_set_pos(bk_ui->memorymode_memory_fermentation1_temp1, 445, 236);
    lv_obj_set_size(bk_ui->memorymode_memory_fermentation1_temp1, 60, 40);

    // TextView: memory_fermentation1_humidity1
    bk_ui->memorymode_memory_fermentation1_humidity1 = lv_label_create(bk_ui->memorymode);
    lv_label_set_text(bk_ui->memorymode_memory_fermentation1_humidity1, "");
    lv_obj_set_style_bg_opa(bk_ui->memorymode_memory_fermentation1_humidity1, 0, 0);
    lv_obj_set_style_text_color(bk_ui->memorymode_memory_fermentation1_humidity1, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->memorymode_memory_fermentation1_humidity1, &lv_font_scdream_regular_26, 0);
    lv_obj_set_style_text_align(bk_ui->memorymode_memory_fermentation1_humidity1, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_set_pos(bk_ui->memorymode_memory_fermentation1_humidity1, 529, 236);
    lv_obj_set_size(bk_ui->memorymode_memory_fermentation1_humidity1, 40, 40);

    // TextView: memory_fermentation1_hour1
    bk_ui->memorymode_memory_fermentation1_hour1 = lv_label_create(bk_ui->memorymode);
    lv_label_set_text(bk_ui->memorymode_memory_fermentation1_hour1, "");
    lv_obj_set_style_bg_opa(bk_ui->memorymode_memory_fermentation1_hour1, 0, 0);
    lv_obj_set_style_text_color(bk_ui->memorymode_memory_fermentation1_hour1, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->memorymode_memory_fermentation1_hour1, &lv_font_scdream_regular_26, 0);
    lv_obj_set_pos(bk_ui->memorymode_memory_fermentation1_hour1, 473, 267);
    lv_obj_set_size(bk_ui->memorymode_memory_fermentation1_hour1, 40, 40);

    // TextView: memory_fermentation1_min1
    bk_ui->memorymode_memory_fermentation1_min1 = lv_label_create(bk_ui->memorymode);
    lv_label_set_text(bk_ui->memorymode_memory_fermentation1_min1, "");
    lv_obj_set_style_bg_opa(bk_ui->memorymode_memory_fermentation1_min1, 0, 0);
    lv_obj_set_style_text_color(bk_ui->memorymode_memory_fermentation1_min1, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->memorymode_memory_fermentation1_min1, &lv_font_scdream_regular_26, 0);
    lv_obj_set_pos(bk_ui->memorymode_memory_fermentation1_min1, 537, 267);
    lv_obj_set_size(bk_ui->memorymode_memory_fermentation1_min1, 40, 40);

    // TextView: memory_fermentation2_temp1
    bk_ui->memorymode_memory_fermentation2_temp1 = lv_label_create(bk_ui->memorymode);
    lv_label_set_text(bk_ui->memorymode_memory_fermentation2_temp1, "");
    lv_obj_set_style_bg_opa(bk_ui->memorymode_memory_fermentation2_temp1, 0, 0);
    lv_obj_set_style_text_color(bk_ui->memorymode_memory_fermentation2_temp1, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->memorymode_memory_fermentation2_temp1, &lv_font_scdream_regular_26, 0);
    lv_obj_set_style_text_align(bk_ui->memorymode_memory_fermentation2_temp1, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_set_pos(bk_ui->memorymode_memory_fermentation2_temp1, 615, 236);
    lv_obj_set_size(bk_ui->memorymode_memory_fermentation2_temp1, 60, 40);

    // TextView: memory_fermentation2_humidity1
    bk_ui->memorymode_memory_fermentation2_humidity1 = lv_label_create(bk_ui->memorymode);
    lv_label_set_text(bk_ui->memorymode_memory_fermentation2_humidity1, "");
    lv_obj_set_style_bg_opa(bk_ui->memorymode_memory_fermentation2_humidity1, 0, 0);
    lv_obj_set_style_text_color(bk_ui->memorymode_memory_fermentation2_humidity1, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->memorymode_memory_fermentation2_humidity1, &lv_font_scdream_regular_26, 0);
    lv_obj_set_style_text_align(bk_ui->memorymode_memory_fermentation2_humidity1, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_set_pos(bk_ui->memorymode_memory_fermentation2_humidity1, 699, 236);
    lv_obj_set_size(bk_ui->memorymode_memory_fermentation2_humidity1, 40, 40);

    // TextView: memory_fermentation2_hour1
    bk_ui->memorymode_memory_fermentation2_hour1 = lv_label_create(bk_ui->memorymode);
    lv_label_set_text(bk_ui->memorymode_memory_fermentation2_hour1, "");
    lv_obj_set_style_bg_opa(bk_ui->memorymode_memory_fermentation2_hour1, 0, 0);
    lv_obj_set_style_text_color(bk_ui->memorymode_memory_fermentation2_hour1, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->memorymode_memory_fermentation2_hour1, &lv_font_scdream_regular_26, 0);
    lv_obj_set_pos(bk_ui->memorymode_memory_fermentation2_hour1, 643, 267);
    lv_obj_set_size(bk_ui->memorymode_memory_fermentation2_hour1, 40, 40);

    // TextView: memory_fermentation2_min1
    bk_ui->memorymode_memory_fermentation2_min1 = lv_label_create(bk_ui->memorymode);
    lv_label_set_text(bk_ui->memorymode_memory_fermentation2_min1, "");
    lv_obj_set_style_bg_opa(bk_ui->memorymode_memory_fermentation2_min1, 0, 0);
    lv_obj_set_style_text_color(bk_ui->memorymode_memory_fermentation2_min1, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->memorymode_memory_fermentation2_min1, &lv_font_scdream_regular_26, 0);
    lv_obj_set_pos(bk_ui->memorymode_memory_fermentation2_min1, 707, 267);
    lv_obj_set_size(bk_ui->memorymode_memory_fermentation2_min1, 40, 40);

    // TextView: memory_completehour1
    bk_ui->memorymode_memory_completehour1 = lv_label_create(bk_ui->memorymode);
    lv_label_set_text(bk_ui->memorymode_memory_completehour1, "");
    lv_obj_set_style_bg_opa(bk_ui->memorymode_memory_completehour1, 0, 0);
    lv_obj_set_style_text_color(bk_ui->memorymode_memory_completehour1, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->memorymode_memory_completehour1, &lv_font_scdream_regular_26, 0);
    lv_obj_set_style_text_align(bk_ui->memorymode_memory_completehour1, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_set_pos(bk_ui->memorymode_memory_completehour1, 785, 251);
    lv_obj_set_size(bk_ui->memorymode_memory_completehour1, 40, 40);

    // TextView: memory_completemin1
    bk_ui->memorymode_memory_completemin1 = lv_label_create(bk_ui->memorymode);
    lv_label_set_text(bk_ui->memorymode_memory_completemin1, "");
    lv_obj_set_style_bg_opa(bk_ui->memorymode_memory_completemin1, 0, 0);
    lv_obj_set_style_text_color(bk_ui->memorymode_memory_completemin1, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->memorymode_memory_completemin1, &lv_font_scdream_regular_26, 0);
    lv_obj_set_pos(bk_ui->memorymode_memory_completemin1, 847, 251);
    lv_obj_set_size(bk_ui->memorymode_memory_completemin1, 40, 40);

    // ImageView: memory_check2im
    bk_ui->memorymode_memory_check2im = lv_image_create(bk_ui->memorymode);
    lv_obj_set_pos(bk_ui->memorymode_memory_check2im, 925, 244);
    lv_obj_set_size(bk_ui->memorymode_memory_check2im, 40, 40);

    // Button: memory_check2bt
    bk_ui->memorymode_memory_check2bt = lv_button_create(bk_ui->memorymode);
    lv_obj_add_flag(bk_ui->memorymode_memory_check2bt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->memorymode_memory_check2bt, memorymode_memory_check2bt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->memorymode_memory_check2bt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->memorymode_memory_check2bt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->memorymode_memory_check2bt, 0, 0);
    lv_obj_set_pos(bk_ui->memorymode_memory_check2bt, 925, 244);
    lv_obj_set_size(bk_ui->memorymode_memory_check2bt, 40, 40);

    // ImageView: memorybox2
    bk_ui->memorymode_memorybox2 = lv_image_create(bk_ui->memorymode);
    lv_obj_set_pos(bk_ui->memorymode_memorybox2, 12, 303);
    lv_obj_set_size(bk_ui->memorymode_memorybox2, 1000, 70);

    // TextView: memory_number2
    bk_ui->memorymode_memory_number2 = lv_label_create(bk_ui->memorymode);
    lv_label_set_text(bk_ui->memorymode_memory_number2, "");
    lv_obj_set_style_bg_opa(bk_ui->memorymode_memory_number2, 0, 0);
    lv_obj_set_style_text_color(bk_ui->memorymode_memory_number2, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->memorymode_memory_number2, &lv_font_scdream_regular_26, 0);
    lv_obj_set_pos(bk_ui->memorymode_memory_number2, 16+2, 314+10);
    lv_obj_set_size(bk_ui->memorymode_memory_number2, 60, 60);
    lv_obj_set_style_text_align(bk_ui->memorymode_memory_number2, LV_TEXT_ALIGN_CENTER, 0);

    // TextView: memory_Day_Period2
    bk_ui->memorymode_memory_Day_Period2 = lv_label_create(bk_ui->memorymode);
    lv_label_set_text(bk_ui->memorymode_memory_Day_Period2, "");
    lv_obj_set_style_bg_opa(bk_ui->memorymode_memory_Day_Period2, 0, 0);
    lv_obj_set_style_text_color(bk_ui->memorymode_memory_Day_Period2, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->memorymode_memory_Day_Period2, &lv_font_scdream_regular_26, 0);
    lv_obj_set_style_text_align(bk_ui->memorymode_memory_Day_Period2, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_set_pos(bk_ui->memorymode_memory_Day_Period2, 95, 324);
    lv_obj_set_size(bk_ui->memorymode_memory_Day_Period2, 40, 40);
    lv_label_set_long_mode(bk_ui->memorymode_memory_Day_Period2, LV_LABEL_LONG_CLIP);

    // TextView: memory_freeze_temp2
    bk_ui->memorymode_memory_freeze_temp2 = lv_label_create(bk_ui->memorymode);
    lv_label_set_text(bk_ui->memorymode_memory_freeze_temp2, "");
    lv_obj_set_style_bg_opa(bk_ui->memorymode_memory_freeze_temp2, 0, 0);
    lv_obj_set_style_text_color(bk_ui->memorymode_memory_freeze_temp2, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->memorymode_memory_freeze_temp2, &lv_font_scdream_regular_26, 0);
    lv_obj_set_style_text_align(bk_ui->memorymode_memory_freeze_temp2, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_set_pos(bk_ui->memorymode_memory_freeze_temp2, 190, 324);
    lv_obj_set_size(bk_ui->memorymode_memory_freeze_temp2, 60, 40);

    // TextView: memory_defrost_temp2
    bk_ui->memorymode_memory_defrost_temp2 = lv_label_create(bk_ui->memorymode);
    lv_label_set_text(bk_ui->memorymode_memory_defrost_temp2, "");
    lv_obj_set_style_bg_opa(bk_ui->memorymode_memory_defrost_temp2, 0, 0);
    lv_obj_set_style_text_color(bk_ui->memorymode_memory_defrost_temp2, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->memorymode_memory_defrost_temp2, &lv_font_scdream_regular_26, 0);
    lv_obj_set_style_text_align(bk_ui->memorymode_memory_defrost_temp2, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_set_pos(bk_ui->memorymode_memory_defrost_temp2, 326, 308);
    lv_obj_set_size(bk_ui->memorymode_memory_defrost_temp2, 60, 40);

    // TextView: memory_defrost_hour2
    bk_ui->memorymode_memory_defrost_hour2 = lv_label_create(bk_ui->memorymode);
    lv_label_set_text(bk_ui->memorymode_memory_defrost_hour2, "");
    lv_obj_set_style_bg_opa(bk_ui->memorymode_memory_defrost_hour2, 0, 0);
    lv_obj_set_style_text_color(bk_ui->memorymode_memory_defrost_hour2, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->memorymode_memory_defrost_hour2, &lv_font_scdream_regular_26, 0);
    lv_obj_set_pos(bk_ui->memorymode_memory_defrost_hour2, 318, 339);
    lv_obj_set_size(bk_ui->memorymode_memory_defrost_hour2, 80, 40);

    // TextView: memory_defrost_min2
    bk_ui->memorymode_memory_defrost_min2 = lv_label_create(bk_ui->memorymode);
    lv_label_set_text(bk_ui->memorymode_memory_defrost_min2, "");
    lv_obj_set_style_bg_opa(bk_ui->memorymode_memory_defrost_min2, 0, 0);
    lv_obj_set_style_text_color(bk_ui->memorymode_memory_defrost_min2, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->memorymode_memory_defrost_min2, &lv_font_scdream_regular_26, 0);
    lv_obj_set_pos(bk_ui->memorymode_memory_defrost_min2, 382, 339);
    lv_obj_set_size(bk_ui->memorymode_memory_defrost_min2, 40, 40);

    // TextView: memory_fermentation1_temp2
    bk_ui->memorymode_memory_fermentation1_temp2 = lv_label_create(bk_ui->memorymode);
    lv_label_set_text(bk_ui->memorymode_memory_fermentation1_temp2, "");
    lv_obj_set_style_bg_opa(bk_ui->memorymode_memory_fermentation1_temp2, 0, 0);
    lv_obj_set_style_text_color(bk_ui->memorymode_memory_fermentation1_temp2, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->memorymode_memory_fermentation1_temp2, &lv_font_scdream_regular_26, 0);
    lv_obj_set_style_text_align(bk_ui->memorymode_memory_fermentation1_temp2, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_set_pos(bk_ui->memorymode_memory_fermentation1_temp2, 445, 308);
    lv_obj_set_size(bk_ui->memorymode_memory_fermentation1_temp2, 60, 40);

    // TextView: memory_fermentation1_humidity2
    bk_ui->memorymode_memory_fermentation1_humidity2 = lv_label_create(bk_ui->memorymode);
    lv_label_set_text(bk_ui->memorymode_memory_fermentation1_humidity2, "");
    lv_obj_set_style_bg_opa(bk_ui->memorymode_memory_fermentation1_humidity2, 0, 0);
    lv_obj_set_style_text_color(bk_ui->memorymode_memory_fermentation1_humidity2, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->memorymode_memory_fermentation1_humidity2, &lv_font_scdream_regular_26, 0);
    lv_obj_set_style_text_align(bk_ui->memorymode_memory_fermentation1_humidity2, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_set_pos(bk_ui->memorymode_memory_fermentation1_humidity2, 529, 308);
    lv_obj_set_size(bk_ui->memorymode_memory_fermentation1_humidity2, 40, 40);

    // TextView: memory_fermentation1_hour2
    bk_ui->memorymode_memory_fermentation1_hour2 = lv_label_create(bk_ui->memorymode);
    lv_label_set_text(bk_ui->memorymode_memory_fermentation1_hour2, "");
    lv_obj_set_style_bg_opa(bk_ui->memorymode_memory_fermentation1_hour2, 0, 0);
    lv_obj_set_style_text_color(bk_ui->memorymode_memory_fermentation1_hour2, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->memorymode_memory_fermentation1_hour2, &lv_font_scdream_regular_26, 0);
    lv_obj_set_pos(bk_ui->memorymode_memory_fermentation1_hour2, 473, 339);
    lv_obj_set_size(bk_ui->memorymode_memory_fermentation1_hour2, 40, 40);

    // TextView: memory_fermentation1_min2
    bk_ui->memorymode_memory_fermentation1_min2 = lv_label_create(bk_ui->memorymode);
    lv_label_set_text(bk_ui->memorymode_memory_fermentation1_min2, "");
    lv_obj_set_style_bg_opa(bk_ui->memorymode_memory_fermentation1_min2, 0, 0);
    lv_obj_set_style_text_color(bk_ui->memorymode_memory_fermentation1_min2, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->memorymode_memory_fermentation1_min2, &lv_font_scdream_regular_26, 0);
    lv_obj_set_pos(bk_ui->memorymode_memory_fermentation1_min2, 537, 339);
    lv_obj_set_size(bk_ui->memorymode_memory_fermentation1_min2, 40, 40);

    // TextView: memory_fermentation2_temp2
    bk_ui->memorymode_memory_fermentation2_temp2 = lv_label_create(bk_ui->memorymode);
    lv_label_set_text(bk_ui->memorymode_memory_fermentation2_temp2, "");
    lv_obj_set_style_bg_opa(bk_ui->memorymode_memory_fermentation2_temp2, 0, 0);
    lv_obj_set_style_text_color(bk_ui->memorymode_memory_fermentation2_temp2, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->memorymode_memory_fermentation2_temp2, &lv_font_scdream_regular_26, 0);
    lv_obj_set_style_text_align(bk_ui->memorymode_memory_fermentation2_temp2, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_set_pos(bk_ui->memorymode_memory_fermentation2_temp2, 615, 308);
    lv_obj_set_size(bk_ui->memorymode_memory_fermentation2_temp2, 60, 40);

    // TextView: memory_fermentation2_humidity2
    bk_ui->memorymode_memory_fermentation2_humidity2 = lv_label_create(bk_ui->memorymode);
    lv_label_set_text(bk_ui->memorymode_memory_fermentation2_humidity2, "");
    lv_obj_set_style_bg_opa(bk_ui->memorymode_memory_fermentation2_humidity2, 0, 0);
    lv_obj_set_style_text_color(bk_ui->memorymode_memory_fermentation2_humidity2, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->memorymode_memory_fermentation2_humidity2, &lv_font_scdream_regular_26, 0);
    lv_obj_set_style_text_align(bk_ui->memorymode_memory_fermentation2_humidity2, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_set_pos(bk_ui->memorymode_memory_fermentation2_humidity2, 699, 308);
    lv_obj_set_size(bk_ui->memorymode_memory_fermentation2_humidity2, 40, 40);

    // TextView: memory_fermentation2_hour2
    bk_ui->memorymode_memory_fermentation2_hour2 = lv_label_create(bk_ui->memorymode);
    lv_label_set_text(bk_ui->memorymode_memory_fermentation2_hour2, "");
    lv_obj_set_style_bg_opa(bk_ui->memorymode_memory_fermentation2_hour2, 0, 0);
    lv_obj_set_style_text_color(bk_ui->memorymode_memory_fermentation2_hour2, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->memorymode_memory_fermentation2_hour2, &lv_font_scdream_regular_26, 0);
    lv_obj_set_pos(bk_ui->memorymode_memory_fermentation2_hour2, 643, 339);
    lv_obj_set_size(bk_ui->memorymode_memory_fermentation2_hour2, 40, 40);

    // TextView: memory_fermentation2_min2
    bk_ui->memorymode_memory_fermentation2_min2 = lv_label_create(bk_ui->memorymode);
    lv_label_set_text(bk_ui->memorymode_memory_fermentation2_min2, "");
    lv_obj_set_style_bg_opa(bk_ui->memorymode_memory_fermentation2_min2, 0, 0);
    lv_obj_set_style_text_color(bk_ui->memorymode_memory_fermentation2_min2, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->memorymode_memory_fermentation2_min2, &lv_font_scdream_regular_26, 0);
    lv_obj_set_pos(bk_ui->memorymode_memory_fermentation2_min2, 707, 339);
    lv_obj_set_size(bk_ui->memorymode_memory_fermentation2_min2, 40, 40);

    // TextView: memory_completehour2
    bk_ui->memorymode_memory_completehour2 = lv_label_create(bk_ui->memorymode);
    lv_label_set_text(bk_ui->memorymode_memory_completehour2, "");
    lv_obj_set_style_bg_opa(bk_ui->memorymode_memory_completehour2, 0, 0);
    lv_obj_set_style_text_color(bk_ui->memorymode_memory_completehour2, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->memorymode_memory_completehour2, &lv_font_scdream_regular_26, 0);
    lv_obj_set_style_text_align(bk_ui->memorymode_memory_completehour2, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_set_pos(bk_ui->memorymode_memory_completehour2, 785, 323);
    lv_obj_set_size(bk_ui->memorymode_memory_completehour2, 40, 40);

    // TextView: memory_completemin2
    bk_ui->memorymode_memory_completemin2 = lv_label_create(bk_ui->memorymode);
    lv_label_set_text(bk_ui->memorymode_memory_completemin2, "");
    lv_obj_set_style_bg_opa(bk_ui->memorymode_memory_completemin2, 0, 0);
    lv_obj_set_style_text_color(bk_ui->memorymode_memory_completemin2, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->memorymode_memory_completemin2, &lv_font_scdream_regular_26, 0);
    lv_obj_set_pos(bk_ui->memorymode_memory_completemin2, 847, 323);
    lv_obj_set_size(bk_ui->memorymode_memory_completemin2, 40, 40);

    // ImageView: memory_check3im
    bk_ui->memorymode_memory_check3im = lv_image_create(bk_ui->memorymode);
    lv_obj_set_pos(bk_ui->memorymode_memory_check3im, 925, 316);
    lv_obj_set_size(bk_ui->memorymode_memory_check3im, 40, 40);

    // Button: memory_check3bt
    bk_ui->memorymode_memory_check3bt = lv_button_create(bk_ui->memorymode);
    lv_obj_add_flag(bk_ui->memorymode_memory_check3bt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->memorymode_memory_check3bt, memorymode_memory_check3bt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->memorymode_memory_check3bt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->memorymode_memory_check3bt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->memorymode_memory_check3bt, 0, 0);
    lv_obj_set_pos(bk_ui->memorymode_memory_check3bt, 925, 316);
    lv_obj_set_size(bk_ui->memorymode_memory_check3bt, 40, 40);

    // ImageView: memorybox3
    bk_ui->memorymode_memorybox3 = lv_image_create(bk_ui->memorymode);
    lv_obj_set_pos(bk_ui->memorymode_memorybox3, 10, 375);
    lv_obj_set_size(bk_ui->memorymode_memorybox3, 1000, 70);

    // TextView: memory_number3
    bk_ui->memorymode_memory_number3 = lv_label_create(bk_ui->memorymode);
    lv_label_set_text(bk_ui->memorymode_memory_number3, "");
    lv_obj_set_style_bg_opa(bk_ui->memorymode_memory_number3, 0, 0);
    lv_obj_set_style_text_color(bk_ui->memorymode_memory_number3, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->memorymode_memory_number3, &lv_font_scdream_regular_26, 0);
    lv_obj_set_pos(bk_ui->memorymode_memory_number3, 16+2, 386+10);
    lv_obj_set_size(bk_ui->memorymode_memory_number3, 60, 60);
    lv_obj_set_style_text_align(bk_ui->memorymode_memory_number3, LV_TEXT_ALIGN_CENTER, 0);


    // TextView: memory_Day_Period3
    bk_ui->memorymode_memory_Day_Period3 = lv_label_create(bk_ui->memorymode);
    lv_label_set_text(bk_ui->memorymode_memory_Day_Period3, "");
    lv_obj_set_style_bg_opa(bk_ui->memorymode_memory_Day_Period3, 0, 0);
    lv_obj_set_style_text_color(bk_ui->memorymode_memory_Day_Period3, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->memorymode_memory_Day_Period3, &lv_font_scdream_regular_26, 0);
    lv_obj_set_style_text_align(bk_ui->memorymode_memory_Day_Period3, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_set_pos(bk_ui->memorymode_memory_Day_Period3, 95, 396);
    lv_obj_set_size(bk_ui->memorymode_memory_Day_Period3, 40, 40);
    lv_label_set_long_mode(bk_ui->memorymode_memory_Day_Period3, LV_LABEL_LONG_CLIP);

    // TextView: memory_freeze_temp3
    bk_ui->memorymode_memory_freeze_temp3 = lv_label_create(bk_ui->memorymode);
    lv_label_set_text(bk_ui->memorymode_memory_freeze_temp3, "");
    lv_obj_set_style_bg_opa(bk_ui->memorymode_memory_freeze_temp3, 0, 0);
    lv_obj_set_style_text_color(bk_ui->memorymode_memory_freeze_temp3, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->memorymode_memory_freeze_temp3, &lv_font_scdream_regular_26, 0);
    lv_obj_set_style_text_align(bk_ui->memorymode_memory_freeze_temp3, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_set_pos(bk_ui->memorymode_memory_freeze_temp3, 190, 396);
    lv_obj_set_size(bk_ui->memorymode_memory_freeze_temp3, 60, 40);

    // TextView: memory_defrost_temp3
    bk_ui->memorymode_memory_defrost_temp3 = lv_label_create(bk_ui->memorymode);
    lv_label_set_text(bk_ui->memorymode_memory_defrost_temp3, "");
    lv_obj_set_style_bg_opa(bk_ui->memorymode_memory_defrost_temp3, 0, 0);
    lv_obj_set_style_text_color(bk_ui->memorymode_memory_defrost_temp3, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->memorymode_memory_defrost_temp3, &lv_font_scdream_regular_26, 0);
    lv_obj_set_style_text_align(bk_ui->memorymode_memory_defrost_temp3, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_set_pos(bk_ui->memorymode_memory_defrost_temp3, 326, 380);
    lv_obj_set_size(bk_ui->memorymode_memory_defrost_temp3, 60, 40);

    // TextView: memory_defrost_hour3
    bk_ui->memorymode_memory_defrost_hour3 = lv_label_create(bk_ui->memorymode);
    lv_label_set_text(bk_ui->memorymode_memory_defrost_hour3, "");
    lv_obj_set_style_bg_opa(bk_ui->memorymode_memory_defrost_hour3, 0, 0);
    lv_obj_set_style_text_color(bk_ui->memorymode_memory_defrost_hour3, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->memorymode_memory_defrost_hour3, &lv_font_scdream_regular_26, 0);
    lv_obj_set_pos(bk_ui->memorymode_memory_defrost_hour3, 318, 411);
    lv_obj_set_size(bk_ui->memorymode_memory_defrost_hour3, 80, 40);

    // TextView: memory_defrost_min3
    bk_ui->memorymode_memory_defrost_min3 = lv_label_create(bk_ui->memorymode);
    lv_label_set_text(bk_ui->memorymode_memory_defrost_min3, "");
    lv_obj_set_style_bg_opa(bk_ui->memorymode_memory_defrost_min3, 0, 0);
    lv_obj_set_style_text_color(bk_ui->memorymode_memory_defrost_min3, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->memorymode_memory_defrost_min3, &lv_font_scdream_regular_26, 0);
    lv_obj_set_pos(bk_ui->memorymode_memory_defrost_min3, 382, 411);
    lv_obj_set_size(bk_ui->memorymode_memory_defrost_min3, 40, 40);

    // TextView: memory_fermentation1_temp3
    bk_ui->memorymode_memory_fermentation1_temp3 = lv_label_create(bk_ui->memorymode);
    lv_label_set_text(bk_ui->memorymode_memory_fermentation1_temp3, "");
    lv_obj_set_style_bg_opa(bk_ui->memorymode_memory_fermentation1_temp3, 0, 0);
    lv_obj_set_style_text_color(bk_ui->memorymode_memory_fermentation1_temp3, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->memorymode_memory_fermentation1_temp3, &lv_font_scdream_regular_26, 0);
    lv_obj_set_style_text_align(bk_ui->memorymode_memory_fermentation1_temp3, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_set_pos(bk_ui->memorymode_memory_fermentation1_temp3, 445, 380);
    lv_obj_set_size(bk_ui->memorymode_memory_fermentation1_temp3, 60, 40);

    // TextView: memory_fermentation1_humidity3
    bk_ui->memorymode_memory_fermentation1_humidity3 = lv_label_create(bk_ui->memorymode);
    lv_label_set_text(bk_ui->memorymode_memory_fermentation1_humidity3, "");
    lv_obj_set_style_bg_opa(bk_ui->memorymode_memory_fermentation1_humidity3, 0, 0);
    lv_obj_set_style_text_color(bk_ui->memorymode_memory_fermentation1_humidity3, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->memorymode_memory_fermentation1_humidity3, &lv_font_scdream_regular_26, 0);
    lv_obj_set_style_text_align(bk_ui->memorymode_memory_fermentation1_humidity3, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_set_pos(bk_ui->memorymode_memory_fermentation1_humidity3, 529, 380);
    lv_obj_set_size(bk_ui->memorymode_memory_fermentation1_humidity3, 40, 40);

    // TextView: memory_fermentation1_hour3
    bk_ui->memorymode_memory_fermentation1_hour3 = lv_label_create(bk_ui->memorymode);
    lv_label_set_text(bk_ui->memorymode_memory_fermentation1_hour3, "");
    lv_obj_set_style_bg_opa(bk_ui->memorymode_memory_fermentation1_hour3, 0, 0);
    lv_obj_set_style_text_color(bk_ui->memorymode_memory_fermentation1_hour3, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->memorymode_memory_fermentation1_hour3, &lv_font_scdream_regular_26, 0);
    lv_obj_set_pos(bk_ui->memorymode_memory_fermentation1_hour3, 473, 411);
    lv_obj_set_size(bk_ui->memorymode_memory_fermentation1_hour3, 40, 40);

    // TextView: memory_fermentation1_min3
    bk_ui->memorymode_memory_fermentation1_min3 = lv_label_create(bk_ui->memorymode);
    lv_label_set_text(bk_ui->memorymode_memory_fermentation1_min3, "");
    lv_obj_set_style_bg_opa(bk_ui->memorymode_memory_fermentation1_min3, 0, 0);
    lv_obj_set_style_text_color(bk_ui->memorymode_memory_fermentation1_min3, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->memorymode_memory_fermentation1_min3, &lv_font_scdream_regular_26, 0);
    lv_obj_set_pos(bk_ui->memorymode_memory_fermentation1_min3, 537, 411);
    lv_obj_set_size(bk_ui->memorymode_memory_fermentation1_min3, 40, 40);

    // TextView: memory_fermentation2_temp3
    bk_ui->memorymode_memory_fermentation2_temp3 = lv_label_create(bk_ui->memorymode);
    lv_label_set_text(bk_ui->memorymode_memory_fermentation2_temp3, "");
    lv_obj_set_style_bg_opa(bk_ui->memorymode_memory_fermentation2_temp3, 0, 0);
    lv_obj_set_style_text_color(bk_ui->memorymode_memory_fermentation2_temp3, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->memorymode_memory_fermentation2_temp3, &lv_font_scdream_regular_26, 0);
    lv_obj_set_style_text_align(bk_ui->memorymode_memory_fermentation2_temp3, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_set_pos(bk_ui->memorymode_memory_fermentation2_temp3, 615, 380);
    lv_obj_set_size(bk_ui->memorymode_memory_fermentation2_temp3, 60, 40);

    // TextView: memory_fermentation2_humidity3
    bk_ui->memorymode_memory_fermentation2_humidity3 = lv_label_create(bk_ui->memorymode);
    lv_label_set_text(bk_ui->memorymode_memory_fermentation2_humidity3, "");
    lv_obj_set_style_bg_opa(bk_ui->memorymode_memory_fermentation2_humidity3, 0, 0);
    lv_obj_set_style_text_color(bk_ui->memorymode_memory_fermentation2_humidity3, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->memorymode_memory_fermentation2_humidity3, &lv_font_scdream_regular_26, 0);
    lv_obj_set_style_text_align(bk_ui->memorymode_memory_fermentation2_humidity3, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_set_pos(bk_ui->memorymode_memory_fermentation2_humidity3, 699, 380);
    lv_obj_set_size(bk_ui->memorymode_memory_fermentation2_humidity3, 40, 40);

    // TextView: memory_fermentation2_hour3
    bk_ui->memorymode_memory_fermentation2_hour3 = lv_label_create(bk_ui->memorymode);
    lv_label_set_text(bk_ui->memorymode_memory_fermentation2_hour3, "");
    lv_obj_set_style_bg_opa(bk_ui->memorymode_memory_fermentation2_hour3, 0, 0);
    lv_obj_set_style_text_color(bk_ui->memorymode_memory_fermentation2_hour3, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->memorymode_memory_fermentation2_hour3, &lv_font_scdream_regular_26, 0);
    lv_obj_set_pos(bk_ui->memorymode_memory_fermentation2_hour3, 643, 411);
    lv_obj_set_size(bk_ui->memorymode_memory_fermentation2_hour3, 40, 40);

    // TextView: memory_fermentation2_min3
    bk_ui->memorymode_memory_fermentation2_min3 = lv_label_create(bk_ui->memorymode);
    lv_label_set_text(bk_ui->memorymode_memory_fermentation2_min3, "");
    lv_obj_set_style_bg_opa(bk_ui->memorymode_memory_fermentation2_min3, 0, 0);
    lv_obj_set_style_text_color(bk_ui->memorymode_memory_fermentation2_min3, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->memorymode_memory_fermentation2_min3, &lv_font_scdream_regular_26, 0);
    lv_obj_set_pos(bk_ui->memorymode_memory_fermentation2_min3, 707, 406);
    lv_obj_set_size(bk_ui->memorymode_memory_fermentation2_min3, 40, 40);

    // TextView: memory_completehour3
    bk_ui->memorymode_memory_completehour3 = lv_label_create(bk_ui->memorymode);
    lv_label_set_text(bk_ui->memorymode_memory_completehour3, "");
    lv_obj_set_style_bg_opa(bk_ui->memorymode_memory_completehour3, 0, 0);
    lv_obj_set_style_text_color(bk_ui->memorymode_memory_completehour3, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->memorymode_memory_completehour3, &lv_font_scdream_regular_26, 0);
    lv_obj_set_style_text_align(bk_ui->memorymode_memory_completehour3, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_set_pos(bk_ui->memorymode_memory_completehour3, 785, 395);
    lv_obj_set_size(bk_ui->memorymode_memory_completehour3, 40, 40);

    // TextView: memory_completemin3
    bk_ui->memorymode_memory_completemin3 = lv_label_create(bk_ui->memorymode);
    lv_label_set_text(bk_ui->memorymode_memory_completemin3, "");
    lv_obj_set_style_bg_opa(bk_ui->memorymode_memory_completemin3, 0, 0);
    lv_obj_set_style_text_color(bk_ui->memorymode_memory_completemin3, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->memorymode_memory_completemin3, &lv_font_scdream_regular_26, 0);
    lv_obj_set_pos(bk_ui->memorymode_memory_completemin3, 847, 395);
    lv_obj_set_size(bk_ui->memorymode_memory_completemin3, 40, 40);

    // ImageView: memory_check4im
    bk_ui->memorymode_memory_check4im = lv_image_create(bk_ui->memorymode);
    lv_obj_set_pos(bk_ui->memorymode_memory_check4im, 925, 388);
    lv_obj_set_size(bk_ui->memorymode_memory_check4im, 40, 40);

    // Button: memory_check4bt
    bk_ui->memorymode_memory_check4bt = lv_button_create(bk_ui->memorymode);
    lv_obj_add_flag(bk_ui->memorymode_memory_check4bt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->memorymode_memory_check4bt, memorymode_memory_check4bt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->memorymode_memory_check4bt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->memorymode_memory_check4bt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->memorymode_memory_check4bt, 0, 0);
    lv_obj_set_pos(bk_ui->memorymode_memory_check4bt, 925, 388);
    lv_obj_set_size(bk_ui->memorymode_memory_check4bt, 40, 40);

    // ImageView: imageview81
    bk_ui->memorymode_imageview81 = lv_image_create(bk_ui->memorymode);
    _img_set_src_deferred(bk_ui->memorymode_imageview81, "/images/memory_left.png");
    lv_obj_set_pos(bk_ui->memorymode_imageview81, 296, 468);
    lv_obj_set_size(bk_ui->memorymode_imageview81, 18, 28);

    // Button: memoryleftbt
    bk_ui->memorymode_memoryleftbt = lv_button_create(bk_ui->memorymode);
    lv_obj_add_flag(bk_ui->memorymode_memoryleftbt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->memorymode_memoryleftbt, memorymode_memoryleftbt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->memorymode_memoryleftbt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->memorymode_memoryleftbt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->memorymode_memoryleftbt, 0, 0);
    lv_obj_set_pos(bk_ui->memorymode_memoryleftbt, 276, 453);
    lv_obj_set_size(bk_ui->memorymode_memoryleftbt, 58, 58);

    // ImageView: memory1im  (src set by _refresh_display in load_event_cb)
    bk_ui->memorymode_memory1im = lv_image_create(bk_ui->memorymode);
    lv_obj_set_pos(bk_ui->memorymode_memory1im, 375, 465);
    lv_obj_set_size(bk_ui->memorymode_memory1im, 22, 39);

    // Button: memory1bt
    bk_ui->memorymode_memory1bt = lv_button_create(bk_ui->memorymode);
    lv_obj_add_flag(bk_ui->memorymode_memory1bt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->memorymode_memory1bt, memorymode_memory1bt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->memorymode_memory1bt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->memorymode_memory1bt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->memorymode_memory1bt, 0, 0);
    lv_obj_set_pos(bk_ui->memorymode_memory1bt, 358, 455);
    lv_obj_set_size(bk_ui->memorymode_memory1bt, 60, 60);

    // ImageView: memory2im
    bk_ui->memorymode_memory2im = lv_image_create(bk_ui->memorymode);
    lv_obj_set_pos(bk_ui->memorymode_memory2im, 501, 465);
    lv_obj_set_size(bk_ui->memorymode_memory2im, 22, 39);

    // Button: memory2bt
    bk_ui->memorymode_memory2bt = lv_button_create(bk_ui->memorymode);
    lv_obj_add_flag(bk_ui->memorymode_memory2bt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->memorymode_memory2bt, memorymode_memory2bt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->memorymode_memory2bt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->memorymode_memory2bt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->memorymode_memory2bt, 0, 0);
    lv_obj_set_pos(bk_ui->memorymode_memory2bt, 480, 455);
    lv_obj_set_size(bk_ui->memorymode_memory2bt, 60, 60);

    // ImageView: memory3im
    bk_ui->memorymode_memory3im = lv_image_create(bk_ui->memorymode);
    lv_obj_set_pos(bk_ui->memorymode_memory3im, 627, 465);
    lv_obj_set_size(bk_ui->memorymode_memory3im, 21, 38);

    // Button: memory3bt
    bk_ui->memorymode_memory3bt = lv_button_create(bk_ui->memorymode);
    lv_obj_add_flag(bk_ui->memorymode_memory3bt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->memorymode_memory3bt, memorymode_memory3bt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->memorymode_memory3bt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->memorymode_memory3bt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->memorymode_memory3bt, 0, 0);
    lv_obj_set_pos(bk_ui->memorymode_memory3bt, 607, 455);
    lv_obj_set_size(bk_ui->memorymode_memory3bt, 60, 60);

    // ImageView: imageview89
    bk_ui->memorymode_imageview89 = lv_image_create(bk_ui->memorymode);
    _img_set_src_deferred(bk_ui->memorymode_imageview89, "/images/memory_right.png");
    lv_obj_set_pos(bk_ui->memorymode_imageview89, 710, 468);
    lv_obj_set_size(bk_ui->memorymode_imageview89, 18, 28);

    // Button: memoryrightbt
    bk_ui->memorymode_memoryrightbt = lv_button_create(bk_ui->memorymode);
    lv_obj_add_flag(bk_ui->memorymode_memoryrightbt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->memorymode_memoryrightbt, memorymode_memoryrightbt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->memorymode_memoryrightbt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->memorymode_memoryrightbt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->memorymode_memoryrightbt, 0, 0);
    lv_obj_set_pos(bk_ui->memorymode_memoryrightbt, 690, 453);
    lv_obj_set_size(bk_ui->memorymode_memoryrightbt, 58, 58);

    // Button: okbt
    bk_ui->memorymode_okbt = lv_button_create(bk_ui->memorymode);
    lv_obj_add_flag(bk_ui->memorymode_okbt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->memorymode_okbt, memorymode_okbt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->memorymode_okbt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->memorymode_okbt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->memorymode_okbt, 0, 0);
    lv_obj_set_pos(bk_ui->memorymode_okbt, 849, 447);
    lv_obj_set_size(bk_ui->memorymode_okbt, 164, 74);

    // ImageView: imageview92
    bk_ui->memorymode_imageview92 = lv_image_create(bk_ui->memorymode);
    _img_set_src_deferred(bk_ui->memorymode_imageview92, "/images/ok.png");
    lv_obj_set_pos(bk_ui->memorymode_imageview92, 849, 447);
    lv_obj_set_size(bk_ui->memorymode_imageview92, 164, 74);

    // ImageView: deleteim
    bk_ui->memorymode_deleteim = lv_image_create(bk_ui->memorymode);
    _img_set_src_deferred(bk_ui->memorymode_deleteim, "/images/delete.png");
    lv_obj_set_pos(bk_ui->memorymode_deleteim, 825, 13);
    lv_obj_set_size(bk_ui->memorymode_deleteim, 179, 74);

    // Button: deletebt
    bk_ui->memorymode_deletebt = lv_button_create(bk_ui->memorymode);
    lv_obj_add_flag(bk_ui->memorymode_deletebt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->memorymode_deletebt, memorymode_deletebt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->memorymode_deletebt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->memorymode_deletebt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->memorymode_deletebt, 0, 0);
    lv_obj_set_pos(bk_ui->memorymode_deletebt, 825, 13);
    lv_obj_set_size(bk_ui->memorymode_deletebt, 179, 74);

}

rendererFuncStatus_t init_page_memorymode_with_step(bk_lv_ui_t *bk_ui)
{
    return RENDERER_FUNC_DONE; // test stub
    // return RENDERER_FUNC_FAILED;
}