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
extern void settingmodemanual_backbt_event_cb(lv_event_t *e);
extern void settingmodemanual_setting_manual_autobt_event_cb(lv_event_t *e);
extern void settingmodemanual_setting_manual_manualbt_event_cb(lv_event_t *e);
extern void settingmodemanual_setting_manual_drybt_event_cb(lv_event_t *e);
extern void settingmodemanual_setting_manual_memorybt_event_cb(lv_event_t *e);
extern void settingmodemanual_setting_manual_settingbt_event_cb(lv_event_t *e);
extern void settingmodemanual_load_start_event_cb(lv_event_t *e);
extern void settingmodemanual_loaded_event_cb(lv_event_t *e);
extern void settingmodemanual_unload_start_event_cb(lv_event_t *e);
extern void settingmodemanual_unloaded_event_cb(lv_event_t *e);

void destroy_page_settingmodemanual(bk_lv_ui_t *bk_ui)
{
    if (bk_ui == NULL) {
        return;
    }
    if (bk_ui->settingmodemanual != NULL) {
        lv_obj_del(bk_ui->settingmodemanual);
        bk_ui->settingmodemanual = NULL;
    }
}

void init_page_settingmodemanual(bk_lv_ui_t * bk_ui) {
    if (bk_ui->settingmodemanual != NULL && lv_obj_is_valid(bk_ui->settingmodemanual)) {
        destroy_page_settingmodemanual(bk_ui);
    }

    ui_lang_reset_settingmodemanual_cache();

#if UI_PRENDERING_ENABLE
    bk_ui->settingmodemanual = lv_obj_create(preRenderRoot);
    lv_obj_remove_style_all(bk_ui->settingmodemanual);
    lv_obj_set_size(bk_ui->settingmodemanual, 1024, 600);
    lv_obj_set_pos(bk_ui->settingmodemanual, 0, 0);
    lv_obj_set_style_radius(bk_ui->settingmodemanual, 0, LV_PART_MAIN);
    lv_obj_set_scrollbar_mode(bk_ui->settingmodemanual, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_event_cb(bk_ui->settingmodemanual, settingmodemanual_load_start_event_cb, UI_EVENT_PAGE_SHOW_START, NULL);
    lv_obj_add_event_cb(bk_ui->settingmodemanual, settingmodemanual_loaded_event_cb, UI_EVENT_PAGE_SHOWN,     NULL);
    lv_obj_add_event_cb(bk_ui->settingmodemanual, settingmodemanual_unload_start_event_cb, UI_EVENT_PAGE_HIDE_START, NULL);
    lv_obj_add_event_cb(bk_ui->settingmodemanual, settingmodemanual_unloaded_event_cb, UI_EVENT_PAGE_HIDDEN,     NULL);
#else
    bk_ui->settingmodemanual = lv_obj_create(NULL);
    lv_obj_set_size(bk_ui->settingmodemanual, 1024, 600);
    lv_obj_set_scrollbar_mode(bk_ui->settingmodemanual, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_event_cb(bk_ui->settingmodemanual, settingmodemanual_load_start_event_cb, LV_EVENT_SCREEN_LOAD_START, NULL);
    lv_obj_add_event_cb(bk_ui->settingmodemanual, settingmodemanual_loaded_event_cb, LV_EVENT_SCREEN_LOADED,     NULL);
    lv_obj_add_event_cb(bk_ui->settingmodemanual, settingmodemanual_unload_start_event_cb, LV_EVENT_SCREEN_UNLOAD_START, NULL);
    lv_obj_add_event_cb(bk_ui->settingmodemanual, settingmodemanual_unloaded_event_cb, LV_EVENT_SCREEN_UNLOADED,     NULL);
#endif /* UI_PRENDERING_ENABLE */
    bk_ui->settingmodemanual_bg = lv_image_create(bk_ui->settingmodemanual);
    _bg_set(bk_ui->settingmodemanual_bg);
    lv_obj_set_pos(bk_ui->settingmodemanual_bg, 0, 0);

    // ImageView: title
    bk_ui->settingmodemanual_title = lv_image_create(bk_ui->settingmodemanual);
    ui_page_build_set_image_src(bk_ui->settingmodemanual_title, "/images/usermanual_title.png");
    lv_obj_set_pos(bk_ui->settingmodemanual_title, 0, 10);
    lv_obj_set_size(bk_ui->settingmodemanual_title, 380, 80);
    lv_image_set_inner_align(bk_ui->settingmodemanual_title, LV_IMAGE_ALIGN_TOP_LEFT);

    // Button: backbt
    bk_ui->settingmodemanual_backbt = lv_button_create(bk_ui->settingmodemanual);
    lv_obj_add_flag(bk_ui->settingmodemanual_backbt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->settingmodemanual_backbt, settingmodemanual_backbt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->settingmodemanual_backbt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->settingmodemanual_backbt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->settingmodemanual_backbt, 0, 0);
    lv_obj_set_pos(bk_ui->settingmodemanual_backbt, 825, 13);
    lv_obj_set_size(bk_ui->settingmodemanual_backbt, 179, 74);

    // ImageView: imageview3
    bk_ui->settingmodemanual_imageview3 = lv_image_create(bk_ui->settingmodemanual);
    ui_page_build_set_image_src(bk_ui->settingmodemanual_imageview3, "/images/exit_bt.png");
    lv_obj_set_pos(bk_ui->settingmodemanual_imageview3, 825, 13);
    lv_obj_set_size(bk_ui->settingmodemanual_imageview3, 179, 74);

    // ImageView: setting_manual_autoim
    bk_ui->settingmodemanual_setting_manual_autoim = lv_image_create(bk_ui->settingmodemanual);
    ui_page_build_set_image_src(bk_ui->settingmodemanual_setting_manual_autoim, "/images/setting_manual_auto_off.png");
    lv_obj_set_pos(bk_ui->settingmodemanual_setting_manual_autoim, 23, 120);
    lv_obj_set_size(bk_ui->settingmodemanual_setting_manual_autoim, 445, 66);

    // Button: setting_manual_autobt
    bk_ui->settingmodemanual_setting_manual_autobt = lv_button_create(bk_ui->settingmodemanual);
    lv_obj_add_flag(bk_ui->settingmodemanual_setting_manual_autobt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->settingmodemanual_setting_manual_autobt, settingmodemanual_setting_manual_autobt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->settingmodemanual_setting_manual_autobt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->settingmodemanual_setting_manual_autobt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->settingmodemanual_setting_manual_autobt, 0, 0);
    lv_obj_set_pos(bk_ui->settingmodemanual_setting_manual_autobt, 23, 120);
    lv_obj_set_size(bk_ui->settingmodemanual_setting_manual_autobt, 445, 66);

    // ImageView: setting_manual_manualim
    bk_ui->settingmodemanual_setting_manual_manualim = lv_image_create(bk_ui->settingmodemanual);
    ui_page_build_set_image_src(bk_ui->settingmodemanual_setting_manual_manualim, "/images/setting_manual_manual_off.png");
    lv_obj_set_pos(bk_ui->settingmodemanual_setting_manual_manualim, 23, 198);
    lv_obj_set_size(bk_ui->settingmodemanual_setting_manual_manualim, 445, 66);

    // Button: setting_manual_manualbt
    bk_ui->settingmodemanual_setting_manual_manualbt = lv_button_create(bk_ui->settingmodemanual);
    lv_obj_add_flag(bk_ui->settingmodemanual_setting_manual_manualbt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->settingmodemanual_setting_manual_manualbt, settingmodemanual_setting_manual_manualbt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->settingmodemanual_setting_manual_manualbt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->settingmodemanual_setting_manual_manualbt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->settingmodemanual_setting_manual_manualbt, 0, 0);
    lv_obj_set_pos(bk_ui->settingmodemanual_setting_manual_manualbt, 23, 198);
    lv_obj_set_size(bk_ui->settingmodemanual_setting_manual_manualbt, 445, 66);

    // ImageView: setting_manual_dryim
    bk_ui->settingmodemanual_setting_manual_dryim = lv_image_create(bk_ui->settingmodemanual);
    ui_page_build_set_image_src(bk_ui->settingmodemanual_setting_manual_dryim, "/images/setting_manual_dry_off.png");
    lv_obj_set_pos(bk_ui->settingmodemanual_setting_manual_dryim, 23, 276);
    lv_obj_set_size(bk_ui->settingmodemanual_setting_manual_dryim, 445, 66);

    // Button: setting_manual_drybt
    bk_ui->settingmodemanual_setting_manual_drybt = lv_button_create(bk_ui->settingmodemanual);
    lv_obj_add_flag(bk_ui->settingmodemanual_setting_manual_drybt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->settingmodemanual_setting_manual_drybt, settingmodemanual_setting_manual_drybt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->settingmodemanual_setting_manual_drybt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->settingmodemanual_setting_manual_drybt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->settingmodemanual_setting_manual_drybt, 0, 0);
    lv_obj_set_pos(bk_ui->settingmodemanual_setting_manual_drybt, 23, 276);
    lv_obj_set_size(bk_ui->settingmodemanual_setting_manual_drybt, 445, 66);

    // ImageView: setting_manual_memoryim
    bk_ui->settingmodemanual_setting_manual_memoryim = lv_image_create(bk_ui->settingmodemanual);
    ui_page_build_set_image_src(bk_ui->settingmodemanual_setting_manual_memoryim, "/images/setting_manual_memory_off.png");
    lv_obj_set_pos(bk_ui->settingmodemanual_setting_manual_memoryim, 23, 354);
    lv_obj_set_size(bk_ui->settingmodemanual_setting_manual_memoryim, 445, 66);

    // Button: setting_manual_memorybt
    bk_ui->settingmodemanual_setting_manual_memorybt = lv_button_create(bk_ui->settingmodemanual);
    lv_obj_add_flag(bk_ui->settingmodemanual_setting_manual_memorybt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->settingmodemanual_setting_manual_memorybt, settingmodemanual_setting_manual_memorybt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->settingmodemanual_setting_manual_memorybt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->settingmodemanual_setting_manual_memorybt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->settingmodemanual_setting_manual_memorybt, 0, 0);
    lv_obj_set_pos(bk_ui->settingmodemanual_setting_manual_memorybt, 23, 354);
    lv_obj_set_size(bk_ui->settingmodemanual_setting_manual_memorybt, 445, 66);

    // ImageView: setting_manual_settingim
    bk_ui->settingmodemanual_setting_manual_settingim = lv_image_create(bk_ui->settingmodemanual);
    ui_page_build_set_image_src(bk_ui->settingmodemanual_setting_manual_settingim, "/images/setting_manual_setting_off.png");
    lv_obj_set_pos(bk_ui->settingmodemanual_setting_manual_settingim, 23, 432);
    lv_obj_set_size(bk_ui->settingmodemanual_setting_manual_settingim, 445, 66);

    // Button: setting_manual_settingbt
    bk_ui->settingmodemanual_setting_manual_settingbt = lv_button_create(bk_ui->settingmodemanual);
    lv_obj_add_flag(bk_ui->settingmodemanual_setting_manual_settingbt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->settingmodemanual_setting_manual_settingbt, settingmodemanual_setting_manual_settingbt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->settingmodemanual_setting_manual_settingbt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->settingmodemanual_setting_manual_settingbt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->settingmodemanual_setting_manual_settingbt, 0, 0);
    lv_obj_set_pos(bk_ui->settingmodemanual_setting_manual_settingbt, 23, 432);
    lv_obj_set_size(bk_ui->settingmodemanual_setting_manual_settingbt, 445, 66);

    // ImageView: setting_manual_boxim
    bk_ui->settingmodemanual_setting_manual_boxim = lv_image_create(bk_ui->settingmodemanual);
    _img_set_src_deferred(bk_ui->settingmodemanual_setting_manual_boxim, "/images/setting_manual_box.png");
    lv_obj_add_flag(bk_ui->settingmodemanual_setting_manual_boxim, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_pos(bk_ui->settingmodemanual_setting_manual_boxim, 498, 128);
    lv_obj_set_size(bk_ui->settingmodemanual_setting_manual_boxim, 506, 378);

    // TextView: setting_manual_title
    bk_ui->settingmodemanual_setting_manual_title = lv_label_create(bk_ui->settingmodemanual);
    lv_label_set_text(bk_ui->settingmodemanual_setting_manual_title, "자동설정");
    lv_obj_set_style_bg_opa(bk_ui->settingmodemanual_setting_manual_title, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmodemanual_setting_manual_title, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmodemanual_setting_manual_title, &lv_font_scdream_regular_32, 0);
    lv_obj_add_flag(bk_ui->settingmodemanual_setting_manual_title, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_pos(bk_ui->settingmodemanual_setting_manual_title, 529, 138);
    lv_obj_set_size(bk_ui->settingmodemanual_setting_manual_title, 200, 40);

    // TextView: setting_manual_value
    bk_ui->settingmodemanual_setting_manual_value = lv_label_create(bk_ui->settingmodemanual);
    lv_label_set_text(bk_ui->settingmodemanual_setting_manual_value, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmodemanual_setting_manual_value, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmodemanual_setting_manual_value, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmodemanual_setting_manual_value, &lv_font_scdream_regular_22, 0);
    lv_obj_add_flag(bk_ui->settingmodemanual_setting_manual_value, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_pos(bk_ui->settingmodemanual_setting_manual_value, 535, 195);
    lv_obj_set_size(bk_ui->settingmodemanual_setting_manual_value, 430, 280);

}
