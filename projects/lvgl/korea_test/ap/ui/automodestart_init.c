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
extern void automodestart_startbt_event_cb(lv_event_t *e);
extern void automodestart_load_start_event_cb(lv_event_t *e);
extern void automodestart_loaded_event_cb(lv_event_t *e);
extern void automodestart_unload_start_event_cb(lv_event_t *e);
extern void automodestart_unloaded_event_cb(lv_event_t *e);

static void _freeze_gif_ext_draw_cb(lv_event_t *e)
{
    lv_event_set_ext_draw_size(e, 10); /* rotation 잔상 방지: 45×45 대각선 여유분 */
}


void destroy_page_automodestart(bk_lv_ui_t *bk_ui)
{
    if (bk_ui == NULL) {
        return;
    }
    if (bk_ui->automodestart != NULL) {
        lv_obj_del(bk_ui->automodestart);
        bk_ui->automodestart = NULL;
    }
}

void init_page_automodestart(bk_lv_ui_t * bk_ui) {
    if (bk_ui->automodestart != NULL && lv_obj_is_valid(bk_ui->automodestart)) {
        destroy_page_automodestart(bk_ui);
    }

    /* 오브젝트를 새로 만드므로 ui_lang 캐시를 무효화 — 다음 ui_lang_apply_automodestart()가
     * 이전 세션과 언어/단위/모드가 우연히 같아도 반드시 새 이미지를 채우게 함 */
    ui_lang_reset_automodestart_cache();

#if UI_PRENDERING_ENABLE
    bk_ui->automodestart = lv_obj_create(preRenderRoot);
    lv_obj_remove_style_all(bk_ui->automodestart);
    lv_obj_set_size(bk_ui->automodestart, 1024, 600);
    lv_obj_set_pos(bk_ui->automodestart, 0, 0);
    lv_obj_set_style_radius(bk_ui->automodestart, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(bk_ui->automodestart, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_color(bk_ui->automodestart, lv_color_hex(0xD9D9D9), 0);
    lv_obj_set_style_bg_opa(bk_ui->automodestart, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_scrollbar_mode(bk_ui->automodestart, LV_SCROLLBAR_MODE_OFF);
    // 원래 LV_EVENT_ALL로 등록되어 있었음
    lv_obj_add_event_cb(bk_ui->automodestart, automodestart_load_start_event_cb, UI_EVENT_PAGE_SHOW_START, NULL);
    lv_obj_add_event_cb(bk_ui->automodestart, automodestart_loaded_event_cb, UI_EVENT_PAGE_SHOWN,     NULL);
    lv_obj_add_event_cb(bk_ui->automodestart, automodestart_unload_start_event_cb, UI_EVENT_PAGE_HIDE_START, NULL);
    lv_obj_add_event_cb(bk_ui->automodestart, automodestart_unloaded_event_cb, UI_EVENT_PAGE_HIDDEN,     NULL);
#else
    bk_ui->automodestart = lv_obj_create(NULL);
    lv_obj_set_size(bk_ui->automodestart, 1024, 600);
    lv_obj_set_scrollbar_mode(bk_ui->automodestart, LV_SCROLLBAR_MODE_OFF);
    // 원래 LV_EVENT_ALL로 등록되어 있었음
    lv_obj_add_event_cb(bk_ui->automodestart, automodestart_load_start_event_cb, LV_EVENT_SCREEN_LOAD_START, NULL);
    lv_obj_add_event_cb(bk_ui->automodestart, automodestart_loaded_event_cb, LV_EVENT_SCREEN_LOADED,     NULL);
    lv_obj_add_event_cb(bk_ui->automodestart, automodestart_unload_start_event_cb, LV_EVENT_SCREEN_UNLOAD_START, NULL);
    lv_obj_add_event_cb(bk_ui->automodestart, automodestart_unloaded_event_cb, LV_EVENT_SCREEN_UNLOADED,     NULL);
#endif /* UI_PRENDERING_ENABLE */

    // ImageView: auto_bg
    bk_ui->automodestart_auto_bg = lv_image_create(bk_ui->automodestart);
    ui_page_build_set_image_src(bk_ui->automodestart_auto_bg, "/images/auto_mode_start_bgi.jpg");
    lv_obj_set_pos(bk_ui->automodestart_auto_bg, 0, 0);
    lv_obj_set_size(bk_ui->automodestart_auto_bg, 1024, 540);


    // ImageView: title
    bk_ui->automodestart_title = lv_image_create(bk_ui->automodestart);
    ui_page_build_set_image_src(bk_ui->automodestart_title, "/images/automode_title.png");
    lv_obj_set_pos(bk_ui->automodestart_title, 0, 10);
    lv_obj_set_size(bk_ui->automodestart_title, 380, 80);
    lv_image_set_inner_align(bk_ui->automodestart_title, LV_IMAGE_ALIGN_TOP_LEFT);

    // ImageView: AutoFreezeIm
    bk_ui->automodestart_AutoFreezeIm = lv_image_create(bk_ui->automodestart);
    lv_obj_add_flag(bk_ui->automodestart_AutoFreezeIm, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_pos(bk_ui->automodestart_AutoFreezeIm, 362, 117);
    lv_obj_set_size(bk_ui->automodestart_AutoFreezeIm, 300, 300);

    // ImageView: AutoDefrostIm
    bk_ui->automodestart_AutoDefrostIm = lv_image_create(bk_ui->automodestart);
    lv_obj_add_flag(bk_ui->automodestart_AutoDefrostIm, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_pos(bk_ui->automodestart_AutoDefrostIm, 362, 117);
    lv_obj_set_size(bk_ui->automodestart_AutoDefrostIm, 300, 300);

    // ImageView: AutoFermentation1Im
    bk_ui->automodestart_AutoFermentation1Im = lv_image_create(bk_ui->automodestart);
    lv_obj_add_flag(bk_ui->automodestart_AutoFermentation1Im, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_pos(bk_ui->automodestart_AutoFermentation1Im, 362, 117);
    lv_obj_set_size(bk_ui->automodestart_AutoFermentation1Im, 300, 300);

    // ImageView: AutoFermentation2Im
    bk_ui->automodestart_AutoFermentation2Im = lv_image_create(bk_ui->automodestart);
    lv_obj_add_flag(bk_ui->automodestart_AutoFermentation2Im, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_pos(bk_ui->automodestart_AutoFermentation2Im, 362, 117);
    lv_obj_set_size(bk_ui->automodestart_AutoFermentation2Im, 300, 300);

    // Arc: 진도 표시 — automodestart_cb.c _update_arc_ams 에서 canvas로 직접 렌더
    bk_ui->automodestart_freeze_arc  = NULL;
    bk_ui->automodestart_defrost_arc = NULL;
    bk_ui->automodestart_ferm1_arc   = NULL;
    bk_ui->automodestart_ferm2_arc   = NULL;

    // Button: startbt
    bk_ui->automodestart_startbt = lv_button_create(bk_ui->automodestart);
    lv_obj_add_flag(bk_ui->automodestart_startbt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->automodestart_startbt, automodestart_startbt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->automodestart_startbt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->automodestart_startbt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->automodestart_startbt, 0, 0);
    lv_obj_set_pos(bk_ui->automodestart_startbt, 847, 445);
    lv_obj_set_size(bk_ui->automodestart_startbt, 164, 74);

    // ImageView: imageview8
    bk_ui->automodestart_imageview8 = lv_image_create(bk_ui->automodestart);
    ui_page_build_set_image_src(bk_ui->automodestart_imageview8, "/images/stop_bt.png");
    lv_obj_set_pos(bk_ui->automodestart_imageview8, 847, 445);
    lv_obj_set_size(bk_ui->automodestart_imageview8, 164, 74);

    // ImageView: auto_tempbox
    bk_ui->automodestart_auto_tempbox = lv_image_create(bk_ui->automodestart);
    ui_page_build_set_image_src(bk_ui->automodestart_auto_tempbox, "/images/tempbox.png");
    lv_obj_set_pos(bk_ui->automodestart_auto_tempbox, 222, 445);
    lv_obj_set_size(bk_ui->automodestart_auto_tempbox, 580, 74);

    // TextView: tempbox_current_temp
    bk_ui->automodestart_tempbox_current_temp = lv_label_create(bk_ui->automodestart);
    lv_label_set_text(bk_ui->automodestart_tempbox_current_temp, "");
    lv_obj_set_style_bg_opa(bk_ui->automodestart_tempbox_current_temp, 0, 0);
    lv_obj_set_style_text_color(bk_ui->automodestart_tempbox_current_temp, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->automodestart_tempbox_current_temp, &lv_font_scdream_regular_40, 0);
    lv_obj_set_style_text_align(bk_ui->automodestart_tempbox_current_temp, LV_TEXT_ALIGN_RIGHT, 0);
    /* 세 자리 값(예: 111°F)이 폭 부족으로 다음 줄로 줄바꿈되던 문제 —
     * 줄바꿈 없이 한 줄 유지 + 박스 확장(우측 정렬 기준 좌측으로 확장) */
    lv_label_set_long_mode(bk_ui->automodestart_tempbox_current_temp, LV_LABEL_LONG_CLIP);
    lv_obj_set_pos(bk_ui->automodestart_tempbox_current_temp, 376-20, 463);
    lv_obj_set_size(bk_ui->automodestart_tempbox_current_temp, 110, 50);

    // TextView: tempbox_current_humidity
    bk_ui->automodestart_tempbox_current_humidity = lv_label_create(bk_ui->automodestart);
    lv_label_set_text(bk_ui->automodestart_tempbox_current_humidity, "");
    lv_obj_set_style_bg_opa(bk_ui->automodestart_tempbox_current_humidity, 0, 0);
    lv_obj_set_style_text_color(bk_ui->automodestart_tempbox_current_humidity, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->automodestart_tempbox_current_humidity, &lv_font_scdream_regular_40, 0);
    lv_obj_set_style_text_align(bk_ui->automodestart_tempbox_current_humidity, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_set_pos(bk_ui->automodestart_tempbox_current_humidity, 655, 463);
    lv_obj_set_size(bk_ui->automodestart_tempbox_current_humidity, 90, 50);

    // ImageView: imageview12
    bk_ui->automodestart_imageview12 = lv_image_create(bk_ui->automodestart);
    ui_page_build_set_image_src(bk_ui->automodestart_imageview12, "/images/auto_mode_start_box_time.png");
    lv_obj_set_pos(bk_ui->automodestart_imageview12, 479, 15);
    lv_obj_set_size(bk_ui->automodestart_imageview12, 528, 70);

    // TextView: AutoModeCompleteYear
    bk_ui->automodestart_AutoModeCompleteYear = lv_label_create(bk_ui->automodestart);
    lv_label_set_text(bk_ui->automodestart_AutoModeCompleteYear, "");
    lv_obj_set_style_bg_opa(bk_ui->automodestart_AutoModeCompleteYear, 0, 0);
    lv_obj_set_style_text_color(bk_ui->automodestart_AutoModeCompleteYear, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->automodestart_AutoModeCompleteYear, &lv_font_scdream_regular_33, 0);
    lv_obj_set_style_text_align(bk_ui->automodestart_AutoModeCompleteYear, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_pos(bk_ui->automodestart_AutoModeCompleteYear, 656, 33);
    lv_obj_set_size(bk_ui->automodestart_AutoModeCompleteYear, 80, 44);

    // TextView: AutoModeCompleteMonth
    bk_ui->automodestart_AutoModeCompleteMonth = lv_label_create(bk_ui->automodestart);
    lv_label_set_text(bk_ui->automodestart_AutoModeCompleteMonth, "");
    lv_obj_set_style_bg_opa(bk_ui->automodestart_AutoModeCompleteMonth, 0, 0);
    lv_obj_set_style_text_color(bk_ui->automodestart_AutoModeCompleteMonth, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->automodestart_AutoModeCompleteMonth, &lv_font_scdream_regular_33, 0);
    lv_obj_set_style_text_align(bk_ui->automodestart_AutoModeCompleteMonth, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_pos(bk_ui->automodestart_AutoModeCompleteMonth, 752, 33);
    lv_obj_set_size(bk_ui->automodestart_AutoModeCompleteMonth, 40, 44);

    // TextView: AutoModeCompleteDay
    bk_ui->automodestart_AutoModeCompleteDay = lv_label_create(bk_ui->automodestart);
    lv_label_set_text(bk_ui->automodestart_AutoModeCompleteDay, "");
    lv_obj_set_style_bg_opa(bk_ui->automodestart_AutoModeCompleteDay, 0, 0);
    lv_obj_set_style_text_color(bk_ui->automodestart_AutoModeCompleteDay, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->automodestart_AutoModeCompleteDay, &lv_font_scdream_regular_33, 0);
    lv_obj_set_style_text_align(bk_ui->automodestart_AutoModeCompleteDay, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_pos(bk_ui->automodestart_AutoModeCompleteDay, 808, 33);
    lv_obj_set_size(bk_ui->automodestart_AutoModeCompleteDay, 40, 44);

    // TextView: AutoModeCompleteHour
    bk_ui->automodestart_AutoModeCompleteHour = lv_label_create(bk_ui->automodestart);
    lv_label_set_text(bk_ui->automodestart_AutoModeCompleteHour, "");
    lv_obj_set_style_bg_opa(bk_ui->automodestart_AutoModeCompleteHour, 0, 0);
    lv_obj_set_style_text_color(bk_ui->automodestart_AutoModeCompleteHour, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->automodestart_AutoModeCompleteHour, &lv_font_scdream_regular_33, 0);
    lv_obj_set_style_text_align(bk_ui->automodestart_AutoModeCompleteHour, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_pos(bk_ui->automodestart_AutoModeCompleteHour, 868, 33);
    lv_obj_set_size(bk_ui->automodestart_AutoModeCompleteHour, 40, 44);

    // TextView: AutoModeCompleteMin
    bk_ui->automodestart_AutoModeCompleteMin = lv_label_create(bk_ui->automodestart);
    lv_label_set_text(bk_ui->automodestart_AutoModeCompleteMin, "");
    lv_obj_set_style_bg_opa(bk_ui->automodestart_AutoModeCompleteMin, 0, 0);
    lv_obj_set_style_text_color(bk_ui->automodestart_AutoModeCompleteMin, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->automodestart_AutoModeCompleteMin, &lv_font_scdream_regular_33, 0);
    lv_obj_set_style_text_align(bk_ui->automodestart_AutoModeCompleteMin, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_pos(bk_ui->automodestart_AutoModeCompleteMin, 936, 33);
    lv_obj_set_size(bk_ui->automodestart_AutoModeCompleteMin, 40, 44);

    // TextView: AutoFreezeTempTxt
    bk_ui->automodestart_AutoFreezeTempTxt = lv_label_create(bk_ui->automodestart);
    lv_label_set_text(bk_ui->automodestart_AutoFreezeTempTxt, "");
    lv_obj_set_style_bg_opa(bk_ui->automodestart_AutoFreezeTempTxt, 0, 0);
    lv_obj_set_style_text_color(bk_ui->automodestart_AutoFreezeTempTxt, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->automodestart_AutoFreezeTempTxt, &lv_font_scdream_regular_40, 0);
    lv_obj_set_style_text_align(bk_ui->automodestart_AutoFreezeTempTxt, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_set_pos(bk_ui->automodestart_AutoFreezeTempTxt, 55, 164);
    lv_obj_set_size(bk_ui->automodestart_AutoFreezeTempTxt, 80, 50);

    // TextView: AutoFreezeTimeHourTxt
    bk_ui->automodestart_AutoFreezeTimeHourTxt = lv_label_create(bk_ui->automodestart);
    lv_label_set_text(bk_ui->automodestart_AutoFreezeTimeHourTxt, "");
    lv_obj_set_style_bg_opa(bk_ui->automodestart_AutoFreezeTimeHourTxt, 0, 0);
    lv_obj_set_style_text_color(bk_ui->automodestart_AutoFreezeTimeHourTxt, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->automodestart_AutoFreezeTimeHourTxt, &lv_font_scdream_regular_40, 0);
    lv_obj_set_style_text_align(bk_ui->automodestart_AutoFreezeTimeHourTxt, LV_TEXT_ALIGN_CENTER, 0);
    /* 완료시간을 2일 이상으로 설정하면 냉동 잔여시간이 100시간을 넘어 3자리
     * ("120" 등)가 될 수 있다 — 기존 60px 폭(2자리 기준)에서는 기본 long_mode
     * (WRAP)로 인해 두 줄로 줄바꿈되어 표시됐다. 폭을 넓히고 CLIP으로 바꿔
     * 줄바꿈 대신 한 줄 안에 표시되게 한다(좌측 TempTxt와는 70px 여유,
     * 우측 MinTxt와는 5px 여유가 있어 겹치지 않음). */
    lv_label_set_long_mode(bk_ui->automodestart_AutoFreezeTimeHourTxt, LV_LABEL_LONG_CLIP);
    lv_obj_set_pos(bk_ui->automodestart_AutoFreezeTimeHourTxt, 190, 164);
    lv_obj_set_size(bk_ui->automodestart_AutoFreezeTimeHourTxt, 90, 50);

    // TextView: AutoFreezeTimeMinTxt
    bk_ui->automodestart_AutoFreezeTimeMinTxt = lv_label_create(bk_ui->automodestart);
    lv_label_set_text(bk_ui->automodestart_AutoFreezeTimeMinTxt, "");
    lv_obj_set_style_bg_opa(bk_ui->automodestart_AutoFreezeTimeMinTxt, 0, 0);
    lv_obj_set_style_text_color(bk_ui->automodestart_AutoFreezeTimeMinTxt, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->automodestart_AutoFreezeTimeMinTxt, &lv_font_scdream_regular_40, 0);
    lv_obj_set_style_text_align(bk_ui->automodestart_AutoFreezeTimeMinTxt, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_pos(bk_ui->automodestart_AutoFreezeTimeMinTxt, 285, 164);
    lv_obj_set_size(bk_ui->automodestart_AutoFreezeTimeMinTxt, 60, 50);

    // ImageView: freeze_gif
    bk_ui->automodestart_freeze_gif = lv_image_create(bk_ui->automodestart);
    _img_set_src_deferred(bk_ui->automodestart_freeze_gif, "/images/freeze_gif.png");
    lv_obj_add_flag(bk_ui->automodestart_freeze_gif, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_pos(bk_ui->automodestart_freeze_gif, 449, 185);
    lv_obj_set_size(bk_ui->automodestart_freeze_gif, 45, 45);
    lv_obj_add_event_cb(bk_ui->automodestart_freeze_gif, _freeze_gif_ext_draw_cb, LV_EVENT_REFR_EXT_DRAW_SIZE, NULL);
    lv_obj_refresh_ext_draw_size(bk_ui->automodestart_freeze_gif);

    // ImageView: freeze_gif_basic
    bk_ui->automodestart_freeze_gif_basic = lv_image_create(bk_ui->automodestart);
    ui_page_build_set_image_src(bk_ui->automodestart_freeze_gif_basic, "/images/freeze_gif.png");
    lv_obj_set_pos(bk_ui->automodestart_freeze_gif_basic, 449, 185);
    lv_obj_set_size(bk_ui->automodestart_freeze_gif_basic, 45, 45);

    // ImageView: defrost_gif (55×45)
    bk_ui->automodestart_defrost_gif = lv_image_create(bk_ui->automodestart);
    _img_set_src_deferred(bk_ui->automodestart_defrost_gif, "/images/defrost_gif.png");
    lv_obj_add_flag(bk_ui->automodestart_defrost_gif, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_pos(bk_ui->automodestart_defrost_gif, 522, 188-3);
    lv_obj_set_size(bk_ui->automodestart_defrost_gif, 63, 45);

    // ImageView: defrost_gif_basic
    bk_ui->automodestart_defrost_gif_basic = lv_image_create(bk_ui->automodestart);
    ui_page_build_set_image_src(bk_ui->automodestart_defrost_gif_basic, "/images/defrost_gif.png");
    lv_obj_set_pos(bk_ui->automodestart_defrost_gif_basic, 522, 188-3);
    lv_obj_set_size(bk_ui->automodestart_defrost_gif_basic, 63, 45);

    // ImageView: fermentation1_gif (55×45)
    bk_ui->automodestart_fermentation1_gif = lv_image_create(bk_ui->automodestart);
    _img_set_src_deferred(bk_ui->automodestart_fermentation1_gif, "/images/fermentation1_gif.png");
    lv_obj_add_flag(bk_ui->automodestart_fermentation1_gif, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_pos(bk_ui->automodestart_fermentation1_gif, 522, 275-3);
    lv_obj_set_size(bk_ui->automodestart_fermentation1_gif, 63, 45);

    // ImageView: fermentation1_gif_basic
    bk_ui->automodestart_fermentation1_gif_basic = lv_image_create(bk_ui->automodestart);
    _img_set_src_deferred(bk_ui->automodestart_fermentation1_gif_basic, "/images/fermentation1_gif.png");
    lv_obj_set_pos(bk_ui->automodestart_fermentation1_gif_basic, 522, 275-3);
    lv_obj_set_size(bk_ui->automodestart_fermentation1_gif_basic, 63, 45);

    // ImageView: fermentation2_gif (55×45)
    bk_ui->automodestart_fermentation2_gif = lv_image_create(bk_ui->automodestart);
    _img_set_src_deferred(bk_ui->automodestart_fermentation2_gif, "/images/fermentation2_gif.png");
    lv_obj_add_flag(bk_ui->automodestart_fermentation2_gif, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_pos(bk_ui->automodestart_fermentation2_gif, 440, 275-3);
    lv_obj_set_size(bk_ui->automodestart_fermentation2_gif, 63, 45);

    // ImageView: fermentation2_gif_basic
    bk_ui->automodestart_fermentation2_gif_basic = lv_image_create(bk_ui->automodestart);
    _img_set_src_deferred(bk_ui->automodestart_fermentation2_gif_basic, "/images/fermentation2_gif.png");
    lv_obj_set_pos(bk_ui->automodestart_fermentation2_gif_basic, 440, 275-3);
    lv_obj_set_size(bk_ui->automodestart_fermentation2_gif_basic, 63, 45);

    // TextView: AutoDefrostTempTxt
    bk_ui->automodestart_AutoDefrostTempTxt = lv_label_create(bk_ui->automodestart);
    lv_label_set_text(bk_ui->automodestart_AutoDefrostTempTxt, "");
    lv_obj_set_style_bg_opa(bk_ui->automodestart_AutoDefrostTempTxt, 0, 0);
    lv_obj_set_style_text_color(bk_ui->automodestart_AutoDefrostTempTxt, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->automodestart_AutoDefrostTempTxt, &lv_font_scdream_regular_40, 0);
    lv_obj_set_style_text_align(bk_ui->automodestart_AutoDefrostTempTxt, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_set_pos(bk_ui->automodestart_AutoDefrostTempTxt, 701, 164);
    lv_obj_set_size(bk_ui->automodestart_AutoDefrostTempTxt, 80, 50);

    // TextView: AutoDefrostTimeHourTxt
    bk_ui->automodestart_AutoDefrostTimeHourTxt = lv_label_create(bk_ui->automodestart);
    lv_label_set_text(bk_ui->automodestart_AutoDefrostTimeHourTxt, "");
    lv_obj_set_style_bg_opa(bk_ui->automodestart_AutoDefrostTimeHourTxt, 0, 0);
    lv_obj_set_style_text_color(bk_ui->automodestart_AutoDefrostTimeHourTxt, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->automodestart_AutoDefrostTimeHourTxt, &lv_font_scdream_regular_40, 0);
    lv_obj_set_style_text_align(bk_ui->automodestart_AutoDefrostTimeHourTxt, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_pos(bk_ui->automodestart_AutoDefrostTimeHourTxt, 850, 164);
    lv_obj_set_size(bk_ui->automodestart_AutoDefrostTimeHourTxt, 60, 50);

    // TextView: AutoDefrostTimeMinTxt
    bk_ui->automodestart_AutoDefrostTimeMinTxt = lv_label_create(bk_ui->automodestart);
    lv_label_set_text(bk_ui->automodestart_AutoDefrostTimeMinTxt, "");
    lv_obj_set_style_bg_opa(bk_ui->automodestart_AutoDefrostTimeMinTxt, 0, 0);
    lv_obj_set_style_text_color(bk_ui->automodestart_AutoDefrostTimeMinTxt, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->automodestart_AutoDefrostTimeMinTxt, &lv_font_scdream_regular_42, 0);
    lv_obj_set_style_text_align(bk_ui->automodestart_AutoDefrostTimeMinTxt, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_pos(bk_ui->automodestart_AutoDefrostTimeMinTxt, 930, 164);
    lv_obj_set_size(bk_ui->automodestart_AutoDefrostTimeMinTxt, 60, 50);

    // TextView: AutoFermentation2TempTxt
    bk_ui->automodestart_AutoFermentation2TempTxt = lv_label_create(bk_ui->automodestart);
    lv_label_set_text(bk_ui->automodestart_AutoFermentation2TempTxt, "");
    lv_obj_set_style_bg_opa(bk_ui->automodestart_AutoFermentation2TempTxt, 0, 0);
    lv_obj_set_style_text_color(bk_ui->automodestart_AutoFermentation2TempTxt, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->automodestart_AutoFermentation2TempTxt, &lv_font_scdream_regular_40, 0);
    lv_obj_set_style_text_align(bk_ui->automodestart_AutoFermentation2TempTxt, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_set_pos(bk_ui->automodestart_AutoFermentation2TempTxt, 13, 324);
    lv_obj_set_size(bk_ui->automodestart_AutoFermentation2TempTxt, 80, 50);

    // TextView: AutoFermentation2HumidityTxt
    bk_ui->automodestart_AutoFermentation2HumidityTxt = lv_label_create(bk_ui->automodestart);
    lv_label_set_text(bk_ui->automodestart_AutoFermentation2HumidityTxt, "");
    lv_obj_set_style_bg_opa(bk_ui->automodestart_AutoFermentation2HumidityTxt, 0, 0);
    lv_obj_set_style_text_color(bk_ui->automodestart_AutoFermentation2HumidityTxt, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->automodestart_AutoFermentation2HumidityTxt, &lv_font_scdream_regular_40, 0);
    lv_obj_set_style_text_align(bk_ui->automodestart_AutoFermentation2HumidityTxt, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_set_pos(bk_ui->automodestart_AutoFermentation2HumidityTxt, 122, 324);
    lv_obj_set_size(bk_ui->automodestart_AutoFermentation2HumidityTxt, 60, 50);

    // TextView: AutoFermentation2TimeHourTxt
    bk_ui->automodestart_AutoFermentation2TimeHourTxt = lv_label_create(bk_ui->automodestart);
    lv_label_set_text(bk_ui->automodestart_AutoFermentation2TimeHourTxt, "");
    lv_obj_set_style_bg_opa(bk_ui->automodestart_AutoFermentation2TimeHourTxt, 0, 0);
    lv_obj_set_style_text_color(bk_ui->automodestart_AutoFermentation2TimeHourTxt, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->automodestart_AutoFermentation2TimeHourTxt, &lv_font_scdream_regular_40, 0);
    lv_obj_set_style_text_align(bk_ui->automodestart_AutoFermentation2TimeHourTxt, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_pos(bk_ui->automodestart_AutoFermentation2TimeHourTxt, 214, 324);
    lv_obj_set_size(bk_ui->automodestart_AutoFermentation2TimeHourTxt, 60, 50);

    // TextView: AutoFermentation2TimeMinTxt
    bk_ui->automodestart_AutoFermentation2TimeMinTxt = lv_label_create(bk_ui->automodestart);
    lv_label_set_text(bk_ui->automodestart_AutoFermentation2TimeMinTxt, "");
    lv_obj_set_style_bg_opa(bk_ui->automodestart_AutoFermentation2TimeMinTxt, 0, 0);
    lv_obj_set_style_text_color(bk_ui->automodestart_AutoFermentation2TimeMinTxt, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->automodestart_AutoFermentation2TimeMinTxt, &lv_font_scdream_regular_40, 0);
    lv_obj_set_style_text_align(bk_ui->automodestart_AutoFermentation2TimeMinTxt, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_pos(bk_ui->automodestart_AutoFermentation2TimeMinTxt, 294, 324);
    lv_obj_set_size(bk_ui->automodestart_AutoFermentation2TimeMinTxt, 60, 50);

    // TextView: AutoFermentation1TempTxt
    bk_ui->automodestart_AutoFermentation1TempTxt = lv_label_create(bk_ui->automodestart);
    lv_label_set_text(bk_ui->automodestart_AutoFermentation1TempTxt, "");
    lv_obj_set_style_bg_opa(bk_ui->automodestart_AutoFermentation1TempTxt, 0, 0);
    lv_obj_set_style_text_color(bk_ui->automodestart_AutoFermentation1TempTxt, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->automodestart_AutoFermentation1TempTxt, &lv_font_scdream_regular_40, 0);
    lv_obj_set_style_text_align(bk_ui->automodestart_AutoFermentation1TempTxt, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_set_pos(bk_ui->automodestart_AutoFermentation1TempTxt, 659, 324);
    lv_obj_set_size(bk_ui->automodestart_AutoFermentation1TempTxt, 80, 50);

    // TextView: AutoFermentation1HumidityTxt
    bk_ui->automodestart_AutoFermentation1HumidityTxt = lv_label_create(bk_ui->automodestart);
    lv_label_set_text(bk_ui->automodestart_AutoFermentation1HumidityTxt, "");
    lv_obj_set_style_bg_opa(bk_ui->automodestart_AutoFermentation1HumidityTxt, 0, 0);
    lv_obj_set_style_text_color(bk_ui->automodestart_AutoFermentation1HumidityTxt, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->automodestart_AutoFermentation1HumidityTxt, &lv_font_scdream_regular_40, 0);
    lv_obj_set_style_text_align(bk_ui->automodestart_AutoFermentation1HumidityTxt, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_set_pos(bk_ui->automodestart_AutoFermentation1HumidityTxt, 769, 324);
    lv_obj_set_size(bk_ui->automodestart_AutoFermentation1HumidityTxt, 60, 50);

    // TextView: AutoFermentation1TimeHourTxt
    bk_ui->automodestart_AutoFermentation1TimeHourTxt = lv_label_create(bk_ui->automodestart);
    lv_label_set_text(bk_ui->automodestart_AutoFermentation1TimeHourTxt, "");
    lv_obj_set_style_bg_opa(bk_ui->automodestart_AutoFermentation1TimeHourTxt, 0, 0);
    lv_obj_set_style_text_color(bk_ui->automodestart_AutoFermentation1TimeHourTxt, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->automodestart_AutoFermentation1TimeHourTxt, &lv_font_scdream_regular_40, 0);
    lv_obj_set_style_text_align(bk_ui->automodestart_AutoFermentation1TimeHourTxt, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_pos(bk_ui->automodestart_AutoFermentation1TimeHourTxt, 860, 324);
    lv_obj_set_size(bk_ui->automodestart_AutoFermentation1TimeHourTxt, 60, 50);

    // TextView: AutoFermentation1TimeMinTxt
    bk_ui->automodestart_AutoFermentation1TimeMinTxt = lv_label_create(bk_ui->automodestart);
    lv_label_set_text(bk_ui->automodestart_AutoFermentation1TimeMinTxt, "");
    lv_obj_set_style_bg_opa(bk_ui->automodestart_AutoFermentation1TimeMinTxt, 0, 0);
    lv_obj_set_style_text_color(bk_ui->automodestart_AutoFermentation1TimeMinTxt, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->automodestart_AutoFermentation1TimeMinTxt, &lv_font_scdream_regular_40, 0);
    lv_obj_set_style_text_align(bk_ui->automodestart_AutoFermentation1TimeMinTxt, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_pos(bk_ui->automodestart_AutoFermentation1TimeMinTxt, 940, 324);
    lv_obj_set_size(bk_ui->automodestart_AutoFermentation1TimeMinTxt, 60, 50);

    // ImageView: blackout
    bk_ui->automodestart_blackout = lv_image_create(bk_ui->automodestart);
    _img_set_src_deferred(bk_ui->automodestart_blackout, "/images/blackout.png");
    lv_obj_add_flag(bk_ui->automodestart_blackout, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_pos(bk_ui->automodestart_blackout, 841, 384);
    lv_obj_set_size(bk_ui->automodestart_blackout, LV_SIZE_CONTENT, LV_SIZE_CONTENT);

}
