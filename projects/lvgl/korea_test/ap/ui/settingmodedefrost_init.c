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
extern void settingmodedefrost_backbt_event_cb(lv_event_t *e);
extern void settingmodedefrost_load_start_event_cb(lv_event_t *e);
extern void settingmodedefrost_loaded_event_cb(lv_event_t *e);
extern void settingmodedefrost_unload_start_event_cb(lv_event_t *e);
extern void settingmodedefrost_unloaded_event_cb(lv_event_t *e);

void destroy_page_settingmodedefrost(bk_lv_ui_t *bk_ui)
{
    if (bk_ui == NULL) {
        return;
    }
    if (bk_ui->settingmodedefrost != NULL) {
        lv_obj_del(bk_ui->settingmodedefrost);
        bk_ui->settingmodedefrost = NULL;
    }
}

void init_page_settingmodedefrost(bk_lv_ui_t * bk_ui) {
    if (bk_ui->settingmodedefrost != NULL && lv_obj_is_valid(bk_ui->settingmodedefrost)) {
        destroy_page_settingmodedefrost(bk_ui);
    }

    ui_lang_reset_settingmodedefrost_cache();

#if UI_PRENDERING_ENABLE
    bk_ui->settingmodedefrost = lv_obj_create(preRenderRoot);
    lv_obj_remove_style_all(bk_ui->settingmodedefrost);
    lv_obj_set_size(bk_ui->settingmodedefrost, 1024, 600);
    lv_obj_set_pos(bk_ui->settingmodedefrost, 0, 0);
    lv_obj_set_style_radius(bk_ui->settingmodedefrost, 0, LV_PART_MAIN);
    lv_obj_set_scrollbar_mode(bk_ui->settingmodedefrost, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_event_cb(bk_ui->settingmodedefrost, settingmodedefrost_load_start_event_cb, UI_EVENT_PAGE_SHOW_START, NULL);
    lv_obj_add_event_cb(bk_ui->settingmodedefrost, settingmodedefrost_loaded_event_cb, UI_EVENT_PAGE_SHOWN,     NULL);
    lv_obj_add_event_cb(bk_ui->settingmodedefrost, settingmodedefrost_unload_start_event_cb, UI_EVENT_PAGE_HIDE_START, NULL);
    lv_obj_add_event_cb(bk_ui->settingmodedefrost, settingmodedefrost_unloaded_event_cb, UI_EVENT_PAGE_HIDDEN,     NULL);
#else
    bk_ui->settingmodedefrost = lv_obj_create(NULL);
    lv_obj_set_size(bk_ui->settingmodedefrost, 1024, 600);
    lv_obj_set_scrollbar_mode(bk_ui->settingmodedefrost, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_event_cb(bk_ui->settingmodedefrost, settingmodedefrost_load_start_event_cb, LV_EVENT_SCREEN_LOAD_START, NULL);
    lv_obj_add_event_cb(bk_ui->settingmodedefrost, settingmodedefrost_loaded_event_cb, LV_EVENT_SCREEN_LOADED,     NULL);
    lv_obj_add_event_cb(bk_ui->settingmodedefrost, settingmodedefrost_unload_start_event_cb, LV_EVENT_SCREEN_UNLOAD_START, NULL);
    lv_obj_add_event_cb(bk_ui->settingmodedefrost, settingmodedefrost_unloaded_event_cb, LV_EVENT_SCREEN_UNLOADED,     NULL);
#endif /* UI_PRENDERING_ENABLE */
    bk_ui->settingmodedefrost_bg = lv_image_create(bk_ui->settingmodedefrost);
    _bg_set(bk_ui->settingmodedefrost_bg);
    lv_obj_set_pos(bk_ui->settingmodedefrost_bg, 0, 0);

    // ImageView: title
    bk_ui->settingmodedefrost_title = lv_image_create(bk_ui->settingmodedefrost);
    ui_page_build_set_image_src(bk_ui->settingmodedefrost_title, "/images/setting_title.png");
    lv_obj_set_pos(bk_ui->settingmodedefrost_title, 0, 10);
    lv_obj_set_size(bk_ui->settingmodedefrost_title, 380, 80);
    lv_image_set_inner_align(bk_ui->settingmodedefrost_title, LV_IMAGE_ALIGN_TOP_LEFT);

    // Button: backbt
    bk_ui->settingmodedefrost_backbt = lv_button_create(bk_ui->settingmodedefrost);
    lv_obj_add_flag(bk_ui->settingmodedefrost_backbt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->settingmodedefrost_backbt, settingmodedefrost_backbt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->settingmodedefrost_backbt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->settingmodedefrost_backbt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->settingmodedefrost_backbt, 0, 0);
    lv_obj_set_pos(bk_ui->settingmodedefrost_backbt, 13, 445);
    lv_obj_set_size(bk_ui->settingmodedefrost_backbt, 179, 74);

    // ImageView: imageview3
    bk_ui->settingmodedefrost_imageview3 = lv_image_create(bk_ui->settingmodedefrost);
    ui_page_build_set_image_src(bk_ui->settingmodedefrost_imageview3, "/images/exit_bt.png");
    lv_obj_set_pos(bk_ui->settingmodedefrost_imageview3, 13, 445);
    lv_obj_set_size(bk_ui->settingmodedefrost_imageview3, 179, 74);

}
