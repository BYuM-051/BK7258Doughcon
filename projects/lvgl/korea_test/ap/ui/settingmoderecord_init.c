#include "lvgl.h"
#include "beken_ui.h"
#include "custom_func.h"
#include "event_runtime.h"
#include "ui_lang.h"
#include <stdio.h>
#include <string.h>

#include "ui_config.h"
#include "preRenderer.h"
#include "settings.h"

#define TAG "[settingmoderecord_init.c] "
#include "preRenderer.h"
// #define bk_printf(fmt, ...) do {if(0) bk_printf(fmt, ##__VA_ARGS__); } while(0) // disable printf

static uint32_t currentStep = RENDER_STEP_CREATE_PAGE;
static uint32_t currentImageStep = 0;

extern bk_lv_ui_t bk_lv_tool_ui;
extern lv_obj_t *preRenderRoot;

static bool stepInitMode = false;
extern void settingmoderecord_deletebt_event_cb(lv_event_t *e);
extern void settingmoderecord_backbt_event_cb(lv_event_t *e);
extern void settingmoderecord_load_start_event_cb(lv_event_t *e);
extern void settingmoderecord_loaded_event_cb(lv_event_t *e);
extern void settingmoderecord_unload_start_event_cb(lv_event_t *e);
extern void settingmoderecord_unloaded_event_cb(lv_event_t *e);

void destroy_page_settingmoderecord(bk_lv_ui_t *bk_ui)
{
    if (bk_ui == NULL) {
        return;
    }
    if (bk_ui->settingmoderecord != NULL) {
        lv_obj_del(bk_ui->settingmoderecord);
        bk_ui->settingmoderecord = NULL;
    }

    currentStep = RENDER_STEP_CREATE_PAGE;
    currentImageStep = 0;
    preRenderPageState[PAGE_SETTINGMODERECORD].isRendered = false;

    const uint32_t imageCount = preRenderPageConfig[PAGE_SETTINGMODERECORD].preRenderImageCount;
    for(uint32_t i = 0; i < imageCount; i++)
    {
        const preRenderImageInfo_t *imageInfo = &preRenderPageConfig[PAGE_SETTINGMODERECORD].preRenderImageInfo[i];
        const char *languageSuffix = imageInfo->hasLanguageVariant ?
                                     (settings_get_int("LANGUAGE") == 1 ? "_china" :
                                      settings_get_int("LANGUAGE") == 2 ? "_english" : "") : "";
        const char *degreeSuffix = imageInfo->hasDegreeVariant &&
                                   strcmp(settings_get_str("Degree"), "\xc2\xb0""F") == 0 ? "_f" : "";
        const char *extension = imageInfo->fileExtension != NULL ? imageInfo->fileExtension : ".png";
        char imagePath[128] = {0};

        snprintf(imagePath, sizeof(imagePath), "%s%s%s%s",
                 imageInfo->imagePath, degreeSuffix, languageSuffix, extension);
        lv_image_cache_drop(imagePath);
    }
}

void init_page_settingmoderecord(bk_lv_ui_t * bk_ui) {
    if(stepInitMode && currentStep == RENDER_STEP_CREATE_CHILD)
    {
        goto create_children;
    }

    if (bk_ui->settingmoderecord != NULL && lv_obj_is_valid(bk_ui->settingmoderecord)) {
        destroy_page_settingmoderecord(bk_ui);
    }

    ui_lang_reset_settingmoderecord_cache();

#if UI_PRENDERING_ENABLE
    bk_ui->settingmoderecord = lv_obj_create(preRenderRoot);
    lv_obj_remove_style_all(bk_ui->settingmoderecord);
    lv_obj_set_size(bk_ui->settingmoderecord, 1024, 600);
    lv_obj_set_pos(bk_ui->settingmoderecord, 0, 0);
    lv_obj_set_style_radius(bk_ui->settingmoderecord, 0, LV_PART_MAIN);
    lv_obj_set_scrollbar_mode(bk_ui->settingmoderecord, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_event_cb(bk_ui->settingmoderecord, settingmoderecord_load_start_event_cb, UI_EVENT_PAGE_SHOW_START, NULL);
    lv_obj_add_event_cb(bk_ui->settingmoderecord, settingmoderecord_loaded_event_cb, UI_EVENT_PAGE_SHOWN,     NULL);
    lv_obj_add_event_cb(bk_ui->settingmoderecord, settingmoderecord_unload_start_event_cb, UI_EVENT_PAGE_HIDE_START, NULL);
    lv_obj_add_event_cb(bk_ui->settingmoderecord, settingmoderecord_unloaded_event_cb, UI_EVENT_PAGE_HIDDEN,     NULL);
#else
    bk_ui->settingmoderecord = lv_obj_create(NULL);
    lv_obj_set_size(bk_ui->settingmoderecord, 1024, 600);
    lv_obj_set_scrollbar_mode(bk_ui->settingmoderecord, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_event_cb(bk_ui->settingmoderecord, settingmoderecord_load_start_event_cb, LV_EVENT_SCREEN_LOAD_START, NULL);
    lv_obj_add_event_cb(bk_ui->settingmoderecord, settingmoderecord_loaded_event_cb, LV_EVENT_SCREEN_LOADED,     NULL);
    lv_obj_add_event_cb(bk_ui->settingmoderecord, settingmoderecord_unload_start_event_cb, LV_EVENT_SCREEN_UNLOAD_START, NULL);
    lv_obj_add_event_cb(bk_ui->settingmoderecord, settingmoderecord_unloaded_event_cb, LV_EVENT_SCREEN_UNLOADED,     NULL);
#endif /* UI_PRENDERING_ENABLE */

    if(stepInitMode)
    {
        return;
    }

create_children:
    /* 배경 — bg.jpg 대신 단색(0xd9d9d9) */
    bk_ui->settingmoderecord_bg = lv_image_create(bk_ui->settingmoderecord);
    lv_obj_add_flag(bk_ui->settingmoderecord_bg, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_style_bg_color(bk_ui->settingmoderecord, lv_color_hex(0xd9d9d9), 0);
    lv_obj_set_style_bg_opa(bk_ui->settingmoderecord, LV_OPA_COVER, 0);
    lv_obj_set_pos(bk_ui->settingmoderecord_bg, 0, 0);

    // ImageView: title
    bk_ui->settingmoderecord_title = lv_image_create(bk_ui->settingmoderecord);
    _img_set_src_timed(bk_ui->settingmoderecord_title, "/images/record_title.png");
    lv_obj_set_pos(bk_ui->settingmoderecord_title, 0, 10);
    lv_obj_set_size(bk_ui->settingmoderecord_title, 380, 80);
    lv_image_set_inner_align(bk_ui->settingmoderecord_title, LV_IMAGE_ALIGN_TOP_LEFT);

    // ImageView: imageview2
    bk_ui->settingmoderecord_imageview2 = lv_image_create(bk_ui->settingmoderecord);
    _img_set_src_timed(bk_ui->settingmoderecord_imageview2, "/images/deleteall_bt.png");
    lv_obj_set_pos(bk_ui->settingmoderecord_imageview2, 647, 13);
    lv_obj_set_size(bk_ui->settingmoderecord_imageview2, 179, 74);

    // Button: deletebt
    bk_ui->settingmoderecord_deletebt = lv_button_create(bk_ui->settingmoderecord);
    lv_obj_add_flag(bk_ui->settingmoderecord_deletebt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->settingmoderecord_deletebt, settingmoderecord_deletebt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->settingmoderecord_deletebt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->settingmoderecord_deletebt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->settingmoderecord_deletebt, 0, 0);
    lv_obj_set_pos(bk_ui->settingmoderecord_deletebt, 647, 13);
    lv_obj_set_size(bk_ui->settingmoderecord_deletebt, 179, 74);

    // Button: backbt
    bk_ui->settingmoderecord_backbt = lv_button_create(bk_ui->settingmoderecord);
    lv_obj_add_flag(bk_ui->settingmoderecord_backbt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->settingmoderecord_backbt, settingmoderecord_backbt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->settingmoderecord_backbt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->settingmoderecord_backbt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->settingmoderecord_backbt, 0, 0);
    lv_obj_set_pos(bk_ui->settingmoderecord_backbt, 825, 13);
    lv_obj_set_size(bk_ui->settingmoderecord_backbt, 179, 74);

    // ImageView: imageview5
    bk_ui->settingmoderecord_imageview5 = lv_image_create(bk_ui->settingmoderecord);
    _img_set_src_timed(bk_ui->settingmoderecord_imageview5, "/images/exit_bt.png");
    lv_obj_set_pos(bk_ui->settingmoderecord_imageview5, 825, 13);
    lv_obj_set_size(bk_ui->settingmoderecord_imageview5, 179, 74);

    // ImageView: imageview6
    bk_ui->settingmoderecord_imageview6 = lv_image_create(bk_ui->settingmoderecord);
    _img_set_src_timed(bk_ui->settingmoderecord_imageview6, "/images/setting_record_chart_title.png");
    lv_obj_set_pos(bk_ui->settingmoderecord_imageview6, 12, 93);
    lv_obj_set_size(bk_ui->settingmoderecord_imageview6, 1000, 66);

    // ImageView: chartbox0
    bk_ui->settingmoderecord_chartbox0 = lv_image_create(bk_ui->settingmoderecord);
    _img_set_src_timed(bk_ui->settingmoderecord_chartbox0, "/images/setting_record_chart_box.png");
    lv_obj_set_pos(bk_ui->settingmoderecord_chartbox0, 12, 161);
    lv_obj_set_size(bk_ui->settingmoderecord_chartbox0, 1000, 66);

    // ImageView: chartbox1
    bk_ui->settingmoderecord_chartbox1 = lv_image_create(bk_ui->settingmoderecord);
    _img_set_src_timed(bk_ui->settingmoderecord_chartbox1, "/images/setting_record_chart_box.png");
    lv_obj_set_pos(bk_ui->settingmoderecord_chartbox1, 12, 231);
    lv_obj_set_size(bk_ui->settingmoderecord_chartbox1, 1000, 66);

    // ImageView: chartbox2
    bk_ui->settingmoderecord_chartbox2 = lv_image_create(bk_ui->settingmoderecord);
    _img_set_src_timed(bk_ui->settingmoderecord_chartbox2, "/images/setting_record_chart_box.png");
    lv_obj_set_pos(bk_ui->settingmoderecord_chartbox2, 12, 301);
    lv_obj_set_size(bk_ui->settingmoderecord_chartbox2, 1000, 66);

    // ImageView: chartbox3
    bk_ui->settingmoderecord_chartbox3 = lv_image_create(bk_ui->settingmoderecord);
    _img_set_src_timed(bk_ui->settingmoderecord_chartbox3, "/images/setting_record_chart_box.png");
    lv_obj_set_pos(bk_ui->settingmoderecord_chartbox3, 12, 371);
    lv_obj_set_size(bk_ui->settingmoderecord_chartbox3, 1000, 66);

    // ImageView: chartbox4
    bk_ui->settingmoderecord_chartbox4 = lv_image_create(bk_ui->settingmoderecord);
    _img_set_src_timed(bk_ui->settingmoderecord_chartbox4, "/images/setting_record_chart_box.png");
    lv_obj_set_pos(bk_ui->settingmoderecord_chartbox4, 12, 441);
    lv_obj_set_size(bk_ui->settingmoderecord_chartbox4, 1000, 66);

    // TextView: label12
    bk_ui->settingmoderecord_imageview12 = lv_label_create(bk_ui->settingmoderecord);
    lv_label_set_text(bk_ui->settingmoderecord_imageview12, "1");
    lv_obj_set_style_bg_opa(bk_ui->settingmoderecord_imageview12, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmoderecord_imageview12, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmoderecord_imageview12, &lv_font_scdream_regular_22, 0);
    lv_obj_set_pos(bk_ui->settingmoderecord_imageview12, 42, 181);
    lv_obj_set_size(bk_ui->settingmoderecord_imageview12, 30, 30);

    // TextView: RecordFreezeTemp0
    bk_ui->settingmoderecord_RecordFreezeTemp0 = lv_label_create(bk_ui->settingmoderecord);
    lv_label_set_text(bk_ui->settingmoderecord_RecordFreezeTemp0, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmoderecord_RecordFreezeTemp0, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmoderecord_RecordFreezeTemp0, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmoderecord_RecordFreezeTemp0, &lv_font_scdream_regular_22, 0);
    lv_obj_set_pos(bk_ui->settingmoderecord_RecordFreezeTemp0, 103, 167);
    lv_obj_set_size(bk_ui->settingmoderecord_RecordFreezeTemp0, 60, 30);
    lv_obj_set_style_text_align(bk_ui->settingmoderecord_RecordFreezeTemp0, LV_TEXT_ALIGN_CENTER, 0);

    // TextView: RecordFreezeTimeHour0
    bk_ui->settingmoderecord_RecordFreezeTimeHour0 = lv_label_create(bk_ui->settingmoderecord);
    lv_label_set_text(bk_ui->settingmoderecord_RecordFreezeTimeHour0, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmoderecord_RecordFreezeTimeHour0, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmoderecord_RecordFreezeTimeHour0, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmoderecord_RecordFreezeTimeHour0, &lv_font_scdream_regular_22, 0);
    lv_obj_set_pos(bk_ui->settingmoderecord_RecordFreezeTimeHour0, 95, 197);
    lv_obj_set_size(bk_ui->settingmoderecord_RecordFreezeTimeHour0, 40, 30);
    lv_obj_set_style_text_align(bk_ui->settingmoderecord_RecordFreezeTimeHour0, LV_TEXT_ALIGN_CENTER, 0);

    // TextView: RecordFreezeTimeMin0
    bk_ui->settingmoderecord_RecordFreezeTimeMin0 = lv_label_create(bk_ui->settingmoderecord);
    lv_label_set_text(bk_ui->settingmoderecord_RecordFreezeTimeMin0, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmoderecord_RecordFreezeTimeMin0, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmoderecord_RecordFreezeTimeMin0, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmoderecord_RecordFreezeTimeMin0, &lv_font_scdream_regular_22, 0);
    lv_obj_set_pos(bk_ui->settingmoderecord_RecordFreezeTimeMin0, 156, 197);
    lv_obj_set_size(bk_ui->settingmoderecord_RecordFreezeTimeMin0, 40, 30);
    lv_obj_set_style_text_align(bk_ui->settingmoderecord_RecordFreezeTimeMin0, LV_TEXT_ALIGN_CENTER, 0);

    // TextView: RecordDefreezeTemp0
    bk_ui->settingmoderecord_RecordDefreezeTemp0 = lv_label_create(bk_ui->settingmoderecord);
    lv_label_set_text(bk_ui->settingmoderecord_RecordDefreezeTemp0, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmoderecord_RecordDefreezeTemp0, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmoderecord_RecordDefreezeTemp0, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmoderecord_RecordDefreezeTemp0, &lv_font_scdream_regular_22, 0);
    lv_obj_set_pos(bk_ui->settingmoderecord_RecordDefreezeTemp0, 248, 167);
    lv_obj_set_size(bk_ui->settingmoderecord_RecordDefreezeTemp0, 60, 30);
    lv_obj_set_style_text_align(bk_ui->settingmoderecord_RecordDefreezeTemp0, LV_TEXT_ALIGN_CENTER, 0);

    // TextView: RecordDefreezeTimeHour0
    bk_ui->settingmoderecord_RecordDefreezeTimeHour0 = lv_label_create(bk_ui->settingmoderecord);
    lv_label_set_text(bk_ui->settingmoderecord_RecordDefreezeTimeHour0, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmoderecord_RecordDefreezeTimeHour0, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmoderecord_RecordDefreezeTimeHour0, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmoderecord_RecordDefreezeTimeHour0, &lv_font_scdream_regular_22, 0);
    lv_obj_set_pos(bk_ui->settingmoderecord_RecordDefreezeTimeHour0, 235, 197);
    lv_obj_set_size(bk_ui->settingmoderecord_RecordDefreezeTimeHour0, 40, 30);
    lv_obj_set_style_text_align(bk_ui->settingmoderecord_RecordDefreezeTimeHour0, LV_TEXT_ALIGN_CENTER, 0);

    // TextView: RecordDefreezeTimeMin0
    bk_ui->settingmoderecord_RecordDefreezeTimeMin0 = lv_label_create(bk_ui->settingmoderecord);
    lv_label_set_text(bk_ui->settingmoderecord_RecordDefreezeTimeMin0, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmoderecord_RecordDefreezeTimeMin0, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmoderecord_RecordDefreezeTimeMin0, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmoderecord_RecordDefreezeTimeMin0, &lv_font_scdream_regular_22, 0);
    lv_obj_set_pos(bk_ui->settingmoderecord_RecordDefreezeTimeMin0, 296, 197);
    lv_obj_set_size(bk_ui->settingmoderecord_RecordDefreezeTimeMin0, 40, 30);
    lv_obj_set_style_text_align(bk_ui->settingmoderecord_RecordDefreezeTimeMin0, LV_TEXT_ALIGN_CENTER, 0);

    // TextView: RecordFermentation1Temp0
    bk_ui->settingmoderecord_RecordFermentation1Temp0 = lv_label_create(bk_ui->settingmoderecord);
    lv_label_set_text(bk_ui->settingmoderecord_RecordFermentation1Temp0, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmoderecord_RecordFermentation1Temp0, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmoderecord_RecordFermentation1Temp0, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmoderecord_RecordFermentation1Temp0, &lv_font_scdream_regular_22, 0);
    lv_obj_set_pos(bk_ui->settingmoderecord_RecordFermentation1Temp0, 376, 167);
    lv_obj_set_size(bk_ui->settingmoderecord_RecordFermentation1Temp0, 50, 30);
    lv_obj_set_style_text_align(bk_ui->settingmoderecord_RecordFermentation1Temp0, LV_TEXT_ALIGN_CENTER, 0);

    // TextView: RecordFermentation1Humidity0
    bk_ui->settingmoderecord_RecordFermentation1Humidity0 = lv_label_create(bk_ui->settingmoderecord);
    lv_label_set_text(bk_ui->settingmoderecord_RecordFermentation1Humidity0, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmoderecord_RecordFermentation1Humidity0, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmoderecord_RecordFermentation1Humidity0, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmoderecord_RecordFermentation1Humidity0, &lv_font_scdream_regular_22, 0);
    lv_obj_set_pos(bk_ui->settingmoderecord_RecordFermentation1Humidity0, 435, 167);
    lv_obj_set_size(bk_ui->settingmoderecord_RecordFermentation1Humidity0, 50, 30);
    lv_obj_set_style_text_align(bk_ui->settingmoderecord_RecordFermentation1Humidity0, LV_TEXT_ALIGN_CENTER, 0);

    // TextView: RecordFermentation1TimeHour0
    bk_ui->settingmoderecord_RecordFermentation1TimeHour0 = lv_label_create(bk_ui->settingmoderecord);
    lv_label_set_text(bk_ui->settingmoderecord_RecordFermentation1TimeHour0, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmoderecord_RecordFermentation1TimeHour0, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmoderecord_RecordFermentation1TimeHour0, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmoderecord_RecordFermentation1TimeHour0, &lv_font_scdream_regular_22, 0);
    lv_obj_set_pos(bk_ui->settingmoderecord_RecordFermentation1TimeHour0, 381, 197);
    lv_obj_set_size(bk_ui->settingmoderecord_RecordFermentation1TimeHour0, 40, 30);
    lv_obj_set_style_text_align(bk_ui->settingmoderecord_RecordFermentation1TimeHour0, LV_TEXT_ALIGN_CENTER, 0);

    // TextView: RecordFermentation1TimeMin0
    bk_ui->settingmoderecord_RecordFermentation1TimeMin0 = lv_label_create(bk_ui->settingmoderecord);
    lv_label_set_text(bk_ui->settingmoderecord_RecordFermentation1TimeMin0, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmoderecord_RecordFermentation1TimeMin0, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmoderecord_RecordFermentation1TimeMin0, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmoderecord_RecordFermentation1TimeMin0, &lv_font_scdream_regular_22, 0);
    lv_obj_set_pos(bk_ui->settingmoderecord_RecordFermentation1TimeMin0, 440, 197);
    lv_obj_set_size(bk_ui->settingmoderecord_RecordFermentation1TimeMin0, 40, 30);
    lv_obj_set_style_text_align(bk_ui->settingmoderecord_RecordFermentation1TimeMin0, LV_TEXT_ALIGN_CENTER, 0);

    // TextView: RecordFermentation2Temp0
    bk_ui->settingmoderecord_RecordFermentation2Temp0 = lv_label_create(bk_ui->settingmoderecord);
    lv_label_set_text(bk_ui->settingmoderecord_RecordFermentation2Temp0, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmoderecord_RecordFermentation2Temp0, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmoderecord_RecordFermentation2Temp0, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmoderecord_RecordFermentation2Temp0, &lv_font_scdream_regular_22, 0);
    lv_obj_set_pos(bk_ui->settingmoderecord_RecordFermentation2Temp0, 518, 167);
    lv_obj_set_size(bk_ui->settingmoderecord_RecordFermentation2Temp0, 50, 30);
    lv_obj_set_style_text_align(bk_ui->settingmoderecord_RecordFermentation2Temp0, LV_TEXT_ALIGN_CENTER, 0);

    // TextView: RecordFermentation2Humidity0
    bk_ui->settingmoderecord_RecordFermentation2Humidity0 = lv_label_create(bk_ui->settingmoderecord);
    lv_label_set_text(bk_ui->settingmoderecord_RecordFermentation2Humidity0, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmoderecord_RecordFermentation2Humidity0, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmoderecord_RecordFermentation2Humidity0, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmoderecord_RecordFermentation2Humidity0, &lv_font_scdream_regular_22, 0);
    lv_obj_set_pos(bk_ui->settingmoderecord_RecordFermentation2Humidity0, 577, 167);
    lv_obj_set_size(bk_ui->settingmoderecord_RecordFermentation2Humidity0, 50, 30);
    lv_obj_set_style_text_align(bk_ui->settingmoderecord_RecordFermentation2Humidity0, LV_TEXT_ALIGN_CENTER, 0);

    // TextView: RecordFermentation2TimeHour0
    bk_ui->settingmoderecord_RecordFermentation2TimeHour0 = lv_label_create(bk_ui->settingmoderecord);
    lv_label_set_text(bk_ui->settingmoderecord_RecordFermentation2TimeHour0, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmoderecord_RecordFermentation2TimeHour0, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmoderecord_RecordFermentation2TimeHour0, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmoderecord_RecordFermentation2TimeHour0, &lv_font_scdream_regular_22, 0);
    lv_obj_set_pos(bk_ui->settingmoderecord_RecordFermentation2TimeHour0, 523, 197);
    lv_obj_set_size(bk_ui->settingmoderecord_RecordFermentation2TimeHour0, 40, 30);
    lv_obj_set_style_text_align(bk_ui->settingmoderecord_RecordFermentation2TimeHour0, LV_TEXT_ALIGN_CENTER, 0);

    // TextView: RecordFermentation2TimeMin0
    bk_ui->settingmoderecord_RecordFermentation2TimeMin0 = lv_label_create(bk_ui->settingmoderecord);
    lv_label_set_text(bk_ui->settingmoderecord_RecordFermentation2TimeMin0, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmoderecord_RecordFermentation2TimeMin0, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmoderecord_RecordFermentation2TimeMin0, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmoderecord_RecordFermentation2TimeMin0, &lv_font_scdream_regular_22, 0);
    lv_obj_set_pos(bk_ui->settingmoderecord_RecordFermentation2TimeMin0, 582, 197);
    lv_obj_set_size(bk_ui->settingmoderecord_RecordFermentation2TimeMin0, 40, 30);
    lv_obj_set_style_text_align(bk_ui->settingmoderecord_RecordFermentation2TimeMin0, LV_TEXT_ALIGN_CENTER, 0);

    // TextView: RecordStartMonth0
    bk_ui->settingmoderecord_RecordStartMonth0 = lv_label_create(bk_ui->settingmoderecord);
    lv_label_set_text(bk_ui->settingmoderecord_RecordStartMonth0, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmoderecord_RecordStartMonth0, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmoderecord_RecordStartMonth0, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmoderecord_RecordStartMonth0, &lv_font_scdream_regular_22, 0);
    lv_obj_set_pos(bk_ui->settingmoderecord_RecordStartMonth0, 651, 179);
    lv_obj_set_size(bk_ui->settingmoderecord_RecordStartMonth0, 40, 30);
    lv_obj_set_style_text_align(bk_ui->settingmoderecord_RecordStartMonth0, LV_TEXT_ALIGN_CENTER, 0);

    // TextView: RecordStartDay0
    bk_ui->settingmoderecord_RecordStartDay0 = lv_label_create(bk_ui->settingmoderecord);
    lv_label_set_text(bk_ui->settingmoderecord_RecordStartDay0, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmoderecord_RecordStartDay0, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmoderecord_RecordStartDay0, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmoderecord_RecordStartDay0, &lv_font_scdream_regular_22, 0);
    lv_obj_set_pos(bk_ui->settingmoderecord_RecordStartDay0, 695, 179);
    lv_obj_set_size(bk_ui->settingmoderecord_RecordStartDay0, 40, 30);
    lv_obj_set_style_text_align(bk_ui->settingmoderecord_RecordStartDay0, LV_TEXT_ALIGN_CENTER, 0);

    // TextView: RecordStartHour0
    bk_ui->settingmoderecord_RecordStartHour0 = lv_label_create(bk_ui->settingmoderecord);
    lv_label_set_text(bk_ui->settingmoderecord_RecordStartHour0, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmoderecord_RecordStartHour0, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmoderecord_RecordStartHour0, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmoderecord_RecordStartHour0, &lv_font_scdream_regular_22, 0);
    lv_obj_set_pos(bk_ui->settingmoderecord_RecordStartHour0, 735, 179);
    lv_obj_set_size(bk_ui->settingmoderecord_RecordStartHour0, 40, 30);
    lv_obj_set_style_text_align(bk_ui->settingmoderecord_RecordStartHour0, LV_TEXT_ALIGN_CENTER, 0);

    // TextView: RecordStartMin0
    bk_ui->settingmoderecord_RecordStartMin0 = lv_label_create(bk_ui->settingmoderecord);
    lv_label_set_text(bk_ui->settingmoderecord_RecordStartMin0, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmoderecord_RecordStartMin0, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmoderecord_RecordStartMin0, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmoderecord_RecordStartMin0, &lv_font_scdream_regular_22, 0);
    lv_obj_set_pos(bk_ui->settingmoderecord_RecordStartMin0, 781, 179);
    lv_obj_set_size(bk_ui->settingmoderecord_RecordStartMin0, 40, 30);
    lv_obj_set_style_text_align(bk_ui->settingmoderecord_RecordStartMin0, LV_TEXT_ALIGN_CENTER, 0);

    // TextView: RecordEndMonth0
    bk_ui->settingmoderecord_RecordEndMonth0 = lv_label_create(bk_ui->settingmoderecord);
    lv_label_set_text(bk_ui->settingmoderecord_RecordEndMonth0, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmoderecord_RecordEndMonth0, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmoderecord_RecordEndMonth0, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmoderecord_RecordEndMonth0, &lv_font_scdream_regular_22, 0);
    lv_obj_set_pos(bk_ui->settingmoderecord_RecordEndMonth0, 831, 179);
    lv_obj_set_size(bk_ui->settingmoderecord_RecordEndMonth0, 40, 30);
    lv_obj_set_style_text_align(bk_ui->settingmoderecord_RecordEndMonth0, LV_TEXT_ALIGN_CENTER, 0);

    // TextView: RecordEndDay0
    bk_ui->settingmoderecord_RecordEndDay0 = lv_label_create(bk_ui->settingmoderecord);
    lv_label_set_text(bk_ui->settingmoderecord_RecordEndDay0, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmoderecord_RecordEndDay0, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmoderecord_RecordEndDay0, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmoderecord_RecordEndDay0, &lv_font_scdream_regular_22, 0);
    lv_obj_set_pos(bk_ui->settingmoderecord_RecordEndDay0, 875, 179);
    lv_obj_set_size(bk_ui->settingmoderecord_RecordEndDay0, 40, 30);
    lv_obj_set_style_text_align(bk_ui->settingmoderecord_RecordEndDay0, LV_TEXT_ALIGN_CENTER, 0);

    // TextView: RecordEndHour0
    bk_ui->settingmoderecord_RecordEndHour0 = lv_label_create(bk_ui->settingmoderecord);
    lv_label_set_text(bk_ui->settingmoderecord_RecordEndHour0, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmoderecord_RecordEndHour0, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmoderecord_RecordEndHour0, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmoderecord_RecordEndHour0, &lv_font_scdream_regular_22, 0);
    lv_obj_set_pos(bk_ui->settingmoderecord_RecordEndHour0, 915, 179);
    lv_obj_set_size(bk_ui->settingmoderecord_RecordEndHour0, 40, 30);
    lv_obj_set_style_text_align(bk_ui->settingmoderecord_RecordEndHour0, LV_TEXT_ALIGN_CENTER, 0);

    // TextView: RecordEndMin0
    bk_ui->settingmoderecord_RecordEndMin0 = lv_label_create(bk_ui->settingmoderecord);
    lv_label_set_text(bk_ui->settingmoderecord_RecordEndMin0, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmoderecord_RecordEndMin0, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmoderecord_RecordEndMin0, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmoderecord_RecordEndMin0, &lv_font_scdream_regular_22, 0);
    lv_obj_set_pos(bk_ui->settingmoderecord_RecordEndMin0, 961, 179);
    lv_obj_set_size(bk_ui->settingmoderecord_RecordEndMin0, 40, 30);
    lv_obj_set_style_text_align(bk_ui->settingmoderecord_RecordEndMin0, LV_TEXT_ALIGN_CENTER, 0);

    // TextView: label35
    bk_ui->settingmoderecord_imageview35 = lv_label_create(bk_ui->settingmoderecord);
    lv_label_set_text(bk_ui->settingmoderecord_imageview35, "2");
    lv_obj_set_style_bg_opa(bk_ui->settingmoderecord_imageview35, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmoderecord_imageview35, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmoderecord_imageview35, &lv_font_scdream_regular_22, 0);
    lv_obj_set_pos(bk_ui->settingmoderecord_imageview35, 42, 251);
    lv_obj_set_size(bk_ui->settingmoderecord_imageview35, 30, 30);

    // TextView: RecordFreezeTemp1
    bk_ui->settingmoderecord_RecordFreezeTemp1 = lv_label_create(bk_ui->settingmoderecord);
    lv_label_set_text(bk_ui->settingmoderecord_RecordFreezeTemp1, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmoderecord_RecordFreezeTemp1, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmoderecord_RecordFreezeTemp1, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmoderecord_RecordFreezeTemp1, &lv_font_scdream_regular_22, 0);
    lv_obj_set_pos(bk_ui->settingmoderecord_RecordFreezeTemp1, 103, 237);
    lv_obj_set_size(bk_ui->settingmoderecord_RecordFreezeTemp1, 60, 30);
    lv_obj_set_style_text_align(bk_ui->settingmoderecord_RecordFreezeTemp1, LV_TEXT_ALIGN_CENTER, 0);

    // TextView: RecordFreezeTimeHour1
    bk_ui->settingmoderecord_RecordFreezeTimeHour1 = lv_label_create(bk_ui->settingmoderecord);
    lv_label_set_text(bk_ui->settingmoderecord_RecordFreezeTimeHour1, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmoderecord_RecordFreezeTimeHour1, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmoderecord_RecordFreezeTimeHour1, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmoderecord_RecordFreezeTimeHour1, &lv_font_scdream_regular_22, 0);
    lv_obj_set_pos(bk_ui->settingmoderecord_RecordFreezeTimeHour1, 95, 267);
    lv_obj_set_size(bk_ui->settingmoderecord_RecordFreezeTimeHour1, 40, 30);
    lv_obj_set_style_text_align(bk_ui->settingmoderecord_RecordFreezeTimeHour1, LV_TEXT_ALIGN_CENTER, 0);

    // TextView: RecordFreezeTimeMin1
    bk_ui->settingmoderecord_RecordFreezeTimeMin1 = lv_label_create(bk_ui->settingmoderecord);
    lv_label_set_text(bk_ui->settingmoderecord_RecordFreezeTimeMin1, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmoderecord_RecordFreezeTimeMin1, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmoderecord_RecordFreezeTimeMin1, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmoderecord_RecordFreezeTimeMin1, &lv_font_scdream_regular_22, 0);
    lv_obj_set_pos(bk_ui->settingmoderecord_RecordFreezeTimeMin1, 156, 267);
    lv_obj_set_size(bk_ui->settingmoderecord_RecordFreezeTimeMin1, 40, 30);
    lv_obj_set_style_text_align(bk_ui->settingmoderecord_RecordFreezeTimeMin1, LV_TEXT_ALIGN_CENTER, 0);

    // TextView: RecordDefreezeTemp1
    bk_ui->settingmoderecord_RecordDefreezeTemp1 = lv_label_create(bk_ui->settingmoderecord);
    lv_label_set_text(bk_ui->settingmoderecord_RecordDefreezeTemp1, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmoderecord_RecordDefreezeTemp1, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmoderecord_RecordDefreezeTemp1, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmoderecord_RecordDefreezeTemp1, &lv_font_scdream_regular_22, 0);
    lv_obj_set_pos(bk_ui->settingmoderecord_RecordDefreezeTemp1, 248, 237);
    lv_obj_set_size(bk_ui->settingmoderecord_RecordDefreezeTemp1, 60, 30);
    lv_obj_set_style_text_align(bk_ui->settingmoderecord_RecordDefreezeTemp1, LV_TEXT_ALIGN_CENTER, 0);

    // TextView: RecordDefreezeTimeHour1
    bk_ui->settingmoderecord_RecordDefreezeTimeHour1 = lv_label_create(bk_ui->settingmoderecord);
    lv_label_set_text(bk_ui->settingmoderecord_RecordDefreezeTimeHour1, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmoderecord_RecordDefreezeTimeHour1, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmoderecord_RecordDefreezeTimeHour1, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmoderecord_RecordDefreezeTimeHour1, &lv_font_scdream_regular_22, 0);
    lv_obj_set_pos(bk_ui->settingmoderecord_RecordDefreezeTimeHour1, 235, 267);
    lv_obj_set_size(bk_ui->settingmoderecord_RecordDefreezeTimeHour1, 40, 30);
    lv_obj_set_style_text_align(bk_ui->settingmoderecord_RecordDefreezeTimeHour1, LV_TEXT_ALIGN_CENTER, 0);

    // TextView: RecordDefreezeTimeMin1
    bk_ui->settingmoderecord_RecordDefreezeTimeMin1 = lv_label_create(bk_ui->settingmoderecord);
    lv_label_set_text(bk_ui->settingmoderecord_RecordDefreezeTimeMin1, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmoderecord_RecordDefreezeTimeMin1, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmoderecord_RecordDefreezeTimeMin1, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmoderecord_RecordDefreezeTimeMin1, &lv_font_scdream_regular_22, 0);
    lv_obj_set_pos(bk_ui->settingmoderecord_RecordDefreezeTimeMin1, 296, 267);
    lv_obj_set_size(bk_ui->settingmoderecord_RecordDefreezeTimeMin1, 40, 30);
    lv_obj_set_style_text_align(bk_ui->settingmoderecord_RecordDefreezeTimeMin1, LV_TEXT_ALIGN_CENTER, 0);

    // TextView: RecordFermentation1Temp1
    bk_ui->settingmoderecord_RecordFermentation1Temp1 = lv_label_create(bk_ui->settingmoderecord);
    lv_label_set_text(bk_ui->settingmoderecord_RecordFermentation1Temp1, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmoderecord_RecordFermentation1Temp1, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmoderecord_RecordFermentation1Temp1, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmoderecord_RecordFermentation1Temp1, &lv_font_scdream_regular_22, 0);
    lv_obj_set_pos(bk_ui->settingmoderecord_RecordFermentation1Temp1, 376, 237);
    lv_obj_set_size(bk_ui->settingmoderecord_RecordFermentation1Temp1, 50, 30);
    lv_obj_set_style_text_align(bk_ui->settingmoderecord_RecordFermentation1Temp1, LV_TEXT_ALIGN_CENTER, 0);

    // TextView: RecordFermentation1Humidity1
    bk_ui->settingmoderecord_RecordFermentation1Humidity1 = lv_label_create(bk_ui->settingmoderecord);
    lv_label_set_text(bk_ui->settingmoderecord_RecordFermentation1Humidity1, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmoderecord_RecordFermentation1Humidity1, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmoderecord_RecordFermentation1Humidity1, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmoderecord_RecordFermentation1Humidity1, &lv_font_scdream_regular_22, 0);
    lv_obj_set_pos(bk_ui->settingmoderecord_RecordFermentation1Humidity1, 435, 237);
    lv_obj_set_size(bk_ui->settingmoderecord_RecordFermentation1Humidity1, 50, 30);
    lv_obj_set_style_text_align(bk_ui->settingmoderecord_RecordFermentation1Humidity1, LV_TEXT_ALIGN_CENTER, 0);

    // TextView: RecordFermentation1TimeHour1
    bk_ui->settingmoderecord_RecordFermentation1TimeHour1 = lv_label_create(bk_ui->settingmoderecord);
    lv_label_set_text(bk_ui->settingmoderecord_RecordFermentation1TimeHour1, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmoderecord_RecordFermentation1TimeHour1, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmoderecord_RecordFermentation1TimeHour1, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmoderecord_RecordFermentation1TimeHour1, &lv_font_scdream_regular_22, 0);
    lv_obj_set_pos(bk_ui->settingmoderecord_RecordFermentation1TimeHour1, 381, 267);
    lv_obj_set_size(bk_ui->settingmoderecord_RecordFermentation1TimeHour1, 40, 30);
    lv_obj_set_style_text_align(bk_ui->settingmoderecord_RecordFermentation1TimeHour1, LV_TEXT_ALIGN_CENTER, 0);

    // TextView: RecordFermentation1TimeMin1
    bk_ui->settingmoderecord_RecordFermentation1TimeMin1 = lv_label_create(bk_ui->settingmoderecord);
    lv_label_set_text(bk_ui->settingmoderecord_RecordFermentation1TimeMin1, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmoderecord_RecordFermentation1TimeMin1, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmoderecord_RecordFermentation1TimeMin1, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmoderecord_RecordFermentation1TimeMin1, &lv_font_scdream_regular_22, 0);
    lv_obj_set_pos(bk_ui->settingmoderecord_RecordFermentation1TimeMin1, 440, 267);
    lv_obj_set_size(bk_ui->settingmoderecord_RecordFermentation1TimeMin1, 40, 30);
    lv_obj_set_style_text_align(bk_ui->settingmoderecord_RecordFermentation1TimeMin1, LV_TEXT_ALIGN_CENTER, 0);

    // TextView: RecordFermentation2Temp1
    bk_ui->settingmoderecord_RecordFermentation2Temp1 = lv_label_create(bk_ui->settingmoderecord);
    lv_label_set_text(bk_ui->settingmoderecord_RecordFermentation2Temp1, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmoderecord_RecordFermentation2Temp1, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmoderecord_RecordFermentation2Temp1, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmoderecord_RecordFermentation2Temp1, &lv_font_scdream_regular_22, 0);
    lv_obj_set_pos(bk_ui->settingmoderecord_RecordFermentation2Temp1, 518, 237);
    lv_obj_set_size(bk_ui->settingmoderecord_RecordFermentation2Temp1, 50, 30);
    lv_obj_set_style_text_align(bk_ui->settingmoderecord_RecordFermentation2Temp1, LV_TEXT_ALIGN_CENTER, 0);

    // TextView: RecordFermentation2Humidity1
    bk_ui->settingmoderecord_RecordFermentation2Humidity1 = lv_label_create(bk_ui->settingmoderecord);
    lv_label_set_text(bk_ui->settingmoderecord_RecordFermentation2Humidity1, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmoderecord_RecordFermentation2Humidity1, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmoderecord_RecordFermentation2Humidity1, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmoderecord_RecordFermentation2Humidity1, &lv_font_scdream_regular_22, 0);
    lv_obj_set_pos(bk_ui->settingmoderecord_RecordFermentation2Humidity1, 577, 237);
    lv_obj_set_size(bk_ui->settingmoderecord_RecordFermentation2Humidity1, 50, 30);
    lv_obj_set_style_text_align(bk_ui->settingmoderecord_RecordFermentation2Humidity1, LV_TEXT_ALIGN_CENTER, 0);

    // TextView: RecordFermentation2TimeHour1
    bk_ui->settingmoderecord_RecordFermentation2TimeHour1 = lv_label_create(bk_ui->settingmoderecord);
    lv_label_set_text(bk_ui->settingmoderecord_RecordFermentation2TimeHour1, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmoderecord_RecordFermentation2TimeHour1, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmoderecord_RecordFermentation2TimeHour1, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmoderecord_RecordFermentation2TimeHour1, &lv_font_scdream_regular_22, 0);
    lv_obj_set_pos(bk_ui->settingmoderecord_RecordFermentation2TimeHour1, 523, 267);
    lv_obj_set_size(bk_ui->settingmoderecord_RecordFermentation2TimeHour1, 40, 30);
    lv_obj_set_style_text_align(bk_ui->settingmoderecord_RecordFermentation2TimeHour1, LV_TEXT_ALIGN_CENTER, 0);

    // TextView: RecordFermentation2TimeMin1
    bk_ui->settingmoderecord_RecordFermentation2TimeMin1 = lv_label_create(bk_ui->settingmoderecord);
    lv_label_set_text(bk_ui->settingmoderecord_RecordFermentation2TimeMin1, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmoderecord_RecordFermentation2TimeMin1, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmoderecord_RecordFermentation2TimeMin1, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmoderecord_RecordFermentation2TimeMin1, &lv_font_scdream_regular_22, 0);
    lv_obj_set_pos(bk_ui->settingmoderecord_RecordFermentation2TimeMin1, 582, 267);
    lv_obj_set_size(bk_ui->settingmoderecord_RecordFermentation2TimeMin1, 40, 30);
    lv_obj_set_style_text_align(bk_ui->settingmoderecord_RecordFermentation2TimeMin1, LV_TEXT_ALIGN_CENTER, 0);

    // TextView: RecordStartMonth1
    bk_ui->settingmoderecord_RecordStartMonth1 = lv_label_create(bk_ui->settingmoderecord);
    lv_label_set_text(bk_ui->settingmoderecord_RecordStartMonth1, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmoderecord_RecordStartMonth1, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmoderecord_RecordStartMonth1, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmoderecord_RecordStartMonth1, &lv_font_scdream_regular_22, 0);
    lv_obj_set_pos(bk_ui->settingmoderecord_RecordStartMonth1, 651, 249);
    lv_obj_set_size(bk_ui->settingmoderecord_RecordStartMonth1, 40, 30);
    lv_obj_set_style_text_align(bk_ui->settingmoderecord_RecordStartMonth1, LV_TEXT_ALIGN_CENTER, 0);

    // TextView: RecordStartDay1
    bk_ui->settingmoderecord_RecordStartDay1 = lv_label_create(bk_ui->settingmoderecord);
    lv_label_set_text(bk_ui->settingmoderecord_RecordStartDay1, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmoderecord_RecordStartDay1, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmoderecord_RecordStartDay1, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmoderecord_RecordStartDay1, &lv_font_scdream_regular_22, 0);
    lv_obj_set_pos(bk_ui->settingmoderecord_RecordStartDay1, 695, 249);
    lv_obj_set_size(bk_ui->settingmoderecord_RecordStartDay1, 40, 30);
    lv_obj_set_style_text_align(bk_ui->settingmoderecord_RecordStartDay1, LV_TEXT_ALIGN_CENTER, 0);

    // TextView: RecordStartHour1
    bk_ui->settingmoderecord_RecordStartHour1 = lv_label_create(bk_ui->settingmoderecord);
    lv_label_set_text(bk_ui->settingmoderecord_RecordStartHour1, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmoderecord_RecordStartHour1, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmoderecord_RecordStartHour1, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmoderecord_RecordStartHour1, &lv_font_scdream_regular_22, 0);
    lv_obj_set_pos(bk_ui->settingmoderecord_RecordStartHour1, 735, 249);
    lv_obj_set_size(bk_ui->settingmoderecord_RecordStartHour1, 40, 30);
    lv_obj_set_style_text_align(bk_ui->settingmoderecord_RecordStartHour1, LV_TEXT_ALIGN_CENTER, 0);

    // TextView: RecordStartMin1
    bk_ui->settingmoderecord_RecordStartMin1 = lv_label_create(bk_ui->settingmoderecord);
    lv_label_set_text(bk_ui->settingmoderecord_RecordStartMin1, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmoderecord_RecordStartMin1, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmoderecord_RecordStartMin1, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmoderecord_RecordStartMin1, &lv_font_scdream_regular_22, 0);
    lv_obj_set_pos(bk_ui->settingmoderecord_RecordStartMin1, 781, 249);
    lv_obj_set_size(bk_ui->settingmoderecord_RecordStartMin1, 40, 30);
    lv_obj_set_style_text_align(bk_ui->settingmoderecord_RecordStartMin1, LV_TEXT_ALIGN_CENTER, 0);

    // TextView: RecordEndMonth1
    bk_ui->settingmoderecord_RecordEndMonth1 = lv_label_create(bk_ui->settingmoderecord);
    lv_label_set_text(bk_ui->settingmoderecord_RecordEndMonth1, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmoderecord_RecordEndMonth1, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmoderecord_RecordEndMonth1, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmoderecord_RecordEndMonth1, &lv_font_scdream_regular_22, 0);
    lv_obj_set_pos(bk_ui->settingmoderecord_RecordEndMonth1, 831, 249);
    lv_obj_set_size(bk_ui->settingmoderecord_RecordEndMonth1, 40, 30);
    lv_obj_set_style_text_align(bk_ui->settingmoderecord_RecordEndMonth1, LV_TEXT_ALIGN_CENTER, 0);

    // TextView: RecordEndDay1
    bk_ui->settingmoderecord_RecordEndDay1 = lv_label_create(bk_ui->settingmoderecord);
    lv_label_set_text(bk_ui->settingmoderecord_RecordEndDay1, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmoderecord_RecordEndDay1, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmoderecord_RecordEndDay1, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmoderecord_RecordEndDay1, &lv_font_scdream_regular_22, 0);
    lv_obj_set_pos(bk_ui->settingmoderecord_RecordEndDay1, 875, 249);
    lv_obj_set_size(bk_ui->settingmoderecord_RecordEndDay1, 40, 30);
    lv_obj_set_style_text_align(bk_ui->settingmoderecord_RecordEndDay1, LV_TEXT_ALIGN_CENTER, 0);

    // TextView: RecordEndHour1
    bk_ui->settingmoderecord_RecordEndHour1 = lv_label_create(bk_ui->settingmoderecord);
    lv_label_set_text(bk_ui->settingmoderecord_RecordEndHour1, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmoderecord_RecordEndHour1, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmoderecord_RecordEndHour1, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmoderecord_RecordEndHour1, &lv_font_scdream_regular_22, 0);
    lv_obj_set_pos(bk_ui->settingmoderecord_RecordEndHour1, 915, 249);
    lv_obj_set_size(bk_ui->settingmoderecord_RecordEndHour1, 40, 30);
    lv_obj_set_style_text_align(bk_ui->settingmoderecord_RecordEndHour1, LV_TEXT_ALIGN_CENTER, 0);

    // TextView: RecordEndMin1
    bk_ui->settingmoderecord_RecordEndMin1 = lv_label_create(bk_ui->settingmoderecord);
    lv_label_set_text(bk_ui->settingmoderecord_RecordEndMin1, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmoderecord_RecordEndMin1, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmoderecord_RecordEndMin1, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmoderecord_RecordEndMin1, &lv_font_scdream_regular_22, 0);
    lv_obj_set_pos(bk_ui->settingmoderecord_RecordEndMin1, 961, 249);
    lv_obj_set_size(bk_ui->settingmoderecord_RecordEndMin1, 40, 30);
    lv_obj_set_style_text_align(bk_ui->settingmoderecord_RecordEndMin1, LV_TEXT_ALIGN_CENTER, 0);

    // TextView: label58
    bk_ui->settingmoderecord_imageview58 = lv_label_create(bk_ui->settingmoderecord);
    lv_label_set_text(bk_ui->settingmoderecord_imageview58, "3");
    lv_obj_set_style_bg_opa(bk_ui->settingmoderecord_imageview58, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmoderecord_imageview58, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmoderecord_imageview58, &lv_font_scdream_regular_22, 0);
    lv_obj_set_pos(bk_ui->settingmoderecord_imageview58, 42, 321);
    lv_obj_set_size(bk_ui->settingmoderecord_imageview58, 30, 30);

    // TextView: RecordFreezeTemp2
    bk_ui->settingmoderecord_RecordFreezeTemp2 = lv_label_create(bk_ui->settingmoderecord);
    lv_label_set_text(bk_ui->settingmoderecord_RecordFreezeTemp2, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmoderecord_RecordFreezeTemp2, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmoderecord_RecordFreezeTemp2, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmoderecord_RecordFreezeTemp2, &lv_font_scdream_regular_22, 0);
    lv_obj_set_pos(bk_ui->settingmoderecord_RecordFreezeTemp2, 103, 307);
    lv_obj_set_size(bk_ui->settingmoderecord_RecordFreezeTemp2, 60, 30);
    lv_obj_set_style_text_align(bk_ui->settingmoderecord_RecordFreezeTemp2, LV_TEXT_ALIGN_CENTER, 0);

    // TextView: RecordFreezeTimeHour2
    bk_ui->settingmoderecord_RecordFreezeTimeHour2 = lv_label_create(bk_ui->settingmoderecord);
    lv_label_set_text(bk_ui->settingmoderecord_RecordFreezeTimeHour2, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmoderecord_RecordFreezeTimeHour2, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmoderecord_RecordFreezeTimeHour2, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmoderecord_RecordFreezeTimeHour2, &lv_font_scdream_regular_22, 0);
    lv_obj_set_pos(bk_ui->settingmoderecord_RecordFreezeTimeHour2, 95, 337);
    lv_obj_set_size(bk_ui->settingmoderecord_RecordFreezeTimeHour2, 40, 30);
    lv_obj_set_style_text_align(bk_ui->settingmoderecord_RecordFreezeTimeHour2, LV_TEXT_ALIGN_CENTER, 0);

    // TextView: RecordFreezeTimeMin2
    bk_ui->settingmoderecord_RecordFreezeTimeMin2 = lv_label_create(bk_ui->settingmoderecord);
    lv_label_set_text(bk_ui->settingmoderecord_RecordFreezeTimeMin2, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmoderecord_RecordFreezeTimeMin2, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmoderecord_RecordFreezeTimeMin2, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmoderecord_RecordFreezeTimeMin2, &lv_font_scdream_regular_22, 0);
    lv_obj_set_pos(bk_ui->settingmoderecord_RecordFreezeTimeMin2, 156, 337);
    lv_obj_set_size(bk_ui->settingmoderecord_RecordFreezeTimeMin2, 40, 30);
    lv_obj_set_style_text_align(bk_ui->settingmoderecord_RecordFreezeTimeMin2, LV_TEXT_ALIGN_CENTER, 0);

    // TextView: RecordDefreezeTemp2
    bk_ui->settingmoderecord_RecordDefreezeTemp2 = lv_label_create(bk_ui->settingmoderecord);
    lv_label_set_text(bk_ui->settingmoderecord_RecordDefreezeTemp2, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmoderecord_RecordDefreezeTemp2, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmoderecord_RecordDefreezeTemp2, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmoderecord_RecordDefreezeTemp2, &lv_font_scdream_regular_22, 0);
    lv_obj_set_pos(bk_ui->settingmoderecord_RecordDefreezeTemp2, 248, 307);
    lv_obj_set_size(bk_ui->settingmoderecord_RecordDefreezeTemp2, 60, 30);
    lv_obj_set_style_text_align(bk_ui->settingmoderecord_RecordDefreezeTemp2, LV_TEXT_ALIGN_CENTER, 0);

    // TextView: RecordDefreezeTimeHour2
    bk_ui->settingmoderecord_RecordDefreezeTimeHour2 = lv_label_create(bk_ui->settingmoderecord);
    lv_label_set_text(bk_ui->settingmoderecord_RecordDefreezeTimeHour2, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmoderecord_RecordDefreezeTimeHour2, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmoderecord_RecordDefreezeTimeHour2, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmoderecord_RecordDefreezeTimeHour2, &lv_font_scdream_regular_22, 0);
    lv_obj_set_pos(bk_ui->settingmoderecord_RecordDefreezeTimeHour2, 235, 337);
    lv_obj_set_size(bk_ui->settingmoderecord_RecordDefreezeTimeHour2, 40, 30);
    lv_obj_set_style_text_align(bk_ui->settingmoderecord_RecordDefreezeTimeHour2, LV_TEXT_ALIGN_CENTER, 0);

    // TextView: RecordDefreezeTimeMin2
    bk_ui->settingmoderecord_RecordDefreezeTimeMin2 = lv_label_create(bk_ui->settingmoderecord);
    lv_label_set_text(bk_ui->settingmoderecord_RecordDefreezeTimeMin2, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmoderecord_RecordDefreezeTimeMin2, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmoderecord_RecordDefreezeTimeMin2, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmoderecord_RecordDefreezeTimeMin2, &lv_font_scdream_regular_22, 0);
    lv_obj_set_pos(bk_ui->settingmoderecord_RecordDefreezeTimeMin2, 296, 337);
    lv_obj_set_size(bk_ui->settingmoderecord_RecordDefreezeTimeMin2, 40, 30);
    lv_obj_set_style_text_align(bk_ui->settingmoderecord_RecordDefreezeTimeMin2, LV_TEXT_ALIGN_CENTER, 0);

    // TextView: RecordFermentation1Temp2
    bk_ui->settingmoderecord_RecordFermentation1Temp2 = lv_label_create(bk_ui->settingmoderecord);
    lv_label_set_text(bk_ui->settingmoderecord_RecordFermentation1Temp2, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmoderecord_RecordFermentation1Temp2, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmoderecord_RecordFermentation1Temp2, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmoderecord_RecordFermentation1Temp2, &lv_font_scdream_regular_22, 0);
    lv_obj_set_pos(bk_ui->settingmoderecord_RecordFermentation1Temp2, 376, 307);
    lv_obj_set_size(bk_ui->settingmoderecord_RecordFermentation1Temp2, 50, 30);
    lv_obj_set_style_text_align(bk_ui->settingmoderecord_RecordFermentation1Temp2, LV_TEXT_ALIGN_CENTER, 0);

    // TextView: RecordFermentation1Humidity2
    bk_ui->settingmoderecord_RecordFermentation1Humidity2 = lv_label_create(bk_ui->settingmoderecord);
    lv_label_set_text(bk_ui->settingmoderecord_RecordFermentation1Humidity2, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmoderecord_RecordFermentation1Humidity2, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmoderecord_RecordFermentation1Humidity2, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmoderecord_RecordFermentation1Humidity2, &lv_font_scdream_regular_22, 0);
    lv_obj_set_pos(bk_ui->settingmoderecord_RecordFermentation1Humidity2, 435, 307);
    lv_obj_set_size(bk_ui->settingmoderecord_RecordFermentation1Humidity2, 50, 30);
    lv_obj_set_style_text_align(bk_ui->settingmoderecord_RecordFermentation1Humidity2, LV_TEXT_ALIGN_CENTER, 0);

    // TextView: RecordFermentation1TimeHour2
    bk_ui->settingmoderecord_RecordFermentation1TimeHour2 = lv_label_create(bk_ui->settingmoderecord);
    lv_label_set_text(bk_ui->settingmoderecord_RecordFermentation1TimeHour2, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmoderecord_RecordFermentation1TimeHour2, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmoderecord_RecordFermentation1TimeHour2, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmoderecord_RecordFermentation1TimeHour2, &lv_font_scdream_regular_22, 0);
    lv_obj_set_pos(bk_ui->settingmoderecord_RecordFermentation1TimeHour2, 381, 337);
    lv_obj_set_size(bk_ui->settingmoderecord_RecordFermentation1TimeHour2, 40, 30);
    lv_obj_set_style_text_align(bk_ui->settingmoderecord_RecordFermentation1TimeHour2, LV_TEXT_ALIGN_CENTER, 0);

    // TextView: RecordFermentation1TimeMin2
    bk_ui->settingmoderecord_RecordFermentation1TimeMin2 = lv_label_create(bk_ui->settingmoderecord);
    lv_label_set_text(bk_ui->settingmoderecord_RecordFermentation1TimeMin2, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmoderecord_RecordFermentation1TimeMin2, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmoderecord_RecordFermentation1TimeMin2, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmoderecord_RecordFermentation1TimeMin2, &lv_font_scdream_regular_22, 0);
    lv_obj_set_pos(bk_ui->settingmoderecord_RecordFermentation1TimeMin2, 440, 337);
    lv_obj_set_size(bk_ui->settingmoderecord_RecordFermentation1TimeMin2, 40, 30);
    lv_obj_set_style_text_align(bk_ui->settingmoderecord_RecordFermentation1TimeMin2, LV_TEXT_ALIGN_CENTER, 0);

    // TextView: RecordFermentation2Temp2
    bk_ui->settingmoderecord_RecordFermentation2Temp2 = lv_label_create(bk_ui->settingmoderecord);
    lv_label_set_text(bk_ui->settingmoderecord_RecordFermentation2Temp2, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmoderecord_RecordFermentation2Temp2, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmoderecord_RecordFermentation2Temp2, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmoderecord_RecordFermentation2Temp2, &lv_font_scdream_regular_22, 0);
    lv_obj_set_pos(bk_ui->settingmoderecord_RecordFermentation2Temp2, 518, 307);
    lv_obj_set_size(bk_ui->settingmoderecord_RecordFermentation2Temp2, 50, 30);
    lv_obj_set_style_text_align(bk_ui->settingmoderecord_RecordFermentation2Temp2, LV_TEXT_ALIGN_CENTER, 0);

    // TextView: RecordFermentation2Humidity2
    bk_ui->settingmoderecord_RecordFermentation2Humidity2 = lv_label_create(bk_ui->settingmoderecord);
    lv_label_set_text(bk_ui->settingmoderecord_RecordFermentation2Humidity2, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmoderecord_RecordFermentation2Humidity2, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmoderecord_RecordFermentation2Humidity2, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmoderecord_RecordFermentation2Humidity2, &lv_font_scdream_regular_22, 0);
    lv_obj_set_pos(bk_ui->settingmoderecord_RecordFermentation2Humidity2, 577, 307);
    lv_obj_set_size(bk_ui->settingmoderecord_RecordFermentation2Humidity2, 50, 30);
    lv_obj_set_style_text_align(bk_ui->settingmoderecord_RecordFermentation2Humidity2, LV_TEXT_ALIGN_CENTER, 0);

    // TextView: RecordFermentation2TimeHour2
    bk_ui->settingmoderecord_RecordFermentation2TimeHour2 = lv_label_create(bk_ui->settingmoderecord);
    lv_label_set_text(bk_ui->settingmoderecord_RecordFermentation2TimeHour2, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmoderecord_RecordFermentation2TimeHour2, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmoderecord_RecordFermentation2TimeHour2, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmoderecord_RecordFermentation2TimeHour2, &lv_font_scdream_regular_22, 0);
    lv_obj_set_pos(bk_ui->settingmoderecord_RecordFermentation2TimeHour2, 523, 337);
    lv_obj_set_size(bk_ui->settingmoderecord_RecordFermentation2TimeHour2, 40, 30);
    lv_obj_set_style_text_align(bk_ui->settingmoderecord_RecordFermentation2TimeHour2, LV_TEXT_ALIGN_CENTER, 0);

    // TextView: RecordFermentation2TimeMin2
    bk_ui->settingmoderecord_RecordFermentation2TimeMin2 = lv_label_create(bk_ui->settingmoderecord);
    lv_label_set_text(bk_ui->settingmoderecord_RecordFermentation2TimeMin2, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmoderecord_RecordFermentation2TimeMin2, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmoderecord_RecordFermentation2TimeMin2, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmoderecord_RecordFermentation2TimeMin2, &lv_font_scdream_regular_22, 0);
    lv_obj_set_pos(bk_ui->settingmoderecord_RecordFermentation2TimeMin2, 582, 337);
    lv_obj_set_size(bk_ui->settingmoderecord_RecordFermentation2TimeMin2, 40, 30);
    lv_obj_set_style_text_align(bk_ui->settingmoderecord_RecordFermentation2TimeMin2, LV_TEXT_ALIGN_CENTER, 0);

    // TextView: RecordStartMonth2
    bk_ui->settingmoderecord_RecordStartMonth2 = lv_label_create(bk_ui->settingmoderecord);
    lv_label_set_text(bk_ui->settingmoderecord_RecordStartMonth2, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmoderecord_RecordStartMonth2, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmoderecord_RecordStartMonth2, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmoderecord_RecordStartMonth2, &lv_font_scdream_regular_22, 0);
    lv_obj_set_pos(bk_ui->settingmoderecord_RecordStartMonth2, 651, 319);
    lv_obj_set_size(bk_ui->settingmoderecord_RecordStartMonth2, 40, 30);
    lv_obj_set_style_text_align(bk_ui->settingmoderecord_RecordStartMonth2, LV_TEXT_ALIGN_CENTER, 0);

    // TextView: RecordStartDay2
    bk_ui->settingmoderecord_RecordStartDay2 = lv_label_create(bk_ui->settingmoderecord);
    lv_label_set_text(bk_ui->settingmoderecord_RecordStartDay2, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmoderecord_RecordStartDay2, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmoderecord_RecordStartDay2, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmoderecord_RecordStartDay2, &lv_font_scdream_regular_22, 0);
    lv_obj_set_pos(bk_ui->settingmoderecord_RecordStartDay2, 695, 319);
    lv_obj_set_size(bk_ui->settingmoderecord_RecordStartDay2, 40, 30);
    lv_obj_set_style_text_align(bk_ui->settingmoderecord_RecordStartDay2, LV_TEXT_ALIGN_CENTER, 0);

    // TextView: RecordStartHour2
    bk_ui->settingmoderecord_RecordStartHour2 = lv_label_create(bk_ui->settingmoderecord);
    lv_label_set_text(bk_ui->settingmoderecord_RecordStartHour2, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmoderecord_RecordStartHour2, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmoderecord_RecordStartHour2, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmoderecord_RecordStartHour2, &lv_font_scdream_regular_22, 0);
    lv_obj_set_pos(bk_ui->settingmoderecord_RecordStartHour2, 735, 319);
    lv_obj_set_size(bk_ui->settingmoderecord_RecordStartHour2, 40, 30);
    lv_obj_set_style_text_align(bk_ui->settingmoderecord_RecordStartHour2, LV_TEXT_ALIGN_CENTER, 0);

    // TextView: RecordStartMin2
    bk_ui->settingmoderecord_RecordStartMin2 = lv_label_create(bk_ui->settingmoderecord);
    lv_label_set_text(bk_ui->settingmoderecord_RecordStartMin2, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmoderecord_RecordStartMin2, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmoderecord_RecordStartMin2, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmoderecord_RecordStartMin2, &lv_font_scdream_regular_22, 0);
    lv_obj_set_pos(bk_ui->settingmoderecord_RecordStartMin2, 781, 319);
    lv_obj_set_size(bk_ui->settingmoderecord_RecordStartMin2, 40, 30);
    lv_obj_set_style_text_align(bk_ui->settingmoderecord_RecordStartMin2, LV_TEXT_ALIGN_CENTER, 0);

    // TextView: RecordEndMonth2
    bk_ui->settingmoderecord_RecordEndMonth2 = lv_label_create(bk_ui->settingmoderecord);
    lv_label_set_text(bk_ui->settingmoderecord_RecordEndMonth2, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmoderecord_RecordEndMonth2, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmoderecord_RecordEndMonth2, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmoderecord_RecordEndMonth2, &lv_font_scdream_regular_22, 0);
    lv_obj_set_pos(bk_ui->settingmoderecord_RecordEndMonth2, 831, 319);
    lv_obj_set_size(bk_ui->settingmoderecord_RecordEndMonth2, 40, 30);
    lv_obj_set_style_text_align(bk_ui->settingmoderecord_RecordEndMonth2, LV_TEXT_ALIGN_CENTER, 0);

    // TextView: RecordEndDay2
    bk_ui->settingmoderecord_RecordEndDay2 = lv_label_create(bk_ui->settingmoderecord);
    lv_label_set_text(bk_ui->settingmoderecord_RecordEndDay2, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmoderecord_RecordEndDay2, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmoderecord_RecordEndDay2, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmoderecord_RecordEndDay2, &lv_font_scdream_regular_22, 0);
    lv_obj_set_pos(bk_ui->settingmoderecord_RecordEndDay2, 875, 319);
    lv_obj_set_size(bk_ui->settingmoderecord_RecordEndDay2, 40, 30);
    lv_obj_set_style_text_align(bk_ui->settingmoderecord_RecordEndDay2, LV_TEXT_ALIGN_CENTER, 0);

    // TextView: RecordEndHour2
    bk_ui->settingmoderecord_RecordEndHour2 = lv_label_create(bk_ui->settingmoderecord);
    lv_label_set_text(bk_ui->settingmoderecord_RecordEndHour2, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmoderecord_RecordEndHour2, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmoderecord_RecordEndHour2, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmoderecord_RecordEndHour2, &lv_font_scdream_regular_22, 0);
    lv_obj_set_pos(bk_ui->settingmoderecord_RecordEndHour2, 915, 319);
    lv_obj_set_size(bk_ui->settingmoderecord_RecordEndHour2, 40, 30);
    lv_obj_set_style_text_align(bk_ui->settingmoderecord_RecordEndHour2, LV_TEXT_ALIGN_CENTER, 0);

    // TextView: RecordEndMin2
    bk_ui->settingmoderecord_RecordEndMin2 = lv_label_create(bk_ui->settingmoderecord);
    lv_label_set_text(bk_ui->settingmoderecord_RecordEndMin2, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmoderecord_RecordEndMin2, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmoderecord_RecordEndMin2, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmoderecord_RecordEndMin2, &lv_font_scdream_regular_22, 0);
    lv_obj_set_pos(bk_ui->settingmoderecord_RecordEndMin2, 961, 319);
    lv_obj_set_size(bk_ui->settingmoderecord_RecordEndMin2, 40, 30);
    lv_obj_set_style_text_align(bk_ui->settingmoderecord_RecordEndMin2, LV_TEXT_ALIGN_CENTER, 0);

    // TextView: label81
    bk_ui->settingmoderecord_imageview81 = lv_label_create(bk_ui->settingmoderecord);
    lv_label_set_text(bk_ui->settingmoderecord_imageview81, "4");
    lv_obj_set_style_bg_opa(bk_ui->settingmoderecord_imageview81, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmoderecord_imageview81, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmoderecord_imageview81, &lv_font_scdream_regular_22, 0);
    lv_obj_set_pos(bk_ui->settingmoderecord_imageview81, 42, 391);
    lv_obj_set_size(bk_ui->settingmoderecord_imageview81, 30, 30);

    // TextView: RecordFreezeTemp3
    bk_ui->settingmoderecord_RecordFreezeTemp3 = lv_label_create(bk_ui->settingmoderecord);
    lv_label_set_text(bk_ui->settingmoderecord_RecordFreezeTemp3, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmoderecord_RecordFreezeTemp3, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmoderecord_RecordFreezeTemp3, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmoderecord_RecordFreezeTemp3, &lv_font_scdream_regular_22, 0);
    lv_obj_set_pos(bk_ui->settingmoderecord_RecordFreezeTemp3, 103, 377);
    lv_obj_set_size(bk_ui->settingmoderecord_RecordFreezeTemp3, 60, 30);
    lv_obj_set_style_text_align(bk_ui->settingmoderecord_RecordFreezeTemp3, LV_TEXT_ALIGN_CENTER, 0);

    // TextView: RecordFreezeTimeHour3
    bk_ui->settingmoderecord_RecordFreezeTimeHour3 = lv_label_create(bk_ui->settingmoderecord);
    lv_label_set_text(bk_ui->settingmoderecord_RecordFreezeTimeHour3, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmoderecord_RecordFreezeTimeHour3, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmoderecord_RecordFreezeTimeHour3, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmoderecord_RecordFreezeTimeHour3, &lv_font_scdream_regular_22, 0);
    lv_obj_set_pos(bk_ui->settingmoderecord_RecordFreezeTimeHour3, 95, 407);
    lv_obj_set_size(bk_ui->settingmoderecord_RecordFreezeTimeHour3, 40, 30);
    lv_obj_set_style_text_align(bk_ui->settingmoderecord_RecordFreezeTimeHour3, LV_TEXT_ALIGN_CENTER, 0);

    // TextView: RecordFreezeTimeMin3
    bk_ui->settingmoderecord_RecordFreezeTimeMin3 = lv_label_create(bk_ui->settingmoderecord);
    lv_label_set_text(bk_ui->settingmoderecord_RecordFreezeTimeMin3, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmoderecord_RecordFreezeTimeMin3, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmoderecord_RecordFreezeTimeMin3, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmoderecord_RecordFreezeTimeMin3, &lv_font_scdream_regular_22, 0);
    lv_obj_set_pos(bk_ui->settingmoderecord_RecordFreezeTimeMin3, 156, 407);
    lv_obj_set_size(bk_ui->settingmoderecord_RecordFreezeTimeMin3, 40, 30);
    lv_obj_set_style_text_align(bk_ui->settingmoderecord_RecordFreezeTimeMin3, LV_TEXT_ALIGN_CENTER, 0);

    // TextView: RecordDefreezeTemp3
    bk_ui->settingmoderecord_RecordDefreezeTemp3 = lv_label_create(bk_ui->settingmoderecord);
    lv_label_set_text(bk_ui->settingmoderecord_RecordDefreezeTemp3, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmoderecord_RecordDefreezeTemp3, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmoderecord_RecordDefreezeTemp3, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmoderecord_RecordDefreezeTemp3, &lv_font_scdream_regular_22, 0);
    lv_obj_set_pos(bk_ui->settingmoderecord_RecordDefreezeTemp3, 248, 377);
    lv_obj_set_size(bk_ui->settingmoderecord_RecordDefreezeTemp3, 60, 30);
    lv_obj_set_style_text_align(bk_ui->settingmoderecord_RecordDefreezeTemp3, LV_TEXT_ALIGN_CENTER, 0);

    // TextView: RecordDefreezeTimeHour3
    bk_ui->settingmoderecord_RecordDefreezeTimeHour3 = lv_label_create(bk_ui->settingmoderecord);
    lv_label_set_text(bk_ui->settingmoderecord_RecordDefreezeTimeHour3, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmoderecord_RecordDefreezeTimeHour3, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmoderecord_RecordDefreezeTimeHour3, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmoderecord_RecordDefreezeTimeHour3, &lv_font_scdream_regular_22, 0);
    lv_obj_set_pos(bk_ui->settingmoderecord_RecordDefreezeTimeHour3, 235, 407);
    lv_obj_set_size(bk_ui->settingmoderecord_RecordDefreezeTimeHour3, 40, 30);
    lv_obj_set_style_text_align(bk_ui->settingmoderecord_RecordDefreezeTimeHour3, LV_TEXT_ALIGN_CENTER, 0);

    // TextView: RecordDefreezeTimeMin3
    bk_ui->settingmoderecord_RecordDefreezeTimeMin3 = lv_label_create(bk_ui->settingmoderecord);
    lv_label_set_text(bk_ui->settingmoderecord_RecordDefreezeTimeMin3, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmoderecord_RecordDefreezeTimeMin3, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmoderecord_RecordDefreezeTimeMin3, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmoderecord_RecordDefreezeTimeMin3, &lv_font_scdream_regular_22, 0);
    lv_obj_set_pos(bk_ui->settingmoderecord_RecordDefreezeTimeMin3, 296, 407);
    lv_obj_set_size(bk_ui->settingmoderecord_RecordDefreezeTimeMin3, 40, 30);
    lv_obj_set_style_text_align(bk_ui->settingmoderecord_RecordDefreezeTimeMin3, LV_TEXT_ALIGN_CENTER, 0);

    // TextView: RecordFermentation1Temp3
    bk_ui->settingmoderecord_RecordFermentation1Temp3 = lv_label_create(bk_ui->settingmoderecord);
    lv_label_set_text(bk_ui->settingmoderecord_RecordFermentation1Temp3, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmoderecord_RecordFermentation1Temp3, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmoderecord_RecordFermentation1Temp3, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmoderecord_RecordFermentation1Temp3, &lv_font_scdream_regular_22, 0);
    lv_obj_set_pos(bk_ui->settingmoderecord_RecordFermentation1Temp3, 376, 377);
    lv_obj_set_size(bk_ui->settingmoderecord_RecordFermentation1Temp3, 50, 30);
    lv_obj_set_style_text_align(bk_ui->settingmoderecord_RecordFermentation1Temp3, LV_TEXT_ALIGN_CENTER, 0);

    // TextView: RecordFermentation1Humidity3
    bk_ui->settingmoderecord_RecordFermentation1Humidity3 = lv_label_create(bk_ui->settingmoderecord);
    lv_label_set_text(bk_ui->settingmoderecord_RecordFermentation1Humidity3, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmoderecord_RecordFermentation1Humidity3, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmoderecord_RecordFermentation1Humidity3, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmoderecord_RecordFermentation1Humidity3, &lv_font_scdream_regular_22, 0);
    lv_obj_set_pos(bk_ui->settingmoderecord_RecordFermentation1Humidity3, 435, 377);
    lv_obj_set_size(bk_ui->settingmoderecord_RecordFermentation1Humidity3, 50, 30);
    lv_obj_set_style_text_align(bk_ui->settingmoderecord_RecordFermentation1Humidity3, LV_TEXT_ALIGN_CENTER, 0);

    // TextView: RecordFermentation1TimeHour3
    bk_ui->settingmoderecord_RecordFermentation1TimeHour3 = lv_label_create(bk_ui->settingmoderecord);
    lv_label_set_text(bk_ui->settingmoderecord_RecordFermentation1TimeHour3, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmoderecord_RecordFermentation1TimeHour3, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmoderecord_RecordFermentation1TimeHour3, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmoderecord_RecordFermentation1TimeHour3, &lv_font_scdream_regular_22, 0);
    lv_obj_set_pos(bk_ui->settingmoderecord_RecordFermentation1TimeHour3, 381, 407);
    lv_obj_set_size(bk_ui->settingmoderecord_RecordFermentation1TimeHour3, 40, 30);
    lv_obj_set_style_text_align(bk_ui->settingmoderecord_RecordFermentation1TimeHour3, LV_TEXT_ALIGN_CENTER, 0);

    // TextView: RecordFermentation1TimeMin3
    bk_ui->settingmoderecord_RecordFermentation1TimeMin3 = lv_label_create(bk_ui->settingmoderecord);
    lv_label_set_text(bk_ui->settingmoderecord_RecordFermentation1TimeMin3, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmoderecord_RecordFermentation1TimeMin3, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmoderecord_RecordFermentation1TimeMin3, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmoderecord_RecordFermentation1TimeMin3, &lv_font_scdream_regular_22, 0);
    lv_obj_set_pos(bk_ui->settingmoderecord_RecordFermentation1TimeMin3, 440, 407);
    lv_obj_set_size(bk_ui->settingmoderecord_RecordFermentation1TimeMin3, 40, 30);
    lv_obj_set_style_text_align(bk_ui->settingmoderecord_RecordFermentation1TimeMin3, LV_TEXT_ALIGN_CENTER, 0);

    // TextView: RecordFermentation2Temp3
    bk_ui->settingmoderecord_RecordFermentation2Temp3 = lv_label_create(bk_ui->settingmoderecord);
    lv_label_set_text(bk_ui->settingmoderecord_RecordFermentation2Temp3, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmoderecord_RecordFermentation2Temp3, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmoderecord_RecordFermentation2Temp3, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmoderecord_RecordFermentation2Temp3, &lv_font_scdream_regular_22, 0);
    lv_obj_set_pos(bk_ui->settingmoderecord_RecordFermentation2Temp3, 518, 377);
    lv_obj_set_size(bk_ui->settingmoderecord_RecordFermentation2Temp3, 50, 30);
    lv_obj_set_style_text_align(bk_ui->settingmoderecord_RecordFermentation2Temp3, LV_TEXT_ALIGN_CENTER, 0);

    // TextView: RecordFermentation2Humidity3
    bk_ui->settingmoderecord_RecordFermentation2Humidity3 = lv_label_create(bk_ui->settingmoderecord);
    lv_label_set_text(bk_ui->settingmoderecord_RecordFermentation2Humidity3, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmoderecord_RecordFermentation2Humidity3, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmoderecord_RecordFermentation2Humidity3, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmoderecord_RecordFermentation2Humidity3, &lv_font_scdream_regular_22, 0);
    lv_obj_set_pos(bk_ui->settingmoderecord_RecordFermentation2Humidity3, 577, 377);
    lv_obj_set_size(bk_ui->settingmoderecord_RecordFermentation2Humidity3, 50, 30);
    lv_obj_set_style_text_align(bk_ui->settingmoderecord_RecordFermentation2Humidity3, LV_TEXT_ALIGN_CENTER, 0);

    // TextView: RecordFermentation2TimeHour3
    bk_ui->settingmoderecord_RecordFermentation2TimeHour3 = lv_label_create(bk_ui->settingmoderecord);
    lv_label_set_text(bk_ui->settingmoderecord_RecordFermentation2TimeHour3, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmoderecord_RecordFermentation2TimeHour3, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmoderecord_RecordFermentation2TimeHour3, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmoderecord_RecordFermentation2TimeHour3, &lv_font_scdream_regular_22, 0);
    lv_obj_set_pos(bk_ui->settingmoderecord_RecordFermentation2TimeHour3, 523, 407);
    lv_obj_set_size(bk_ui->settingmoderecord_RecordFermentation2TimeHour3, 40, 30);
    lv_obj_set_style_text_align(bk_ui->settingmoderecord_RecordFermentation2TimeHour3, LV_TEXT_ALIGN_CENTER, 0);

    // TextView: RecordFermentation2TimeMin3
    bk_ui->settingmoderecord_RecordFermentation2TimeMin3 = lv_label_create(bk_ui->settingmoderecord);
    lv_label_set_text(bk_ui->settingmoderecord_RecordFermentation2TimeMin3, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmoderecord_RecordFermentation2TimeMin3, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmoderecord_RecordFermentation2TimeMin3, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmoderecord_RecordFermentation2TimeMin3, &lv_font_scdream_regular_22, 0);
    lv_obj_set_pos(bk_ui->settingmoderecord_RecordFermentation2TimeMin3, 582, 407);
    lv_obj_set_size(bk_ui->settingmoderecord_RecordFermentation2TimeMin3, 40, 30);
    lv_obj_set_style_text_align(bk_ui->settingmoderecord_RecordFermentation2TimeMin3, LV_TEXT_ALIGN_CENTER, 0);

    // TextView: RecordStartMonth3
    bk_ui->settingmoderecord_RecordStartMonth3 = lv_label_create(bk_ui->settingmoderecord);
    lv_label_set_text(bk_ui->settingmoderecord_RecordStartMonth3, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmoderecord_RecordStartMonth3, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmoderecord_RecordStartMonth3, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmoderecord_RecordStartMonth3, &lv_font_scdream_regular_22, 0);
    lv_obj_set_pos(bk_ui->settingmoderecord_RecordStartMonth3, 651, 389);
    lv_obj_set_size(bk_ui->settingmoderecord_RecordStartMonth3, 40, 30);
    lv_obj_set_style_text_align(bk_ui->settingmoderecord_RecordStartMonth3, LV_TEXT_ALIGN_CENTER, 0);

    // TextView: RecordStartDay3
    bk_ui->settingmoderecord_RecordStartDay3 = lv_label_create(bk_ui->settingmoderecord);
    lv_label_set_text(bk_ui->settingmoderecord_RecordStartDay3, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmoderecord_RecordStartDay3, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmoderecord_RecordStartDay3, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmoderecord_RecordStartDay3, &lv_font_scdream_regular_22, 0);
    lv_obj_set_pos(bk_ui->settingmoderecord_RecordStartDay3, 695, 389);
    lv_obj_set_size(bk_ui->settingmoderecord_RecordStartDay3, 40, 30);
    lv_obj_set_style_text_align(bk_ui->settingmoderecord_RecordStartDay3, LV_TEXT_ALIGN_CENTER, 0);

    // TextView: RecordStartHour3
    bk_ui->settingmoderecord_RecordStartHour3 = lv_label_create(bk_ui->settingmoderecord);
    lv_label_set_text(bk_ui->settingmoderecord_RecordStartHour3, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmoderecord_RecordStartHour3, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmoderecord_RecordStartHour3, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmoderecord_RecordStartHour3, &lv_font_scdream_regular_22, 0);
    lv_obj_set_pos(bk_ui->settingmoderecord_RecordStartHour3, 735, 389);
    lv_obj_set_size(bk_ui->settingmoderecord_RecordStartHour3, 40, 30);
    lv_obj_set_style_text_align(bk_ui->settingmoderecord_RecordStartHour3, LV_TEXT_ALIGN_CENTER, 0);

    // TextView: RecordStartMin3
    bk_ui->settingmoderecord_RecordStartMin3 = lv_label_create(bk_ui->settingmoderecord);
    lv_label_set_text(bk_ui->settingmoderecord_RecordStartMin3, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmoderecord_RecordStartMin3, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmoderecord_RecordStartMin3, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmoderecord_RecordStartMin3, &lv_font_scdream_regular_22, 0);
    lv_obj_set_pos(bk_ui->settingmoderecord_RecordStartMin3, 781, 389);
    lv_obj_set_size(bk_ui->settingmoderecord_RecordStartMin3, 40, 30);
    lv_obj_set_style_text_align(bk_ui->settingmoderecord_RecordStartMin3, LV_TEXT_ALIGN_CENTER, 0);

    // TextView: RecordEndMonth3
    bk_ui->settingmoderecord_RecordEndMonth3 = lv_label_create(bk_ui->settingmoderecord);
    lv_label_set_text(bk_ui->settingmoderecord_RecordEndMonth3, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmoderecord_RecordEndMonth3, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmoderecord_RecordEndMonth3, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmoderecord_RecordEndMonth3, &lv_font_scdream_regular_22, 0);
    lv_obj_set_pos(bk_ui->settingmoderecord_RecordEndMonth3, 831, 389);
    lv_obj_set_size(bk_ui->settingmoderecord_RecordEndMonth3, 40, 30);
    lv_obj_set_style_text_align(bk_ui->settingmoderecord_RecordEndMonth3, LV_TEXT_ALIGN_CENTER, 0);

    // TextView: RecordEndDay3
    bk_ui->settingmoderecord_RecordEndDay3 = lv_label_create(bk_ui->settingmoderecord);
    lv_label_set_text(bk_ui->settingmoderecord_RecordEndDay3, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmoderecord_RecordEndDay3, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmoderecord_RecordEndDay3, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmoderecord_RecordEndDay3, &lv_font_scdream_regular_22, 0);
    lv_obj_set_pos(bk_ui->settingmoderecord_RecordEndDay3, 875, 389);
    lv_obj_set_size(bk_ui->settingmoderecord_RecordEndDay3, 40, 30);
    lv_obj_set_style_text_align(bk_ui->settingmoderecord_RecordEndDay3, LV_TEXT_ALIGN_CENTER, 0);

    // TextView: RecordEndHour3
    bk_ui->settingmoderecord_RecordEndHour3 = lv_label_create(bk_ui->settingmoderecord);
    lv_label_set_text(bk_ui->settingmoderecord_RecordEndHour3, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmoderecord_RecordEndHour3, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmoderecord_RecordEndHour3, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmoderecord_RecordEndHour3, &lv_font_scdream_regular_22, 0);
    lv_obj_set_pos(bk_ui->settingmoderecord_RecordEndHour3, 915, 389);
    lv_obj_set_size(bk_ui->settingmoderecord_RecordEndHour3, 40, 30);
    lv_obj_set_style_text_align(bk_ui->settingmoderecord_RecordEndHour3, LV_TEXT_ALIGN_CENTER, 0);

    // TextView: RecordEndMin3
    bk_ui->settingmoderecord_RecordEndMin3 = lv_label_create(bk_ui->settingmoderecord);
    lv_label_set_text(bk_ui->settingmoderecord_RecordEndMin3, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmoderecord_RecordEndMin3, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmoderecord_RecordEndMin3, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmoderecord_RecordEndMin3, &lv_font_scdream_regular_22, 0);
    lv_obj_set_pos(bk_ui->settingmoderecord_RecordEndMin3, 961, 389);
    lv_obj_set_size(bk_ui->settingmoderecord_RecordEndMin3, 40, 30);
    lv_obj_set_style_text_align(bk_ui->settingmoderecord_RecordEndMin3, LV_TEXT_ALIGN_CENTER, 0);

    // TextView: label104
    bk_ui->settingmoderecord_imageview104 = lv_label_create(bk_ui->settingmoderecord);
    lv_label_set_text(bk_ui->settingmoderecord_imageview104, "5");
    lv_obj_set_style_bg_opa(bk_ui->settingmoderecord_imageview104, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmoderecord_imageview104, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmoderecord_imageview104, &lv_font_scdream_regular_22, 0);
    lv_obj_set_pos(bk_ui->settingmoderecord_imageview104, 42, 461);
    lv_obj_set_size(bk_ui->settingmoderecord_imageview104, 30, 30);

    // TextView: RecordFreezeTemp4
    bk_ui->settingmoderecord_RecordFreezeTemp4 = lv_label_create(bk_ui->settingmoderecord);
    lv_label_set_text(bk_ui->settingmoderecord_RecordFreezeTemp4, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmoderecord_RecordFreezeTemp4, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmoderecord_RecordFreezeTemp4, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmoderecord_RecordFreezeTemp4, &lv_font_scdream_regular_22, 0);
    lv_obj_set_pos(bk_ui->settingmoderecord_RecordFreezeTemp4, 103, 447);
    lv_obj_set_size(bk_ui->settingmoderecord_RecordFreezeTemp4, 60, 30);
    lv_obj_set_style_text_align(bk_ui->settingmoderecord_RecordFreezeTemp4, LV_TEXT_ALIGN_CENTER, 0);

    // TextView: RecordFreezeTimeHour4
    bk_ui->settingmoderecord_RecordFreezeTimeHour4 = lv_label_create(bk_ui->settingmoderecord);
    lv_label_set_text(bk_ui->settingmoderecord_RecordFreezeTimeHour4, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmoderecord_RecordFreezeTimeHour4, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmoderecord_RecordFreezeTimeHour4, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmoderecord_RecordFreezeTimeHour4, &lv_font_scdream_regular_22, 0);
    lv_obj_set_pos(bk_ui->settingmoderecord_RecordFreezeTimeHour4, 95, 477);
    lv_obj_set_size(bk_ui->settingmoderecord_RecordFreezeTimeHour4, 40, 30);
    lv_obj_set_style_text_align(bk_ui->settingmoderecord_RecordFreezeTimeHour4, LV_TEXT_ALIGN_CENTER, 0);

    // TextView: RecordFreezeTimeMin4
    bk_ui->settingmoderecord_RecordFreezeTimeMin4 = lv_label_create(bk_ui->settingmoderecord);
    lv_label_set_text(bk_ui->settingmoderecord_RecordFreezeTimeMin4, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmoderecord_RecordFreezeTimeMin4, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmoderecord_RecordFreezeTimeMin4, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmoderecord_RecordFreezeTimeMin4, &lv_font_scdream_regular_22, 0);
    lv_obj_set_pos(bk_ui->settingmoderecord_RecordFreezeTimeMin4, 156, 477);
    lv_obj_set_size(bk_ui->settingmoderecord_RecordFreezeTimeMin4, 40, 30);
    lv_obj_set_style_text_align(bk_ui->settingmoderecord_RecordFreezeTimeMin4, LV_TEXT_ALIGN_CENTER, 0);

    // TextView: RecordDefreezeTemp4
    bk_ui->settingmoderecord_RecordDefreezeTemp4 = lv_label_create(bk_ui->settingmoderecord);
    lv_label_set_text(bk_ui->settingmoderecord_RecordDefreezeTemp4, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmoderecord_RecordDefreezeTemp4, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmoderecord_RecordDefreezeTemp4, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmoderecord_RecordDefreezeTemp4, &lv_font_scdream_regular_22, 0);
    lv_obj_set_pos(bk_ui->settingmoderecord_RecordDefreezeTemp4, 248, 447);
    lv_obj_set_size(bk_ui->settingmoderecord_RecordDefreezeTemp4, 60, 30);
    lv_obj_set_style_text_align(bk_ui->settingmoderecord_RecordDefreezeTemp4, LV_TEXT_ALIGN_CENTER, 0);

    // TextView: RecordDefreezeTimeHour4
    bk_ui->settingmoderecord_RecordDefreezeTimeHour4 = lv_label_create(bk_ui->settingmoderecord);
    lv_label_set_text(bk_ui->settingmoderecord_RecordDefreezeTimeHour4, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmoderecord_RecordDefreezeTimeHour4, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmoderecord_RecordDefreezeTimeHour4, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmoderecord_RecordDefreezeTimeHour4, &lv_font_scdream_regular_22, 0);
    lv_obj_set_pos(bk_ui->settingmoderecord_RecordDefreezeTimeHour4, 235, 477);
    lv_obj_set_size(bk_ui->settingmoderecord_RecordDefreezeTimeHour4, 40, 30);
    lv_obj_set_style_text_align(bk_ui->settingmoderecord_RecordDefreezeTimeHour4, LV_TEXT_ALIGN_CENTER, 0);

    // TextView: RecordDefreezeTimeMin4
    bk_ui->settingmoderecord_RecordDefreezeTimeMin4 = lv_label_create(bk_ui->settingmoderecord);
    lv_label_set_text(bk_ui->settingmoderecord_RecordDefreezeTimeMin4, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmoderecord_RecordDefreezeTimeMin4, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmoderecord_RecordDefreezeTimeMin4, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmoderecord_RecordDefreezeTimeMin4, &lv_font_scdream_regular_22, 0);
    lv_obj_set_pos(bk_ui->settingmoderecord_RecordDefreezeTimeMin4, 296, 477);
    lv_obj_set_size(bk_ui->settingmoderecord_RecordDefreezeTimeMin4, 40, 30);
    lv_obj_set_style_text_align(bk_ui->settingmoderecord_RecordDefreezeTimeMin4, LV_TEXT_ALIGN_CENTER, 0);

    // TextView: RecordFermentation1Temp4
    bk_ui->settingmoderecord_RecordFermentation1Temp4 = lv_label_create(bk_ui->settingmoderecord);
    lv_label_set_text(bk_ui->settingmoderecord_RecordFermentation1Temp4, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmoderecord_RecordFermentation1Temp4, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmoderecord_RecordFermentation1Temp4, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmoderecord_RecordFermentation1Temp4, &lv_font_scdream_regular_22, 0);
    lv_obj_set_pos(bk_ui->settingmoderecord_RecordFermentation1Temp4, 376, 447);
    lv_obj_set_size(bk_ui->settingmoderecord_RecordFermentation1Temp4, 50, 30);
    lv_obj_set_style_text_align(bk_ui->settingmoderecord_RecordFermentation1Temp4, LV_TEXT_ALIGN_CENTER, 0);

    // TextView: RecordFermentation1Humidity4
    bk_ui->settingmoderecord_RecordFermentation1Humidity4 = lv_label_create(bk_ui->settingmoderecord);
    lv_label_set_text(bk_ui->settingmoderecord_RecordFermentation1Humidity4, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmoderecord_RecordFermentation1Humidity4, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmoderecord_RecordFermentation1Humidity4, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmoderecord_RecordFermentation1Humidity4, &lv_font_scdream_regular_22, 0);
    lv_obj_set_pos(bk_ui->settingmoderecord_RecordFermentation1Humidity4, 435, 447);
    lv_obj_set_size(bk_ui->settingmoderecord_RecordFermentation1Humidity4, 50, 30);
    lv_obj_set_style_text_align(bk_ui->settingmoderecord_RecordFermentation1Humidity4, LV_TEXT_ALIGN_CENTER, 0);

    // TextView: RecordFermentation1TimeHour4
    bk_ui->settingmoderecord_RecordFermentation1TimeHour4 = lv_label_create(bk_ui->settingmoderecord);
    lv_label_set_text(bk_ui->settingmoderecord_RecordFermentation1TimeHour4, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmoderecord_RecordFermentation1TimeHour4, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmoderecord_RecordFermentation1TimeHour4, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmoderecord_RecordFermentation1TimeHour4, &lv_font_scdream_regular_22, 0);
    lv_obj_set_pos(bk_ui->settingmoderecord_RecordFermentation1TimeHour4, 381, 477);
    lv_obj_set_size(bk_ui->settingmoderecord_RecordFermentation1TimeHour4, 40, 30);
    lv_obj_set_style_text_align(bk_ui->settingmoderecord_RecordFermentation1TimeHour4, LV_TEXT_ALIGN_CENTER, 0);

    // TextView: RecordFermentation1TimeMin4
    bk_ui->settingmoderecord_RecordFermentation1TimeMin4 = lv_label_create(bk_ui->settingmoderecord);
    lv_label_set_text(bk_ui->settingmoderecord_RecordFermentation1TimeMin4, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmoderecord_RecordFermentation1TimeMin4, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmoderecord_RecordFermentation1TimeMin4, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmoderecord_RecordFermentation1TimeMin4, &lv_font_scdream_regular_22, 0);
    lv_obj_set_pos(bk_ui->settingmoderecord_RecordFermentation1TimeMin4, 440, 477);
    lv_obj_set_size(bk_ui->settingmoderecord_RecordFermentation1TimeMin4, 40, 30);
    lv_obj_set_style_text_align(bk_ui->settingmoderecord_RecordFermentation1TimeMin4, LV_TEXT_ALIGN_CENTER, 0);

    // TextView: RecordFermentation2Temp4
    bk_ui->settingmoderecord_RecordFermentation2Temp4 = lv_label_create(bk_ui->settingmoderecord);
    lv_label_set_text(bk_ui->settingmoderecord_RecordFermentation2Temp4, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmoderecord_RecordFermentation2Temp4, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmoderecord_RecordFermentation2Temp4, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmoderecord_RecordFermentation2Temp4, &lv_font_scdream_regular_22, 0);
    lv_obj_set_pos(bk_ui->settingmoderecord_RecordFermentation2Temp4, 518, 447);
    lv_obj_set_size(bk_ui->settingmoderecord_RecordFermentation2Temp4, 50, 30);
    lv_obj_set_style_text_align(bk_ui->settingmoderecord_RecordFermentation2Temp4, LV_TEXT_ALIGN_CENTER, 0);

    // TextView: RecordFermentation2Humidity4
    bk_ui->settingmoderecord_RecordFermentation2Humidity4 = lv_label_create(bk_ui->settingmoderecord);
    lv_label_set_text(bk_ui->settingmoderecord_RecordFermentation2Humidity4, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmoderecord_RecordFermentation2Humidity4, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmoderecord_RecordFermentation2Humidity4, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmoderecord_RecordFermentation2Humidity4, &lv_font_scdream_regular_22, 0);
    lv_obj_set_pos(bk_ui->settingmoderecord_RecordFermentation2Humidity4, 577, 447);
    lv_obj_set_size(bk_ui->settingmoderecord_RecordFermentation2Humidity4, 50, 30);
    lv_obj_set_style_text_align(bk_ui->settingmoderecord_RecordFermentation2Humidity4, LV_TEXT_ALIGN_CENTER, 0);

    // TextView: RecordFermentation2TimeHour4
    bk_ui->settingmoderecord_RecordFermentation2TimeHour4 = lv_label_create(bk_ui->settingmoderecord);
    lv_label_set_text(bk_ui->settingmoderecord_RecordFermentation2TimeHour4, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmoderecord_RecordFermentation2TimeHour4, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmoderecord_RecordFermentation2TimeHour4, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmoderecord_RecordFermentation2TimeHour4, &lv_font_scdream_regular_22, 0);
    lv_obj_set_pos(bk_ui->settingmoderecord_RecordFermentation2TimeHour4, 523, 477);
    lv_obj_set_size(bk_ui->settingmoderecord_RecordFermentation2TimeHour4, 40, 30);
    lv_obj_set_style_text_align(bk_ui->settingmoderecord_RecordFermentation2TimeHour4, LV_TEXT_ALIGN_CENTER, 0);

    // TextView: RecordFermentation2TimeMin4
    bk_ui->settingmoderecord_RecordFermentation2TimeMin4 = lv_label_create(bk_ui->settingmoderecord);
    lv_label_set_text(bk_ui->settingmoderecord_RecordFermentation2TimeMin4, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmoderecord_RecordFermentation2TimeMin4, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmoderecord_RecordFermentation2TimeMin4, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmoderecord_RecordFermentation2TimeMin4, &lv_font_scdream_regular_22, 0);
    lv_obj_set_pos(bk_ui->settingmoderecord_RecordFermentation2TimeMin4, 582, 477);
    lv_obj_set_size(bk_ui->settingmoderecord_RecordFermentation2TimeMin4, 40, 30);
    lv_obj_set_style_text_align(bk_ui->settingmoderecord_RecordFermentation2TimeMin4, LV_TEXT_ALIGN_CENTER, 0);

    // TextView: RecordStartMonth4
    bk_ui->settingmoderecord_RecordStartMonth4 = lv_label_create(bk_ui->settingmoderecord);
    lv_label_set_text(bk_ui->settingmoderecord_RecordStartMonth4, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmoderecord_RecordStartMonth4, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmoderecord_RecordStartMonth4, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmoderecord_RecordStartMonth4, &lv_font_scdream_regular_22, 0);
    lv_obj_set_pos(bk_ui->settingmoderecord_RecordStartMonth4, 651, 459);
    lv_obj_set_size(bk_ui->settingmoderecord_RecordStartMonth4, 40, 30);
    lv_obj_set_style_text_align(bk_ui->settingmoderecord_RecordStartMonth4, LV_TEXT_ALIGN_CENTER, 0);

    // TextView: RecordStartDay4
    bk_ui->settingmoderecord_RecordStartDay4 = lv_label_create(bk_ui->settingmoderecord);
    lv_label_set_text(bk_ui->settingmoderecord_RecordStartDay4, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmoderecord_RecordStartDay4, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmoderecord_RecordStartDay4, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmoderecord_RecordStartDay4, &lv_font_scdream_regular_22, 0);
    lv_obj_set_pos(bk_ui->settingmoderecord_RecordStartDay4, 695, 459);
    lv_obj_set_size(bk_ui->settingmoderecord_RecordStartDay4, 40, 30);
    lv_obj_set_style_text_align(bk_ui->settingmoderecord_RecordStartDay4, LV_TEXT_ALIGN_CENTER, 0);

    // TextView: RecordStartHour4
    bk_ui->settingmoderecord_RecordStartHour4 = lv_label_create(bk_ui->settingmoderecord);
    lv_label_set_text(bk_ui->settingmoderecord_RecordStartHour4, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmoderecord_RecordStartHour4, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmoderecord_RecordStartHour4, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmoderecord_RecordStartHour4, &lv_font_scdream_regular_22, 0);
    lv_obj_set_pos(bk_ui->settingmoderecord_RecordStartHour4, 735, 459);
    lv_obj_set_size(bk_ui->settingmoderecord_RecordStartHour4, 40, 30);
    lv_obj_set_style_text_align(bk_ui->settingmoderecord_RecordStartHour4, LV_TEXT_ALIGN_CENTER, 0);

    // TextView: RecordStartMin4
    bk_ui->settingmoderecord_RecordStartMin4 = lv_label_create(bk_ui->settingmoderecord);
    lv_label_set_text(bk_ui->settingmoderecord_RecordStartMin4, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmoderecord_RecordStartMin4, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmoderecord_RecordStartMin4, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmoderecord_RecordStartMin4, &lv_font_scdream_regular_22, 0);
    lv_obj_set_pos(bk_ui->settingmoderecord_RecordStartMin4, 781, 459);
    lv_obj_set_size(bk_ui->settingmoderecord_RecordStartMin4, 40, 30);
    lv_obj_set_style_text_align(bk_ui->settingmoderecord_RecordStartMin4, LV_TEXT_ALIGN_CENTER, 0);

    // TextView: RecordEndMonth4
    bk_ui->settingmoderecord_RecordEndMonth4 = lv_label_create(bk_ui->settingmoderecord);
    lv_label_set_text(bk_ui->settingmoderecord_RecordEndMonth4, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmoderecord_RecordEndMonth4, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmoderecord_RecordEndMonth4, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmoderecord_RecordEndMonth4, &lv_font_scdream_regular_22, 0);
    lv_obj_set_pos(bk_ui->settingmoderecord_RecordEndMonth4, 831, 459);
    lv_obj_set_size(bk_ui->settingmoderecord_RecordEndMonth4, 40, 30);
    lv_obj_set_style_text_align(bk_ui->settingmoderecord_RecordEndMonth4, LV_TEXT_ALIGN_CENTER, 0);

    // TextView: RecordEndDay4
    bk_ui->settingmoderecord_RecordEndDay4 = lv_label_create(bk_ui->settingmoderecord);
    lv_label_set_text(bk_ui->settingmoderecord_RecordEndDay4, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmoderecord_RecordEndDay4, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmoderecord_RecordEndDay4, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmoderecord_RecordEndDay4, &lv_font_scdream_regular_22, 0);
    lv_obj_set_pos(bk_ui->settingmoderecord_RecordEndDay4, 875, 459);
    lv_obj_set_size(bk_ui->settingmoderecord_RecordEndDay4, 40, 30);
    lv_obj_set_style_text_align(bk_ui->settingmoderecord_RecordEndDay4, LV_TEXT_ALIGN_CENTER, 0);

    // TextView: RecordEndHour4
    bk_ui->settingmoderecord_RecordEndHour4 = lv_label_create(bk_ui->settingmoderecord);
    lv_label_set_text(bk_ui->settingmoderecord_RecordEndHour4, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmoderecord_RecordEndHour4, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmoderecord_RecordEndHour4, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmoderecord_RecordEndHour4, &lv_font_scdream_regular_22, 0);
    lv_obj_set_pos(bk_ui->settingmoderecord_RecordEndHour4, 915, 459);
    lv_obj_set_size(bk_ui->settingmoderecord_RecordEndHour4, 40, 30);
    lv_obj_set_style_text_align(bk_ui->settingmoderecord_RecordEndHour4, LV_TEXT_ALIGN_CENTER, 0);

    // TextView: RecordEndMin4
    bk_ui->settingmoderecord_RecordEndMin4 = lv_label_create(bk_ui->settingmoderecord);
    lv_label_set_text(bk_ui->settingmoderecord_RecordEndMin4, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmoderecord_RecordEndMin4, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmoderecord_RecordEndMin4, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmoderecord_RecordEndMin4, &lv_font_scdream_regular_22, 0);
    lv_obj_set_pos(bk_ui->settingmoderecord_RecordEndMin4, 961, 459);
    lv_obj_set_size(bk_ui->settingmoderecord_RecordEndMin4, 40, 30);
    lv_obj_set_style_text_align(bk_ui->settingmoderecord_RecordEndMin4, LV_TEXT_ALIGN_CENTER, 0);

}

rendererFuncStatus_t init_page_settingmoderecord_with_step(bk_lv_ui_t *bk_ui)
{
    static uint32_t renderStartTick = 0;

    if(preRenderPageState[PAGE_SETTINGMODERECORD].isRendered)
    {
        return RENDERER_FUNC_DONE;
    }

    switch(currentStep)
    {
        case RENDER_STEP_CREATE_PAGE:
        {
            renderStartTick = lv_tick_get();
            bk_printf(TAG "[RENDER][SETTINGMODERECORD] start tick=%lu\n", (unsigned long)renderStartTick);

            if(bk_ui == NULL)
            {
                return RENDERER_FUNC_FAILED;
            }

            stepInitMode = true;
            init_page_settingmoderecord(bk_ui);
            stepInitMode = false;
            if(bk_ui->settingmoderecord == NULL || !lv_obj_is_valid(bk_ui->settingmoderecord))
            {
                bk_printf(TAG "[RENDER][SETTINGMODERECORD] CREATE_PAGE failed\n");
                return RENDERER_FUNC_FAILED;
            }

#if UI_PRENDERING_ENABLE
            lv_obj_add_flag(bk_ui->settingmoderecord, LV_OBJ_FLAG_HIDDEN);
#endif
            currentStep = RENDER_STEP_CREATE_CHILD;
            return RENDERER_FUNC_NOT_DONE;
        }

        case RENDER_STEP_CREATE_CHILD:
        {
            stepInitMode = true;
            init_page_settingmoderecord(bk_ui);
            stepInitMode = false;
            currentStep = RENDER_STEP_CACHE_BACKGROUND;
            return RENDERER_FUNC_NOT_DONE;
        }

        case RENDER_STEP_CACHE_BACKGROUND:
        {
            if(preRenderPageConfig[PAGE_SETTINGMODERECORD].backgroundImageAssetId != SHARED_IMAGE_NONE)
            {
                const sharedImageAssetId_t assetId =
                    preRenderPageConfig[PAGE_SETTINGMODERECORD].backgroundImageAssetId;
                if(set_shared_image_asset(bk_ui->settingmoderecord_bg, assetId) != RENDERER_FUNC_DONE)
                {
                    return RENDERER_FUNC_FAILED;
                }
            }

            currentStep = RENDER_STEP_CACHE_IMAGE;
            return RENDERER_FUNC_NOT_DONE;
        }

        case RENDER_STEP_CACHE_IMAGE:
        {
            const uint32_t imageCount = preRenderPageConfig[PAGE_SETTINGMODERECORD].preRenderImageCount;
            if(currentImageStep < imageCount)
            {
                const preRenderImageInfo_t *imageInfo =
                    &preRenderPageConfig[PAGE_SETTINGMODERECORD].preRenderImageInfo[currentImageStep];
                const char *languageSuffix = imageInfo->hasLanguageVariant ?
                                             (settings_get_int("LANGUAGE") == 1 ? "_china" :
                                              settings_get_int("LANGUAGE") == 2 ? "_english" : "") : "";
                const char *degreeSuffix = imageInfo->hasDegreeVariant &&
                                           strcmp(settings_get_str("Degree"), "\xc2\xb0""F") == 0 ? "_f" : "";
                const char *extension =
                    imageInfo->fileExtension != NULL ? imageInfo->fileExtension : ".png";
                char imagePath[128] = {0};
                uint32_t imageStartTick = lv_tick_get();

                snprintf(imagePath, sizeof(imagePath), "%s%s%s%s",
                         imageInfo->imagePath, degreeSuffix, languageSuffix, extension);

                lv_result_t result = lv_image_decoder_prewarm(imagePath);
                if(result != LV_RESULT_OK)
                {
                    bk_printf(TAG "[PREWARM][SETTINGMODERECORD] image %lu/%lu failed: %s (%lu ms)\n",
                              (unsigned long)(currentImageStep + 1),
                              (unsigned long)imageCount,
                              imagePath,
                              (unsigned long)lv_tick_elaps(imageStartTick));
                    return RENDERER_FUNC_FAILED;
                }

                bk_printf(TAG "[PREWARM][SETTINGMODERECORD] image %lu/%lu done: %s (%lu ms)\n",
                          (unsigned long)(currentImageStep + 1),
                          (unsigned long)imageCount,
                          imagePath,
                          (unsigned long)lv_tick_elaps(imageStartTick));
                currentImageStep++;
                return RENDERER_FUNC_NOT_DONE;
            }

            currentImageStep = 0;
            currentStep = RENDER_STEP_ATTACH_EVENT;
            return RENDERER_FUNC_NOT_DONE;
        }

        case RENDER_STEP_ATTACH_EVENT:
        {
            /* init_page_settingmoderecord() also attaches the page and control callbacks. */
            currentStep = RENDER_STEP_DONE;
            return RENDERER_FUNC_NOT_DONE;
        }

        case RENDER_STEP_DONE:
        {
            bk_printf(TAG "[RENDER][SETTINGMODERECORD] done total=%lu ms\n",
                      (unsigned long)lv_tick_elaps(renderStartTick));

            currentStep = RENDER_STEP_CREATE_PAGE;
            currentImageStep = 0;
            renderStartTick = 0;
            preRenderPageState[PAGE_SETTINGMODERECORD].isRendered = true;
            return RENDERER_FUNC_DONE;
        }

        default:
        {
            bk_printf(TAG "[RENDER][SETTINGMODERECORD] invalid step=%lu\n",
                      (unsigned long)currentStep);
            return RENDERER_FUNC_FAILED;
        }
    }
}
