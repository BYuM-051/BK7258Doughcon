#include "lvgl.h"
#include "beken_ui.h"
#include "custom_func.h"
#include "event_runtime.h"
#include "ui_lang.h"
#include <stdio.h>
#include <string.h>
#include "device_state.h"

#include "ui_config.h"
#include "preRenderer.h"

extern bk_lv_ui_t bk_lv_tool_ui;
extern lv_obj_t *preRenderRoot;
extern void settingmodelanguage_backbt_event_cb(lv_event_t *e);
extern void settingmodelanguage_koreanbt_event_cb(lv_event_t *e);
extern void settingmodelanguage_englishbt_event_cb(lv_event_t *e);
extern void settingmodelanguage_chinabt_event_cb(lv_event_t *e);
extern void settingmodelanguage_load_start_event_cb(lv_event_t *e);
extern void settingmodelanguage_loaded_event_cb(lv_event_t *e);
extern void settingmodelanguage_unload_start_event_cb(lv_event_t *e);
extern void settingmodelanguage_unloaded_event_cb(lv_event_t *e);
extern void _update_language_ui();

void destroy_page_settingmodelanguage(bk_lv_ui_t *bk_ui)
{
    if (bk_ui == NULL) {
        return;
    }
    if (bk_ui->settingmodelanguage != NULL) {
        lv_obj_del(bk_ui->settingmodelanguage);
        bk_ui->settingmodelanguage = NULL;
    }
}

void init_page_settingmodelanguage(bk_lv_ui_t * bk_ui) {
    if (bk_ui->settingmodelanguage != NULL && lv_obj_is_valid(bk_ui->settingmodelanguage)) {
        destroy_page_settingmodelanguage(bk_ui);
    }

    ui_lang_reset_settingmodelanguage_cache();

#if UI_PRENDERING_ENABLE
    bk_ui->settingmodelanguage = lv_obj_create(preRenderRoot);
    lv_obj_remove_style_all(bk_ui->settingmodelanguage);
    lv_obj_set_size(bk_ui->settingmodelanguage, 1024, 600);
    lv_obj_set_pos(bk_ui->settingmodelanguage, 0, 0);
    lv_obj_set_style_radius(bk_ui->settingmodelanguage, 0, LV_PART_MAIN);
    lv_obj_set_scrollbar_mode(bk_ui->settingmodelanguage, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_event_cb(bk_ui->settingmodelanguage, settingmodelanguage_load_start_event_cb, UI_EVENT_PAGE_SHOW_START, NULL);
    lv_obj_add_event_cb(bk_ui->settingmodelanguage, settingmodelanguage_loaded_event_cb, UI_EVENT_PAGE_SHOWN,     NULL);
    lv_obj_add_event_cb(bk_ui->settingmodelanguage, settingmodelanguage_unload_start_event_cb, UI_EVENT_PAGE_HIDE_START, NULL);
    lv_obj_add_event_cb(bk_ui->settingmodelanguage, settingmodelanguage_unloaded_event_cb, UI_EVENT_PAGE_HIDDEN,     NULL);
#else
    bk_ui->settingmodelanguage = lv_obj_create(NULL);
    lv_obj_set_size(bk_ui->settingmodelanguage, 1024, 600);
    lv_obj_set_scrollbar_mode(bk_ui->settingmodelanguage, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_event_cb(bk_ui->settingmodelanguage, settingmodelanguage_load_start_event_cb, LV_EVENT_SCREEN_LOAD_START, NULL);
    lv_obj_add_event_cb(bk_ui->settingmodelanguage, settingmodelanguage_loaded_event_cb, LV_EVENT_SCREEN_LOADED,     NULL);
    lv_obj_add_event_cb(bk_ui->settingmodelanguage, settingmodelanguage_unload_start_event_cb, LV_EVENT_SCREEN_UNLOAD_START, NULL);
    lv_obj_add_event_cb(bk_ui->settingmodelanguage, settingmodelanguage_unloaded_event_cb, LV_EVENT_SCREEN_UNLOADED,     NULL);
#endif /* UI_PRENDERING_ENABLE */
    /* 배경 — bg.jpg 대신 단색(0xd9d9d9) */
    bk_ui->settingmodelanguage_bg = lv_image_create(bk_ui->settingmodelanguage);
    lv_obj_add_flag(bk_ui->settingmodelanguage_bg, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_style_bg_color(bk_ui->settingmodelanguage, lv_color_hex(0xd9d9d9), 0);
    lv_obj_set_style_bg_opa(bk_ui->settingmodelanguage, LV_OPA_COVER, 0);
    lv_obj_set_pos(bk_ui->settingmodelanguage_bg, 0, 0);

    // ImageView: title
    bk_ui->settingmodelanguage_title = lv_image_create(bk_ui->settingmodelanguage);
    ui_page_build_set_image_src(bk_ui->settingmodelanguage_title, "/images/language_title.png");
    lv_obj_set_pos(bk_ui->settingmodelanguage_title, 0, 10);
    lv_obj_set_size(bk_ui->settingmodelanguage_title, 380, 80);
    lv_image_set_inner_align(bk_ui->settingmodelanguage_title, LV_IMAGE_ALIGN_TOP_LEFT);

    // Button: backbt
    bk_ui->settingmodelanguage_backbt = lv_button_create(bk_ui->settingmodelanguage);
    lv_obj_add_flag(bk_ui->settingmodelanguage_backbt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->settingmodelanguage_backbt, settingmodelanguage_backbt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->settingmodelanguage_backbt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->settingmodelanguage_backbt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->settingmodelanguage_backbt, 0, 0);
    lv_obj_set_pos(bk_ui->settingmodelanguage_backbt, 825, 13);
    lv_obj_set_size(bk_ui->settingmodelanguage_backbt, 179, 74);

    // ImageView: exitim
    bk_ui->settingmodelanguage_exitim = lv_image_create(bk_ui->settingmodelanguage);
    ui_page_build_set_image_src(bk_ui->settingmodelanguage_exitim, "/images/exit_bt.png");
    lv_obj_set_pos(bk_ui->settingmodelanguage_exitim, 825, 13);
    lv_obj_set_size(bk_ui->settingmodelanguage_exitim, 179, 74);

    // Button: koreanbt
    bk_ui->settingmodelanguage_koreanbt = lv_button_create(bk_ui->settingmodelanguage);
    lv_obj_add_flag(bk_ui->settingmodelanguage_koreanbt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->settingmodelanguage_koreanbt, settingmodelanguage_koreanbt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->settingmodelanguage_koreanbt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->settingmodelanguage_koreanbt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->settingmodelanguage_koreanbt, 0, 0);
    lv_obj_set_pos(bk_ui->settingmodelanguage_koreanbt, 711, 192);
    lv_obj_set_size(bk_ui->settingmodelanguage_koreanbt, 296, 156);

    // ImageView: koreanim
    bk_ui->settingmodelanguage_koreanim = lv_image_create(bk_ui->settingmodelanguage);
    ui_page_build_set_image_src(bk_ui->settingmodelanguage_koreanim, "/images/language_korean_off.png");
    lv_obj_set_pos(bk_ui->settingmodelanguage_koreanim, 711, 192);
    lv_obj_set_size(bk_ui->settingmodelanguage_koreanim, 296, 156);
    //  bk_ui->settingmodelanguage_koreanim = lv_image_create(bk_ui->settingmodelanguage);
    // _img_set_src_timed(bk_ui->settingmodelanguage_koreanim, "/images/language_korean_on.png");
    // lv_obj_set_pos(bk_ui->settingmodelanguage_koreanim, 711, 192);
    // lv_obj_set_size(bk_ui->settingmodelanguage_koreanim, 296, 156);
    // lv_obj_add_flag(bk_ui->settingmodelanguage_koreanim, LV_OBJ_FLAG_HIDDEN);



    // Button: englishbt
    bk_ui->settingmodelanguage_englishbt = lv_button_create(bk_ui->settingmodelanguage);
    lv_obj_add_flag(bk_ui->settingmodelanguage_englishbt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->settingmodelanguage_englishbt, settingmodelanguage_englishbt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->settingmodelanguage_englishbt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->settingmodelanguage_englishbt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->settingmodelanguage_englishbt, 0, 0);
    lv_obj_set_pos(bk_ui->settingmodelanguage_englishbt, 365, 192);
    lv_obj_set_size(bk_ui->settingmodelanguage_englishbt, 296, 156);

    // ImageView: englishim
    bk_ui->settingmodelanguage_englishim = lv_image_create(bk_ui->settingmodelanguage);
    ui_page_build_set_image_src(bk_ui->settingmodelanguage_englishim, "/images/language_english_off.png");
    lv_obj_set_pos(bk_ui->settingmodelanguage_englishim, 365, 192);
    lv_obj_set_size(bk_ui->settingmodelanguage_englishim, 296, 156);
    //  bk_ui->settingmodelanguage_englishim = lv_image_create(bk_ui->settingmodelanguage);
    // _img_set_src_timed(bk_ui->settingmodelanguage_englishim, "/images/language_english_off.png");
    // lv_obj_set_pos(bk_ui->settingmodelanguage_englishim, 365, 192);
    // lv_obj_set_size(bk_ui->settingmodelanguage_englishim, 296, 156);

    // Button: chinabt
    bk_ui->settingmodelanguage_chinabt = lv_button_create(bk_ui->settingmodelanguage);
    lv_obj_add_flag(bk_ui->settingmodelanguage_chinabt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->settingmodelanguage_chinabt, settingmodelanguage_chinabt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->settingmodelanguage_chinabt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->settingmodelanguage_chinabt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->settingmodelanguage_chinabt, 0, 0);
    lv_obj_set_pos(bk_ui->settingmodelanguage_chinabt, 19, 192);
    lv_obj_set_size(bk_ui->settingmodelanguage_chinabt, 296, 156);

    // ImageView: chinaim
    bk_ui->settingmodelanguage_chinaim = lv_image_create(bk_ui->settingmodelanguage);
    ui_page_build_set_image_src(bk_ui->settingmodelanguage_chinaim, "/images/language_china_off.png");
    lv_obj_set_pos(bk_ui->settingmodelanguage_chinaim, 19, 192);
    lv_obj_set_size(bk_ui->settingmodelanguage_chinaim, 296, 156);
    _update_language_ui();

}
