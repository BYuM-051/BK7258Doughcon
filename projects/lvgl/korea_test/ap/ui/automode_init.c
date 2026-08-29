#include "lvgl.h"
#include "beken_ui.h"
#include "custom_func.h"
#include "event_runtime.h"
#include "ui_lang.h"
#include <stdio.h>
#include <string.h>

#include "ui_config.h"
#include "preRenderer.h"

#define TAG "[automode_init.c] "
#include "preRenderer.h"
// #define bk_printf(fmt, ...) do {if(0) bk_printf(fmt, ##__VA_ARGS__); } while(0) // disable printf
extern bk_lv_ui_t bk_lv_tool_ui;
extern lv_obj_t *preRenderRoot;

// void init_keypad_group(bk_lv_ui_t *bk_ui);
extern void automode_backbt_event_cb(lv_event_t *e);
extern void automode_startbt_event_cb(lv_event_t *e);
extern void automode_AutoModeCompleteYearBt_event_cb(lv_event_t *e);
extern void automode_AutoModeCompleteMonthBt_event_cb(lv_event_t *e);
extern void automode_AutoModeCompleteDayBt_event_cb(lv_event_t *e);
extern void automode_AutoModeCompleteHourBt_event_cb(lv_event_t *e);
extern void automode_AutoModeCompleteMinBt_event_cb(lv_event_t *e);
extern void automode_loadbt_event_cb(lv_event_t *e);
extern void automode_savebt_event_cb(lv_event_t *e);
extern void automode_AutoFreezeTempBt_event_cb(lv_event_t *e);
extern void automode_AutoDefrostTempBt_event_cb(lv_event_t *e);
extern void automode_AutoDefrostTimeHourBt_event_cb(lv_event_t *e);
extern void automode_AutoDefrostTimeMinBt_event_cb(lv_event_t *e);
extern void automode_AutoFermentation1TempBt_event_cb(lv_event_t *e);
extern void automode_AutoFermentation1HumidityBt_event_cb(lv_event_t *e);
extern void automode_AutoFermentation1TimeHourBt_event_cb(lv_event_t *e);
extern void automode_AutoFermentation1TimeMinBt_event_cb(lv_event_t *e);
extern void automode_AutoFermentation2TempBt_event_cb(lv_event_t *e);
extern void automode_AutoFermentation2HumidityBt_event_cb(lv_event_t *e);
extern void automode_AutoFermentation2TimeHourBt_event_cb(lv_event_t *e);
extern void automode_AutoFermentation2TimeMinBt_event_cb(lv_event_t *e);
extern void keypad_touch_event_cb(lv_event_t *e);
extern void automode_keypadhide_event_cb(lv_event_t *e);
extern void automode_load_start_event_cb(lv_event_t *e);
extern void automode_loaded_event_cb(lv_event_t *e);
extern void automode_unload_start_event_cb(lv_event_t *e);
extern void automode_unloaded_event_cb(lv_event_t *e);

void destroy_page_automode(bk_lv_ui_t *bk_ui)
{
    bk_printf(TAG "[SCREEN] destroy_page_automode() called\n");
    if (bk_ui == NULL) return;
    if (bk_ui->automode != NULL) {
        lv_obj_del(bk_ui->automode);
        bk_ui->automode = NULL;
    }
    bk_printf(TAG "[SCREEN] destroyed\n");
    /* Reset lazily-initialized child pointers. lv_obj_del() deletes all children,
     * leaving these as dangling pointers. If not cleared, lazy-create guards like
     * "if (!bk_ui->automode_auto_f1)" skip recreation on next visit. */
    bk_ui->automode_auto_f1 = NULL;
    bk_ui->automode_auto_f2 = NULL;
    bk_ui->automode_auto_f3 = NULL;
    bk_ui->automode_auto_f4 = NULL;
    bk_ui->automode_keypadbaseim = NULL;
    bk_ui->automode_keypadhide_im = NULL;
    for (int i = 0; i < 12; i++) {
        bk_ui->automode_KeyPadBt[i] = NULL;
        bk_ui->automode_KeyPadIm[i] = NULL;
    }
    bk_ui->automode_AutoModeCompleteYearUnderBarIm       = NULL;
    bk_ui->automode_AutoModeCompleteMonthUnderBarIm      = NULL;
    bk_ui->automode_AutoModeCompleteDayUnderBarIm        = NULL;
    bk_ui->automode_AutoModeCompleteHourUnderBarIm       = NULL;
    bk_ui->automode_AutoModeCompleteMinUnderBarIm        = NULL;
    bk_ui->automode_AutoModeFreezeTempUnderBarIm         = NULL;
    bk_ui->automode_AutoModeDefrostTempUnderBarIm        = NULL;
    bk_ui->automode_AutoModeDefrostTimeHourUnderBarIm    = NULL;
    bk_ui->automode_AutoModeDefrostTimeMinUnderBarIm     = NULL;
    bk_ui->automode_AutoModeFermentation1TempUnderBarIm      = NULL;
    bk_ui->automode_AutoModeFermentation1HumidityUnderBarIm  = NULL;
    bk_ui->automode_AutoModeFermentation1TimeHourUnderBarIm  = NULL;
    bk_ui->automode_AutoModeFermentation1TimeMinUnderBarIm   = NULL;
    bk_ui->automode_AutoModeFermentation2TempUnderBarIm      = NULL;
    bk_ui->automode_AutoModeFermentation2HumidityUnderBarIm  = NULL;
    bk_ui->automode_AutoModeFermentation2TimeHourUnderBarIm  = NULL;
    bk_ui->automode_AutoModeFermentation2TimeMinUnderBarIm   = NULL;
    bk_printf(TAG "[SCREEN] child pointers reset\n");

#if UI_PRENDERING_ENABLE
    // TODO : free background image buffer if allocated
    
#endif /* UI_PRENDERING_ENABLE */
}


void init_page_automode(bk_lv_ui_t * bk_ui) 
{
    uint32_t _t_start = lv_tick_get();
    bk_printf(TAG "[IMGTIME] ===== automode init(create) start =====\n");

    if (bk_ui->automode != NULL && lv_obj_is_valid(bk_ui->automode)) {
        destroy_page_automode(bk_ui);
    }
    /* 오브젝트를 새로 만드므로 ui_lang 캐시를 무효화 — 다음 SCREEN_LOAD_START의
     * ui_lang_apply_automode()가 언어 변경 여부와 무관하게 반드시 새 이미지를 채우게 함 */
    ui_lang_reset_automode_cache();

#if UI_PRENDERING_ENABLE
    bk_ui->automode = lv_obj_create(preRenderRoot);
    lv_obj_remove_style_all(bk_ui->automode);
    lv_obj_set_size(bk_ui->automode, 1024, 600);
    lv_obj_set_style_radius(bk_ui->automode, 0, LV_PART_MAIN);
    lv_obj_set_pos(bk_ui->automode, 0, 0);
    lv_obj_set_scrollbar_mode(bk_ui->automode, LV_SCROLLBAR_MODE_OFF);

    lv_obj_add_event_cb(bk_ui->automode, automode_load_start_event_cb, UI_EVENT_PAGE_SHOW_START,   NULL);
    lv_obj_add_event_cb(bk_ui->automode, automode_loaded_event_cb, UI_EVENT_PAGE_SHOWN,       NULL);
    lv_obj_add_event_cb(bk_ui->automode, automode_unloaded_event_cb, UI_EVENT_PAGE_HIDDEN, NULL);
    // lv_obj_add_event_cb(bk_ui->automode, automode_unload_start_event_cb, UI_EVENT_PAGE_HIDE_START, NULL);
    // bk_ui->automode_bg = lv_image_create(bk_ui->automode);
    // _img_set_src_timed(bk_ui->automode_bg, "/images/auto_mode_bgi.jpg");
    // lv_obj_set_pos(bk_ui->automode_bg, 0, 0);
    
    lv_obj_set_style_bg_color(bk_ui->automode, lv_color_hex(0xD9D9D9), 0);
    lv_obj_set_style_bg_opa(bk_ui->automode, LV_OPA_COVER, 0);
#else
    bk_ui->automode = lv_obj_create(NULL);
    lv_obj_set_size(bk_ui->automode, 1024, 600);
    lv_obj_set_scrollbar_mode(bk_ui->automode, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_event_cb(bk_ui->automode, automode_load_start_event_cb, LV_EVENT_SCREEN_LOAD_START,   NULL);
    lv_obj_add_event_cb(bk_ui->automode, automode_loaded_event_cb, LV_EVENT_SCREEN_LOADED,       NULL);
    lv_obj_add_event_cb(bk_ui->automode, automode_unloaded_event_cb, LV_EVENT_SCREEN_UNLOADED, NULL);
    lv_obj_add_event_cb(bk_ui->automode, automode_unload_start_event_cb, LV_EVENT_SCREEN_UNLOAD_START, NULL);
    bk_ui->automode_bg = lv_image_create(bk_ui->automode);
    // _img_set_src_timed(bk_ui->automode_bg, "/images/auto_mode_bgi.jpg");
    // lv_obj_set_pos(bk_ui->automode_bg, 0, 0);
    
    lv_obj_set_style_bg_color(bk_ui->automode, lv_color_hex(0xD9D9D9), 0);
    lv_obj_set_style_bg_opa(bk_ui->automode, LV_OPA_COVER, 0);

#endif /* UI_PRENDERING_ENABLE */
    // ImageView: title
    bk_ui->automode_title = lv_image_create(bk_ui->automode);
    _img_set_src_timed(bk_ui->automode_title, "/images/automode_title.png");
    lv_obj_set_pos(bk_ui->automode_title, 0, 10);
    lv_obj_set_size(bk_ui->automode_title, 380, 80);
    lv_image_set_inner_align(bk_ui->automode_title, LV_IMAGE_ALIGN_TOP_LEFT);

    // Button: backbt
    bk_ui->automode_backbt = lv_button_create(bk_ui->automode);
    lv_obj_add_flag(bk_ui->automode_backbt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->automode_backbt, automode_backbt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->automode_backbt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->automode_backbt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->automode_backbt, 0, 0);
    lv_obj_set_pos(bk_ui->automode_backbt, 13, 445);
    lv_obj_set_size(bk_ui->automode_backbt, 179, 74);

    // ImageView: imageview3
    bk_ui->automode_imageview3 = lv_image_create(bk_ui->automode);
    _img_set_src_timed(bk_ui->automode_imageview3, "/images/exit_bt.png");
    lv_obj_set_pos(bk_ui->automode_imageview3, 13, 445);
    lv_obj_set_size(bk_ui->automode_imageview3, 179, 74);

    // Button: startbt
    bk_ui->automode_startbt = lv_button_create(bk_ui->automode);
    lv_obj_add_flag(bk_ui->automode_startbt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->automode_startbt, automode_startbt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->automode_startbt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->automode_startbt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->automode_startbt, 0, 0);
    lv_obj_set_pos(bk_ui->automode_startbt, 847, 445);
    lv_obj_set_size(bk_ui->automode_startbt, 164, 74);

    // ImageView: imageview5
    bk_ui->automode_imageview5 = lv_image_create(bk_ui->automode);
    _img_set_src_timed(bk_ui->automode_imageview5, "/images/start_bt.png");
    lv_obj_set_pos(bk_ui->automode_imageview5, 847, 445);
    lv_obj_set_size(bk_ui->automode_imageview5, 164, 74);

    // ImageView: imageview6
    bk_ui->automode_imageview6 = lv_image_create(bk_ui->automode);
    _img_set_src_timed(bk_ui->automode_imageview6, "/images/auto_mode_start_box_time.png");
    lv_obj_set_pos(bk_ui->automode_imageview6, 17, 96);
    lv_obj_set_size(bk_ui->automode_imageview6, 528, 70);

    // TextView: AutoModeCompleteYear
    bk_ui->automode_AutoModeCompleteYear = lv_label_create(bk_ui->automode);
    lv_label_set_text(bk_ui->automode_AutoModeCompleteYear, "");
    lv_obj_set_style_bg_opa(bk_ui->automode_AutoModeCompleteYear, 0, 0);
    lv_obj_set_style_text_color(bk_ui->automode_AutoModeCompleteYear, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->automode_AutoModeCompleteYear, &lv_font_scdream_regular_33, 0);
    lv_obj_set_pos(bk_ui->automode_AutoModeCompleteYear, 191, 109+5);
    lv_obj_set_size(bk_ui->automode_AutoModeCompleteYear, 80, 44);

    // Button: AutoModeCompleteYearBt
    bk_ui->automode_AutoModeCompleteYearBt = lv_button_create(bk_ui->automode);
    lv_obj_add_flag(bk_ui->automode_AutoModeCompleteYearBt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->automode_AutoModeCompleteYearBt, automode_AutoModeCompleteYearBt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->automode_AutoModeCompleteYearBt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->automode_AutoModeCompleteYearBt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->automode_AutoModeCompleteYearBt, 0, 0);
    lv_obj_set_pos(bk_ui->automode_AutoModeCompleteYearBt, 191, 110);
    lv_obj_set_size(bk_ui->automode_AutoModeCompleteYearBt, 80, 44);


    // TextView: AutoModeCompleteMonth
    bk_ui->automode_AutoModeCompleteMonth = lv_label_create(bk_ui->automode);
    lv_label_set_text(bk_ui->automode_AutoModeCompleteMonth, "");
    lv_obj_set_style_bg_opa(bk_ui->automode_AutoModeCompleteMonth, 0, 0);
    lv_obj_set_style_text_color(bk_ui->automode_AutoModeCompleteMonth, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->automode_AutoModeCompleteMonth, &lv_font_scdream_regular_33, 0);
    lv_obj_set_pos(bk_ui->automode_AutoModeCompleteMonth, 288, 109+5);
    lv_obj_set_size(bk_ui->automode_AutoModeCompleteMonth, 40, 44);

    // Button: AutoModeCompleteMonthBt
    bk_ui->automode_AutoModeCompleteMonthBt = lv_button_create(bk_ui->automode);
    lv_obj_add_flag(bk_ui->automode_AutoModeCompleteMonthBt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->automode_AutoModeCompleteMonthBt, automode_AutoModeCompleteMonthBt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->automode_AutoModeCompleteMonthBt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->automode_AutoModeCompleteMonthBt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->automode_AutoModeCompleteMonthBt, 0, 0);
    lv_obj_set_pos(bk_ui->automode_AutoModeCompleteMonthBt, 279, 110);
    lv_obj_set_size(bk_ui->automode_AutoModeCompleteMonthBt, 55, 44);


    // TextView: AutoModeCompleteDay
    bk_ui->automode_AutoModeCompleteDay = lv_label_create(bk_ui->automode);
    lv_label_set_text(bk_ui->automode_AutoModeCompleteDay, "");
    lv_obj_set_style_bg_opa(bk_ui->automode_AutoModeCompleteDay, 0, 0);
    lv_obj_set_style_text_color(bk_ui->automode_AutoModeCompleteDay, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->automode_AutoModeCompleteDay, &lv_font_scdream_regular_33, 0);
    lv_obj_set_pos(bk_ui->automode_AutoModeCompleteDay, 346, 109+5);
    lv_obj_set_size(bk_ui->automode_AutoModeCompleteDay, 40, 44);

    // Button: AutoModeCompleteDayBt
    bk_ui->automode_AutoModeCompleteDayBt = lv_button_create(bk_ui->automode);
    lv_obj_add_flag(bk_ui->automode_AutoModeCompleteDayBt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->automode_AutoModeCompleteDayBt, automode_AutoModeCompleteDayBt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->automode_AutoModeCompleteDayBt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->automode_AutoModeCompleteDayBt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->automode_AutoModeCompleteDayBt, 0, 0);
    lv_obj_set_pos(bk_ui->automode_AutoModeCompleteDayBt, 338, 110);
    lv_obj_set_size(bk_ui->automode_AutoModeCompleteDayBt, 55, 44);


    // TextView: AutoModeCompleteHour
    bk_ui->automode_AutoModeCompleteHour = lv_label_create(bk_ui->automode);
    lv_label_set_text(bk_ui->automode_AutoModeCompleteHour, "");
    lv_obj_set_style_bg_opa(bk_ui->automode_AutoModeCompleteHour, 0, 0);
    lv_obj_set_style_text_color(bk_ui->automode_AutoModeCompleteHour, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->automode_AutoModeCompleteHour, &lv_font_scdream_regular_33, 0);
    lv_obj_set_pos(bk_ui->automode_AutoModeCompleteHour, 408, 109+5);
    lv_obj_set_size(bk_ui->automode_AutoModeCompleteHour, 40, 44);

    // Button: AutoModeCompleteHourBt
    bk_ui->automode_AutoModeCompleteHourBt = lv_button_create(bk_ui->automode);
    lv_obj_add_flag(bk_ui->automode_AutoModeCompleteHourBt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->automode_AutoModeCompleteHourBt, automode_AutoModeCompleteHourBt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->automode_AutoModeCompleteHourBt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->automode_AutoModeCompleteHourBt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->automode_AutoModeCompleteHourBt, 0, 0);
    lv_obj_set_pos(bk_ui->automode_AutoModeCompleteHourBt, 405, 110);
    lv_obj_set_size(bk_ui->automode_AutoModeCompleteHourBt, 50, 44);


    // TextView: AutoModeCompleteMin
    bk_ui->automode_AutoModeCompleteMin = lv_label_create(bk_ui->automode);
    lv_label_set_text(bk_ui->automode_AutoModeCompleteMin, "");
    lv_obj_set_style_bg_opa(bk_ui->automode_AutoModeCompleteMin, 0, 0);
    lv_obj_set_style_text_color(bk_ui->automode_AutoModeCompleteMin, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->automode_AutoModeCompleteMin, &lv_font_scdream_regular_33, 0);
    lv_obj_set_pos(bk_ui->automode_AutoModeCompleteMin, 474, 109+5);
    lv_obj_set_size(bk_ui->automode_AutoModeCompleteMin, 40, 44);

    // Button: AutoModeCompleteMinBt
    bk_ui->automode_AutoModeCompleteMinBt = lv_button_create(bk_ui->automode);
    lv_obj_add_flag(bk_ui->automode_AutoModeCompleteMinBt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->automode_AutoModeCompleteMinBt, automode_AutoModeCompleteMinBt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->automode_AutoModeCompleteMinBt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->automode_AutoModeCompleteMinBt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->automode_AutoModeCompleteMinBt, 0, 0);
    lv_obj_set_pos(bk_ui->automode_AutoModeCompleteMinBt, 468, 110);
    lv_obj_set_size(bk_ui->automode_AutoModeCompleteMinBt, 55, 44);


    // Button: loadbt
    bk_ui->automode_loadbt = lv_button_create(bk_ui->automode);
    lv_obj_add_flag(bk_ui->automode_loadbt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->automode_loadbt, automode_loadbt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->automode_loadbt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->automode_loadbt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->automode_loadbt, 0, 0);
    lv_obj_set_pos(bk_ui->automode_loadbt, 637, 94);
    lv_obj_set_size(bk_ui->automode_loadbt, 200, 70);

    // ImageView: imageview23
    bk_ui->automode_imageview23 = lv_image_create(bk_ui->automode);
    _img_set_src_timed(bk_ui->automode_imageview23, "/images/load_bt.png");
    lv_obj_set_pos(bk_ui->automode_imageview23, 637, 94);
    lv_obj_set_size(bk_ui->automode_imageview23, 204, 74);

    // Button: savebt
    bk_ui->automode_savebt = lv_button_create(bk_ui->automode);
    lv_obj_add_flag(bk_ui->automode_savebt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->automode_savebt, automode_savebt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->automode_savebt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->automode_savebt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->automode_savebt, 0, 0);
    lv_obj_set_pos(bk_ui->automode_savebt, 847, 94);
    lv_obj_set_size(bk_ui->automode_savebt, 164, 74);

    // ImageView: imageview25
    bk_ui->automode_imageview25 = lv_image_create(bk_ui->automode);
    _img_set_src_timed(bk_ui->automode_imageview25, "/images/save_bt.png");
    lv_obj_set_pos(bk_ui->automode_imageview25, 847, 94);
    lv_obj_set_size(bk_ui->automode_imageview25, 164, 74);

    // ImageView: imageview26
    bk_ui->automode_imageview26 = lv_image_create(bk_ui->automode);
    _img_set_src_timed(bk_ui->automode_imageview26, "/images/auto_mode_freeze_board.png");
    lv_obj_set_pos(bk_ui->automode_imageview26, 15, 175);
    lv_obj_set_size(bk_ui->automode_imageview26, 490, 125);

    // Button: AutoFreezeTempBt
    bk_ui->automode_AutoFreezeTempBt = lv_button_create(bk_ui->automode);
    lv_obj_add_flag(bk_ui->automode_AutoFreezeTempBt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->automode_AutoFreezeTempBt, automode_AutoFreezeTempBt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->automode_AutoFreezeTempBt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->automode_AutoFreezeTempBt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->automode_AutoFreezeTempBt, 0, 0);
    lv_obj_set_pos(bk_ui->automode_AutoFreezeTempBt, 136, 188);
    lv_obj_set_size(bk_ui->automode_AutoFreezeTempBt, 110, 99);

    // TextView: AutoFreezeTempTxt
    bk_ui->automode_AutoFreezeTempTxt = lv_label_create(bk_ui->automode);
    lv_label_set_text(bk_ui->automode_AutoFreezeTempTxt, "");
    lv_obj_set_style_bg_opa(bk_ui->automode_AutoFreezeTempTxt, 0, 0);
    lv_obj_set_style_text_color(bk_ui->automode_AutoFreezeTempTxt, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->automode_AutoFreezeTempTxt, &lv_font_scdream_regular_42, 0);
    lv_obj_set_pos(bk_ui->automode_AutoFreezeTempTxt, 135, 210+9);
    lv_obj_set_size(bk_ui->automode_AutoFreezeTempTxt, 80, 50);
    lv_obj_set_style_text_align(bk_ui->automode_AutoFreezeTempTxt, LV_TEXT_ALIGN_RIGHT, 0);



    // ImageView: AutoModeFreezeTempCheckBoxIm
    bk_ui->automode_AutoModeFreezeTempCheckBoxIm = lv_image_create(bk_ui->automode);
    _img_set_src_deferred(bk_ui->automode_AutoModeFreezeTempCheckBoxIm, "/images/auto_temp_checkbox.png");
    lv_obj_add_flag(bk_ui->automode_AutoModeFreezeTempCheckBoxIm, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_pos(bk_ui->automode_AutoModeFreezeTempCheckBoxIm, 136, 188);
    lv_obj_set_size(bk_ui->automode_AutoModeFreezeTempCheckBoxIm, 110, 99);

    // ImageView: imageview31
    bk_ui->automode_imageview31 = lv_image_create(bk_ui->automode);
    _img_set_src_timed(bk_ui->automode_imageview31, "/images/auto_mode_defrost_board.png");
    lv_obj_set_pos(bk_ui->automode_imageview31, 519, 175);
    lv_obj_set_size(bk_ui->automode_imageview31, 490, 125);

    // Button: AutoDefrostTempBt
    bk_ui->automode_AutoDefrostTempBt = lv_button_create(bk_ui->automode);
    lv_obj_add_flag(bk_ui->automode_AutoDefrostTempBt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->automode_AutoDefrostTempBt, automode_AutoDefrostTempBt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->automode_AutoDefrostTempBt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->automode_AutoDefrostTempBt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->automode_AutoDefrostTempBt, 0, 0);
    lv_obj_set_pos(bk_ui->automode_AutoDefrostTempBt, 649, 200);
    lv_obj_set_size(bk_ui->automode_AutoDefrostTempBt, 88, 77);

    // TextView: AutoDefrostTempTxt
    bk_ui->automode_AutoDefrostTempTxt = lv_label_create(bk_ui->automode);
    lv_label_set_text(bk_ui->automode_AutoDefrostTempTxt, "");
    lv_obj_set_style_bg_opa(bk_ui->automode_AutoDefrostTempTxt, 0, 0);
    lv_obj_set_style_text_color(bk_ui->automode_AutoDefrostTempTxt, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->automode_AutoDefrostTempTxt, &lv_font_scdream_regular_42, 0);
    lv_obj_set_pos(bk_ui->automode_AutoDefrostTempTxt, 639, 210+9);
    lv_obj_set_size(bk_ui->automode_AutoDefrostTempTxt, 80, 50);
    lv_obj_set_style_text_align(bk_ui->automode_AutoDefrostTempTxt, LV_TEXT_ALIGN_RIGHT, 0);

    // ImageView: AutoModeDefrostTempCheckBoxIm
    bk_ui->automode_AutoModeDefrostTempCheckBoxIm = lv_image_create(bk_ui->automode);
    _img_set_src_deferred(bk_ui->automode_AutoModeDefrostTempCheckBoxIm, "/images/auto_temp_checkbox.png");
    lv_obj_add_flag(bk_ui->automode_AutoModeDefrostTempCheckBoxIm, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_pos(bk_ui->automode_AutoModeDefrostTempCheckBoxIm, 640, 188);
    lv_obj_set_size(bk_ui->automode_AutoModeDefrostTempCheckBoxIm, 110, 99);

    // Button: AutoDefrostTimeHourBt
    bk_ui->automode_AutoDefrostTimeHourBt = lv_button_create(bk_ui->automode);
    lv_obj_add_flag(bk_ui->automode_AutoDefrostTimeHourBt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->automode_AutoDefrostTimeHourBt, automode_AutoDefrostTimeHourBt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->automode_AutoDefrostTimeHourBt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->automode_AutoDefrostTimeHourBt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->automode_AutoDefrostTimeHourBt, 0, 0);
    lv_obj_set_pos(bk_ui->automode_AutoDefrostTimeHourBt, 750, 190);
    lv_obj_set_size(bk_ui->automode_AutoDefrostTimeHourBt, 120, 100);

    // TextView: AutoDefrostTimeHourTxt
    bk_ui->automode_AutoDefrostTimeHourTxt = lv_label_create(bk_ui->automode);
    lv_label_set_text(bk_ui->automode_AutoDefrostTimeHourTxt, "");
    lv_obj_set_style_bg_opa(bk_ui->automode_AutoDefrostTimeHourTxt, 0, 0);
    lv_obj_set_style_text_color(bk_ui->automode_AutoDefrostTimeHourTxt, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->automode_AutoDefrostTimeHourTxt, &lv_font_scdream_regular_42, 0);
    lv_obj_set_pos(bk_ui->automode_AutoDefrostTimeHourTxt, 800, 210+9);
    lv_obj_set_size(bk_ui->automode_AutoDefrostTimeHourTxt, 70, 50);
    lv_obj_set_style_text_align(bk_ui->automode_AutoDefrostTimeHourTxt, LV_TEXT_ALIGN_CENTER, 0);



    // Button: AutoDefrostTimeMinBt
    bk_ui->automode_AutoDefrostTimeMinBt = lv_button_create(bk_ui->automode);
    lv_obj_add_flag(bk_ui->automode_AutoDefrostTimeMinBt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->automode_AutoDefrostTimeMinBt, automode_AutoDefrostTimeMinBt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->automode_AutoDefrostTimeMinBt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->automode_AutoDefrostTimeMinBt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->automode_AutoDefrostTimeMinBt, 0, 0);
    lv_obj_set_pos(bk_ui->automode_AutoDefrostTimeMinBt, 876, 190);
    lv_obj_set_size(bk_ui->automode_AutoDefrostTimeMinBt, 120, 100);

    // TextView: AutoDefrostTimeMinTxt
    bk_ui->automode_AutoDefrostTimeMinTxt = lv_label_create(bk_ui->automode);
    lv_label_set_text(bk_ui->automode_AutoDefrostTimeMinTxt, "");
    lv_obj_set_style_bg_opa(bk_ui->automode_AutoDefrostTimeMinTxt, 0, 0);
    lv_obj_set_style_text_color(bk_ui->automode_AutoDefrostTimeMinTxt, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->automode_AutoDefrostTimeMinTxt, &lv_font_scdream_regular_42, 0);
    lv_obj_set_pos(bk_ui->automode_AutoDefrostTimeMinTxt, 877, 210+9);
    lv_obj_set_size(bk_ui->automode_AutoDefrostTimeMinTxt, 70, 50);
    lv_obj_set_style_text_align(bk_ui->automode_AutoDefrostTimeMinTxt, LV_TEXT_ALIGN_CENTER, 0);



    // ImageView: AutoModeDefrostTimeCheckBoxIm
    bk_ui->automode_AutoModeDefrostTimeCheckBoxIm = lv_image_create(bk_ui->automode);
    _img_set_src_deferred(bk_ui->automode_AutoModeDefrostTimeCheckBoxIm, "/images/auto_autotime_checkbox.png");
    lv_obj_add_flag(bk_ui->automode_AutoModeDefrostTimeCheckBoxIm, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_pos(bk_ui->automode_AutoModeDefrostTimeCheckBoxIm, 754, 188);
    lv_obj_set_size(bk_ui->automode_AutoModeDefrostTimeCheckBoxIm, 242, 99);

    // ImageView: AutoModeDefrostAutoTime
    bk_ui->automode_AutoModeDefrostAutoTime = lv_image_create(bk_ui->automode);
    _img_set_src_deferred(bk_ui->automode_AutoModeDefrostAutoTime, "/images/defrost_auto_time_box.png");
    lv_obj_add_flag(bk_ui->automode_AutoModeDefrostAutoTime, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_pos(bk_ui->automode_AutoModeDefrostAutoTime, 754, 188);
    lv_obj_set_size(bk_ui->automode_AutoModeDefrostAutoTime, 242, 99);

    // ImageView: imageview44
    bk_ui->automode_imageview44 = lv_image_create(bk_ui->automode);
    _img_set_src_timed(bk_ui->automode_imageview44, "/images/auto_mode_fermentation1_board.png");
    lv_obj_set_pos(bk_ui->automode_imageview44, 15, 307);
    lv_obj_set_size(bk_ui->automode_imageview44, 490, 125);

    // Button: AutoFermentation1TempBt
    bk_ui->automode_AutoFermentation1TempBt = lv_button_create(bk_ui->automode);
    lv_obj_add_flag(bk_ui->automode_AutoFermentation1TempBt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->automode_AutoFermentation1TempBt, automode_AutoFermentation1TempBt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->automode_AutoFermentation1TempBt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->automode_AutoFermentation1TempBt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->automode_AutoFermentation1TempBt, 0, 0);
    lv_obj_set_pos(bk_ui->automode_AutoFermentation1TempBt, 135, 320);
    lv_obj_set_size(bk_ui->automode_AutoFermentation1TempBt, 110, 99);

    // TextView: AutoFermentation1TempTxt
    bk_ui->automode_AutoFermentation1TempTxt = lv_label_create(bk_ui->automode);
    lv_label_set_text(bk_ui->automode_AutoFermentation1TempTxt, "");
    lv_obj_set_style_bg_opa(bk_ui->automode_AutoFermentation1TempTxt, 0, 0);
    lv_obj_set_style_text_color(bk_ui->automode_AutoFermentation1TempTxt, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->automode_AutoFermentation1TempTxt, &lv_font_scdream_regular_42, 0);
    lv_obj_set_pos(bk_ui->automode_AutoFermentation1TempTxt, 135, 342+9);
    lv_obj_set_size(bk_ui->automode_AutoFermentation1TempTxt, 80, 50);
    lv_obj_set_style_text_align(bk_ui->automode_AutoFermentation1TempTxt, LV_TEXT_ALIGN_RIGHT, 0);



    // ImageView: AutoModeFermentation1TempCheckBoxIm
    bk_ui->automode_AutoModeFermentation1TempCheckBoxIm = lv_image_create(bk_ui->automode);
    _img_set_src_deferred(bk_ui->automode_AutoModeFermentation1TempCheckBoxIm, "/images/auto_temp_checkbox.png");
    lv_obj_add_flag(bk_ui->automode_AutoModeFermentation1TempCheckBoxIm, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_pos(bk_ui->automode_AutoModeFermentation1TempCheckBoxIm, 136, 320);
    lv_obj_set_size(bk_ui->automode_AutoModeFermentation1TempCheckBoxIm, 110, 99);

    // Button: AutoFermentation1HumidityBt
    bk_ui->automode_AutoFermentation1HumidityBt = lv_button_create(bk_ui->automode);
    lv_obj_add_flag(bk_ui->automode_AutoFermentation1HumidityBt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->automode_AutoFermentation1HumidityBt, automode_AutoFermentation1HumidityBt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->automode_AutoFermentation1HumidityBt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->automode_AutoFermentation1HumidityBt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->automode_AutoFermentation1HumidityBt, 0, 0);
    lv_obj_set_pos(bk_ui->automode_AutoFermentation1HumidityBt, 247, 320);
    lv_obj_set_size(bk_ui->automode_AutoFermentation1HumidityBt, 100, 99);

    // TextView: AutoFermentation1HumidityTxt
    bk_ui->automode_AutoFermentation1HumidityTxt = lv_label_create(bk_ui->automode);
    lv_label_set_text(bk_ui->automode_AutoFermentation1HumidityTxt, "");
    lv_obj_set_style_bg_opa(bk_ui->automode_AutoFermentation1HumidityTxt, 0, 0);
    lv_obj_set_style_text_color(bk_ui->automode_AutoFermentation1HumidityTxt, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->automode_AutoFermentation1HumidityTxt, &lv_font_scdream_regular_42, 0);
    lv_obj_set_pos(bk_ui->automode_AutoFermentation1HumidityTxt, 247, 342+9);
    lv_obj_set_size(bk_ui->automode_AutoFermentation1HumidityTxt, 80, 50);
    lv_obj_set_style_text_align(bk_ui->automode_AutoFermentation1HumidityTxt, LV_TEXT_ALIGN_CENTER, 0);

    // // ImageView: AutoModeFermentation1HumidityCheckBoxIm
    // bk_ui->automode_AutoModeFermentation1HumidityCheckBoxIm = lv_image_create(bk_ui->automode);
    // _img_set_src_timed(bk_ui->automode_AutoModeFermentation1HumidityCheckBoxIm, "/images/auto_humidity_checkbox.png");
    // lv_obj_add_flag(bk_ui->automode_AutoModeFermentation1HumidityCheckBoxIm, LV_OBJ_FLAG_HIDDEN);
    // lv_obj_set_pos(bk_ui->automode_AutoModeFermentation1HumidityCheckBoxIm, 250, 320);
    // lv_obj_set_size(bk_ui->automode_AutoModeFermentation1HumidityCheckBoxIm, 96, 99);

    // Button: AutoFermentation1TimeHourBt
    bk_ui->automode_AutoFermentation1TimeHourBt = lv_button_create(bk_ui->automode);
    lv_obj_add_flag(bk_ui->automode_AutoFermentation1TimeHourBt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->automode_AutoFermentation1TimeHourBt, automode_AutoFermentation1TimeHourBt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->automode_AutoFermentation1TimeHourBt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->automode_AutoFermentation1TimeHourBt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->automode_AutoFermentation1TimeHourBt, 0, 0);
    lv_obj_set_pos(bk_ui->automode_AutoFermentation1TimeHourBt, 345, 320);
    lv_obj_set_size(bk_ui->automode_AutoFermentation1TimeHourBt, 70, 100);

    // TextView: AutoFermentation1TimeHourTxt
    bk_ui->automode_AutoFermentation1TimeHourTxt = lv_label_create(bk_ui->automode);
    lv_label_set_text(bk_ui->automode_AutoFermentation1TimeHourTxt, "");
    lv_obj_set_style_bg_opa(bk_ui->automode_AutoFermentation1TimeHourTxt, 0, 0);
    lv_obj_set_style_text_color(bk_ui->automode_AutoFermentation1TimeHourTxt, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->automode_AutoFermentation1TimeHourTxt, &lv_font_scdream_regular_42, 0);
    lv_obj_set_pos(bk_ui->automode_AutoFermentation1TimeHourTxt, 338, 342+9);
    lv_obj_set_size(bk_ui->automode_AutoFermentation1TimeHourTxt, 88, 50);
    lv_obj_set_style_text_align(bk_ui->automode_AutoFermentation1TimeHourTxt, LV_TEXT_ALIGN_CENTER, 0);



    // Button: AutoFermentation1TimeMinBt
    bk_ui->automode_AutoFermentation1TimeMinBt = lv_button_create(bk_ui->automode);
    lv_obj_add_flag(bk_ui->automode_AutoFermentation1TimeMinBt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->automode_AutoFermentation1TimeMinBt, automode_AutoFermentation1TimeMinBt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->automode_AutoFermentation1TimeMinBt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->automode_AutoFermentation1TimeMinBt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->automode_AutoFermentation1TimeMinBt, 0, 0);
    lv_obj_set_pos(bk_ui->automode_AutoFermentation1TimeMinBt, 418, 320);
    lv_obj_set_size(bk_ui->automode_AutoFermentation1TimeMinBt, 80, 100);

    // TextView: AutoFermentation1TimeMinTxt
    bk_ui->automode_AutoFermentation1TimeMinTxt = lv_label_create(bk_ui->automode);
    lv_label_set_text(bk_ui->automode_AutoFermentation1TimeMinTxt, "");
    lv_obj_set_style_bg_opa(bk_ui->automode_AutoFermentation1TimeMinTxt, 0, 0);
    lv_obj_set_style_text_color(bk_ui->automode_AutoFermentation1TimeMinTxt, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->automode_AutoFermentation1TimeMinTxt, &lv_font_scdream_regular_42, 0);
    lv_obj_set_pos(bk_ui->automode_AutoFermentation1TimeMinTxt, 415, 342+9);
    lv_obj_set_size(bk_ui->automode_AutoFermentation1TimeMinTxt, 88, 50);
    lv_obj_set_style_text_align(bk_ui->automode_AutoFermentation1TimeMinTxt, LV_TEXT_ALIGN_CENTER, 0);



    // ImageView: AutoModeFermentation1TimeCheckBoxIm
    bk_ui->automode_AutoModeFermentation1TimeCheckBoxIm = lv_image_create(bk_ui->automode);
    _img_set_src_deferred(bk_ui->automode_AutoModeFermentation1TimeCheckBoxIm, "/images/auto_time_checkbox.png");
    lv_obj_add_flag(bk_ui->automode_AutoModeFermentation1TimeCheckBoxIm, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_pos(bk_ui->automode_AutoModeFermentation1TimeCheckBoxIm, 344, 320);
    lv_obj_set_size(bk_ui->automode_AutoModeFermentation1TimeCheckBoxIm, 142, 99);

    // ImageView: imageview60
    bk_ui->automode_imageview60 = lv_image_create(bk_ui->automode);
    _img_set_src_timed(bk_ui->automode_imageview60, "/images/auto_mode_fermentation2_board.png");
    lv_obj_set_pos(bk_ui->automode_imageview60, 519, 307);
    lv_obj_set_size(bk_ui->automode_imageview60, 490, 125);

    // Button: AutoFermentation2TempBt
    bk_ui->automode_AutoFermentation2TempBt = lv_button_create(bk_ui->automode);
    lv_obj_add_flag(bk_ui->automode_AutoFermentation2TempBt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->automode_AutoFermentation2TempBt, automode_AutoFermentation2TempBt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->automode_AutoFermentation2TempBt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->automode_AutoFermentation2TempBt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->automode_AutoFermentation2TempBt, 0, 0);
    lv_obj_set_pos(bk_ui->automode_AutoFermentation2TempBt, 640, 320);
    lv_obj_set_size(bk_ui->automode_AutoFermentation2TempBt, 110, 100);

    // TextView: AutoFermentation2TempTxt
    bk_ui->automode_AutoFermentation2TempTxt = lv_label_create(bk_ui->automode);
    lv_label_set_text(bk_ui->automode_AutoFermentation2TempTxt, "");
    lv_obj_set_style_bg_opa(bk_ui->automode_AutoFermentation2TempTxt, 0, 0);
    lv_obj_set_style_text_color(bk_ui->automode_AutoFermentation2TempTxt, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->automode_AutoFermentation2TempTxt, &lv_font_scdream_regular_42, 0);
    lv_obj_set_pos(bk_ui->automode_AutoFermentation2TempTxt, 638, 342+9);
    lv_obj_set_size(bk_ui->automode_AutoFermentation2TempTxt, 80, 50);
    lv_obj_set_style_text_align(bk_ui->automode_AutoFermentation2TempTxt, LV_TEXT_ALIGN_RIGHT, 0);



    // ImageView: AutoModeFermentation2TempCheckBoxIm
    bk_ui->automode_AutoModeFermentation2TempCheckBoxIm = lv_image_create(bk_ui->automode);
    _img_set_src_deferred(bk_ui->automode_AutoModeFermentation2TempCheckBoxIm, "/images/auto_temp_checkbox.png");
    lv_obj_add_flag(bk_ui->automode_AutoModeFermentation2TempCheckBoxIm, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_pos(bk_ui->automode_AutoModeFermentation2TempCheckBoxIm, 640, 320);
    lv_obj_set_size(bk_ui->automode_AutoModeFermentation2TempCheckBoxIm, 110, 99);

    // Button: AutoFermentation2HumidityBt
    bk_ui->automode_AutoFermentation2HumidityBt = lv_button_create(bk_ui->automode);
    lv_obj_add_flag(bk_ui->automode_AutoFermentation2HumidityBt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->automode_AutoFermentation2HumidityBt, automode_AutoFermentation2HumidityBt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->automode_AutoFermentation2HumidityBt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->automode_AutoFermentation2HumidityBt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->automode_AutoFermentation2HumidityBt, 0, 0);
    lv_obj_set_pos(bk_ui->automode_AutoFermentation2HumidityBt, 755, 323);
    lv_obj_set_size(bk_ui->automode_AutoFermentation2HumidityBt, 90, 100);

    // TextView: AutoFermentation2HumidityTxt
    bk_ui->automode_AutoFermentation2HumidityTxt = lv_label_create(bk_ui->automode);
    lv_label_set_text(bk_ui->automode_AutoFermentation2HumidityTxt, "");
    lv_obj_set_style_bg_opa(bk_ui->automode_AutoFermentation2HumidityTxt, 0, 0);
    lv_obj_set_style_text_color(bk_ui->automode_AutoFermentation2HumidityTxt, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->automode_AutoFermentation2HumidityTxt, &lv_font_scdream_regular_42, 0);
    lv_obj_set_pos(bk_ui->automode_AutoFermentation2HumidityTxt, 747, 342+9);
    lv_obj_set_size(bk_ui->automode_AutoFermentation2HumidityTxt, 88, 50);
    lv_obj_set_style_text_align(bk_ui->automode_AutoFermentation2HumidityTxt, LV_TEXT_ALIGN_CENTER, 0);


    // ImageView: AutoModeFermentation2HumidityCheckBoxIm
    bk_ui->automode_AutoModeFermentation2HumidityCheckBoxIm = lv_image_create(bk_ui->automode);
    _img_set_src_deferred(bk_ui->automode_AutoModeFermentation2HumidityCheckBoxIm, "/images/auto_humidity_checkbox.png");
    lv_obj_add_flag(bk_ui->automode_AutoModeFermentation2HumidityCheckBoxIm, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_pos(bk_ui->automode_AutoModeFermentation2HumidityCheckBoxIm, 754, 320);
    lv_obj_set_size(bk_ui->automode_AutoModeFermentation2HumidityCheckBoxIm, 96, 99);

    // Button: AutoFermentation2TimeHourBt
    bk_ui->automode_AutoFermentation2TimeHourBt = lv_button_create(bk_ui->automode);
    lv_obj_add_flag(bk_ui->automode_AutoFermentation2TimeHourBt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->automode_AutoFermentation2TimeHourBt, automode_AutoFermentation2TimeHourBt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->automode_AutoFermentation2TimeHourBt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->automode_AutoFermentation2TimeHourBt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->automode_AutoFermentation2TimeHourBt, 0, 0);
    lv_obj_set_pos(bk_ui->automode_AutoFermentation2TimeHourBt, 855, 323);
    lv_obj_set_size(bk_ui->automode_AutoFermentation2TimeHourBt, 70, 100);

    // TextView: AutoFermentation2TimeHourTxt
    bk_ui->automode_AutoFermentation2TimeHourTxt = lv_label_create(bk_ui->automode);
    lv_label_set_text(bk_ui->automode_AutoFermentation2TimeHourTxt, "");
    lv_obj_set_style_bg_opa(bk_ui->automode_AutoFermentation2TimeHourTxt, 0, 0);
    lv_obj_set_style_text_color(bk_ui->automode_AutoFermentation2TimeHourTxt, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->automode_AutoFermentation2TimeHourTxt, &lv_font_scdream_regular_42, 0);
    lv_obj_set_pos(bk_ui->automode_AutoFermentation2TimeHourTxt, 848, 342+9);
    lv_obj_set_size(bk_ui->automode_AutoFermentation2TimeHourTxt, 70, 50);
    lv_obj_set_style_text_align(bk_ui->automode_AutoFermentation2TimeHourTxt, LV_TEXT_ALIGN_CENTER, 0);


    // Button: AutoFermentation2TimeMinBt
    bk_ui->automode_AutoFermentation2TimeMinBt = lv_button_create(bk_ui->automode);
    lv_obj_add_flag(bk_ui->automode_AutoFermentation2TimeMinBt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->automode_AutoFermentation2TimeMinBt, automode_AutoFermentation2TimeMinBt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->automode_AutoFermentation2TimeMinBt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->automode_AutoFermentation2TimeMinBt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->automode_AutoFermentation2TimeMinBt, 0, 0);
    lv_obj_set_pos(bk_ui->automode_AutoFermentation2TimeMinBt, 928, 320);
    lv_obj_set_size(bk_ui->automode_AutoFermentation2TimeMinBt, 70, 100);

    // TextView: AutoFermentation2TimeMinTxt
    bk_ui->automode_AutoFermentation2TimeMinTxt = lv_label_create(bk_ui->automode);
    lv_label_set_text(bk_ui->automode_AutoFermentation2TimeMinTxt, "");
    lv_obj_set_style_bg_opa(bk_ui->automode_AutoFermentation2TimeMinTxt, 0, 0);
    lv_obj_set_style_text_color(bk_ui->automode_AutoFermentation2TimeMinTxt, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->automode_AutoFermentation2TimeMinTxt, &lv_font_scdream_regular_42, 0);
    lv_obj_set_pos(bk_ui->automode_AutoFermentation2TimeMinTxt, 927, 342+9);
    lv_obj_set_size(bk_ui->automode_AutoFermentation2TimeMinTxt, 70, 50);
    lv_obj_set_style_text_align(bk_ui->automode_AutoFermentation2TimeMinTxt, LV_TEXT_ALIGN_CENTER, 0);


    // ImageView: AutoModeFermentation2TimeCheckBoxIm
    bk_ui->automode_AutoModeFermentation2TimeCheckBoxIm = lv_image_create(bk_ui->automode);
    _img_set_src_deferred(bk_ui->automode_AutoModeFermentation2TimeCheckBoxIm, "/images/auto_time_checkbox.png");
    lv_obj_add_flag(bk_ui->automode_AutoModeFermentation2TimeCheckBoxIm, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_pos(bk_ui->automode_AutoModeFermentation2TimeCheckBoxIm, 854, 320);
    lv_obj_set_size(bk_ui->automode_AutoModeFermentation2TimeCheckBoxIm, 142, 99);
    // keypadbaseim + KeyPad: lazy-created in _keypad_on_automode on first use
    // auto_f1~f4: lazy-created in automode_load_event_cb only when °F mode active
    bk_printf(TAG "[IMGTIME] ===== automode init total: %lu ms =====\n", lv_tick_elaps(_t_start));
}

void init_page_automode_with_step(bk_lv_ui_t *bk_ui) 
{
    static uint32_t currentStep = 0;
    static uint32_t currentImageStep = 0;

    switch (currentStep)
    {
        case RENDER_STEP_CREATE_PAGE :
        {
            bk_ui->automode = lv_obj_create(preRenderRoot);
            lv_obj_remove_style_all(bk_ui->automode);
            lv_obj_set_size(bk_ui->automode, 1024, 600);
            lv_obj_set_style_radius(bk_ui->automode, 0, LV_PART_MAIN);
            lv_obj_set_pos(bk_ui->automode, 0, 0);
            lv_obj_set_scrollbar_mode(bk_ui->automode, LV_SCROLLBAR_MODE_OFF);            
            lv_obj_set_style_bg_color(bk_ui->automode, lv_color_hex(0xD9D9D9), 0);
            lv_obj_set_style_bg_opa(bk_ui->automode, LV_OPA_COVER, 0);
            currentStep++;
            return;
        }
case RENDER_STEP_CREATE_CHILD :
        {
            // ImageView: title
            bk_ui->automode_title = lv_image_create(bk_ui->automode);
            _img_set_src_timed(bk_ui->automode_title, "/images/automode_title.png");
            lv_obj_set_pos(bk_ui->automode_title, 0, 10);
            lv_obj_set_size(bk_ui->automode_title, 380, 80);
            lv_image_set_inner_align(bk_ui->automode_title, LV_IMAGE_ALIGN_TOP_LEFT);

            // Button: backbt
            bk_ui->automode_backbt = lv_button_create(bk_ui->automode);
            lv_obj_add_flag(bk_ui->automode_backbt, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_set_style_bg_opa(bk_ui->automode_backbt, 0, 0);
            lv_obj_set_style_border_width(bk_ui->automode_backbt, 0, 0);
            lv_obj_set_style_shadow_width(bk_ui->automode_backbt, 0, 0);
            lv_obj_set_pos(bk_ui->automode_backbt, 13, 445);
            lv_obj_set_size(bk_ui->automode_backbt, 179, 74);

            // ImageView: imageview3
            bk_ui->automode_imageview3 = lv_image_create(bk_ui->automode);
            _img_set_src_timed(bk_ui->automode_imageview3, "/images/exit_bt.png");
            lv_obj_set_pos(bk_ui->automode_imageview3, 13, 445);
            lv_obj_set_size(bk_ui->automode_imageview3, 179, 74);

            // Button: startbt
            bk_ui->automode_startbt = lv_button_create(bk_ui->automode);
            lv_obj_add_flag(bk_ui->automode_startbt, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_set_style_bg_opa(bk_ui->automode_startbt, 0, 0);
            lv_obj_set_style_border_width(bk_ui->automode_startbt, 0, 0);
            lv_obj_set_style_shadow_width(bk_ui->automode_startbt, 0, 0);
            lv_obj_set_pos(bk_ui->automode_startbt, 847, 445);
            lv_obj_set_size(bk_ui->automode_startbt, 164, 74);

            // ImageView: imageview5
            bk_ui->automode_imageview5 = lv_image_create(bk_ui->automode);
            _img_set_src_timed(bk_ui->automode_imageview5, "/images/start_bt.png");
            lv_obj_set_pos(bk_ui->automode_imageview5, 847, 445);
            lv_obj_set_size(bk_ui->automode_imageview5, 164, 74);

            // ImageView: imageview6
            bk_ui->automode_imageview6 = lv_image_create(bk_ui->automode);
            _img_set_src_timed(bk_ui->automode_imageview6, "/images/auto_mode_start_box_time.png");
            lv_obj_set_pos(bk_ui->automode_imageview6, 17, 96);
            lv_obj_set_size(bk_ui->automode_imageview6, 528, 70);

            // TextView: AutoModeCompleteYear
            bk_ui->automode_AutoModeCompleteYear = lv_label_create(bk_ui->automode);
            lv_label_set_text(bk_ui->automode_AutoModeCompleteYear, "");
            lv_obj_set_style_bg_opa(bk_ui->automode_AutoModeCompleteYear, 0, 0);
            lv_obj_set_style_text_color(bk_ui->automode_AutoModeCompleteYear, lv_color_hex(0x3C3A3D), 0);
            lv_obj_set_style_text_font(bk_ui->automode_AutoModeCompleteYear, &lv_font_scdream_regular_33, 0);
            lv_obj_set_pos(bk_ui->automode_AutoModeCompleteYear, 191, 109+5);
            lv_obj_set_size(bk_ui->automode_AutoModeCompleteYear, 80, 44);

            // Button: AutoModeCompleteYearBt
            bk_ui->automode_AutoModeCompleteYearBt = lv_button_create(bk_ui->automode);
            lv_obj_add_flag(bk_ui->automode_AutoModeCompleteYearBt, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_set_style_bg_opa(bk_ui->automode_AutoModeCompleteYearBt, 0, 0);
            lv_obj_set_style_border_width(bk_ui->automode_AutoModeCompleteYearBt, 0, 0);
            lv_obj_set_style_shadow_width(bk_ui->automode_AutoModeCompleteYearBt, 0, 0);
            lv_obj_set_pos(bk_ui->automode_AutoModeCompleteYearBt, 191, 110);
            lv_obj_set_size(bk_ui->automode_AutoModeCompleteYearBt, 80, 44);

            // TextView: AutoModeCompleteMonth
            bk_ui->automode_AutoModeCompleteMonth = lv_label_create(bk_ui->automode);
            lv_label_set_text(bk_ui->automode_AutoModeCompleteMonth, "");
            lv_obj_set_style_bg_opa(bk_ui->automode_AutoModeCompleteMonth, 0, 0);
            lv_obj_set_style_text_color(bk_ui->automode_AutoModeCompleteMonth, lv_color_hex(0x3C3A3D), 0);
            lv_obj_set_style_text_font(bk_ui->automode_AutoModeCompleteMonth, &lv_font_scdream_regular_33, 0);
            lv_obj_set_pos(bk_ui->automode_AutoModeCompleteMonth, 288, 109+5);
            lv_obj_set_size(bk_ui->automode_AutoModeCompleteMonth, 40, 44);

            // Button: AutoModeCompleteMonthBt
            bk_ui->automode_AutoModeCompleteMonthBt = lv_button_create(bk_ui->automode);
            lv_obj_add_flag(bk_ui->automode_AutoModeCompleteMonthBt, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_set_style_bg_opa(bk_ui->automode_AutoModeCompleteMonthBt, 0, 0);
            lv_obj_set_style_border_width(bk_ui->automode_AutoModeCompleteMonthBt, 0, 0);
            lv_obj_set_style_shadow_width(bk_ui->automode_AutoModeCompleteMonthBt, 0, 0);
            lv_obj_set_pos(bk_ui->automode_AutoModeCompleteMonthBt, 279, 110);
            lv_obj_set_size(bk_ui->automode_AutoModeCompleteMonthBt, 55, 44);

            // TextView: AutoModeCompleteDay
            bk_ui->automode_AutoModeCompleteDay = lv_label_create(bk_ui->automode);
            lv_label_set_text(bk_ui->automode_AutoModeCompleteDay, "");
            lv_obj_set_style_bg_opa(bk_ui->automode_AutoModeCompleteDay, 0, 0);
            lv_obj_set_style_text_color(bk_ui->automode_AutoModeCompleteDay, lv_color_hex(0x3C3A3D), 0);
            lv_obj_set_style_text_font(bk_ui->automode_AutoModeCompleteDay, &lv_font_scdream_regular_33, 0);
            lv_obj_set_pos(bk_ui->automode_AutoModeCompleteDay, 346, 109+5);
            lv_obj_set_size(bk_ui->automode_AutoModeCompleteDay, 40, 44);

            // Button: AutoModeCompleteDayBt
            bk_ui->automode_AutoModeCompleteDayBt = lv_button_create(bk_ui->automode);
            lv_obj_add_flag(bk_ui->automode_AutoModeCompleteDayBt, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_set_style_bg_opa(bk_ui->automode_AutoModeCompleteDayBt, 0, 0);
            lv_obj_set_style_border_width(bk_ui->automode_AutoModeCompleteDayBt, 0, 0);
            lv_obj_set_style_shadow_width(bk_ui->automode_AutoModeCompleteDayBt, 0, 0);
            lv_obj_set_pos(bk_ui->automode_AutoModeCompleteDayBt, 338, 110);
            lv_obj_set_size(bk_ui->automode_AutoModeCompleteDayBt, 55, 44);

            // TextView: AutoModeCompleteHour
            bk_ui->automode_AutoModeCompleteHour = lv_label_create(bk_ui->automode);
            lv_label_set_text(bk_ui->automode_AutoModeCompleteHour, "");
            lv_obj_set_style_bg_opa(bk_ui->automode_AutoModeCompleteHour, 0, 0);
            lv_obj_set_style_text_color(bk_ui->automode_AutoModeCompleteHour, lv_color_hex(0x3C3A3D), 0);
            lv_obj_set_style_text_font(bk_ui->automode_AutoModeCompleteHour, &lv_font_scdream_regular_33, 0);
            lv_obj_set_pos(bk_ui->automode_AutoModeCompleteHour, 408, 109+5);
            lv_obj_set_size(bk_ui->automode_AutoModeCompleteHour, 40, 44);

            // Button: AutoModeCompleteHourBt
            bk_ui->automode_AutoModeCompleteHourBt = lv_button_create(bk_ui->automode);
            lv_obj_add_flag(bk_ui->automode_AutoModeCompleteHourBt, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_set_style_bg_opa(bk_ui->automode_AutoModeCompleteHourBt, 0, 0);
            lv_obj_set_style_border_width(bk_ui->automode_AutoModeCompleteHourBt, 0, 0);
            lv_obj_set_style_shadow_width(bk_ui->automode_AutoModeCompleteHourBt, 0, 0);
            lv_obj_set_pos(bk_ui->automode_AutoModeCompleteHourBt, 405, 110);
            lv_obj_set_size(bk_ui->automode_AutoModeCompleteHourBt, 50, 44);

            // TextView: AutoModeCompleteMin
            bk_ui->automode_AutoModeCompleteMin = lv_label_create(bk_ui->automode);
            lv_label_set_text(bk_ui->automode_AutoModeCompleteMin, "");
            lv_obj_set_style_bg_opa(bk_ui->automode_AutoModeCompleteMin, 0, 0);
            lv_obj_set_style_text_color(bk_ui->automode_AutoModeCompleteMin, lv_color_hex(0x3C3A3D), 0);
            lv_obj_set_style_text_font(bk_ui->automode_AutoModeCompleteMin, &lv_font_scdream_regular_33, 0);
            lv_obj_set_pos(bk_ui->automode_AutoModeCompleteMin, 474, 109+5);
            lv_obj_set_size(bk_ui->automode_AutoModeCompleteMin, 40, 44);

            // Button: AutoModeCompleteMinBt
            bk_ui->automode_AutoModeCompleteMinBt = lv_button_create(bk_ui->automode);
            lv_obj_add_flag(bk_ui->automode_AutoModeCompleteMinBt, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_set_style_bg_opa(bk_ui->automode_AutoModeCompleteMinBt, 0, 0);
            lv_obj_set_style_border_width(bk_ui->automode_AutoModeCompleteMinBt, 0, 0);
            lv_obj_set_style_shadow_width(bk_ui->automode_AutoModeCompleteMinBt, 0, 0);
            lv_obj_set_pos(bk_ui->automode_AutoModeCompleteMinBt, 468, 110);
            lv_obj_set_size(bk_ui->automode_AutoModeCompleteMinBt, 55, 44);

            // Button: loadbt
            bk_ui->automode_loadbt = lv_button_create(bk_ui->automode);
            lv_obj_add_flag(bk_ui->automode_loadbt, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_set_style_bg_opa(bk_ui->automode_loadbt, 0, 0);
            lv_obj_set_style_border_width(bk_ui->automode_loadbt, 0, 0);
            lv_obj_set_style_shadow_width(bk_ui->automode_loadbt, 0, 0);
            lv_obj_set_pos(bk_ui->automode_loadbt, 637, 94);
            lv_obj_set_size(bk_ui->automode_loadbt, 200, 70);

            // ImageView: imageview23
            bk_ui->automode_imageview23 = lv_image_create(bk_ui->automode);
            _img_set_src_timed(bk_ui->automode_imageview23, "/images/load_bt.png");
            lv_obj_set_pos(bk_ui->automode_imageview23, 637, 94);
            lv_obj_set_size(bk_ui->automode_imageview23, 204, 74);

            // Button: savebt
            bk_ui->automode_savebt = lv_button_create(bk_ui->automode);
            lv_obj_add_flag(bk_ui->automode_savebt, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_set_style_bg_opa(bk_ui->automode_savebt, 0, 0);
            lv_obj_set_style_border_width(bk_ui->automode_savebt, 0, 0);
            lv_obj_set_style_shadow_width(bk_ui->automode_savebt, 0, 0);
            lv_obj_set_pos(bk_ui->automode_savebt, 847, 94);
            lv_obj_set_size(bk_ui->automode_savebt, 164, 74);

            // ImageView: imageview25
            bk_ui->automode_imageview25 = lv_image_create(bk_ui->automode);
            _img_set_src_timed(bk_ui->automode_imageview25, "/images/save_bt.png");
            lv_obj_set_pos(bk_ui->automode_imageview25, 847, 94);
            lv_obj_set_size(bk_ui->automode_imageview25, 164, 74);

            // ImageView: imageview26
            bk_ui->automode_imageview26 = lv_image_create(bk_ui->automode);
            _img_set_src_timed(bk_ui->automode_imageview26, "/images/auto_mode_freeze_board.png");
            lv_obj_set_pos(bk_ui->automode_imageview26, 15, 175);
            lv_obj_set_size(bk_ui->automode_imageview26, 490, 125);

            // Button: AutoFreezeTempBt
            bk_ui->automode_AutoFreezeTempBt = lv_button_create(bk_ui->automode);
            lv_obj_add_flag(bk_ui->automode_AutoFreezeTempBt, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_set_style_bg_opa(bk_ui->automode_AutoFreezeTempBt, 0, 0);
            lv_obj_set_style_border_width(bk_ui->automode_AutoFreezeTempBt, 0, 0);
            lv_obj_set_style_shadow_width(bk_ui->automode_AutoFreezeTempBt, 0, 0);
            lv_obj_set_pos(bk_ui->automode_AutoFreezeTempBt, 136, 188);
            lv_obj_set_size(bk_ui->automode_AutoFreezeTempBt, 110, 99);

            // TextView: AutoFreezeTempTxt
            bk_ui->automode_AutoFreezeTempTxt = lv_label_create(bk_ui->automode);
            lv_label_set_text(bk_ui->automode_AutoFreezeTempTxt, "");
            lv_obj_set_style_bg_opa(bk_ui->automode_AutoFreezeTempTxt, 0, 0);
            lv_obj_set_style_text_color(bk_ui->automode_AutoFreezeTempTxt, lv_color_hex(0x3C3A3D), 0);
            lv_obj_set_style_text_font(bk_ui->automode_AutoFreezeTempTxt, &lv_font_scdream_regular_42, 0);
            lv_obj_set_pos(bk_ui->automode_AutoFreezeTempTxt, 135, 210+9);
            lv_obj_set_size(bk_ui->automode_AutoFreezeTempTxt, 80, 50);
            lv_obj_set_style_text_align(bk_ui->automode_AutoFreezeTempTxt, LV_TEXT_ALIGN_RIGHT, 0);

            // ImageView: AutoModeFreezeTempCheckBoxIm
            bk_ui->automode_AutoModeFreezeTempCheckBoxIm = lv_image_create(bk_ui->automode);
            _img_set_src_deferred(bk_ui->automode_AutoModeFreezeTempCheckBoxIm, "/images/auto_temp_checkbox.png");
            lv_obj_add_flag(bk_ui->automode_AutoModeFreezeTempCheckBoxIm, LV_OBJ_FLAG_HIDDEN);
            lv_obj_set_pos(bk_ui->automode_AutoModeFreezeTempCheckBoxIm, 136, 188);
            lv_obj_set_size(bk_ui->automode_AutoModeFreezeTempCheckBoxIm, 110, 99);

            // ImageView: imageview31
            bk_ui->automode_imageview31 = lv_image_create(bk_ui->automode);
            _img_set_src_timed(bk_ui->automode_imageview31, "/images/auto_mode_defrost_board.png");
            lv_obj_set_pos(bk_ui->automode_imageview31, 519, 175);
            lv_obj_set_size(bk_ui->automode_imageview31, 490, 125);

            // Button: AutoDefrostTempBt
            bk_ui->automode_AutoDefrostTempBt = lv_button_create(bk_ui->automode);
            lv_obj_add_flag(bk_ui->automode_AutoDefrostTempBt, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_set_style_bg_opa(bk_ui->automode_AutoDefrostTempBt, 0, 0);
            lv_obj_set_style_border_width(bk_ui->automode_AutoDefrostTempBt, 0, 0);
            lv_obj_set_style_shadow_width(bk_ui->automode_AutoDefrostTempBt, 0, 0);
            lv_obj_set_pos(bk_ui->automode_AutoDefrostTempBt, 649, 200);
            lv_obj_set_size(bk_ui->automode_AutoDefrostTempBt, 88, 77);

            // TextView: AutoDefrostTempTxt
            bk_ui->automode_AutoDefrostTempTxt = lv_label_create(bk_ui->automode);
            lv_label_set_text(bk_ui->automode_AutoDefrostTempTxt, "");
            lv_obj_set_style_bg_opa(bk_ui->automode_AutoDefrostTempTxt, 0, 0);
            lv_obj_set_style_text_color(bk_ui->automode_AutoDefrostTempTxt, lv_color_hex(0x3C3A3D), 0);
            lv_obj_set_style_text_font(bk_ui->automode_AutoDefrostTempTxt, &lv_font_scdream_regular_42, 0);
            lv_obj_set_pos(bk_ui->automode_AutoDefrostTempTxt, 639, 210+9);
            lv_obj_set_size(bk_ui->automode_AutoDefrostTempTxt, 80, 50);
            lv_obj_set_style_text_align(bk_ui->automode_AutoDefrostTempTxt, LV_TEXT_ALIGN_RIGHT, 0);

            // ImageView: AutoModeDefrostTempCheckBoxIm
            bk_ui->automode_AutoModeDefrostTempCheckBoxIm = lv_image_create(bk_ui->automode);
            _img_set_src_deferred(bk_ui->automode_AutoModeDefrostTempCheckBoxIm, "/images/auto_temp_checkbox.png");
            lv_obj_add_flag(bk_ui->automode_AutoModeDefrostTempCheckBoxIm, LV_OBJ_FLAG_HIDDEN);
            lv_obj_set_pos(bk_ui->automode_AutoModeDefrostTempCheckBoxIm, 640, 188);
            lv_obj_set_size(bk_ui->automode_AutoModeDefrostTempCheckBoxIm, 110, 99);

            // Button: AutoDefrostTimeHourBt
            bk_ui->automode_AutoDefrostTimeHourBt = lv_button_create(bk_ui->automode);
            lv_obj_add_flag(bk_ui->automode_AutoDefrostTimeHourBt, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_set_style_bg_opa(bk_ui->automode_AutoDefrostTimeHourBt, 0, 0);
            lv_obj_set_style_border_width(bk_ui->automode_AutoDefrostTimeHourBt, 0, 0);
            lv_obj_set_style_shadow_width(bk_ui->automode_AutoDefrostTimeHourBt, 0, 0);
            lv_obj_set_pos(bk_ui->automode_AutoDefrostTimeHourBt, 750, 190);
            lv_obj_set_size(bk_ui->automode_AutoDefrostTimeHourBt, 120, 100);

            // TextView: AutoDefrostTimeHourTxt
            bk_ui->automode_AutoDefrostTimeHourTxt = lv_label_create(bk_ui->automode);
            lv_label_set_text(bk_ui->automode_AutoDefrostTimeHourTxt, "");
            lv_obj_set_style_bg_opa(bk_ui->automode_AutoDefrostTimeHourTxt, 0, 0);
            lv_obj_set_style_text_color(bk_ui->automode_AutoDefrostTimeHourTxt, lv_color_hex(0x3C3A3D), 0);
            lv_obj_set_style_text_font(bk_ui->automode_AutoDefrostTimeHourTxt, &lv_font_scdream_regular_42, 0);
            lv_obj_set_pos(bk_ui->automode_AutoDefrostTimeHourTxt, 800, 210+9);
            lv_obj_set_size(bk_ui->automode_AutoDefrostTimeHourTxt, 70, 50);
            lv_obj_set_style_text_align(bk_ui->automode_AutoDefrostTimeHourTxt, LV_TEXT_ALIGN_CENTER, 0);

            // Button: AutoDefrostTimeMinBt
            bk_ui->automode_AutoDefrostTimeMinBt = lv_button_create(bk_ui->automode);
            lv_obj_add_flag(bk_ui->automode_AutoDefrostTimeMinBt, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_set_style_bg_opa(bk_ui->automode_AutoDefrostTimeMinBt, 0, 0);
            lv_obj_set_style_border_width(bk_ui->automode_AutoDefrostTimeMinBt, 0, 0);
            lv_obj_set_style_shadow_width(bk_ui->automode_AutoDefrostTimeMinBt, 0, 0);
            lv_obj_set_pos(bk_ui->automode_AutoDefrostTimeMinBt, 876, 190);
            lv_obj_set_size(bk_ui->automode_AutoDefrostTimeMinBt, 120, 100);

            // TextView: AutoDefrostTimeMinTxt
            bk_ui->automode_AutoDefrostTimeMinTxt = lv_label_create(bk_ui->automode);
            lv_label_set_text(bk_ui->automode_AutoDefrostTimeMinTxt, "");
            lv_obj_set_style_bg_opa(bk_ui->automode_AutoDefrostTimeMinTxt, 0, 0);
            lv_obj_set_style_text_color(bk_ui->automode_AutoDefrostTimeMinTxt, lv_color_hex(0x3C3A3D), 0);
            lv_obj_set_style_text_font(bk_ui->automode_AutoDefrostTimeMinTxt, &lv_font_scdream_regular_42, 0);
            lv_obj_set_pos(bk_ui->automode_AutoDefrostTimeMinTxt, 877, 210+9);
            lv_obj_set_size(bk_ui->automode_AutoDefrostTimeMinTxt, 70, 50);
            lv_obj_set_style_text_align(bk_ui->automode_AutoDefrostTimeMinTxt, LV_TEXT_ALIGN_CENTER, 0);

            // ImageView: AutoModeDefrostTimeCheckBoxIm
            bk_ui->automode_AutoModeDefrostTimeCheckBoxIm = lv_image_create(bk_ui->automode);
            _img_set_src_deferred(bk_ui->automode_AutoModeDefrostTimeCheckBoxIm, "/images/auto_autotime_checkbox.png");
            lv_obj_add_flag(bk_ui->automode_AutoModeDefrostTimeCheckBoxIm, LV_OBJ_FLAG_HIDDEN);
            lv_obj_set_pos(bk_ui->automode_AutoModeDefrostTimeCheckBoxIm, 754, 188);
            lv_obj_set_size(bk_ui->automode_AutoModeDefrostTimeCheckBoxIm, 242, 99);

            // ImageView: AutoModeDefrostAutoTime
            bk_ui->automode_AutoModeDefrostAutoTime = lv_image_create(bk_ui->automode);
            _img_set_src_deferred(bk_ui->automode_AutoModeDefrostAutoTime, "/images/defrost_auto_time_box.png");
            lv_obj_add_flag(bk_ui->automode_AutoModeDefrostAutoTime, LV_OBJ_FLAG_HIDDEN);
            lv_obj_set_pos(bk_ui->automode_AutoModeDefrostAutoTime, 754, 188);
            lv_obj_set_size(bk_ui->automode_AutoModeDefrostAutoTime, 242, 99);

            // ImageView: imageview44
            bk_ui->automode_imageview44 = lv_image_create(bk_ui->automode);
            _img_set_src_timed(bk_ui->automode_imageview44, "/images/auto_mode_fermentation1_board.png");
            lv_obj_set_pos(bk_ui->automode_imageview44, 15, 307);
            lv_obj_set_size(bk_ui->automode_imageview44, 490, 125);

            // Button: AutoFermentation1TempBt
            bk_ui->automode_AutoFermentation1TempBt = lv_button_create(bk_ui->automode);
            lv_obj_add_flag(bk_ui->automode_AutoFermentation1TempBt, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_set_style_bg_opa(bk_ui->automode_AutoFermentation1TempBt, 0, 0);
            lv_obj_set_style_border_width(bk_ui->automode_AutoFermentation1TempBt, 0, 0);
            lv_obj_set_style_shadow_width(bk_ui->automode_AutoFermentation1TempBt, 0, 0);
            lv_obj_set_pos(bk_ui->automode_AutoFermentation1TempBt, 135, 320);
            lv_obj_set_size(bk_ui->automode_AutoFermentation1TempBt, 110, 99);

            // TextView: AutoFermentation1TempTxt
            bk_ui->automode_AutoFermentation1TempTxt = lv_label_create(bk_ui->automode);
            lv_label_set_text(bk_ui->automode_AutoFermentation1TempTxt, "");
            lv_obj_set_style_bg_opa(bk_ui->automode_AutoFermentation1TempTxt, 0, 0);
            lv_obj_set_style_text_color(bk_ui->automode_AutoFermentation1TempTxt, lv_color_hex(0x3C3A3D), 0);
            lv_obj_set_style_text_font(bk_ui->automode_AutoFermentation1TempTxt, &lv_font_scdream_regular_42, 0);
            lv_obj_set_pos(bk_ui->automode_AutoFermentation1TempTxt, 135, 342+9);
            lv_obj_set_size(bk_ui->automode_AutoFermentation1TempTxt, 80, 50);
            lv_obj_set_style_text_align(bk_ui->automode_AutoFermentation1TempTxt, LV_TEXT_ALIGN_RIGHT, 0);

            // ImageView: AutoModeFermentation1TempCheckBoxIm
            bk_ui->automode_AutoModeFermentation1TempCheckBoxIm = lv_image_create(bk_ui->automode);
            _img_set_src_deferred(bk_ui->automode_AutoModeFermentation1TempCheckBoxIm, "/images/auto_temp_checkbox.png");
            lv_obj_add_flag(bk_ui->automode_AutoModeFermentation1TempCheckBoxIm, LV_OBJ_FLAG_HIDDEN);
            lv_obj_set_pos(bk_ui->automode_AutoModeFermentation1TempCheckBoxIm, 136, 320);
            lv_obj_set_size(bk_ui->automode_AutoModeFermentation1TempCheckBoxIm, 110, 99);

            // Button: AutoFermentation1HumidityBt
            bk_ui->automode_AutoFermentation1HumidityBt = lv_button_create(bk_ui->automode);
            lv_obj_add_flag(bk_ui->automode_AutoFermentation1HumidityBt, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_set_style_bg_opa(bk_ui->automode_AutoFermentation1HumidityBt, 0, 0);
            lv_obj_set_style_border_width(bk_ui->automode_AutoFermentation1HumidityBt, 0, 0);
            lv_obj_set_style_shadow_width(bk_ui->automode_AutoFermentation1HumidityBt, 0, 0);
            lv_obj_set_pos(bk_ui->automode_AutoFermentation1HumidityBt, 247, 320);
            lv_obj_set_size(bk_ui->automode_AutoFermentation1HumidityBt, 100, 99);

            // TextView: AutoFermentation1HumidityTxt
            bk_ui->automode_AutoFermentation1HumidityTxt = lv_label_create(bk_ui->automode);
            lv_label_set_text(bk_ui->automode_AutoFermentation1HumidityTxt, "");
            lv_obj_set_style_bg_opa(bk_ui->automode_AutoFermentation1HumidityTxt, 0, 0);
            lv_obj_set_style_text_color(bk_ui->automode_AutoFermentation1HumidityTxt, lv_color_hex(0x3C3A3D), 0);
            lv_obj_set_style_text_font(bk_ui->automode_AutoFermentation1HumidityTxt, &lv_font_scdream_regular_42, 0);
            lv_obj_set_pos(bk_ui->automode_AutoFermentation1HumidityTxt, 247, 342+9);
            lv_obj_set_size(bk_ui->automode_AutoFermentation1HumidityTxt, 80, 50);
            lv_obj_set_style_text_align(bk_ui->automode_AutoFermentation1HumidityTxt, LV_TEXT_ALIGN_CENTER, 0);

            // // ImageView: AutoModeFermentation1HumidityCheckBoxIm
            // bk_ui->automode_AutoModeFermentation1HumidityCheckBoxIm = lv_image_create(bk_ui->automode);
            // _img_set_src_timed(bk_ui->automode_AutoModeFermentation1HumidityCheckBoxIm, "/images/auto_humidity_checkbox.png");
            // lv_obj_add_flag(bk_ui->automode_AutoModeFermentation1HumidityCheckBoxIm, LV_OBJ_FLAG_HIDDEN);
            // lv_obj_set_pos(bk_ui->automode_AutoModeFermentation1HumidityCheckBoxIm, 250, 320);
            // lv_obj_set_size(bk_ui->automode_AutoModeFermentation1HumidityCheckBoxIm, 96, 99);

            // Button: AutoFermentation1TimeHourBt
            bk_ui->automode_AutoFermentation1TimeHourBt = lv_button_create(bk_ui->automode);
            lv_obj_add_flag(bk_ui->automode_AutoFermentation1TimeHourBt, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_set_style_bg_opa(bk_ui->automode_AutoFermentation1TimeHourBt, 0, 0);
            lv_obj_set_style_border_width(bk_ui->automode_AutoFermentation1TimeHourBt, 0, 0);
            lv_obj_set_style_shadow_width(bk_ui->automode_AutoFermentation1TimeHourBt, 0, 0);
            lv_obj_set_pos(bk_ui->automode_AutoFermentation1TimeHourBt, 345, 320);
            lv_obj_set_size(bk_ui->automode_AutoFermentation1TimeHourBt, 70, 100);

            // TextView: AutoFermentation1TimeHourTxt
            bk_ui->automode_AutoFermentation1TimeHourTxt = lv_label_create(bk_ui->automode);
            lv_label_set_text(bk_ui->automode_AutoFermentation1TimeHourTxt, "");
            lv_obj_set_style_bg_opa(bk_ui->automode_AutoFermentation1TimeHourTxt, 0, 0);
            lv_obj_set_style_text_color(bk_ui->automode_AutoFermentation1TimeHourTxt, lv_color_hex(0x3C3A3D), 0);
            lv_obj_set_style_text_font(bk_ui->automode_AutoFermentation1TimeHourTxt, &lv_font_scdream_regular_42, 0);
            lv_obj_set_pos(bk_ui->automode_AutoFermentation1TimeHourTxt, 338, 342+9);
            lv_obj_set_size(bk_ui->automode_AutoFermentation1TimeHourTxt, 88, 50);
            lv_obj_set_style_text_align(bk_ui->automode_AutoFermentation1TimeHourTxt, LV_TEXT_ALIGN_CENTER, 0);

            // Button: AutoFermentation1TimeMinBt
            bk_ui->automode_AutoFermentation1TimeMinBt = lv_button_create(bk_ui->automode);
            lv_obj_add_flag(bk_ui->automode_AutoFermentation1TimeMinBt, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_set_style_bg_opa(bk_ui->automode_AutoFermentation1TimeMinBt, 0, 0);
            lv_obj_set_style_border_width(bk_ui->automode_AutoFermentation1TimeMinBt, 0, 0);
            lv_obj_set_style_shadow_width(bk_ui->automode_AutoFermentation1TimeMinBt, 0, 0);
            lv_obj_set_pos(bk_ui->automode_AutoFermentation1TimeMinBt, 418, 320);
            lv_obj_set_size(bk_ui->automode_AutoFermentation1TimeMinBt, 80, 100);

            // TextView: AutoFermentation1TimeMinTxt
            bk_ui->automode_AutoFermentation1TimeMinTxt = lv_label_create(bk_ui->automode);
            lv_label_set_text(bk_ui->automode_AutoFermentation1TimeMinTxt, "");
            lv_obj_set_style_bg_opa(bk_ui->automode_AutoFermentation1TimeMinTxt, 0, 0);
            lv_obj_set_style_text_color(bk_ui->automode_AutoFermentation1TimeMinTxt, lv_color_hex(0x3C3A3D), 0);
            lv_obj_set_style_text_font(bk_ui->automode_AutoFermentation1TimeMinTxt, &lv_font_scdream_regular_42, 0);
            lv_obj_set_pos(bk_ui->automode_AutoFermentation1TimeMinTxt, 415, 342+9);
            lv_obj_set_size(bk_ui->automode_AutoFermentation1TimeMinTxt, 88, 50);
            lv_obj_set_style_text_align(bk_ui->automode_AutoFermentation1TimeMinTxt, LV_TEXT_ALIGN_CENTER, 0);

            // ImageView: AutoModeFermentation1TimeCheckBoxIm
            bk_ui->automode_AutoModeFermentation1TimeCheckBoxIm = lv_image_create(bk_ui->automode);
            _img_set_src_deferred(bk_ui->automode_AutoModeFermentation1TimeCheckBoxIm, "/images/auto_time_checkbox.png");
            lv_obj_add_flag(bk_ui->automode_AutoModeFermentation1TimeCheckBoxIm, LV_OBJ_FLAG_HIDDEN);
            lv_obj_set_pos(bk_ui->automode_AutoModeFermentation1TimeCheckBoxIm, 344, 320);
            lv_obj_set_size(bk_ui->automode_AutoModeFermentation1TimeCheckBoxIm, 142, 99);

            // ImageView: imageview60
            bk_ui->automode_imageview60 = lv_image_create(bk_ui->automode);
            _img_set_src_timed(bk_ui->automode_imageview60, "/images/auto_mode_fermentation2_board.png");
            lv_obj_set_pos(bk_ui->automode_imageview60, 519, 307);
            lv_obj_set_size(bk_ui->automode_imageview60, 490, 125);

            // Button: AutoFermentation2TempBt
            bk_ui->automode_AutoFermentation2TempBt = lv_button_create(bk_ui->automode);
            lv_obj_add_flag(bk_ui->automode_AutoFermentation2TempBt, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_set_style_bg_opa(bk_ui->automode_AutoFermentation2TempBt, 0, 0);
            lv_obj_set_style_border_width(bk_ui->automode_AutoFermentation2TempBt, 0, 0);
            lv_obj_set_style_shadow_width(bk_ui->automode_AutoFermentation2TempBt, 0, 0);
            lv_obj_set_pos(bk_ui->automode_AutoFermentation2TempBt, 640, 320);
            lv_obj_set_size(bk_ui->automode_AutoFermentation2TempBt, 110, 100);

            // TextView: AutoFermentation2TempTxt
            bk_ui->automode_AutoFermentation2TempTxt = lv_label_create(bk_ui->automode);
            lv_label_set_text(bk_ui->automode_AutoFermentation2TempTxt, "");
            lv_obj_set_style_bg_opa(bk_ui->automode_AutoFermentation2TempTxt, 0, 0);
            lv_obj_set_style_text_color(bk_ui->automode_AutoFermentation2TempTxt, lv_color_hex(0x3C3A3D), 0);
            lv_obj_set_style_text_font(bk_ui->automode_AutoFermentation2TempTxt, &lv_font_scdream_regular_42, 0);
            lv_obj_set_pos(bk_ui->automode_AutoFermentation2TempTxt, 638, 342+9);
            lv_obj_set_size(bk_ui->automode_AutoFermentation2TempTxt, 80, 50);
            lv_obj_set_style_text_align(bk_ui->automode_AutoFermentation2TempTxt, LV_TEXT_ALIGN_RIGHT, 0);

            // ImageView: AutoModeFermentation2TempCheckBoxIm
            bk_ui->automode_AutoModeFermentation2TempCheckBoxIm = lv_image_create(bk_ui->automode);
            _img_set_src_deferred(bk_ui->automode_AutoModeFermentation2TempCheckBoxIm, "/images/auto_temp_checkbox.png");
            lv_obj_add_flag(bk_ui->automode_AutoModeFermentation2TempCheckBoxIm, LV_OBJ_FLAG_HIDDEN);
            lv_obj_set_pos(bk_ui->automode_AutoModeFermentation2TempCheckBoxIm, 640, 320);
            lv_obj_set_size(bk_ui->automode_AutoModeFermentation2TempCheckBoxIm, 110, 99);

            // Button: AutoFermentation2HumidityBt
            bk_ui->automode_AutoFermentation2HumidityBt = lv_button_create(bk_ui->automode);
            lv_obj_add_flag(bk_ui->automode_AutoFermentation2HumidityBt, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_set_style_bg_opa(bk_ui->automode_AutoFermentation2HumidityBt, 0, 0);
            lv_obj_set_style_border_width(bk_ui->automode_AutoFermentation2HumidityBt, 0, 0);
            lv_obj_set_style_shadow_width(bk_ui->automode_AutoFermentation2HumidityBt, 0, 0);
            lv_obj_set_pos(bk_ui->automode_AutoFermentation2HumidityBt, 755, 323);
            lv_obj_set_size(bk_ui->automode_AutoFermentation2HumidityBt, 90, 100);

            // TextView: AutoFermentation2HumidityTxt
            bk_ui->automode_AutoFermentation2HumidityTxt = lv_label_create(bk_ui->automode);
            lv_label_set_text(bk_ui->automode_AutoFermentation2HumidityTxt, "");
            lv_obj_set_style_bg_opa(bk_ui->automode_AutoFermentation2HumidityTxt, 0, 0);
            lv_obj_set_style_text_color(bk_ui->automode_AutoFermentation2HumidityTxt, lv_color_hex(0x3C3A3D), 0);
            lv_obj_set_style_text_font(bk_ui->automode_AutoFermentation2HumidityTxt, &lv_font_scdream_regular_42, 0);
            lv_obj_set_pos(bk_ui->automode_AutoFermentation2HumidityTxt, 747, 342+9);
            lv_obj_set_size(bk_ui->automode_AutoFermentation2HumidityTxt, 88, 50);
            lv_obj_set_style_text_align(bk_ui->automode_AutoFermentation2HumidityTxt, LV_TEXT_ALIGN_CENTER, 0);


            // ImageView: AutoModeFermentation2HumidityCheckBoxIm
            bk_ui->automode_AutoModeFermentation2HumidityCheckBoxIm = lv_image_create(bk_ui->automode);
            _img_set_src_deferred(bk_ui->automode_AutoModeFermentation2HumidityCheckBoxIm, "/images/auto_humidity_checkbox.png");
            lv_obj_add_flag(bk_ui->automode_AutoModeFermentation2HumidityCheckBoxIm, LV_OBJ_FLAG_HIDDEN);
            lv_obj_set_pos(bk_ui->automode_AutoModeFermentation2HumidityCheckBoxIm, 754, 320);
            lv_obj_set_size(bk_ui->automode_AutoModeFermentation2HumidityCheckBoxIm, 96, 99);

            // Button: AutoFermentation2TimeHourBt
            bk_ui->automode_AutoFermentation2TimeHourBt = lv_button_create(bk_ui->automode);
            lv_obj_add_flag(bk_ui->automode_AutoFermentation2TimeHourBt, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_set_style_bg_opa(bk_ui->automode_AutoFermentation2TimeHourBt, 0, 0);
            lv_obj_set_style_border_width(bk_ui->automode_AutoFermentation2TimeHourBt, 0, 0);
            lv_obj_set_style_shadow_width(bk_ui->automode_AutoFermentation2TimeHourBt, 0, 0);
            lv_obj_set_pos(bk_ui->automode_AutoFermentation2TimeHourBt, 855, 323);
            lv_obj_set_size(bk_ui->automode_AutoFermentation2TimeHourBt, 70, 100);

            // TextView: AutoFermentation2TimeHourTxt
            bk_ui->automode_AutoFermentation2TimeHourTxt = lv_label_create(bk_ui->automode);
            lv_label_set_text(bk_ui->automode_AutoFermentation2TimeHourTxt, "");
            lv_obj_set_style_bg_opa(bk_ui->automode_AutoFermentation2TimeHourTxt, 0, 0);
            lv_obj_set_style_text_color(bk_ui->automode_AutoFermentation2TimeHourTxt, lv_color_hex(0x3C3A3D), 0);
            lv_obj_set_style_text_font(bk_ui->automode_AutoFermentation2TimeHourTxt, &lv_font_scdream_regular_42, 0);
            lv_obj_set_pos(bk_ui->automode_AutoFermentation2TimeHourTxt, 848, 342+9);
            lv_obj_set_size(bk_ui->automode_AutoFermentation2TimeHourTxt, 70, 50);
            lv_obj_set_style_text_align(bk_ui->automode_AutoFermentation2TimeHourTxt, LV_TEXT_ALIGN_CENTER, 0);


            // Button: AutoFermentation2TimeMinBt
            bk_ui->automode_AutoFermentation2TimeMinBt = lv_button_create(bk_ui->automode);
            lv_obj_add_flag(bk_ui->automode_AutoFermentation2TimeMinBt, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_set_style_bg_opa(bk_ui->automode_AutoFermentation2TimeMinBt, 0, 0);
            lv_obj_set_style_border_width(bk_ui->automode_AutoFermentation2TimeMinBt, 0, 0);
            lv_obj_set_style_shadow_width(bk_ui->automode_AutoFermentation2TimeMinBt, 0, 0);
            lv_obj_set_pos(bk_ui->automode_AutoFermentation2TimeMinBt, 928, 320);
            lv_obj_set_size(bk_ui->automode_AutoFermentation2TimeMinBt, 70, 100);

            // TextView: AutoFermentation2TimeMinTxt
            bk_ui->automode_AutoFermentation2TimeMinTxt = lv_label_create(bk_ui->automode);
            lv_label_set_text(bk_ui->automode_AutoFermentation2TimeMinTxt, "");
            lv_obj_set_style_bg_opa(bk_ui->automode_AutoFermentation2TimeMinTxt, 0, 0);
            lv_obj_set_style_text_color(bk_ui->automode_AutoFermentation2TimeMinTxt, lv_color_hex(0x3C3A3D), 0);
            lv_obj_set_style_text_font(bk_ui->automode_AutoFermentation2TimeMinTxt, &lv_font_scdream_regular_42, 0);
            lv_obj_set_pos(bk_ui->automode_AutoFermentation2TimeMinTxt, 927, 342+9);
            lv_obj_set_size(bk_ui->automode_AutoFermentation2TimeMinTxt, 70, 50);
            lv_obj_set_style_text_align(bk_ui->automode_AutoFermentation2TimeMinTxt, LV_TEXT_ALIGN_CENTER, 0);


            // ImageView: AutoModeFermentation2TimeCheckBoxIm
            bk_ui->automode_AutoModeFermentation2TimeCheckBoxIm = lv_image_create(bk_ui->automode);
            _img_set_src_deferred(bk_ui->automode_AutoModeFermentation2TimeCheckBoxIm, "/images/auto_time_checkbox.png");
            lv_obj_add_flag(bk_ui->automode_AutoModeFermentation2TimeCheckBoxIm, LV_OBJ_FLAG_HIDDEN);
            lv_obj_set_pos(bk_ui->automode_AutoModeFermentation2TimeCheckBoxIm, 854, 320);
            lv_obj_set_size(bk_ui->automode_AutoModeFermentation2TimeCheckBoxIm, 142, 99);
            // keypadbaseim + KeyPad: lazy-created in _keypad_on_automode on first use
            // auto_f1~f4: lazy-created in automode_load_event_cb only when °F mode active

            currentStep++;
            return;
        }
        case RENDER_STEP_CACHE_IMAGE :
        {
            const uint32_t ImageCount = preRenderPageConfig[PAGE_AUTOMODE].preRenderImageCount;
            if(currentImageStep != ImageCount)
            {
                // TODO : decode and cache the background image
                // TODO : decode and cache the pngimage prop for currentImageStep
                currentImageStep++;
                return;
            }
            else
            {
                currentImageStep = 0;
                currentStep++;
                return;
            }
        }
        case RENDER_STEP_ATTACH_EVENT :
        {
            lv_obj_add_event_cb(bk_ui->automode, automode_load_start_event_cb, UI_EVENT_PAGE_SHOW_START,   NULL);
            lv_obj_add_event_cb(bk_ui->automode, automode_loaded_event_cb, UI_EVENT_PAGE_SHOWN,       NULL);
            lv_obj_add_event_cb(bk_ui->automode, automode_unloaded_event_cb, UI_EVENT_PAGE_HIDDEN, NULL);
            lv_obj_add_event_cb(bk_ui->automode, automode_unload_start_event_cb, UI_EVENT_PAGE_HIDE_START, NULL);
            
            lv_obj_add_event_cb(bk_ui->automode_backbt, automode_backbt_event_cb, LV_EVENT_PRESSED, NULL);
            lv_obj_add_event_cb(bk_ui->automode_startbt, automode_startbt_event_cb, LV_EVENT_PRESSED, NULL);
            lv_obj_add_event_cb(bk_ui->automode_AutoModeCompleteYearBt, automode_AutoModeCompleteYearBt_event_cb, LV_EVENT_PRESSED, NULL);

            lv_obj_add_event_cb(bk_ui->automode_AutoModeCompleteMonthBt, automode_AutoModeCompleteMonthBt_event_cb, LV_EVENT_PRESSED, NULL);
            lv_obj_add_event_cb(bk_ui->automode_AutoModeCompleteDayBt, automode_AutoModeCompleteDayBt_event_cb, LV_EVENT_PRESSED, NULL);
            lv_obj_add_event_cb(bk_ui->automode_AutoModeCompleteHourBt, automode_AutoModeCompleteHourBt_event_cb, LV_EVENT_PRESSED, NULL);
            lv_obj_add_event_cb(bk_ui->automode_AutoModeCompleteMinBt, automode_AutoModeCompleteMinBt_event_cb, LV_EVENT_PRESSED, NULL);
            lv_obj_add_event_cb(bk_ui->automode_loadbt, automode_loadbt_event_cb, LV_EVENT_PRESSED, NULL);
            lv_obj_add_event_cb(bk_ui->automode_savebt, automode_savebt_event_cb, LV_EVENT_PRESSED, NULL);
            lv_obj_add_event_cb(bk_ui->automode_AutoFreezeTempBt, automode_AutoFreezeTempBt_event_cb, LV_EVENT_PRESSED, NULL);
            lv_obj_add_event_cb(bk_ui->automode_AutoDefrostTempBt, automode_AutoDefrostTempBt_event_cb, LV_EVENT_PRESSED, NULL);
            lv_obj_add_event_cb(bk_ui->automode_AutoDefrostTimeHourBt, automode_AutoDefrostTimeHourBt_event_cb, LV_EVENT_PRESSED, NULL);
            lv_obj_add_event_cb(bk_ui->automode_AutoDefrostTimeMinBt, automode_AutoDefrostTimeMinBt_event_cb, LV_EVENT_PRESSED, NULL);
            lv_obj_add_event_cb(bk_ui->automode_AutoFermentation1TempBt, automode_AutoFermentation1TempBt_event_cb, LV_EVENT_PRESSED, NULL);
            lv_obj_add_event_cb(bk_ui->automode_AutoFermentation1HumidityBt, automode_AutoFermentation1HumidityBt_event_cb, LV_EVENT_PRESSED, NULL);
            lv_obj_add_event_cb(bk_ui->automode_AutoFermentation1TimeHourBt, automode_AutoFermentation1TimeHourBt_event_cb, LV_EVENT_PRESSED, NULL);
            lv_obj_add_event_cb(bk_ui->automode_AutoFermentation1TimeMinBt, automode_AutoFermentation1TimeMinBt_event_cb, LV_EVENT_PRESSED, NULL);
            lv_obj_add_event_cb(bk_ui->automode_AutoFermentation2TempBt, automode_AutoFermentation2TempBt_event_cb, LV_EVENT_PRESSED, NULL);
            lv_obj_add_event_cb(bk_ui->automode_AutoFermentation2HumidityBt, automode_AutoFermentation2HumidityBt_event_cb, LV_EVENT_PRESSED, NULL);
            lv_obj_add_event_cb(bk_ui->automode_AutoFermentation2TimeHourBt, automode_AutoFermentation2TimeHourBt_event_cb, LV_EVENT_PRESSED, NULL);
            lv_obj_add_event_cb(bk_ui->automode_AutoFermentation2TimeMinBt, automode_AutoFermentation2TimeMinBt_event_cb, LV_EVENT_PRESSED, NULL);
            currentStep++;
            return;
        }
    }
}

void init_keypad_group(bk_lv_ui_t *bk_ui) {
    /*FIXME: 다음줄부터
    * lv_style_init은 여러번 호출되면 안 됨
    * 왜 이렇게 만들었는지 모르겠는데 나중에 beken_ui.c에서 만들어서 extern으로 데리고 오면 될듯
    * 지금은 일단 손댈게 많으니까 이쪽은 나중에 손대는데 중요노트라 길게 씀 */
    static lv_style_t style_transp;
    lv_style_init(&style_transp);
    lv_style_set_bg_opa(&style_transp, LV_OPA_TRANSP);
    lv_style_set_border_width(&style_transp, 0);
    lv_style_set_shadow_width(&style_transp, 0);

    const int x_start = 20, y_start = 453, x_step = 72;
    const char *keypad_names[] = {"1","2","3","4","5","6","7","8","9","0","minor","back"};
    char path_buf[64];

    for (int i = 0; i < 12; i++) {
        bk_ui->automode_KeyPadBt[i] = lv_button_create(bk_ui->automode);
        lv_obj_add_style(bk_ui->automode_KeyPadBt[i], &style_transp, 0);
        lv_obj_set_pos(bk_ui->automode_KeyPadBt[i], x_start + i * x_step, y_start);
        lv_obj_set_size(bk_ui->automode_KeyPadBt[i], 65, 75);
        lv_obj_add_event_cb(bk_ui->automode_KeyPadBt[i], keypad_touch_event_cb, LV_EVENT_ALL, (void *)(intptr_t)i);
        // printf("/images/keypad%s.png", keypad_names[i]);

        snprintf(path_buf, sizeof(path_buf), "/images/keypad%s.png", keypad_names[i]);
        bk_ui->automode_KeyPadIm[i] = lv_image_create(bk_ui->automode);
        _img_set_src_timed(bk_ui->automode_KeyPadIm[i], path_buf);
        lv_obj_set_pos(bk_ui->automode_KeyPadIm[i], x_start + i * x_step, y_start);
        lv_obj_set_size(bk_ui->automode_KeyPadIm[i], 65, 75);
        lv_obj_add_flag(bk_ui->automode_KeyPadIm[i], LV_OBJ_FLAG_HIDDEN);
        lv_obj_remove_flag(bk_ui->automode_KeyPadIm[i], LV_OBJ_FLAG_CLICKABLE);
    }

    bk_ui->automode_keypadhide = lv_button_create(bk_ui->automode);
    lv_obj_add_style(bk_ui->automode_keypadhide, &style_transp, 0);
    lv_obj_add_event_cb(bk_ui->automode_keypadhide, automode_keypadhide_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_pos(bk_ui->automode_keypadhide, 884, 453);
    lv_obj_set_size(bk_ui->automode_keypadhide, 120, 75);
    lv_obj_add_flag(bk_ui->automode_keypadhide, LV_OBJ_FLAG_CLICKABLE);
}
