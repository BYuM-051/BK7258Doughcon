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
extern void detailsettinghumidity_backbt_event_cb(lv_event_t *e);
extern void detailsettinghumidity_settingbt1_event_cb(lv_event_t *e);
extern void detailsettinghumidity_settingbt2_event_cb(lv_event_t *e);
extern void detailsettinghumidity_settingbt3_event_cb(lv_event_t *e);
extern void detailsettinghumidity_changebt_event_cb(lv_event_t *e);
extern void detailsettinghumidity_roller_event_cb(lv_event_t *e);
extern void detailsettinghumidity_load_start_event_cb(lv_event_t *e);
extern void detailsettinghumidity_loaded_event_cb(lv_event_t *e);
extern void detailsettinghumidity_unload_start_event_cb(lv_event_t *e);
extern void detailsettinghumidity_unloaded_event_cb(lv_event_t *e);

void destroy_page_detailsettinghumidity(bk_lv_ui_t *bk_ui)
{
    if (bk_ui == NULL) {
        return;
    }
    if (bk_ui->detailsettinghumidity != NULL) {
        lv_obj_del(bk_ui->detailsettinghumidity);
        bk_ui->detailsettinghumidity = NULL;
    }
}

void init_page_detailsettinghumidity(bk_lv_ui_t * bk_ui) {
    if (bk_ui->detailsettinghumidity != NULL && lv_obj_is_valid(bk_ui->detailsettinghumidity)) {
        destroy_page_detailsettinghumidity(bk_ui);
    }

    ui_lang_reset_detailsettinghumidity_cache();

#if UI_PRENDERING_ENABLE
    bk_ui->detailsettinghumidity = lv_obj_create(preRenderRoot);
    lv_obj_remove_style_all(bk_ui->detailsettinghumidity);
    lv_obj_set_size(bk_ui->detailsettinghumidity, 1024, 600);
    lv_obj_set_pos(bk_ui->detailsettinghumidity, 0, 0);
    lv_obj_set_style_radius(bk_ui->detailsettinghumidity, 0, LV_PART_MAIN);
    lv_obj_set_scrollbar_mode(bk_ui->detailsettinghumidity, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_event_cb(bk_ui->detailsettinghumidity, detailsettinghumidity_load_start_event_cb, UI_EVENT_PAGE_SHOW_START, NULL);
    lv_obj_add_event_cb(bk_ui->detailsettinghumidity, detailsettinghumidity_loaded_event_cb, UI_EVENT_PAGE_SHOWN,     NULL);
    lv_obj_add_event_cb(bk_ui->detailsettinghumidity, detailsettinghumidity_unload_start_event_cb, UI_EVENT_PAGE_HIDE_START, NULL);
    lv_obj_add_event_cb(bk_ui->detailsettinghumidity, detailsettinghumidity_unloaded_event_cb, UI_EVENT_PAGE_HIDDEN,     NULL);
#else
    bk_ui->detailsettinghumidity = lv_obj_create(NULL);
    lv_obj_set_size(bk_ui->detailsettinghumidity, 1024, 600);
    lv_obj_set_scrollbar_mode(bk_ui->detailsettinghumidity, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_event_cb(bk_ui->detailsettinghumidity, detailsettinghumidity_load_start_event_cb, LV_EVENT_SCREEN_LOAD_START, NULL);
    lv_obj_add_event_cb(bk_ui->detailsettinghumidity, detailsettinghumidity_loaded_event_cb, LV_EVENT_SCREEN_LOADED,     NULL);
    lv_obj_add_event_cb(bk_ui->detailsettinghumidity, detailsettinghumidity_unload_start_event_cb, LV_EVENT_SCREEN_UNLOAD_START, NULL);
    lv_obj_add_event_cb(bk_ui->detailsettinghumidity, detailsettinghumidity_unloaded_event_cb, LV_EVENT_SCREEN_UNLOADED,     NULL);
#endif /* UI_PRENDERING_ENABLE */
    bk_ui->detailsettinghumidity_bg = lv_image_create(bk_ui->detailsettinghumidity);
    lv_obj_add_flag(bk_ui->detailsettinghumidity_bg, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_style_bg_color(bk_ui->detailsettinghumidity, lv_color_hex(0xd9d9d9), 0);
    lv_obj_set_style_bg_opa(bk_ui->detailsettinghumidity, LV_OPA_COVER, 0);
    lv_obj_set_pos(bk_ui->detailsettinghumidity_bg, 0, 0);

    // ImageView: title
    bk_ui->detailsettinghumidity_title = lv_image_create(bk_ui->detailsettinghumidity);
    lv_obj_set_pos(bk_ui->detailsettinghumidity_title, 0, 10);
    lv_obj_set_size(bk_ui->detailsettinghumidity_title, 380, 80);
    lv_image_set_inner_align(bk_ui->detailsettinghumidity_title, LV_IMAGE_ALIGN_TOP_LEFT);

    // Button: backbt
    bk_ui->detailsettinghumidity_backbt = lv_button_create(bk_ui->detailsettinghumidity);
    lv_obj_add_flag(bk_ui->detailsettinghumidity_backbt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->detailsettinghumidity_backbt, detailsettinghumidity_backbt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->detailsettinghumidity_backbt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->detailsettinghumidity_backbt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->detailsettinghumidity_backbt, 0, 0);
    lv_obj_set_pos(bk_ui->detailsettinghumidity_backbt, 825, 13);
    lv_obj_set_size(bk_ui->detailsettinghumidity_backbt, 179, 74);

    // ImageView: imageview3
    bk_ui->detailsettinghumidity_imageview3 = lv_image_create(bk_ui->detailsettinghumidity);
    lv_obj_set_pos(bk_ui->detailsettinghumidity_imageview3, 825, 13);
    lv_obj_set_size(bk_ui->detailsettinghumidity_imageview3, 179, 74);

    // ImageView: settingim1
    bk_ui->detailsettinghumidity_settingim1 = lv_image_create(bk_ui->detailsettinghumidity);
    lv_obj_set_pos(bk_ui->detailsettinghumidity_settingim1, 23, 120);
    lv_obj_set_size(bk_ui->detailsettinghumidity_settingim1, 538, 66);

    // TextView: settingtxt1
    bk_ui->detailsettinghumidity_settingtxt1 = lv_label_create(bk_ui->detailsettinghumidity);
    lv_label_set_text(bk_ui->detailsettinghumidity_settingtxt1, "");
    lv_obj_set_style_bg_opa(bk_ui->detailsettinghumidity_settingtxt1, 0, 0);
    lv_obj_set_style_text_color(bk_ui->detailsettinghumidity_settingtxt1, lv_color_hex(0xA6A6A6), 0);
    lv_obj_set_style_text_font(bk_ui->detailsettinghumidity_settingtxt1, &lv_font_scdream_regular_32, 0);
    /* 폭 50px는 "-10" 같은 3자리 값이 한 줄에 안 들어가 자동 줄바꿈(2줄)되던 원인 —
     * 우측 정렬 기준(오른쪽 끝 x=470)은 유지한 채 왼쪽으로 20px 넓히고, long_mode를
     * CLIP으로 고정해 항상 한 줄로 표시되게 함. */
    lv_obj_set_pos(bk_ui->detailsettinghumidity_settingtxt1, 400, 135);
    lv_obj_set_size(bk_ui->detailsettinghumidity_settingtxt1, 70, 42);
    lv_label_set_long_mode(bk_ui->detailsettinghumidity_settingtxt1, LV_LABEL_LONG_CLIP);
    lv_obj_set_style_text_align(bk_ui->detailsettinghumidity_settingtxt1, LV_TEXT_ALIGN_RIGHT, 0);

    // Button: settingbt1
    bk_ui->detailsettinghumidity_settingbt1 = lv_button_create(bk_ui->detailsettinghumidity);
    lv_obj_add_flag(bk_ui->detailsettinghumidity_settingbt1, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->detailsettinghumidity_settingbt1, detailsettinghumidity_settingbt1_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->detailsettinghumidity_settingbt1, 0, 0);
    lv_obj_set_style_border_width(bk_ui->detailsettinghumidity_settingbt1, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->detailsettinghumidity_settingbt1, 0, 0);
    lv_obj_set_pos(bk_ui->detailsettinghumidity_settingbt1, 23, 120);
    lv_obj_set_size(bk_ui->detailsettinghumidity_settingbt1, 538, 66);

    // ImageView: settingim2
    bk_ui->detailsettinghumidity_settingim2 = lv_image_create(bk_ui->detailsettinghumidity);
    lv_obj_set_pos(bk_ui->detailsettinghumidity_settingim2, 23, 198);
    lv_obj_set_size(bk_ui->detailsettinghumidity_settingim2, 538, 66);

    // TextView: settingtxt2
    bk_ui->detailsettinghumidity_settingtxt2 = lv_label_create(bk_ui->detailsettinghumidity);
    lv_label_set_text(bk_ui->detailsettinghumidity_settingtxt2, "");
    lv_obj_set_style_bg_opa(bk_ui->detailsettinghumidity_settingtxt2, 0, 0);
    lv_obj_set_style_text_color(bk_ui->detailsettinghumidity_settingtxt2, lv_color_hex(0xA6A6A6), 0);
    lv_obj_set_style_text_font(bk_ui->detailsettinghumidity_settingtxt2, &lv_font_scdream_regular_32, 0);
    lv_obj_set_pos(bk_ui->detailsettinghumidity_settingtxt2, 420, 213);
    lv_obj_set_size(bk_ui->detailsettinghumidity_settingtxt2, 50, 42);
    lv_obj_set_style_text_align(bk_ui->detailsettinghumidity_settingtxt2, LV_TEXT_ALIGN_RIGHT, 0);

    // Button: settingbt2
    bk_ui->detailsettinghumidity_settingbt2 = lv_button_create(bk_ui->detailsettinghumidity);
    lv_obj_add_flag(bk_ui->detailsettinghumidity_settingbt2, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->detailsettinghumidity_settingbt2, detailsettinghumidity_settingbt2_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->detailsettinghumidity_settingbt2, 0, 0);
    lv_obj_set_style_border_width(bk_ui->detailsettinghumidity_settingbt2, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->detailsettinghumidity_settingbt2, 0, 0);
    lv_obj_set_pos(bk_ui->detailsettinghumidity_settingbt2, 23, 198);
    lv_obj_set_size(bk_ui->detailsettinghumidity_settingbt2, 538, 66);

    // ImageView: settingim3
    bk_ui->detailsettinghumidity_settingim3 = lv_image_create(bk_ui->detailsettinghumidity);
    lv_obj_set_pos(bk_ui->detailsettinghumidity_settingim3, 23, 276);
    lv_obj_set_size(bk_ui->detailsettinghumidity_settingim3, 538, 66);

    // TextView: settingtxt3
    bk_ui->detailsettinghumidity_settingtxt3 = lv_label_create(bk_ui->detailsettinghumidity);
    lv_label_set_text(bk_ui->detailsettinghumidity_settingtxt3, "");
    lv_obj_set_style_bg_opa(bk_ui->detailsettinghumidity_settingtxt3, 0, 0);
    lv_obj_set_style_text_color(bk_ui->detailsettinghumidity_settingtxt3, lv_color_hex(0xA6A6A6), 0);
    lv_obj_set_style_text_font(bk_ui->detailsettinghumidity_settingtxt3, &lv_font_scdream_regular_32, 0);
    /* settingtxt1과 동일한 이유(폭 50px에서 "-10"이 2줄로 줄바꿈됨) — 우측 정렬
     * 기준(오른쪽 끝 x=470)은 유지한 채 왼쪽으로 20px 넓히고 CLIP으로 고정. */
    lv_obj_set_pos(bk_ui->detailsettinghumidity_settingtxt3, 400, 291);
    lv_obj_set_size(bk_ui->detailsettinghumidity_settingtxt3, 70, 42);
    lv_label_set_long_mode(bk_ui->detailsettinghumidity_settingtxt3, LV_LABEL_LONG_CLIP);
    lv_obj_set_style_text_align(bk_ui->detailsettinghumidity_settingtxt3, LV_TEXT_ALIGN_RIGHT, 0);

    // Button: settingbt3
    bk_ui->detailsettinghumidity_settingbt3 = lv_button_create(bk_ui->detailsettinghumidity);
    lv_obj_add_flag(bk_ui->detailsettinghumidity_settingbt3, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->detailsettinghumidity_settingbt3, detailsettinghumidity_settingbt3_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->detailsettinghumidity_settingbt3, 0, 0);
    lv_obj_set_style_border_width(bk_ui->detailsettinghumidity_settingbt3, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->detailsettinghumidity_settingbt3, 0, 0);
    lv_obj_set_pos(bk_ui->detailsettinghumidity_settingbt3, 23, 276);
    lv_obj_set_size(bk_ui->detailsettinghumidity_settingbt3, 538, 66);

    // ImageView: pickerbox
    bk_ui->detailsettinghumidity_pickerbox = lv_image_create(bk_ui->detailsettinghumidity);
    ui_lang_apply_picker(bk_ui->detailsettinghumidity_pickerbox, 1);
    lv_obj_set_pos(bk_ui->detailsettinghumidity_pickerbox, 624, 120);
    lv_obj_set_size(bk_ui->detailsettinghumidity_pickerbox, 376, 376);
    lv_obj_add_flag(bk_ui->detailsettinghumidity_pickerbox, LV_OBJ_FLAG_HIDDEN);


    // NumberPicker: settemp_setn1 (lv_roller)
    bk_ui->detailsettinghumidity_settemp_setn1 = lv_roller_create(bk_ui->detailsettinghumidity);
    lv_roller_set_options(bk_ui->detailsettinghumidity_settemp_setn1, "0", LV_ROLLER_MODE_NORMAL);
    /* 폰트 스타일을 먼저 적용 — visible_row_count는 현재 폰트의 line height로 높이를
     * 계산하므로 폰트 적용 전에 호출하면 롤러 높이가 작게 계산된다 */
    lv_obj_set_style_bg_opa(bk_ui->detailsettinghumidity_settemp_setn1, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(bk_ui->detailsettinghumidity_settemp_setn1, 0, LV_PART_MAIN);
    lv_obj_set_style_text_color(bk_ui->detailsettinghumidity_settemp_setn1, lv_color_hex(0xCCCCCC), LV_PART_MAIN);
    lv_obj_set_style_text_font(bk_ui->detailsettinghumidity_settemp_setn1, &lv_font_scdream_regular_72, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(bk_ui->detailsettinghumidity_settemp_setn1, LV_OPA_TRANSP, LV_PART_SELECTED);
    lv_obj_set_style_text_color(bk_ui->detailsettinghumidity_settemp_setn1, lv_color_hex(0x333333), LV_PART_SELECTED);
    lv_obj_set_style_text_font(bk_ui->detailsettinghumidity_settemp_setn1, &lv_font_scdream_regular_90, LV_PART_SELECTED);
    lv_roller_set_visible_row_count(bk_ui->detailsettinghumidity_settemp_setn1, 3);
    lv_obj_add_event_cb(bk_ui->detailsettinghumidity_settemp_setn1, detailsettinghumidity_roller_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_add_flag(bk_ui->detailsettinghumidity_settemp_setn1, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_pos(bk_ui->detailsettinghumidity_settemp_setn1, 700-10, 136-5-2-1);
    lv_obj_set_width(bk_ui->detailsettinghumidity_settemp_setn1, 240);

    // Button: changebt
    bk_ui->detailsettinghumidity_changebt = lv_button_create(bk_ui->detailsettinghumidity);
    lv_obj_add_flag(bk_ui->detailsettinghumidity_changebt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->detailsettinghumidity_changebt, detailsettinghumidity_changebt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->detailsettinghumidity_changebt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->detailsettinghumidity_changebt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->detailsettinghumidity_changebt, 0, 0);
    lv_obj_set_pos(bk_ui->detailsettinghumidity_changebt, 590, 420);
    lv_obj_set_size(bk_ui->detailsettinghumidity_changebt, 410, 80);

    ui_lang_apply_detailsettinghumidity(bk_ui);
}
