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

#define TAG "[detailsettingdamper_init.c] "
#include "preRenderer.h"
// #define bk_printf(fmt, ...) do {if(0) bk_printf(fmt, ##__VA_ARGS__); } while(0) // disable printf

static uint32_t currentStep = RENDER_STEP_CREATE_PAGE;
static uint32_t currentImageStep = 0;

extern bk_lv_ui_t bk_lv_tool_ui;
extern lv_obj_t *preRenderRoot;

static bool stepInitMode = false;
extern void detailsettingdamper_backbt_event_cb(lv_event_t *e);
extern void detailsettingdamper_settingbt1_event_cb(lv_event_t *e);
extern void detailsettingdamper_settingbt2_event_cb(lv_event_t *e);
extern void detailsettingdamper_settingbt3_event_cb(lv_event_t *e);
extern void detailsettingdamper_settingbt4_event_cb(lv_event_t *e);
extern void detailsettingdamper_changebt_event_cb(lv_event_t *e);
extern void detailsettingdamper_roller_event_cb(lv_event_t *e);
extern void detailsettingdamper_load_start_event_cb(lv_event_t *e);
extern void detailsettingdamper_loaded_event_cb(lv_event_t *e);
extern void detailsettingdamper_unload_start_event_cb(lv_event_t *e);
extern void detailsettingdamper_unloaded_event_cb(lv_event_t *e);

void destroy_page_detailsettingdamper(bk_lv_ui_t *bk_ui)
{
    if (bk_ui == NULL) {
        return;
    }
    if (bk_ui->detailsettingdamper != NULL) {
        lv_obj_del(bk_ui->detailsettingdamper);
        bk_ui->detailsettingdamper = NULL;
    }

    currentStep = RENDER_STEP_CREATE_PAGE;
    currentImageStep = 0;
    preRenderPageState[PAGE_DETAILSETTINGDAMPER].isRendered = false;

    const uint32_t imageCount = preRenderPageConfig[PAGE_DETAILSETTINGDAMPER].preRenderImageCount;
    for(uint32_t i = 0; i < imageCount; i++)
    {
        const preRenderImageInfo_t *imageInfo = &preRenderPageConfig[PAGE_DETAILSETTINGDAMPER].preRenderImageInfo[i];
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

void init_page_detailsettingdamper(bk_lv_ui_t * bk_ui) {
    if(stepInitMode && currentStep == RENDER_STEP_CREATE_CHILD)
    {
        goto create_children;
    }

    if (bk_ui->detailsettingdamper != NULL && lv_obj_is_valid(bk_ui->detailsettingdamper)) {
        destroy_page_detailsettingdamper(bk_ui);
    }

    ui_lang_reset_detailsettingdamper_cache();

#if UI_PRENDERING_ENABLE
    bk_ui->detailsettingdamper = lv_obj_create(preRenderRoot);
    lv_obj_remove_style_all(bk_ui->detailsettingdamper);
    lv_obj_set_size(bk_ui->detailsettingdamper, 1024, 600);
    lv_obj_set_pos(bk_ui->detailsettingdamper, 0, 0);
    lv_obj_set_style_radius(bk_ui->detailsettingdamper, 0, LV_PART_MAIN);
    lv_obj_set_scrollbar_mode(bk_ui->detailsettingdamper, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_event_cb(bk_ui->detailsettingdamper, detailsettingdamper_load_start_event_cb, UI_EVENT_PAGE_SHOW_START, NULL);
    lv_obj_add_event_cb(bk_ui->detailsettingdamper, detailsettingdamper_loaded_event_cb, UI_EVENT_PAGE_SHOWN,     NULL);
    lv_obj_add_event_cb(bk_ui->detailsettingdamper, detailsettingdamper_unload_start_event_cb, UI_EVENT_PAGE_HIDE_START, NULL);
    lv_obj_add_event_cb(bk_ui->detailsettingdamper, detailsettingdamper_unloaded_event_cb, UI_EVENT_PAGE_HIDDEN,     NULL);
#else
    bk_ui->detailsettingdamper = lv_obj_create(NULL);
    lv_obj_set_size(bk_ui->detailsettingdamper, 1024, 600);
    lv_obj_set_scrollbar_mode(bk_ui->detailsettingdamper, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_event_cb(bk_ui->detailsettingdamper, detailsettingdamper_load_start_event_cb, LV_EVENT_SCREEN_LOAD_START, NULL);
    lv_obj_add_event_cb(bk_ui->detailsettingdamper, detailsettingdamper_loaded_event_cb, LV_EVENT_SCREEN_LOADED,     NULL);
    lv_obj_add_event_cb(bk_ui->detailsettingdamper, detailsettingdamper_unload_start_event_cb, LV_EVENT_SCREEN_UNLOAD_START, NULL);
    lv_obj_add_event_cb(bk_ui->detailsettingdamper, detailsettingdamper_unloaded_event_cb, LV_EVENT_SCREEN_UNLOADED,     NULL);
#endif /* UI_PRENDERING_ENABLE */

    if(stepInitMode)
    {
        return;
    }

create_children:
    bk_ui->detailsettingdamper_bg = lv_image_create(bk_ui->detailsettingdamper);
    lv_obj_add_flag(bk_ui->detailsettingdamper_bg, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_style_bg_color(bk_ui->detailsettingdamper, lv_color_hex(0xd9d9d9), 0);
    lv_obj_set_style_bg_opa(bk_ui->detailsettingdamper, LV_OPA_COVER, 0);
    lv_obj_set_pos(bk_ui->detailsettingdamper_bg, 0, 0);

    // ImageView: title
    bk_ui->detailsettingdamper_title = lv_image_create(bk_ui->detailsettingdamper);
    lv_obj_set_pos(bk_ui->detailsettingdamper_title, 0, 10);
    lv_obj_set_size(bk_ui->detailsettingdamper_title, 380, 80);
    lv_image_set_inner_align(bk_ui->detailsettingdamper_title, LV_IMAGE_ALIGN_TOP_LEFT);

    // Button: backbt
    bk_ui->detailsettingdamper_backbt = lv_button_create(bk_ui->detailsettingdamper);
    lv_obj_add_flag(bk_ui->detailsettingdamper_backbt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->detailsettingdamper_backbt, detailsettingdamper_backbt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->detailsettingdamper_backbt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->detailsettingdamper_backbt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->detailsettingdamper_backbt, 0, 0);
    lv_obj_set_pos(bk_ui->detailsettingdamper_backbt, 825, 13);
    lv_obj_set_size(bk_ui->detailsettingdamper_backbt, 179, 74);

    // ImageView: imageview3
    bk_ui->detailsettingdamper_imageview3 = lv_image_create(bk_ui->detailsettingdamper);
    lv_obj_set_pos(bk_ui->detailsettingdamper_imageview3, 825, 13);
    lv_obj_set_size(bk_ui->detailsettingdamper_imageview3, 179, 74);

    // ImageView: settingim1
    bk_ui->detailsettingdamper_settingim1 = lv_image_create(bk_ui->detailsettingdamper);
    lv_obj_set_pos(bk_ui->detailsettingdamper_settingim1, 23, 120);
    lv_obj_set_size(bk_ui->detailsettingdamper_settingim1, 538, 66);

    // TextView: settingtxt1
    bk_ui->detailsettingdamper_settingtxt1 = lv_label_create(bk_ui->detailsettingdamper);
    lv_label_set_text(bk_ui->detailsettingdamper_settingtxt1, "");
    lv_obj_set_style_bg_opa(bk_ui->detailsettingdamper_settingtxt1, 0, 0);
    lv_obj_set_style_text_color(bk_ui->detailsettingdamper_settingtxt1, lv_color_hex(0xA6A6A6), 0);
    lv_obj_set_style_text_font(bk_ui->detailsettingdamper_settingtxt1, &lv_font_scdream_regular_32, 0);
    lv_obj_set_pos(bk_ui->detailsettingdamper_settingtxt1, 420, 135);
    lv_obj_set_size(bk_ui->detailsettingdamper_settingtxt1, 50, 42);
    lv_obj_set_style_text_align(bk_ui->detailsettingdamper_settingtxt1, LV_TEXT_ALIGN_RIGHT, 0);

    // Button: settingbt1
    bk_ui->detailsettingdamper_settingbt1 = lv_button_create(bk_ui->detailsettingdamper);
    lv_obj_add_flag(bk_ui->detailsettingdamper_settingbt1, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->detailsettingdamper_settingbt1, detailsettingdamper_settingbt1_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->detailsettingdamper_settingbt1, 0, 0);
    lv_obj_set_style_border_width(bk_ui->detailsettingdamper_settingbt1, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->detailsettingdamper_settingbt1, 0, 0);
    lv_obj_set_pos(bk_ui->detailsettingdamper_settingbt1, 23, 120);
    lv_obj_set_size(bk_ui->detailsettingdamper_settingbt1, 538, 66);

    // ImageView: settingim2
    bk_ui->detailsettingdamper_settingim2 = lv_image_create(bk_ui->detailsettingdamper);
    lv_obj_set_pos(bk_ui->detailsettingdamper_settingim2, 23, 198);
    lv_obj_set_size(bk_ui->detailsettingdamper_settingim2, 538, 66);

    // TextView: settingtxt2
    bk_ui->detailsettingdamper_settingtxt2 = lv_label_create(bk_ui->detailsettingdamper);
    lv_label_set_text(bk_ui->detailsettingdamper_settingtxt2, "");
    lv_obj_set_style_bg_opa(bk_ui->detailsettingdamper_settingtxt2, 0, 0);
    lv_obj_set_style_text_color(bk_ui->detailsettingdamper_settingtxt2, lv_color_hex(0xA6A6A6), 0);
    lv_obj_set_style_text_font(bk_ui->detailsettingdamper_settingtxt2, &lv_font_scdream_regular_32, 0);
    lv_obj_set_pos(bk_ui->detailsettingdamper_settingtxt2, 420, 213);
    lv_obj_set_size(bk_ui->detailsettingdamper_settingtxt2, 50, 42);
    lv_obj_set_style_text_align(bk_ui->detailsettingdamper_settingtxt2, LV_TEXT_ALIGN_RIGHT, 0);

    // Button: settingbt2
    bk_ui->detailsettingdamper_settingbt2 = lv_button_create(bk_ui->detailsettingdamper);
    lv_obj_add_flag(bk_ui->detailsettingdamper_settingbt2, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->detailsettingdamper_settingbt2, detailsettingdamper_settingbt2_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->detailsettingdamper_settingbt2, 0, 0);
    lv_obj_set_style_border_width(bk_ui->detailsettingdamper_settingbt2, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->detailsettingdamper_settingbt2, 0, 0);
    lv_obj_set_pos(bk_ui->detailsettingdamper_settingbt2, 23, 198);
    lv_obj_set_size(bk_ui->detailsettingdamper_settingbt2, 538, 66);

    // ImageView: settingim3
    bk_ui->detailsettingdamper_settingim3 = lv_image_create(bk_ui->detailsettingdamper);
    lv_obj_set_pos(bk_ui->detailsettingdamper_settingim3, 23, 276);
    lv_obj_set_size(bk_ui->detailsettingdamper_settingim3, 538, 66);

    // TextView: settingtxt3
    bk_ui->detailsettingdamper_settingtxt3 = lv_label_create(bk_ui->detailsettingdamper);
    lv_label_set_text(bk_ui->detailsettingdamper_settingtxt3, "");
    lv_obj_set_style_bg_opa(bk_ui->detailsettingdamper_settingtxt3, 0, 0);
    lv_obj_set_style_text_color(bk_ui->detailsettingdamper_settingtxt3, lv_color_hex(0xA6A6A6), 0);
    lv_obj_set_style_text_font(bk_ui->detailsettingdamper_settingtxt3, &lv_font_scdream_regular_32, 0);
    lv_obj_set_pos(bk_ui->detailsettingdamper_settingtxt3, 410, 291);
    lv_obj_set_size(bk_ui->detailsettingdamper_settingtxt3, 60, 42);
    lv_obj_set_style_text_align(bk_ui->detailsettingdamper_settingtxt3, LV_TEXT_ALIGN_RIGHT, 0);

    // Button: settingbt3
    bk_ui->detailsettingdamper_settingbt3 = lv_button_create(bk_ui->detailsettingdamper);
    lv_obj_add_flag(bk_ui->detailsettingdamper_settingbt3, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->detailsettingdamper_settingbt3, detailsettingdamper_settingbt3_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->detailsettingdamper_settingbt3, 0, 0);
    lv_obj_set_style_border_width(bk_ui->detailsettingdamper_settingbt3, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->detailsettingdamper_settingbt3, 0, 0);
    lv_obj_set_pos(bk_ui->detailsettingdamper_settingbt3, 23, 276);
    lv_obj_set_size(bk_ui->detailsettingdamper_settingbt3, 538, 66);

    // ImageView: settingim4
    bk_ui->detailsettingdamper_settingim4 = lv_image_create(bk_ui->detailsettingdamper);
    lv_obj_set_pos(bk_ui->detailsettingdamper_settingim4, 23, 354);
    lv_obj_set_size(bk_ui->detailsettingdamper_settingim4, 538, 66);

    // TextView: settingtxt4
    bk_ui->detailsettingdamper_settingtxt4 = lv_label_create(bk_ui->detailsettingdamper);
    lv_label_set_text(bk_ui->detailsettingdamper_settingtxt4, "");
    lv_obj_set_style_bg_opa(bk_ui->detailsettingdamper_settingtxt4, 0, 0);
    lv_obj_set_style_text_color(bk_ui->detailsettingdamper_settingtxt4, lv_color_hex(0xA6A6A6), 0);
    lv_obj_set_style_text_font(bk_ui->detailsettingdamper_settingtxt4, &lv_font_scdream_regular_32, 0);
    lv_obj_set_pos(bk_ui->detailsettingdamper_settingtxt4, 410, 369);
    lv_obj_set_size(bk_ui->detailsettingdamper_settingtxt4, 60, 42);
    lv_obj_set_style_text_align(bk_ui->detailsettingdamper_settingtxt4, LV_TEXT_ALIGN_RIGHT, 0);

    // Button: settingbt4
    bk_ui->detailsettingdamper_settingbt4 = lv_button_create(bk_ui->detailsettingdamper);
    lv_obj_add_flag(bk_ui->detailsettingdamper_settingbt4, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->detailsettingdamper_settingbt4, detailsettingdamper_settingbt4_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->detailsettingdamper_settingbt4, 0, 0);
    lv_obj_set_style_border_width(bk_ui->detailsettingdamper_settingbt4, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->detailsettingdamper_settingbt4, 0, 0);
    lv_obj_set_pos(bk_ui->detailsettingdamper_settingbt4, 23, 354);
    lv_obj_set_size(bk_ui->detailsettingdamper_settingbt4, 538, 66);

    // ImageView: pickerbox (src changed dynamically in cb: picker_1.png or picker_2.png)
    bk_ui->detailsettingdamper_pickerbox = lv_image_create(bk_ui->detailsettingdamper);
    ui_lang_apply_picker(bk_ui->detailsettingdamper_pickerbox, 2);
    lv_obj_set_pos(bk_ui->detailsettingdamper_pickerbox, 624, 120);
    lv_obj_set_size(bk_ui->detailsettingdamper_pickerbox, 376, 376);
    lv_obj_add_flag(bk_ui->detailsettingdamper_pickerbox, LV_OBJ_FLAG_HIDDEN);


#define _DAMPER_ROLLER_STYLE(obj) \
    lv_obj_set_style_bg_opa((obj), LV_OPA_TRANSP, LV_PART_MAIN); \
    lv_obj_set_style_border_width((obj), 0, LV_PART_MAIN); \
    lv_obj_set_style_text_color((obj), lv_color_hex(0xCCCCCC), LV_PART_MAIN); \
    lv_obj_set_style_text_font((obj), &lv_font_scdream_regular_72, LV_PART_MAIN); \
    lv_obj_set_style_bg_opa((obj), LV_OPA_TRANSP, LV_PART_SELECTED); \
    lv_obj_set_style_text_color((obj), lv_color_hex(0x333333), LV_PART_SELECTED); \
    lv_obj_set_style_text_font((obj), &lv_font_scdream_regular_90, LV_PART_SELECTED)

    // setn1: hundreds digit (bt3/bt4 triple roller), damperSec0
    bk_ui->detailsettingdamper_settemp_setn1 = lv_roller_create(bk_ui->detailsettingdamper);
    lv_roller_set_options(bk_ui->detailsettingdamper_settemp_setn1, "0", LV_ROLLER_MODE_NORMAL);
    /* 폰트 스타일을 먼저 적용해야 함: lv_roller_set_visible_row_count()는 "현재"
     * 적용된 폰트의 line height로 높이를 계산하므로, 스타일(66/72px 폰트) 적용 전에
     * 호출하면 기본 테마 폰트(훨씬 작음) 기준으로 계산되어 롤러 높이가 너무 작아짐
     * → 3개 행이 서로 겹쳐 항상 "0"만 보이는 것처럼 보이던 문제의 원인. */
    _DAMPER_ROLLER_STYLE(bk_ui->detailsettingdamper_settemp_setn1);
    lv_roller_set_visible_row_count(bk_ui->detailsettingdamper_settemp_setn1, 3);
    lv_obj_add_event_cb(bk_ui->detailsettingdamper_settemp_setn1, detailsettingdamper_roller_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_add_flag(bk_ui->detailsettingdamper_settemp_setn1, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_pos(bk_ui->detailsettingdamper_settemp_setn1, 651, 136-2-1-5);
    lv_obj_set_width(bk_ui->detailsettingdamper_settemp_setn1, 90);

    // setn2: tens digit (bt3/bt4 triple roller), damperSec1
    bk_ui->detailsettingdamper_settemp_setn2 = lv_roller_create(bk_ui->detailsettingdamper);
    lv_roller_set_options(bk_ui->detailsettingdamper_settemp_setn2, "0", LV_ROLLER_MODE_NORMAL);
    _DAMPER_ROLLER_STYLE(bk_ui->detailsettingdamper_settemp_setn2);
    lv_roller_set_visible_row_count(bk_ui->detailsettingdamper_settemp_setn2, 3);
    lv_obj_add_event_cb(bk_ui->detailsettingdamper_settemp_setn2, detailsettingdamper_roller_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_add_flag(bk_ui->detailsettingdamper_settemp_setn2, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_pos(bk_ui->detailsettingdamper_settemp_setn2, 766, 136-2-1-5);
    lv_obj_set_width(bk_ui->detailsettingdamper_settemp_setn2, 90);

    // setn3: units digit (bt3/bt4 triple roller), damperSec1
    bk_ui->detailsettingdamper_settemp_setn3 = lv_roller_create(bk_ui->detailsettingdamper);
    lv_roller_set_options(bk_ui->detailsettingdamper_settemp_setn3, "0", LV_ROLLER_MODE_NORMAL);
    _DAMPER_ROLLER_STYLE(bk_ui->detailsettingdamper_settemp_setn3);
    lv_roller_set_visible_row_count(bk_ui->detailsettingdamper_settemp_setn3, 3);
    lv_obj_add_event_cb(bk_ui->detailsettingdamper_settemp_setn3, detailsettingdamper_roller_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_add_flag(bk_ui->detailsettingdamper_settemp_setn3, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_pos(bk_ui->detailsettingdamper_settemp_setn3, 882, 136-2-1-5);
    lv_obj_set_width(bk_ui->detailsettingdamper_settemp_setn3, 90);

    // setn4: single roller for bt1/bt2 (damperFan 1-20)
    bk_ui->detailsettingdamper_settemp_setn4 = lv_roller_create(bk_ui->detailsettingdamper);
    lv_roller_set_options(bk_ui->detailsettingdamper_settemp_setn4, "0", LV_ROLLER_MODE_NORMAL);
    _DAMPER_ROLLER_STYLE(bk_ui->detailsettingdamper_settemp_setn4);
    lv_roller_set_visible_row_count(bk_ui->detailsettingdamper_settemp_setn4, 3);
    lv_obj_add_event_cb(bk_ui->detailsettingdamper_settemp_setn4, detailsettingdamper_roller_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_add_flag(bk_ui->detailsettingdamper_settemp_setn4, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_pos(bk_ui->detailsettingdamper_settemp_setn4, 700-10, 136-2-5-1);
    lv_obj_set_width(bk_ui->detailsettingdamper_settemp_setn4, 240);

#undef _DAMPER_ROLLER_STYLE

    // ImageView: imageview21
    bk_ui->detailsettingdamper_imageview21 = lv_image_create(bk_ui->detailsettingdamper);
    lv_obj_set_pos(bk_ui->detailsettingdamper_imageview21, 604, 420);
    lv_obj_set_size(bk_ui->detailsettingdamper_imageview21, 400, 50);

    // Button: changebt
    bk_ui->detailsettingdamper_changebt = lv_button_create(bk_ui->detailsettingdamper);
    lv_obj_add_flag(bk_ui->detailsettingdamper_changebt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->detailsettingdamper_changebt, detailsettingdamper_changebt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->detailsettingdamper_changebt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->detailsettingdamper_changebt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->detailsettingdamper_changebt, 0, 0);
    lv_obj_set_pos(bk_ui->detailsettingdamper_changebt, 604, 420);
    lv_obj_set_size(bk_ui->detailsettingdamper_changebt, 400, 50);

    ui_lang_apply_detailsettingdamper(bk_ui);
}

rendererFuncStatus_t init_page_detailsettingdamper_with_step(bk_lv_ui_t *bk_ui)
{
    static uint32_t renderStartTick = 0;

    if(preRenderPageState[PAGE_DETAILSETTINGDAMPER].isRendered)
    {
        return RENDERER_FUNC_DONE;
    }

    switch(currentStep)
    {
        case RENDER_STEP_CREATE_PAGE:
        {
            renderStartTick = lv_tick_get();
            bk_printf(TAG "[RENDER][DETAILSETTINGDAMPER] start tick=%lu\n", (unsigned long)renderStartTick);

            if(bk_ui == NULL)
            {
                return RENDERER_FUNC_FAILED;
            }

            stepInitMode = true;
            init_page_detailsettingdamper(bk_ui);
            stepInitMode = false;
            if(bk_ui->detailsettingdamper == NULL || !lv_obj_is_valid(bk_ui->detailsettingdamper))
            {
                bk_printf(TAG "[RENDER][DETAILSETTINGDAMPER] CREATE_PAGE failed\n");
                return RENDERER_FUNC_FAILED;
            }

#if UI_PRENDERING_ENABLE
            lv_obj_add_flag(bk_ui->detailsettingdamper, LV_OBJ_FLAG_HIDDEN);
#endif
            currentStep = RENDER_STEP_CREATE_CHILD;
            return RENDERER_FUNC_NOT_DONE;
        }

        case RENDER_STEP_CREATE_CHILD:
        {
            stepInitMode = true;
            init_page_detailsettingdamper(bk_ui);
            stepInitMode = false;
            currentStep = RENDER_STEP_CACHE_BACKGROUND;
            return RENDERER_FUNC_NOT_DONE;
        }

        case RENDER_STEP_CACHE_BACKGROUND:
        {
            if(preRenderPageConfig[PAGE_DETAILSETTINGDAMPER].backgroundImageAssetId != SHARED_IMAGE_NONE)
            {
                const sharedImageAssetId_t assetId =
                    preRenderPageConfig[PAGE_DETAILSETTINGDAMPER].backgroundImageAssetId;
                if(set_shared_image_asset(bk_ui->detailsettingdamper_bg, assetId) != RENDERER_FUNC_DONE)
                {
                    return RENDERER_FUNC_FAILED;
                }
            }

            currentStep = RENDER_STEP_CACHE_IMAGE;
            return RENDERER_FUNC_NOT_DONE;
        }

        case RENDER_STEP_CACHE_IMAGE:
        {
            const uint32_t imageCount = preRenderPageConfig[PAGE_DETAILSETTINGDAMPER].preRenderImageCount;
            if(currentImageStep < imageCount)
            {
                const preRenderImageInfo_t *imageInfo =
                    &preRenderPageConfig[PAGE_DETAILSETTINGDAMPER].preRenderImageInfo[currentImageStep];
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
                    bk_printf(TAG "[PREWARM][DETAILSETTINGDAMPER] image %lu/%lu failed: %s (%lu ms)\n",
                              (unsigned long)(currentImageStep + 1),
                              (unsigned long)imageCount,
                              imagePath,
                              (unsigned long)lv_tick_elaps(imageStartTick));
                    return RENDERER_FUNC_FAILED;
                }

                bk_printf(TAG "[PREWARM][DETAILSETTINGDAMPER] image %lu/%lu done: %s (%lu ms)\n",
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
            /* init_page_detailsettingdamper() also attaches the page and control callbacks. */
            currentStep = RENDER_STEP_DONE;
            return RENDERER_FUNC_NOT_DONE;
        }

        case RENDER_STEP_DONE:
        {
            bk_printf(TAG "[RENDER][DETAILSETTINGDAMPER] done total=%lu ms\n",
                      (unsigned long)lv_tick_elaps(renderStartTick));

            currentStep = RENDER_STEP_CREATE_PAGE;
            currentImageStep = 0;
            renderStartTick = 0;
            preRenderPageState[PAGE_DETAILSETTINGDAMPER].isRendered = true;
            return RENDERER_FUNC_DONE;
        }

        default:
        {
            bk_printf(TAG "[RENDER][DETAILSETTINGDAMPER] invalid step=%lu\n",
                      (unsigned long)currentStep);
            return RENDERER_FUNC_FAILED;
        }
    }
}
