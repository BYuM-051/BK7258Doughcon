#include "lvgl.h"
#include "beken_ui.h"
#include "custom_func.h"
#include "event_runtime.h"
#include "ui_lang.h"
#include <stdio.h>
#include <string.h>

#include "ui_config.h"
#include "preRenderer.h"

#define TAG "[autodrymode_init.c] "
// #define bk_printf(fmt, ...) do {} while(0) // disable printf

extern bk_lv_ui_t bk_lv_tool_ui;
extern lv_obj_t *preRenderRoot;
extern void autodrymode_backbt_event_cb(lv_event_t *e);
extern void autodrymode_auto_dry_temp_bt_event_cb(lv_event_t *e);
extern void autodrymode_auto_dry_humidity_bt_event_cb(lv_event_t *e);
extern void autodrymode_auto_dry_hour_bt_event_cb(lv_event_t *e);
extern void autodrymode_auto_dry_min_bt_event_cb(lv_event_t *e);
extern void autodrymode_auto_dry_start_event_cb(lv_event_t *e);
extern void autodrymode_keypad_event_cb(lv_event_t *e);
extern void autodrymode_keypadhide_event_cb(lv_event_t *e);
extern void autodrymode_load_event_cb(lv_event_t *e);

void destroy_page_autodrymode(bk_lv_ui_t *bk_ui)
{
    if (bk_ui == NULL) {
        return;
    }
    if (bk_ui->autodrymode != NULL) {
        lv_obj_del(bk_ui->autodrymode);
        bk_ui->autodrymode = NULL;
    }
}

void init_page_autodrymode(bk_lv_ui_t * bk_ui) {
    if (bk_ui->autodrymode != NULL && lv_obj_is_valid(bk_ui->autodrymode)) {
        destroy_page_autodrymode(bk_ui);
    }

    // Clear lazily-created child pointers (stale after destroy+re-init)
    bk_ui->autodrymode_keypadbaseim              = NULL;
    bk_ui->autodrymode_keypadhide                = NULL;
    bk_ui->autodrymode_keypadhide_im              = NULL;
    bk_ui->autodrymode_auto_dry_temp_underbar    = NULL;
    bk_ui->autodrymode_auto_dry_humidity_underbar= NULL;
    bk_ui->autodrymode_auto_dry_hour_underbar    = NULL;
    bk_ui->autodrymode_auto_dry_min_underbar     = NULL;
    bk_ui->autodrymode_run_arc                   = NULL;
    bk_ui->autodrymode_run_arc_bg_white           = NULL;
    for (int i = 0; i < 12; i++) {
        bk_ui->autodrymode_KeyPadBt[i] = NULL;
        bk_ui->autodrymode_KeyPadIm[i] = NULL;
    }

    ui_lang_reset_autodrymode_cache();

#if UI_PRENDERING_ENABLE
    bk_ui->autodrymode = lv_obj_create(preRenderRoot);
    lv_obj_remove_style_all(bk_ui->autodrymode);
    lv_obj_set_size(bk_ui->autodrymode, 1024, 600);
    lv_obj_set_scrollbar_mode(bk_ui->autodrymode, LV_SCROLLBAR_MODE_OFF);
    // TODO : 이 아래 이새끼도 loaded event구만.
    lv_obj_add_event_cb(bk_ui->autodrymode, autodrymode_load_event_cb, LV_EVENT_ALL, NULL);
    // bk_ui->autodrymode_bg = lv_image_create(bk_ui->autodrymode);
    // _img_set_src_timed(bk_ui->autodrymode_bg, "./images/auto_dry_bg.jpg");
    lv_obj_set_scrollbar_mode(bk_ui->autodrymode, LV_SCROLLBAR_MODE_OFF);
    // lv_obj_set_style_bg_color(bk_ui->autodrymode, lv_color_hex(0x4DA212), 0);
    // lv_obj_set_pos(bk_ui->autodrymode_bg, 0, 0);
    lv_obj_set_style_bg_color(bk_ui->autodrymode, lv_color_hex(0x49b206), 0);
    lv_obj_set_style_radius(bk_ui->autodrymode, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(bk_ui->autodrymode, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(bk_ui->autodrymode, LV_OPA_COVER, LV_PART_MAIN);
#else
    bk_ui->autodrymode = lv_obj_create(NULL);
    lv_obj_set_size(bk_ui->autodrymode, 1024, 600);
    lv_obj_set_scrollbar_mode(bk_ui->autodrymode, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_event_cb(bk_ui->autodrymode, autodrymode_load_event_cb, LV_EVENT_ALL, NULL);
    // bk_ui->autodrymode_bg = lv_image_create(bk_ui->autodrymode);
    // _img_set_src_timed(bk_ui->autodrymode_bg, "./images/auto_dry_bg.jpg");
    lv_obj_set_scrollbar_mode(bk_ui->autodrymode, LV_SCROLLBAR_MODE_OFF);
    // lv_obj_set_style_bg_color(bk_ui->autodrymode, lv_color_hex(0x4DA212), 0);
    // lv_obj_set_pos(bk_ui->autodrymode_bg, 0, 0);
    lv_obj_set_style_bg_color(bk_ui->autodrymode, lv_color_hex(0x49b206), 0);
#endif /* UI_PRENDERING_ENABLE */

    // ImageView: title
    bk_ui->autodrymode_title = lv_image_create(bk_ui->autodrymode);
    _img_set_src_timed(bk_ui->autodrymode_title, "/images/autodrymode_title.png");
    lv_obj_set_pos(bk_ui->autodrymode_title, 0, 10);
    lv_obj_set_size(bk_ui->autodrymode_title, 380, 80);
    lv_image_set_inner_align(bk_ui->autodrymode_title, LV_IMAGE_ALIGN_TOP_LEFT);

    // Button: backbt
    bk_ui->autodrymode_backbt = lv_button_create(bk_ui->autodrymode);
    lv_obj_add_flag(bk_ui->autodrymode_backbt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->autodrymode_backbt, autodrymode_backbt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->autodrymode_backbt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->autodrymode_backbt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->autodrymode_backbt, 0, 0);
    lv_obj_set_pos(bk_ui->autodrymode_backbt, 13, 445);
    lv_obj_set_size(bk_ui->autodrymode_backbt, 179, 74);

    // ImageView: backim
    bk_ui->autodrymode_backim = lv_image_create(bk_ui->autodrymode);
    _img_set_src_timed(bk_ui->autodrymode_backim, "/images/exit_bt.png");
    lv_obj_set_pos(bk_ui->autodrymode_backim, 13, 445);
    lv_obj_set_size(bk_ui->autodrymode_backim, 179, 74);

    // ImageView: auto_dry_circle_basic  (src deferred to load_event_cb — avoids 5×PNG read at init)
    bk_ui->autodrymode_auto_dry_circle_basic = lv_image_create(bk_ui->autodrymode);
    lv_obj_set_pos(bk_ui->autodrymode_auto_dry_circle_basic, 362, 117);
    lv_obj_set_size(bk_ui->autodrymode_auto_dry_circle_basic, 300, 300);

    // ImageView: auto_dry_circle_gif (src deferred to load_event_cb) — 원본 PNG(300x300) 사용,
    // arc 직접 그리기 대신 실제 이미지로 복원.
    bk_ui->autodrymode_auto_dry_circle_gif = lv_image_create(bk_ui->autodrymode);
    lv_obj_set_pos(bk_ui->autodrymode_auto_dry_circle_gif, 362, 117);
    lv_obj_set_size(bk_ui->autodrymode_auto_dry_circle_gif, 300, 300);
    lv_obj_remove_flag(bk_ui->autodrymode_auto_dry_circle_gif, LV_OBJ_FLAG_CLICKABLE);

    // ImageView: auto_dry_txt_basic
    bk_ui->autodrymode_auto_dry_txt_basic = lv_image_create(bk_ui->autodrymode);
    lv_obj_set_pos(bk_ui->autodrymode_auto_dry_txt_basic, 362, 117);
    lv_obj_set_size(bk_ui->autodrymode_auto_dry_txt_basic, 300, 300);

    // ImageView: auto_dry_gif — 원본 PNG 52x52(Pillow 확인), 박스를 원본 크기와 일치시켜 클리핑/중심틀어짐 방지
    bk_ui->autodrymode_auto_dry_gif = lv_image_create(bk_ui->autodrymode);
    lv_obj_add_flag(bk_ui->autodrymode_auto_dry_gif, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_pos(bk_ui->autodrymode_auto_dry_gif, 486, 171);
    lv_obj_set_size(bk_ui->autodrymode_auto_dry_gif, 52, 52);

    // ImageView: auto_dry_gif_basic
    bk_ui->autodrymode_auto_dry_gif_basic = lv_image_create(bk_ui->autodrymode);
    lv_obj_set_pos(bk_ui->autodrymode_auto_dry_gif_basic, 486, 171);
    lv_obj_set_size(bk_ui->autodrymode_auto_dry_gif_basic, 52, 52);

    // TextView: auto_dry_temp_txt
    bk_ui->autodrymode_auto_dry_temp_txt = lv_label_create(bk_ui->autodrymode);
    lv_label_set_text(bk_ui->autodrymode_auto_dry_temp_txt, "000");
    lv_obj_set_style_bg_opa(bk_ui->autodrymode_auto_dry_temp_txt, 0, 0);
    lv_obj_set_style_text_color(bk_ui->autodrymode_auto_dry_temp_txt, lv_color_hex(0x49B206), 0);
    lv_obj_set_style_text_font(bk_ui->autodrymode_auto_dry_temp_txt, &lv_font_scdream_regular_49, 0);
    lv_obj_set_pos(bk_ui->autodrymode_auto_dry_temp_txt, 402, 270+5+5);
    lv_obj_set_size(bk_ui->autodrymode_auto_dry_temp_txt, 90, 55);
    lv_obj_set_style_text_align(bk_ui->autodrymode_auto_dry_temp_txt, LV_TEXT_ALIGN_RIGHT, 0);


    // TextView: auto_dry_humidity_txt
    bk_ui->autodrymode_auto_dry_humidity_txt = lv_label_create(bk_ui->autodrymode);
    lv_label_set_text(bk_ui->autodrymode_auto_dry_humidity_txt, "00");
    lv_obj_set_style_bg_opa(bk_ui->autodrymode_auto_dry_humidity_txt, 0, 0);
    lv_obj_set_style_text_color(bk_ui->autodrymode_auto_dry_humidity_txt, lv_color_hex(0x49B206), 0);
    lv_obj_set_style_text_font(bk_ui->autodrymode_auto_dry_humidity_txt, &lv_font_scdream_regular_49, 0);
    lv_obj_set_pos(bk_ui->autodrymode_auto_dry_humidity_txt, 524+7, 270+5+5);
    lv_obj_set_size(bk_ui->autodrymode_auto_dry_humidity_txt, 65, 55);

    // underbars: lazy-created in _common_click_autodrymode on first field tap

    // Button: auto_dry_temp_bt
    bk_ui->autodrymode_auto_dry_temp_bt = lv_button_create(bk_ui->autodrymode);
    lv_obj_add_flag(bk_ui->autodrymode_auto_dry_temp_bt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->autodrymode_auto_dry_temp_bt, autodrymode_auto_dry_temp_bt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->autodrymode_auto_dry_temp_bt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->autodrymode_auto_dry_temp_bt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->autodrymode_auto_dry_temp_bt, 0, 0);
    lv_obj_set_pos(bk_ui->autodrymode_auto_dry_temp_bt, 400, 260);
    lv_obj_set_size(bk_ui->autodrymode_auto_dry_temp_bt, 110, 80);

    // Button: auto_dry_humidity_bt
    bk_ui->autodrymode_auto_dry_humidity_bt = lv_button_create(bk_ui->autodrymode);
    lv_obj_add_flag(bk_ui->autodrymode_auto_dry_humidity_bt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->autodrymode_auto_dry_humidity_bt, autodrymode_auto_dry_humidity_bt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->autodrymode_auto_dry_humidity_bt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->autodrymode_auto_dry_humidity_bt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->autodrymode_auto_dry_humidity_bt, 0, 0);
    lv_obj_set_pos(bk_ui->autodrymode_auto_dry_humidity_bt, 530, 260);
    lv_obj_set_size(bk_ui->autodrymode_auto_dry_humidity_bt, 90, 80);

    // ImageView: test_circle_line
    bk_ui->autodrymode_test_circle_line = lv_image_create(bk_ui->autodrymode);
    _img_set_src_deferred(bk_ui->autodrymode_test_circle_line, "/images/dry_circle_modify.png");
    lv_obj_add_flag(bk_ui->autodrymode_test_circle_line, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_pos(bk_ui->autodrymode_test_circle_line, 362, 117);
    lv_obj_set_size(bk_ui->autodrymode_test_circle_line, 300, 300);

    // ImageView: imageview16
    bk_ui->autodrymode_imageview16 = lv_image_create(bk_ui->autodrymode);
    _img_set_src_timed(bk_ui->autodrymode_imageview16, "/images/auto_dry_time_box.png");
    lv_obj_set_pos(bk_ui->autodrymode_imageview16, 705, 237);
    lv_obj_set_size(bk_ui->autodrymode_imageview16, 302, 66);

    // TextView: auto_dry_hour_txt
    bk_ui->autodrymode_auto_dry_hour_txt = lv_label_create(bk_ui->autodrymode);
    lv_label_set_text(bk_ui->autodrymode_auto_dry_hour_txt, "00");
    lv_obj_set_style_bg_opa(bk_ui->autodrymode_auto_dry_hour_txt, 0, 0);
    lv_obj_set_style_text_color(bk_ui->autodrymode_auto_dry_hour_txt, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->autodrymode_auto_dry_hour_txt, &lv_font_scdream_regular_33, 0);
    lv_obj_set_pos(bk_ui->autodrymode_auto_dry_hour_txt, 875, 248+5);
    lv_obj_set_size(bk_ui->autodrymode_auto_dry_hour_txt, 40, 35);

    // Button: auto_dry_hour_bt
    bk_ui->autodrymode_auto_dry_hour_bt = lv_button_create(bk_ui->autodrymode);
    lv_obj_add_flag(bk_ui->autodrymode_auto_dry_hour_bt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->autodrymode_auto_dry_hour_bt, autodrymode_auto_dry_hour_bt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->autodrymode_auto_dry_hour_bt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->autodrymode_auto_dry_hour_bt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->autodrymode_auto_dry_hour_bt, 0, 0);
    lv_obj_set_pos(bk_ui->autodrymode_auto_dry_hour_bt, 868, 248);
    lv_obj_set_size(bk_ui->autodrymode_auto_dry_hour_bt, 55, 45);

    // TextView: auto_dry_min_txt
    bk_ui->autodrymode_auto_dry_min_txt = lv_label_create(bk_ui->autodrymode);
    lv_label_set_text(bk_ui->autodrymode_auto_dry_min_txt, "00");
    lv_obj_set_style_bg_opa(bk_ui->autodrymode_auto_dry_min_txt, 0, 0);
    lv_obj_set_style_text_color(bk_ui->autodrymode_auto_dry_min_txt, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->autodrymode_auto_dry_min_txt, &lv_font_scdream_regular_33, 0);
    lv_obj_set_pos(bk_ui->autodrymode_auto_dry_min_txt, 941, 248+5);
    lv_obj_set_size(bk_ui->autodrymode_auto_dry_min_txt, 40, 35);

    // Button: auto_dry_min_bt
    bk_ui->autodrymode_auto_dry_min_bt = lv_button_create(bk_ui->autodrymode);
    lv_obj_add_flag(bk_ui->autodrymode_auto_dry_min_bt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->autodrymode_auto_dry_min_bt, autodrymode_auto_dry_min_bt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->autodrymode_auto_dry_min_bt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->autodrymode_auto_dry_min_bt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->autodrymode_auto_dry_min_bt, 0, 0);
    lv_obj_set_pos(bk_ui->autodrymode_auto_dry_min_bt, 935, 248);
    lv_obj_set_size(bk_ui->autodrymode_auto_dry_min_bt, 55, 45);

    // ImageView: auto_tempbox
    bk_ui->autodrymode_auto_tempbox = lv_image_create(bk_ui->autodrymode);
    _img_set_src_deferred(bk_ui->autodrymode_auto_tempbox, "/images/tempbox.png");
    // lv_obj_add_flag(bk_ui->autodrymode_auto_tempbox, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_pos(bk_ui->autodrymode_auto_tempbox, 222, 445);
    lv_obj_set_size(bk_ui->autodrymode_auto_tempbox, 580, 74);

    // TextView: tempbox_current_temp
    bk_ui->autodrymode_tempbox_current_temp = lv_label_create(bk_ui->autodrymode);
    lv_label_set_text(bk_ui->autodrymode_tempbox_current_temp, "");
    // lv_obj_add_flag(bk_ui->autodrymode_tempbox_current_temp, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_style_bg_opa(bk_ui->autodrymode_tempbox_current_temp, 0, 0);
    lv_obj_set_style_text_color(bk_ui->autodrymode_tempbox_current_temp, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->autodrymode_tempbox_current_temp, &lv_font_scdream_regular_40, 0);
    /* 세 자리 값(예: 111°F)이 폭 부족으로 다음 줄로 줄바꿈되던 문제 —
     * 줄바꿈 없이 한 줄 유지 + 박스 확장(우측 정렬 기준 좌측으로 확장) */
    lv_label_set_long_mode(bk_ui->autodrymode_tempbox_current_temp, LV_LABEL_LONG_CLIP);
    lv_obj_set_pos(bk_ui->autodrymode_tempbox_current_temp, 376-20, 463);
    lv_obj_set_size(bk_ui->autodrymode_tempbox_current_temp, 110, 50);
    lv_obj_set_style_text_align(bk_ui->autodrymode_tempbox_current_temp, LV_TEXT_ALIGN_RIGHT, 0);


    // TextView: tempbox_current_humidity
    bk_ui->autodrymode_tempbox_current_humidity = lv_label_create(bk_ui->autodrymode);
    lv_label_set_text(bk_ui->autodrymode_tempbox_current_humidity, "65");
    // lv_obj_add_flag(bk_ui->autodrymode_tempbox_current_humidity, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_style_bg_opa(bk_ui->autodrymode_tempbox_current_humidity, 0, 0);
    lv_obj_set_style_text_color(bk_ui->autodrymode_tempbox_current_humidity, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->autodrymode_tempbox_current_humidity, &lv_font_scdream_regular_40, 0);
    lv_obj_set_pos(bk_ui->autodrymode_tempbox_current_humidity, 655, 458+5);
    lv_obj_set_size(bk_ui->autodrymode_tempbox_current_humidity, 90, 50);
    lv_obj_set_style_text_align(bk_ui->autodrymode_tempbox_current_humidity, LV_TEXT_ALIGN_RIGHT, 0);


    // Button: auto_dry_start
    bk_ui->autodrymode_auto_dry_start = lv_button_create(bk_ui->autodrymode);
    lv_obj_add_flag(bk_ui->autodrymode_auto_dry_start, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->autodrymode_auto_dry_start, autodrymode_auto_dry_start_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->autodrymode_auto_dry_start, 0, 0);
    lv_obj_set_style_border_width(bk_ui->autodrymode_auto_dry_start, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->autodrymode_auto_dry_start, 0, 0);
    lv_obj_set_pos(bk_ui->autodrymode_auto_dry_start, 849, 447);
    lv_obj_set_size(bk_ui->autodrymode_auto_dry_start, 169, 64);

    // ImageView: auto_dry_startim
    bk_ui->autodrymode_auto_dry_startim = lv_image_create(bk_ui->autodrymode);
    _img_set_src_timed(bk_ui->autodrymode_auto_dry_startim, "/images/start_bt.png");
    lv_obj_set_pos(bk_ui->autodrymode_auto_dry_startim, 847, 445);
    lv_obj_set_size(bk_ui->autodrymode_auto_dry_startim, 164, 74);

    // keypadbaseim + KeyPadBt/Im[12] + keypadhide: lazy-created in _keypad_on_autodrymode on first use

    // ImageView: blackout
    bk_ui->autodrymode_blackout = lv_image_create(bk_ui->autodrymode);
    _img_set_src_deferred(bk_ui->autodrymode_blackout, "/images/blackout.png");
    lv_obj_add_flag(bk_ui->autodrymode_blackout, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_pos(bk_ui->autodrymode_blackout, 841, 384);
    // lv_obj_set_size(bk_ui->autodrymode_blackout, 0, 0);
    // init_keypad_group

}
