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

#define TAG "[settingmode_init.c] "
// #define bk_printf(fmt, ...) do {if(0) bk_printf(fmt, ##__VA_ARGS__); } while(0) // disable printf

extern bk_lv_ui_t bk_lv_tool_ui;
extern lv_obj_t *preRenderRoot;
static uint32_t currentStep = 0;
static uint32_t currentImageStep = 0;

extern void settingmode_setting_detailsettingbt_event_cb(lv_event_t *e);
extern void settingmode_setting_degreebt_event_cb(lv_event_t *e);
extern void settingmode_setting_recordbt_event_cb(lv_event_t *e);
extern void settingmode_setting_testbt_event_cb(lv_event_t *e);
extern void settingmode_setting_timebt_event_cb(lv_event_t *e);
extern void settingmode_setting_languagebt_event_cb(lv_event_t *e);
extern void settingmode_backbt_event_cb(lv_event_t *e);
extern void settingmode_loaded_event_cb(lv_event_t *e);
extern void settingmode_unload_start_event_cb(lv_event_t *e);
extern void settingmode_unloaded_event_cb(lv_event_t *e);
extern void settingmode_load_start_event_cb(lv_event_t *e);


void destroy_page_settingmode(bk_lv_ui_t *bk_ui)
{
    if (bk_ui == NULL) {
        return;
    }
    if (bk_ui->settingmode != NULL) {
        lv_obj_del(bk_ui->settingmode);
        bk_ui->settingmode = NULL;
    }

    currentStep = 0;
    currentImageStep = 0;
    preRenderPageState[PAGE_SETTINGMODE].isRendered = false;

    const uint32_t imageCount = preRenderPageConfig[PAGE_SETTINGMODE].preRenderImageCount;
    for(uint32_t i = 0; i < imageCount; i++)
    {
        const preRenderImageInfo_t *imageInfo = &preRenderPageConfig[PAGE_SETTINGMODE].preRenderImageInfo[i];
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

// NOTE : discontinued initialize method. check _with_step()
void init_page_settingmode(bk_lv_ui_t * bk_ui) 
{
    if (bk_ui->settingmode != NULL && lv_obj_is_valid(bk_ui->settingmode)) 
    {
        destroy_page_settingmode(bk_ui);
    }

    ui_lang_reset_settingmode_cache();

#if UI_PRENDERING_ENABLE
    bk_ui->settingmode = lv_obj_create(preRenderRoot);
    lv_obj_remove_style_all(bk_ui->settingmode);
    lv_obj_set_size(bk_ui->settingmode, 1024, 600);
    lv_obj_set_pos(bk_ui->settingmode, 0, 0);
    lv_obj_set_style_radius(bk_ui->settingmode, 0, LV_PART_MAIN);
    lv_obj_set_scrollbar_mode(bk_ui->settingmode, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_event_cb(bk_ui->settingmode, settingmode_load_start_event_cb, UI_EVENT_PAGE_SHOW_START,   NULL);
    lv_obj_add_event_cb(bk_ui->settingmode, settingmode_loaded_event_cb,     UI_EVENT_PAGE_SHOWN,       NULL);
    lv_obj_add_event_cb(bk_ui->settingmode, settingmode_unload_start_event_cb, UI_EVENT_PAGE_HIDE_START, NULL);
    lv_obj_add_event_cb(bk_ui->settingmode, settingmode_unloaded_event_cb,   UI_EVENT_PAGE_HIDDEN,     NULL);
#else
    bk_ui->settingmode = lv_obj_create(NULL);
    lv_obj_set_size(bk_ui->settingmode, 1024, 600);
    lv_obj_set_scrollbar_mode(bk_ui->settingmode, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_event_cb(bk_ui->settingmode, settingmode_load_start_event_cb, LV_EVENT_SCREEN_LOAD_START,   NULL);
    lv_obj_add_event_cb(bk_ui->settingmode, settingmode_loaded_event_cb,     LV_EVENT_SCREEN_LOADED,       NULL);
    lv_obj_add_event_cb(bk_ui->settingmode, settingmode_unload_start_event_cb, LV_EVENT_SCREEN_UNLOAD_START, NULL);
    lv_obj_add_event_cb(bk_ui->settingmode, settingmode_unloaded_event_cb,   LV_EVENT_SCREEN_UNLOADED,     NULL);
#endif /* UI_PRENDERING_ENABLE */
    bk_ui->settingmode_bg = lv_image_create(bk_ui->settingmode);
#if !UI_SETTINGMODE_COMBINED_BG_ENABLE
    _bg_set(bk_ui->settingmode_bg);
#endif
    lv_obj_set_pos(bk_ui->settingmode_bg, 0, 0);

    // ImageView: title
#if !UI_SETTINGMODE_COMBINED_BG_ENABLE
    bk_ui->settingmode_title = lv_image_create(bk_ui->settingmode);
    _img_set_src_timed(bk_ui->settingmode_title, "/images/setting_title.png");
    lv_obj_set_pos(bk_ui->settingmode_title, 0, 10);
    lv_obj_set_size(bk_ui->settingmode_title, 380, 80);
    lv_image_set_inner_align(bk_ui->settingmode_title, LV_IMAGE_ALIGN_TOP_LEFT);
#endif

    // Button: setting_detailsettingbt
    bk_ui->settingmode_setting_detailsettingbt = lv_button_create(bk_ui->settingmode);
    lv_obj_add_flag(bk_ui->settingmode_setting_detailsettingbt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->settingmode_setting_detailsettingbt, settingmode_setting_detailsettingbt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->settingmode_setting_detailsettingbt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->settingmode_setting_detailsettingbt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->settingmode_setting_detailsettingbt, 0, 0);
    lv_obj_set_pos(bk_ui->settingmode_setting_detailsettingbt, 408, 320);
    lv_obj_set_size(bk_ui->settingmode_setting_detailsettingbt, 207, 182);

    // ImageView: imageview3
#if !UI_SETTINGMODE_COMBINED_BG_ENABLE
    bk_ui->settingmode_imageview3 = lv_image_create(bk_ui->settingmode);
    _img_set_src_timed(bk_ui->settingmode_imageview3, "/images/setting_mode_detailsetting.png");
    lv_obj_set_pos(bk_ui->settingmode_imageview3, 408, 320);
    lv_obj_set_size(bk_ui->settingmode_imageview3, 207, 182);
#endif

    // Button: setting_degreebt
    bk_ui->settingmode_setting_degreebt = lv_button_create(bk_ui->settingmode);
    lv_obj_add_flag(bk_ui->settingmode_setting_degreebt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->settingmode_setting_degreebt, settingmode_setting_degreebt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->settingmode_setting_degreebt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->settingmode_setting_degreebt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->settingmode_setting_degreebt, 0, 0);
    lv_obj_set_pos(bk_ui->settingmode_setting_degreebt, 703, 120);
    lv_obj_set_size(bk_ui->settingmode_setting_degreebt, 207, 182);

    // ImageView: imageview5
#if !UI_SETTINGMODE_COMBINED_BG_ENABLE
    bk_ui->settingmode_imageview5 = lv_image_create(bk_ui->settingmode);
    _img_set_src_timed(bk_ui->settingmode_imageview5, "/images/setting_mode_degree.png");
    lv_obj_set_pos(bk_ui->settingmode_imageview5, 703, 120);
    lv_obj_set_size(bk_ui->settingmode_imageview5, 207, 182);
#endif

    // Button: setting_recordbt
    bk_ui->settingmode_setting_recordbt = lv_button_create(bk_ui->settingmode);
    lv_obj_add_flag(bk_ui->settingmode_setting_recordbt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->settingmode_setting_recordbt, settingmode_setting_recordbt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->settingmode_setting_recordbt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->settingmode_setting_recordbt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->settingmode_setting_recordbt, 0, 0);
    lv_obj_set_pos(bk_ui->settingmode_setting_recordbt, 703, 320);
    lv_obj_set_size(bk_ui->settingmode_setting_recordbt, 207, 182);

    // ImageView: imageview7
#if !UI_SETTINGMODE_COMBINED_BG_ENABLE
    bk_ui->settingmode_imageview7 = lv_image_create(bk_ui->settingmode);
    _img_set_src_timed(bk_ui->settingmode_imageview7, "/images/setting_mode_record.png");
    lv_obj_set_pos(bk_ui->settingmode_imageview7, 703, 320);
    lv_obj_set_size(bk_ui->settingmode_imageview7, 207, 182);
#endif

    // Button: setting_testbt
    bk_ui->settingmode_setting_testbt = lv_button_create(bk_ui->settingmode);
    lv_obj_add_flag(bk_ui->settingmode_setting_testbt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->settingmode_setting_testbt, settingmode_setting_testbt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->settingmode_setting_testbt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->settingmode_setting_testbt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->settingmode_setting_testbt, 0, 0);
    lv_obj_set_pos(bk_ui->settingmode_setting_testbt, 113, 320);
    lv_obj_set_size(bk_ui->settingmode_setting_testbt, 207, 182);

    // ImageView: imageview9
#if !UI_SETTINGMODE_COMBINED_BG_ENABLE
    bk_ui->settingmode_imageview9 = lv_image_create(bk_ui->settingmode);
    _img_set_src_timed(bk_ui->settingmode_imageview9, "/images/setting_mode_test.png");
    lv_obj_set_pos(bk_ui->settingmode_imageview9, 113, 320);
    lv_obj_set_size(bk_ui->settingmode_imageview9, 207, 182);
#endif

    // Button: setting_timebt
    bk_ui->settingmode_setting_timebt = lv_button_create(bk_ui->settingmode);
    lv_obj_add_flag(bk_ui->settingmode_setting_timebt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->settingmode_setting_timebt, settingmode_setting_timebt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->settingmode_setting_timebt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->settingmode_setting_timebt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->settingmode_setting_timebt, 0, 0);
    lv_obj_set_pos(bk_ui->settingmode_setting_timebt, 113, 120);
    lv_obj_set_size(bk_ui->settingmode_setting_timebt, 207, 182);

    // ImageView: imageview11
#if !UI_SETTINGMODE_COMBINED_BG_ENABLE
    bk_ui->settingmode_imageview11 = lv_image_create(bk_ui->settingmode);
    _img_set_src_timed(bk_ui->settingmode_imageview11, "/images/setting_mode_time.png");
    lv_obj_set_pos(bk_ui->settingmode_imageview11, 113, 120);
    lv_obj_set_size(bk_ui->settingmode_imageview11, 207, 182);
#endif

    // Button: setting_languagebt
    bk_ui->settingmode_setting_languagebt = lv_button_create(bk_ui->settingmode);
    lv_obj_add_flag(bk_ui->settingmode_setting_languagebt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->settingmode_setting_languagebt, settingmode_setting_languagebt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->settingmode_setting_languagebt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->settingmode_setting_languagebt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->settingmode_setting_languagebt, 0, 0);
    lv_obj_set_pos(bk_ui->settingmode_setting_languagebt, 408, 120);
    lv_obj_set_size(bk_ui->settingmode_setting_languagebt, 207, 182);

    // ImageView: imageview13
#if !UI_SETTINGMODE_COMBINED_BG_ENABLE
    bk_ui->settingmode_imageview13 = lv_image_create(bk_ui->settingmode);
    _img_set_src_timed(bk_ui->settingmode_imageview13, "/images/setting_mode_language.png");
    lv_obj_set_pos(bk_ui->settingmode_imageview13, 408, 120);
    lv_obj_set_size(bk_ui->settingmode_imageview13, 207, 182);
#endif

    // Button: backbt
    bk_ui->settingmode_backbt = lv_button_create(bk_ui->settingmode);
    lv_obj_add_flag(bk_ui->settingmode_backbt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->settingmode_backbt, settingmode_backbt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->settingmode_backbt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->settingmode_backbt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->settingmode_backbt, 0, 0);
    lv_obj_set_pos(bk_ui->settingmode_backbt, 825, 13);
    lv_obj_set_size(bk_ui->settingmode_backbt, 179, 74);

    // ImageView: imageview15
#if !UI_SETTINGMODE_COMBINED_BG_ENABLE
    bk_ui->settingmode_imageview15 = lv_image_create(bk_ui->settingmode);
    _img_set_src_timed(bk_ui->settingmode_imageview15, "/images/exit_bt.png");
    lv_obj_set_pos(bk_ui->settingmode_imageview15, 825, 13);
    lv_obj_set_size(bk_ui->settingmode_imageview15, 179, 74);
#endif

}

rendererFuncStatus_t init_page_settingmode_with_step(bk_lv_ui_t *bk_ui)
{
    static uint32_t renderStartTick = 0;
    if(preRenderPageState[PAGE_SETTINGMODE].isRendered)
    {
        return RENDERER_FUNC_DONE;
    }

    switch (currentStep)
    {
        case RENDER_STEP_CREATE_PAGE :
        {
            uint32_t stepStartTick = lv_tick_get();
            renderStartTick = stepStartTick;

            bk_printf(TAG "[RENDER][SETTINGMODE] start tick=%lu\n", (unsigned long)renderStartTick);

            ui_lang_reset_settingmode_cache();

            bk_ui->settingmode = lv_obj_create(preRenderRoot);
            lv_obj_add_flag(bk_ui->settingmode, LV_OBJ_FLAG_HIDDEN);
            lv_obj_remove_style_all(bk_ui->settingmode);
            lv_obj_set_size(bk_ui->settingmode, 1024, 600);
            lv_obj_set_pos(bk_ui->settingmode, 0, 0);
            lv_obj_set_style_bg_opa(bk_ui->settingmode, LV_OPA_TRANSP, LV_PART_MAIN);
            lv_obj_set_style_radius(bk_ui->settingmode, 0, LV_PART_MAIN);
            lv_obj_set_scrollbar_mode(bk_ui->settingmode, LV_SCROLLBAR_MODE_OFF);

            bk_printf(TAG "[RENDER][SETTINGMODE] CREATE_PAGE done elapsed=%lu ms\n", (unsigned long)lv_tick_elaps(stepStartTick));

            currentStep = RENDER_STEP_CREATE_CHILD;
            return RENDERER_FUNC_NOT_DONE;
        }
        case RENDER_STEP_CREATE_CHILD :
        {
            uint32_t stepStartTick = lv_tick_get();

            bk_printf(TAG "[RENDER][SETTINGMODE] CREATE_CHILD start\n");

            bk_ui->settingmode_bg = lv_image_create(bk_ui->settingmode);
            lv_obj_set_pos(bk_ui->settingmode_bg, 0, 0);

            // ImageView: title
            bk_ui->settingmode_title = lv_image_create(bk_ui->settingmode);
            _img_set_src_timed(bk_ui->settingmode_title, "/images/setting_title.png");
            lv_obj_set_pos(bk_ui->settingmode_title, 0, 10);
            lv_obj_set_size(bk_ui->settingmode_title, 380, 80);
            lv_image_set_inner_align(bk_ui->settingmode_title, LV_IMAGE_ALIGN_TOP_LEFT);

            // Button: setting_detailsettingbt
            bk_ui->settingmode_setting_detailsettingbt = lv_button_create(bk_ui->settingmode);
            lv_obj_add_flag(bk_ui->settingmode_setting_detailsettingbt, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_set_style_bg_opa(bk_ui->settingmode_setting_detailsettingbt, 0, 0);
            lv_obj_set_style_border_width(bk_ui->settingmode_setting_detailsettingbt, 0, 0);
            lv_obj_set_style_shadow_width(bk_ui->settingmode_setting_detailsettingbt, 0, 0);
            lv_obj_set_pos(bk_ui->settingmode_setting_detailsettingbt, 408, 320);
            lv_obj_set_size(bk_ui->settingmode_setting_detailsettingbt, 207, 182);

            // ImageView: imageview3
            bk_ui->settingmode_imageview3 = lv_image_create(bk_ui->settingmode);
            _img_set_src_timed(bk_ui->settingmode_imageview3, "/images/setting_mode_detailsetting.png");
            lv_obj_set_pos(bk_ui->settingmode_imageview3, 408, 320);
            lv_obj_set_size(bk_ui->settingmode_imageview3, 207, 182);

            // Button: setting_degreebt
            bk_ui->settingmode_setting_degreebt = lv_button_create(bk_ui->settingmode);
            lv_obj_add_flag(bk_ui->settingmode_setting_degreebt, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_set_style_bg_opa(bk_ui->settingmode_setting_degreebt, 0, 0);
            lv_obj_set_style_border_width(bk_ui->settingmode_setting_degreebt, 0, 0);
            lv_obj_set_style_shadow_width(bk_ui->settingmode_setting_degreebt, 0, 0);
            lv_obj_set_pos(bk_ui->settingmode_setting_degreebt, 703, 120);
            lv_obj_set_size(bk_ui->settingmode_setting_degreebt, 207, 182);

            // ImageView: imageview5
            bk_ui->settingmode_imageview5 = lv_image_create(bk_ui->settingmode);
            _img_set_src_timed(bk_ui->settingmode_imageview5, "/images/setting_mode_degree.png");
            lv_obj_set_pos(bk_ui->settingmode_imageview5, 703, 120);
            lv_obj_set_size(bk_ui->settingmode_imageview5, 207, 182);

            // Button: setting_recordbt
            bk_ui->settingmode_setting_recordbt = lv_button_create(bk_ui->settingmode);
            lv_obj_add_flag(bk_ui->settingmode_setting_recordbt, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_set_style_bg_opa(bk_ui->settingmode_setting_recordbt, 0, 0);
            lv_obj_set_style_border_width(bk_ui->settingmode_setting_recordbt, 0, 0);
            lv_obj_set_style_shadow_width(bk_ui->settingmode_setting_recordbt, 0, 0);
            lv_obj_set_pos(bk_ui->settingmode_setting_recordbt, 703, 320);
            lv_obj_set_size(bk_ui->settingmode_setting_recordbt, 207, 182);

            // ImageView: imageview7
            bk_ui->settingmode_imageview7 = lv_image_create(bk_ui->settingmode);
            _img_set_src_timed(bk_ui->settingmode_imageview7, "/images/setting_mode_record.png");
            lv_obj_set_pos(bk_ui->settingmode_imageview7, 703, 320);
            lv_obj_set_size(bk_ui->settingmode_imageview7, 207, 182);

            // Button: setting_testbt
            bk_ui->settingmode_setting_testbt = lv_button_create(bk_ui->settingmode);
            lv_obj_add_flag(bk_ui->settingmode_setting_testbt, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_set_style_bg_opa(bk_ui->settingmode_setting_testbt, 0, 0);
            lv_obj_set_style_border_width(bk_ui->settingmode_setting_testbt, 0, 0);
            lv_obj_set_style_shadow_width(bk_ui->settingmode_setting_testbt, 0, 0);
            lv_obj_set_pos(bk_ui->settingmode_setting_testbt, 113, 320);
            lv_obj_set_size(bk_ui->settingmode_setting_testbt, 207, 182);

            // ImageView: imageview9
            bk_ui->settingmode_imageview9 = lv_image_create(bk_ui->settingmode);
            _img_set_src_timed(bk_ui->settingmode_imageview9, "/images/setting_mode_test.png");
            lv_obj_set_pos(bk_ui->settingmode_imageview9, 113, 320);
            lv_obj_set_size(bk_ui->settingmode_imageview9, 207, 182);

            // Button: setting_timebt
            bk_ui->settingmode_setting_timebt = lv_button_create(bk_ui->settingmode);
            lv_obj_add_flag(bk_ui->settingmode_setting_timebt, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_set_style_bg_opa(bk_ui->settingmode_setting_timebt, 0, 0);
            lv_obj_set_style_border_width(bk_ui->settingmode_setting_timebt, 0, 0);
            lv_obj_set_style_shadow_width(bk_ui->settingmode_setting_timebt, 0, 0);
            lv_obj_set_pos(bk_ui->settingmode_setting_timebt, 113, 120);
            lv_obj_set_size(bk_ui->settingmode_setting_timebt, 207, 182);

            // ImageView: imageview11
            bk_ui->settingmode_imageview11 = lv_image_create(bk_ui->settingmode);
            _img_set_src_timed(bk_ui->settingmode_imageview11, "/images/setting_mode_time.png");
            lv_obj_set_pos(bk_ui->settingmode_imageview11, 113, 120);
            lv_obj_set_size(bk_ui->settingmode_imageview11, 207, 182);

            // Button: setting_languagebt
            bk_ui->settingmode_setting_languagebt = lv_button_create(bk_ui->settingmode);
            lv_obj_add_flag(bk_ui->settingmode_setting_languagebt, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_set_style_bg_opa(bk_ui->settingmode_setting_languagebt, 0, 0);
            lv_obj_set_style_border_width(bk_ui->settingmode_setting_languagebt, 0, 0);
            lv_obj_set_style_shadow_width(bk_ui->settingmode_setting_languagebt, 0, 0);
            lv_obj_set_pos(bk_ui->settingmode_setting_languagebt, 408, 120);
            lv_obj_set_size(bk_ui->settingmode_setting_languagebt, 207, 182);

            // ImageView: imageview13
            bk_ui->settingmode_imageview13 = lv_image_create(bk_ui->settingmode);
            _img_set_src_timed(bk_ui->settingmode_imageview13, "/images/setting_mode_language.png");
            lv_obj_set_pos(bk_ui->settingmode_imageview13, 408, 120);
            lv_obj_set_size(bk_ui->settingmode_imageview13, 207, 182);

            // Button: backbt
            bk_ui->settingmode_backbt = lv_button_create(bk_ui->settingmode);
            lv_obj_add_flag(bk_ui->settingmode_backbt, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_set_style_bg_opa(bk_ui->settingmode_backbt, 0, 0);
            lv_obj_set_style_border_width(bk_ui->settingmode_backbt, 0, 0);
            lv_obj_set_style_shadow_width(bk_ui->settingmode_backbt, 0, 0);
            lv_obj_set_pos(bk_ui->settingmode_backbt, 825, 13);
            lv_obj_set_size(bk_ui->settingmode_backbt, 179, 74);

            // ImageView: imageview15
            bk_ui->settingmode_imageview15 = lv_image_create(bk_ui->settingmode);
            _img_set_src_timed(bk_ui->settingmode_imageview15, "/images/exit_bt.png");
            lv_obj_set_pos(bk_ui->settingmode_imageview15, 825, 13);
            lv_obj_set_size(bk_ui->settingmode_imageview15, 179, 74);
            currentStep = RENDER_STEP_CACHE_BACKGROUND;
            return RENDERER_FUNC_NOT_DONE;
        }
        case RENDER_STEP_CACHE_BACKGROUND :
        {
            if(preRenderPageConfig[PAGE_SETTINGMODE].backgroundImageAssetId != SHARED_IMAGE_NONE)
            {
                const sharedImageAssetId_t bgImageId = preRenderPageConfig[PAGE_SETTINGMODE].backgroundImageAssetId;
                if(set_shared_image_asset(bk_ui->settingmode_bg, bgImageId) != RENDERER_FUNC_DONE)
                {
                    return RENDERER_FUNC_FAILED;
                }
            }
            currentStep = RENDER_STEP_CACHE_IMAGE;
            return RENDERER_FUNC_NOT_DONE;
        }
        case RENDER_STEP_CACHE_IMAGE :
        {
            const uint32_t imageCount = preRenderPageConfig[PAGE_SETTINGMODE].preRenderImageCount;

            bk_printf(TAG "[PREWARM][SETTINGMODE] CACHE_IMAGE start (%lu images)\n", (unsigned long)imageCount);

            if(currentImageStep < imageCount)
            {
                const preRenderImageInfo_t *imageInfo = &preRenderPageConfig[PAGE_SETTINGMODE].preRenderImageInfo[currentImageStep];
                char imagePath[128] = {0};
                const char *languageSuffix = imageInfo->hasLanguageVariant ?
                                             (settings_get_int("LANGUAGE") == 1 ? "_china" :
                                              settings_get_int("LANGUAGE") == 2 ? "_english" : "") : "";
                const char *degreeSuffix = imageInfo->hasDegreeVariant &&
                                           strcmp(settings_get_str("Degree"), "\xc2\xb0""F") == 0 ? "_f" : "";
                const char *extension = imageInfo->fileExtension != NULL ? imageInfo->fileExtension : ".png";
                uint32_t imageStartTick = lv_tick_get();
                snprintf(imagePath, sizeof(imagePath), "%s%s%s%s",
                         imageInfo->imagePath, degreeSuffix, languageSuffix, extension);

                bk_printf(TAG "[PREWARM][SETTINGMODE] image %lu/%lu start: %s\n",
                          (unsigned long)(currentImageStep + 1),
                          (unsigned long)imageCount,
                          imagePath);

                lv_result_t res = lv_image_decoder_prewarm(imagePath);
                uint32_t imageElapsed = lv_tick_elaps(imageStartTick);

                if(res != LV_RESULT_OK)
                {
                    bk_printf(TAG "[PREWARM][SETTINGMODE] image %lu/%lu FAILED res=%d elapsed=%lu ms path=%s\n",
                              (unsigned long)(currentImageStep + 1),
                              (unsigned long)imageCount,
                              (int)res,
                              (unsigned long)imageElapsed,
                              imagePath);
                    return RENDERER_FUNC_FAILED;
                }

                bk_printf(TAG "[PREWARM][SETTINGMODE] image %lu/%lu OK elapsed=%lu ms path=%s\n",
                          (unsigned long)(currentImageStep + 1),
                          (unsigned long)imageCount,
                          (unsigned long)imageElapsed,
                          imagePath);

                currentImageStep++;
                return RENDERER_FUNC_NOT_DONE;
            }

            bk_printf(TAG "[PREWARM][SETTINGMODE] all images done count=%lu total=%lu ms\n",
                      (unsigned long)imageCount,
                      (unsigned long)lv_tick_elaps(renderStartTick));

            currentImageStep = 0;
            currentStep = RENDER_STEP_ATTACH_EVENT;
            return RENDERER_FUNC_NOT_DONE;
        }
        case RENDER_STEP_ATTACH_EVENT :
        {
            uint32_t stepStartTick = lv_tick_get();
            lv_obj_add_event_cb(bk_ui->settingmode, settingmode_load_start_event_cb, UI_EVENT_PAGE_SHOW_START,   NULL);
            lv_obj_add_event_cb(bk_ui->settingmode, settingmode_loaded_event_cb,     UI_EVENT_PAGE_SHOWN,       NULL);
            lv_obj_add_event_cb(bk_ui->settingmode, settingmode_unload_start_event_cb, UI_EVENT_PAGE_HIDE_START, NULL);
            lv_obj_add_event_cb(bk_ui->settingmode, settingmode_unloaded_event_cb,   UI_EVENT_PAGE_HIDDEN,     NULL);

            lv_obj_add_event_cb(bk_ui->settingmode_setting_detailsettingbt, settingmode_setting_detailsettingbt_event_cb, LV_EVENT_PRESSED, NULL);
            lv_obj_add_event_cb(bk_ui->settingmode_setting_degreebt, settingmode_setting_degreebt_event_cb, LV_EVENT_PRESSED, NULL);
            lv_obj_add_event_cb(bk_ui->settingmode_setting_recordbt, settingmode_setting_recordbt_event_cb, LV_EVENT_PRESSED, NULL);
            lv_obj_add_event_cb(bk_ui->settingmode_setting_testbt, settingmode_setting_testbt_event_cb, LV_EVENT_PRESSED, NULL);
            lv_obj_add_event_cb(bk_ui->settingmode_setting_timebt, settingmode_setting_timebt_event_cb, LV_EVENT_PRESSED, NULL);
            lv_obj_add_event_cb(bk_ui->settingmode_setting_languagebt, settingmode_setting_languagebt_event_cb, LV_EVENT_PRESSED, NULL);
            lv_obj_add_event_cb(bk_ui->settingmode_backbt, settingmode_backbt_event_cb, LV_EVENT_PRESSED, NULL);

            bk_printf(TAG "[RENDER][SETTINGMODE] ATTACH_EVENT done elapsed=%lu ms\n",
                      (unsigned long)lv_tick_elaps(stepStartTick));

            currentStep = RENDER_STEP_DONE;
            return RENDERER_FUNC_NOT_DONE;
        }
        case RENDER_STEP_DONE :
        {
            uint32_t totalElapsed = lv_tick_elaps(renderStartTick);

            bk_printf(TAG "[RENDER][SETTINGMODE] DONE total=%lu ms\n", (unsigned long)totalElapsed);

            currentStep = 0;
            currentImageStep = 0;
            renderStartTick = 0;
            preRenderPageState[PAGE_SETTINGMODE].isRendered = true;
            return RENDERER_FUNC_DONE;
        }
        default :
        {
            bk_printf(TAG "[RENDER][SETTINGMODE] INVALID STEP: %lu\n", (unsigned long)currentStep);
            return RENDERER_FUNC_FAILED;
        }
    }
    


    return RENDERER_FUNC_FAILED;
}
