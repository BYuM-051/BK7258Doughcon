#include "lvgl.h"
#include "beken_ui.h"
#include "custom_func.h"
#include "event_runtime.h"
#include "ui_lang.h"
#include <stdio.h>
#include <string.h>

extern bk_lv_ui_t bk_lv_tool_ui;
extern void manualmodestart_backbt_event_cb(lv_event_t *e);
extern void manualmodestart_startbt_event_cb(lv_event_t *e);
extern void manualmodestart_manual_freeze_temp_bt_event_cb(lv_event_t *e);
extern void manualmodestart_manual_defrost_temp_bt_event_cb(lv_event_t *e);
extern void manualmodestart_manual_fermentation_temp_bt_event_cb(lv_event_t *e);
extern void manualmodestart_manual_fermentation_humidity_bt_event_cb(lv_event_t *e);
extern void manualmodestart_keypad_event_cb(lv_event_t *e);
extern void manualmodestart_keypadhide_event_cb(lv_event_t *e);
extern void manualmodestart_load_event_cb(lv_event_t *e);

void destroy_page_manualmodestart(bk_lv_ui_t *bk_ui)
{
    if (bk_ui == NULL) {
        return;
    }
    if (bk_ui->manualmodestart != NULL) {
        lv_obj_del(bk_ui->manualmodestart);
        bk_ui->manualmodestart = NULL;
    }
}

void init_page_manualmodestart(bk_lv_ui_t * bk_ui) {
    if (bk_ui->manualmodestart != NULL && lv_obj_is_valid(bk_ui->manualmodestart)) {
        destroy_page_manualmodestart(bk_ui);
    }

    /* 오브젝트를 새로 만드므로 ui_lang 캐시를 무효화 — 다음 ui_lang_apply_manualmodestart()가
     * 이전 세션과 언어/단위/모드가 우연히 같아도 반드시 새 이미지를 채우게 함 */
    ui_lang_reset_manualmodestart_cache();

    // Clear lazily-created child pointers (stale after destroy+re-init)
    bk_ui->manualmodestart_keypadbaseim            = NULL;
    bk_ui->manualmodestart_keypad1                 = NULL;
    bk_ui->manualmodestart_keypad1_im              = NULL;
    bk_ui->manualmodestart_keypad2                 = NULL;
    bk_ui->manualmodestart_keypad2_im              = NULL;
    bk_ui->manualmodestart_keypad3                 = NULL;
    bk_ui->manualmodestart_keypad3_im              = NULL;
    bk_ui->manualmodestart_keypad4                 = NULL;
    bk_ui->manualmodestart_keypad4_im              = NULL;
    bk_ui->manualmodestart_keypad5                 = NULL;
    bk_ui->manualmodestart_keypad5_im              = NULL;
    bk_ui->manualmodestart_keypad6                 = NULL;
    bk_ui->manualmodestart_keypad6_im              = NULL;
    bk_ui->manualmodestart_keypad7                 = NULL;
    bk_ui->manualmodestart_keypad7_im              = NULL;
    bk_ui->manualmodestart_keypad8                 = NULL;
    bk_ui->manualmodestart_keypad8_im              = NULL;
    bk_ui->manualmodestart_keypad9                 = NULL;
    bk_ui->manualmodestart_keypad9_im              = NULL;
    bk_ui->manualmodestart_keypad0                 = NULL;
    bk_ui->manualmodestart_keypad0_im              = NULL;
    bk_ui->manualmodestart_keypadminor             = NULL;
    bk_ui->manualmodestart_keypadminor_im          = NULL;
    bk_ui->manualmodestart_keypadbackspace         = NULL;
    bk_ui->manualmodestart_keypadbackspace_im      = NULL;
    bk_ui->manualmodestart_keypadhide              = NULL;
    bk_ui->manualmodestart_keypadhide_im           = NULL;
    bk_ui->manualmodestart_manual_fermentation_temp_underbar     = NULL;
    bk_ui->manualmodestart_manual_fermentation_humidity_underbar = NULL;
    bk_ui->manualmodestart_manual_freeze_temp_underbar           = NULL;
    bk_ui->manualmodestart_manual_defrost_temp_underbar          = NULL;
    bk_ui->manualmodestart_manual_circle_basic                   = NULL;
    bk_ui->manualmodestart_manual_circle_gif                     = NULL;
    bk_ui->manualmodestart_manual_txt_basic                      = NULL;
    bk_ui->manualmodestart_manual_gif                            = NULL;
    bk_ui->manualmodestart_manual_gif_basic                      = NULL;
    bk_ui->manualmodestart_run_arc                               = NULL;

    bk_ui->manualmodestart = lv_obj_create(NULL);
    lv_obj_set_size(bk_ui->manualmodestart, 1024, 600);
    lv_obj_set_scrollbar_mode(bk_ui->manualmodestart, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_event_cb(bk_ui->manualmodestart, manualmodestart_load_event_cb, LV_EVENT_ALL, NULL);

    // ImageView: bg (must be first child — lowest z-order)
    bk_ui->manualmodestart_bg = lv_image_create(bk_ui->manualmodestart);
    lv_obj_set_pos(bk_ui->manualmodestart_bg, 0, 0);
    lv_obj_set_size(bk_ui->manualmodestart_bg, 1024, 540);
    lv_obj_remove_flag(bk_ui->manualmodestart_bg, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(bk_ui->manualmodestart_bg, LV_OBJ_FLAG_HIDDEN);

    // ImageView: title
    bk_ui->manualmodestart_title = lv_image_create(bk_ui->manualmodestart);
    _img_set_src_timed(bk_ui->manualmodestart_title, "/images/manualmode_title.png");
    lv_obj_set_pos(bk_ui->manualmodestart_title, 0, 10);
    lv_obj_set_size(bk_ui->manualmodestart_title, 380, 80);
    lv_image_set_inner_align(bk_ui->manualmodestart_title, LV_IMAGE_ALIGN_TOP_LEFT);

    // TextView: over_time_korea
    bk_ui->manualmodestart_over_time_korea = lv_label_create(bk_ui->manualmodestart);
    lv_label_set_text(bk_ui->manualmodestart_over_time_korea, "");
    lv_obj_set_style_bg_opa(bk_ui->manualmodestart_over_time_korea, 0, 0);
    lv_obj_set_style_text_color(bk_ui->manualmodestart_over_time_korea, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(bk_ui->manualmodestart_over_time_korea, &lv_font_scdream_regular_26, 0);
    lv_obj_set_style_text_align(bk_ui->manualmodestart_over_time_korea, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_add_flag(bk_ui->manualmodestart_over_time_korea, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_pos(bk_ui->manualmodestart_over_time_korea, 147, 220+5);
    lv_obj_set_size(bk_ui->manualmodestart_over_time_korea, 40, 30);

    // TextView: over_time_china
    bk_ui->manualmodestart_over_time_china = lv_label_create(bk_ui->manualmodestart);
    lv_label_set_text(bk_ui->manualmodestart_over_time_china, "");
    lv_obj_set_style_bg_opa(bk_ui->manualmodestart_over_time_china, 0, 0);
    lv_obj_set_style_text_color(bk_ui->manualmodestart_over_time_china, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(bk_ui->manualmodestart_over_time_china, &lv_font_scdream_regular_26, 0);
    lv_obj_set_style_text_align(bk_ui->manualmodestart_over_time_china, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_add_flag(bk_ui->manualmodestart_over_time_china, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_pos(bk_ui->manualmodestart_over_time_china, 135, 218+5);
    lv_obj_set_size(bk_ui->manualmodestart_over_time_china, 40, 35);

    // TextView: over_time_english
    bk_ui->manualmodestart_over_time_english = lv_label_create(bk_ui->manualmodestart);
    lv_label_set_text(bk_ui->manualmodestart_over_time_english, "");
    lv_obj_set_style_bg_opa(bk_ui->manualmodestart_over_time_english, 0, 0);
    lv_obj_set_style_text_color(bk_ui->manualmodestart_over_time_english, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(bk_ui->manualmodestart_over_time_english, &lv_font_scdream_regular_22, 0);
    lv_obj_set_style_text_align(bk_ui->manualmodestart_over_time_english, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_add_flag(bk_ui->manualmodestart_over_time_english, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_pos(bk_ui->manualmodestart_over_time_english, 73, 211+7);
    lv_obj_set_size(bk_ui->manualmodestart_over_time_english, 30, 30);

    // Button: backbt
    bk_ui->manualmodestart_backbt = lv_button_create(bk_ui->manualmodestart);
    lv_obj_add_flag(bk_ui->manualmodestart_backbt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->manualmodestart_backbt, manualmodestart_backbt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->manualmodestart_backbt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->manualmodestart_backbt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->manualmodestart_backbt, 0, 0);
    lv_obj_set_pos(bk_ui->manualmodestart_backbt, 13, 445);
    lv_obj_set_size(bk_ui->manualmodestart_backbt, 179, 74);

    // ImageView: backim
    bk_ui->manualmodestart_backim = lv_image_create(bk_ui->manualmodestart);
    _img_set_src_timed(bk_ui->manualmodestart_backim, "/images/exit_bt.png");
    lv_obj_set_pos(bk_ui->manualmodestart_backim, 13, 445);
    lv_obj_set_size(bk_ui->manualmodestart_backim, 179, 74);

    // Button: startbt
    bk_ui->manualmodestart_startbt = lv_button_create(bk_ui->manualmodestart);
    lv_obj_add_flag(bk_ui->manualmodestart_startbt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->manualmodestart_startbt, manualmodestart_startbt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->manualmodestart_startbt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->manualmodestart_startbt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->manualmodestart_startbt, 0, 0);
    lv_obj_set_pos(bk_ui->manualmodestart_startbt, 847, 445);
    lv_obj_set_size(bk_ui->manualmodestart_startbt, 164, 74);

    // ImageView: startim
    bk_ui->manualmodestart_startim = lv_image_create(bk_ui->manualmodestart);
    _img_set_src_timed(bk_ui->manualmodestart_startim, "/images/start_bt.png");
    lv_obj_set_pos(bk_ui->manualmodestart_startim, 847, 445);
    lv_obj_set_size(bk_ui->manualmodestart_startim, 164, 74);

    // ImageView: tempbox
    bk_ui->manualmodestart_tempbox = lv_image_create(bk_ui->manualmodestart);
    _img_set_src_deferred(bk_ui->manualmodestart_tempbox, "/images/tempbox.jpg");
    lv_obj_add_flag(bk_ui->manualmodestart_tempbox, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_pos(bk_ui->manualmodestart_tempbox, 222, 445);
    lv_obj_set_size(bk_ui->manualmodestart_tempbox, 580, 74);

    // TextView: tempbox_current_temp
    bk_ui->manualmodestart_tempbox_current_temp = lv_label_create(bk_ui->manualmodestart);
    lv_label_set_text(bk_ui->manualmodestart_tempbox_current_temp, "");
    lv_obj_set_style_bg_opa(bk_ui->manualmodestart_tempbox_current_temp, 0, 0);
    lv_obj_set_style_text_color(bk_ui->manualmodestart_tempbox_current_temp, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->manualmodestart_tempbox_current_temp, &lv_font_scdream_regular_40, 0);
    lv_obj_set_style_text_align(bk_ui->manualmodestart_tempbox_current_temp, LV_TEXT_ALIGN_RIGHT, 0);
    /* 세 자리 값(예: 111°F)이 폭 부족으로 다음 줄로 줄바꿈되던 문제 —
     * 줄바꿈 없이 한 줄 유지 + 박스 확장(우측 정렬 기준 좌측으로 확장) */
    lv_label_set_long_mode(bk_ui->manualmodestart_tempbox_current_temp, LV_LABEL_LONG_CLIP);
    lv_obj_add_flag(bk_ui->manualmodestart_tempbox_current_temp, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_pos(bk_ui->manualmodestart_tempbox_current_temp, 376-20, 463);
    lv_obj_set_size(bk_ui->manualmodestart_tempbox_current_temp, 110, 50);

    // TextView: tempbox_current_humidity
    bk_ui->manualmodestart_tempbox_current_humidity = lv_label_create(bk_ui->manualmodestart);
    lv_label_set_text(bk_ui->manualmodestart_tempbox_current_humidity, "");
    lv_obj_set_style_bg_opa(bk_ui->manualmodestart_tempbox_current_humidity, 0, 0);
    lv_obj_set_style_text_color(bk_ui->manualmodestart_tempbox_current_humidity, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->manualmodestart_tempbox_current_humidity, &lv_font_scdream_regular_40, 0);
    lv_obj_set_style_text_align(bk_ui->manualmodestart_tempbox_current_humidity, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_add_flag(bk_ui->manualmodestart_tempbox_current_humidity, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_pos(bk_ui->manualmodestart_tempbox_current_humidity, 655, 463);
    lv_obj_set_size(bk_ui->manualmodestart_tempbox_current_humidity, 90, 50);

    // circle images: lazy-created in manualmodestart_load_event_cb on first load

    // TextView: manual_freeze_temp_txt
    bk_ui->manualmodestart_manual_freeze_temp_txt = lv_label_create(bk_ui->manualmodestart);
    lv_label_set_text(bk_ui->manualmodestart_manual_freeze_temp_txt, "");
    lv_obj_set_style_bg_opa(bk_ui->manualmodestart_manual_freeze_temp_txt, 0, 0);
    lv_obj_set_style_text_color(bk_ui->manualmodestart_manual_freeze_temp_txt, lv_color_hex(0x162A9E), 0);
    lv_obj_set_style_text_font(bk_ui->manualmodestart_manual_freeze_temp_txt, &lv_font_scdream_regular_66, 0);
    lv_obj_set_style_text_align(bk_ui->manualmodestart_manual_freeze_temp_txt, LV_TEXT_ALIGN_RIGHT,0);
    /* 세 자리 값("-25" 등)이 2단으로 줄바꿈되던 문제 — 줄바꿈 없이 한 줄 유지 + 박스 확장 */
    lv_label_set_long_mode(bk_ui->manualmodestart_manual_freeze_temp_txt, LV_LABEL_LONG_CLIP);
    lv_obj_add_flag(bk_ui->manualmodestart_manual_freeze_temp_txt, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_pos(bk_ui->manualmodestart_manual_freeze_temp_txt, 430, 278);
    lv_obj_set_size(bk_ui->manualmodestart_manual_freeze_temp_txt, 120, 75);

    // TextView: manual_defrost_temp_txt
    bk_ui->manualmodestart_manual_defrost_temp_txt = lv_label_create(bk_ui->manualmodestart);
    lv_label_set_text(bk_ui->manualmodestart_manual_defrost_temp_txt, "");
    lv_obj_set_style_bg_opa(bk_ui->manualmodestart_manual_defrost_temp_txt, 0, 0);
    lv_obj_set_style_text_color(bk_ui->manualmodestart_manual_defrost_temp_txt, lv_color_hex(0x53BAE4), 0);
    lv_obj_set_style_text_font(bk_ui->manualmodestart_manual_defrost_temp_txt, &lv_font_scdream_regular_66, 0);
    lv_obj_set_style_text_align(bk_ui->manualmodestart_manual_defrost_temp_txt, LV_TEXT_ALIGN_RIGHT,0);
    lv_label_set_long_mode(bk_ui->manualmodestart_manual_defrost_temp_txt, LV_LABEL_LONG_CLIP);
    lv_obj_add_flag(bk_ui->manualmodestart_manual_defrost_temp_txt, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_pos(bk_ui->manualmodestart_manual_defrost_temp_txt, 430, 278);
    lv_obj_set_size(bk_ui->manualmodestart_manual_defrost_temp_txt, 120, 75);

    // TextView: manual_fermentation_temp_txt
    bk_ui->manualmodestart_manual_fermentation_temp_txt = lv_label_create(bk_ui->manualmodestart);
    lv_label_set_text(bk_ui->manualmodestart_manual_fermentation_temp_txt, "");
    lv_obj_set_style_bg_opa(bk_ui->manualmodestart_manual_fermentation_temp_txt, 0, 0);
    lv_obj_set_style_text_color(bk_ui->manualmodestart_manual_fermentation_temp_txt, lv_color_hex(0xD1232A), 0);
    lv_obj_set_style_text_font(bk_ui->manualmodestart_manual_fermentation_temp_txt, &lv_font_scdream_regular_49, 0);
    lv_obj_set_style_text_align(bk_ui->manualmodestart_manual_fermentation_temp_txt, LV_TEXT_ALIGN_RIGHT,0);
    lv_label_set_long_mode(bk_ui->manualmodestart_manual_fermentation_temp_txt, LV_LABEL_LONG_CLIP);
    lv_obj_add_flag(bk_ui->manualmodestart_manual_fermentation_temp_txt, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_pos(bk_ui->manualmodestart_manual_fermentation_temp_txt, 404,278);
    lv_obj_set_size(bk_ui->manualmodestart_manual_fermentation_temp_txt, 90, 75);

    // TextView: manual_fermentation_humidity_txt
    bk_ui->manualmodestart_manual_fermentation_humidity_txt = lv_label_create(bk_ui->manualmodestart);
    lv_label_set_text(bk_ui->manualmodestart_manual_fermentation_humidity_txt, "");
    lv_obj_set_style_bg_opa(bk_ui->manualmodestart_manual_fermentation_humidity_txt, 0, 0);
    lv_obj_set_style_text_color(bk_ui->manualmodestart_manual_fermentation_humidity_txt, lv_color_hex(0xD1232A), 0);
    lv_obj_set_style_text_font(bk_ui->manualmodestart_manual_fermentation_humidity_txt, &lv_font_scdream_regular_49, 0);
    lv_obj_set_style_text_align(bk_ui->manualmodestart_manual_fermentation_humidity_txt, LV_TEXT_ALIGN_RIGHT,0);
    lv_label_set_long_mode(bk_ui->manualmodestart_manual_fermentation_humidity_txt, LV_LABEL_LONG_CLIP);
    lv_obj_add_flag(bk_ui->manualmodestart_manual_fermentation_humidity_txt, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_pos(bk_ui->manualmodestart_manual_fermentation_humidity_txt, 524, 278);
    lv_obj_set_size(bk_ui->manualmodestart_manual_fermentation_humidity_txt, 65, 55);

    // underbars: lazy-created in _common_click_mms on first field tap

    // Button: manual_freeze_temp_bt
    bk_ui->manualmodestart_manual_freeze_temp_bt = lv_button_create(bk_ui->manualmodestart);
    lv_obj_add_flag(bk_ui->manualmodestart_manual_freeze_temp_bt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->manualmodestart_manual_freeze_temp_bt, manualmodestart_manual_freeze_temp_bt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->manualmodestart_manual_freeze_temp_bt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->manualmodestart_manual_freeze_temp_bt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->manualmodestart_manual_freeze_temp_bt, 0, 0);
    lv_obj_set_pos(bk_ui->manualmodestart_manual_freeze_temp_bt, 425, 272);
    lv_obj_set_size(bk_ui->manualmodestart_manual_freeze_temp_bt, 170, 90);
    lv_obj_add_flag(bk_ui->manualmodestart_manual_freeze_temp_bt, LV_OBJ_FLAG_HIDDEN);

    // Button: manual_defrost_temp_bt
    bk_ui->manualmodestart_manual_defrost_temp_bt = lv_button_create(bk_ui->manualmodestart);
    lv_obj_add_flag(bk_ui->manualmodestart_manual_defrost_temp_bt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->manualmodestart_manual_defrost_temp_bt, manualmodestart_manual_defrost_temp_bt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->manualmodestart_manual_defrost_temp_bt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->manualmodestart_manual_defrost_temp_bt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->manualmodestart_manual_defrost_temp_bt, 0, 0);
    lv_obj_set_pos(bk_ui->manualmodestart_manual_defrost_temp_bt, 425, 272);
    lv_obj_set_size(bk_ui->manualmodestart_manual_defrost_temp_bt, 170, 90);
    lv_obj_add_flag(bk_ui->manualmodestart_manual_defrost_temp_bt, LV_OBJ_FLAG_HIDDEN);

    // Button: manual_fermentation_temp_bt
    bk_ui->manualmodestart_manual_fermentation_temp_bt = lv_button_create(bk_ui->manualmodestart);
    lv_obj_add_flag(bk_ui->manualmodestart_manual_fermentation_temp_bt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->manualmodestart_manual_fermentation_temp_bt, manualmodestart_manual_fermentation_temp_bt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->manualmodestart_manual_fermentation_temp_bt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->manualmodestart_manual_fermentation_temp_bt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->manualmodestart_manual_fermentation_temp_bt, 0, 0);
    lv_obj_set_pos(bk_ui->manualmodestart_manual_fermentation_temp_bt, 410, 252);
    lv_obj_set_size(bk_ui->manualmodestart_manual_fermentation_temp_bt, 110, 100);
    lv_obj_add_flag(bk_ui->manualmodestart_manual_fermentation_temp_bt, LV_OBJ_FLAG_HIDDEN);

    // Button: manual_fermentation_humidity_bt
    bk_ui->manualmodestart_manual_fermentation_humidity_bt = lv_button_create(bk_ui->manualmodestart);
    lv_obj_add_flag(bk_ui->manualmodestart_manual_fermentation_humidity_bt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->manualmodestart_manual_fermentation_humidity_bt, manualmodestart_manual_fermentation_humidity_bt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->manualmodestart_manual_fermentation_humidity_bt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->manualmodestart_manual_fermentation_humidity_bt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->manualmodestart_manual_fermentation_humidity_bt, 0, 0);
    lv_obj_set_pos(bk_ui->manualmodestart_manual_fermentation_humidity_bt, 530, 252);
    lv_obj_set_size(bk_ui->manualmodestart_manual_fermentation_humidity_bt, 80, 100);
    lv_obj_add_flag(bk_ui->manualmodestart_manual_fermentation_humidity_bt, LV_OBJ_FLAG_HIDDEN);

    // ImageView: test_circle_line
    bk_ui->manualmodestart_test_circle_line = lv_image_create(bk_ui->manualmodestart);
    _img_set_src_deferred(bk_ui->manualmodestart_test_circle_line, "/images/circle_modify.jpg");
    lv_obj_add_flag(bk_ui->manualmodestart_test_circle_line, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_pos(bk_ui->manualmodestart_test_circle_line, 360, 116);
    lv_obj_set_size(bk_ui->manualmodestart_test_circle_line, 300, 300);

    // keypadbaseim + keypad1~keypadhide: lazy-created in _keypad_on_manualmodestart on first use

    // ImageView: blackout
    bk_ui->manualmodestart_blackout = lv_image_create(bk_ui->manualmodestart);
    _img_set_src_deferred(bk_ui->manualmodestart_blackout, "/images/blackout.png");
    lv_obj_add_flag(bk_ui->manualmodestart_blackout, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_pos(bk_ui->manualmodestart_blackout, 841, 384);
    lv_obj_set_size(bk_ui->manualmodestart_blackout, LV_SIZE_CONTENT, LV_SIZE_CONTENT);

}
