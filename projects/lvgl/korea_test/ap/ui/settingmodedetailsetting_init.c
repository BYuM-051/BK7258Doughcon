#include "lvgl.h"
#include "beken_ui.h"
#include "custom_func.h"
#include "event_runtime.h"
#include "ui_lang.h"
#include <stdio.h>
#include <string.h>

#include "ui_config.h"
#include "settings.h"

#define TAG "[settingmodedetailsetting_init.c] "
#include "pageManager.h"
// #define bk_printf(fmt, ...) do {if(0) bk_printf(fmt, ##__VA_ARGS__); } while(0) // disable printf

static uint32_t currentStep = RENDER_STEP_CREATE_PAGE;
static uint32_t currentImageStep = 0;

extern bk_lv_ui_t bk_lv_tool_ui;
extern lv_obj_t *preRenderRoot;

static bool stepInitMode = false;
extern void settingmodedetailsetting_backbt_event_cb(lv_event_t *e);
extern void settingmodedetailsetting_detail_temp_bt_event_cb(lv_event_t *e);
extern void settingmodedetailsetting_detail_humidity_bt_event_cb(lv_event_t *e);
extern void settingmodedetailsetting_detail_time_bt_event_cb(lv_event_t *e);
extern void settingmodedetailsetting_detail_damper_bt_event_cb(lv_event_t *e);
extern void settingmodedetailsetting_detail_defrost_bt_event_cb(lv_event_t *e);
extern void settingmodedetailsetting_detail_reset_bt_event_cb(lv_event_t *e);
extern void settingmodedetailsetting_load_start_event_cb(lv_event_t *e);
extern void settingmodedetailsetting_loaded_event_cb(lv_event_t *e);
extern void settingmodedetailsetting_unloaded_event_cb(lv_event_t *e);
extern void settingmodedetailsetting_unload_start_event_cb(lv_event_t *e);

void destroy_page_settingmodedetailsetting(bk_lv_ui_t *bk_ui)
{
    if (bk_ui == NULL) {
        return;
    }
    if (bk_ui->settingmodedetailsetting != NULL) {
        lv_obj_del(bk_ui->settingmodedetailsetting);
        bk_ui->settingmodedetailsetting = NULL;
    }

    currentStep = RENDER_STEP_CREATE_PAGE;
    currentImageStep = 0;
    preRenderPageState[PAGE_SETTINGMODEDETAILSETTING].isRendered = false;

    const uint32_t imageCount = preRenderPageConfig[PAGE_SETTINGMODEDETAILSETTING].preRenderImageCount;
    for(uint32_t i = 0; i < imageCount; i++)
    {
        const preRenderImageInfo_t *imageInfo = &preRenderPageConfig[PAGE_SETTINGMODEDETAILSETTING].preRenderImageInfo[i];
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

void init_page_settingmodedetailsetting(bk_lv_ui_t * bk_ui) {
    if(stepInitMode && currentStep == RENDER_STEP_CREATE_CHILD)
    {
        goto create_children;
    }

    if (bk_ui->settingmodedetailsetting != NULL && lv_obj_is_valid(bk_ui->settingmodedetailsetting)) {
        destroy_page_settingmodedetailsetting(bk_ui);
    }

    ui_lang_reset_settingmodedetailsetting_cache();

#if UI_PRENDERING_ENABLE
    bk_ui->settingmodedetailsetting = lv_obj_create(preRenderRoot);
    lv_obj_remove_style_all(bk_ui->settingmodedetailsetting);
    lv_obj_set_size(bk_ui->settingmodedetailsetting, 1024, 600);
    lv_obj_set_pos(bk_ui->settingmodedetailsetting, 0, 0);
    lv_obj_set_style_radius(bk_ui->settingmodedetailsetting, 0, LV_PART_MAIN);
    lv_obj_set_scrollbar_mode(bk_ui->settingmodedetailsetting, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_event_cb(bk_ui->settingmodedetailsetting, settingmodedetailsetting_load_start_event_cb, UI_EVENT_PAGE_SHOW_START,   NULL);
    lv_obj_add_event_cb(bk_ui->settingmodedetailsetting, settingmodedetailsetting_loaded_event_cb, UI_EVENT_PAGE_SHOWN,       NULL);
    lv_obj_add_event_cb(bk_ui->settingmodedetailsetting, settingmodedetailsetting_unloaded_event_cb, UI_EVENT_PAGE_HIDDEN, NULL);
    lv_obj_add_event_cb(bk_ui->settingmodedetailsetting, settingmodedetailsetting_unload_start_event_cb, UI_EVENT_PAGE_HIDE_START, NULL);
#else
    bk_ui->settingmodedetailsetting = lv_obj_create(NULL);
    lv_obj_set_size(bk_ui->settingmodedetailsetting, 1024, 600);
    lv_obj_set_scrollbar_mode(bk_ui->settingmodedetailsetting, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_event_cb(bk_ui->settingmodedetailsetting, settingmodedetailsetting_load_start_event_cb, LV_EVENT_SCREEN_LOAD_START,   NULL);
    lv_obj_add_event_cb(bk_ui->settingmodedetailsetting, settingmodedetailsetting_loaded_event_cb, LV_EVENT_SCREEN_LOADED,       NULL);
    lv_obj_add_event_cb(bk_ui->settingmodedetailsetting, settingmodedetailsetting_unloaded_event_cb, LV_EVENT_SCREEN_UNLOADED, NULL);
    lv_obj_add_event_cb(bk_ui->settingmodedetailsetting, settingmodedetailsetting_unload_start_event_cb, LV_EVENT_SCREEN_UNLOAD_START, NULL);
#endif /* UI_PRENDERING_ENABLE */

    if(stepInitMode)
    {
        return;
    }

create_children:
    bk_ui->settingmodedetailsetting_bg = lv_image_create(bk_ui->settingmodedetailsetting);
#if !UI_SETTINGMODEDETAILSETTING_COMBINED_BG_ENABLE
    _bg_set(bk_ui->settingmodedetailsetting_bg);
#endif
    lv_obj_set_pos(bk_ui->settingmodedetailsetting_bg, 0, 0);

    // ImageView: title
#if !UI_SETTINGMODEDETAILSETTING_COMBINED_BG_ENABLE
    bk_ui->settingmodedetailsetting_title = lv_image_create(bk_ui->settingmodedetailsetting);
    _img_set_src_timed(bk_ui->settingmodedetailsetting_title, "/images/detail_title.png");
    lv_obj_set_pos(bk_ui->settingmodedetailsetting_title, 0, 10);
    lv_obj_set_size(bk_ui->settingmodedetailsetting_title, 380, 80);
    lv_image_set_inner_align(bk_ui->settingmodedetailsetting_title, LV_IMAGE_ALIGN_TOP_LEFT);
#endif

    // Button: backbt
    bk_ui->settingmodedetailsetting_backbt = lv_button_create(bk_ui->settingmodedetailsetting);
    lv_obj_add_flag(bk_ui->settingmodedetailsetting_backbt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->settingmodedetailsetting_backbt, settingmodedetailsetting_backbt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->settingmodedetailsetting_backbt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->settingmodedetailsetting_backbt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->settingmodedetailsetting_backbt, 0, 0);
    lv_obj_set_pos(bk_ui->settingmodedetailsetting_backbt, 825, 13);
    lv_obj_set_size(bk_ui->settingmodedetailsetting_backbt, 179, 74);

    // ImageView: imageview3
#if !UI_SETTINGMODEDETAILSETTING_COMBINED_BG_ENABLE
    bk_ui->settingmodedetailsetting_imageview3 = lv_image_create(bk_ui->settingmodedetailsetting);
    _img_set_src_timed(bk_ui->settingmodedetailsetting_imageview3, "/images/exit_bt.png");
    lv_obj_set_pos(bk_ui->settingmodedetailsetting_imageview3, 825, 13);
    lv_obj_set_size(bk_ui->settingmodedetailsetting_imageview3, 179, 74);
#endif

    // ImageView: imageview4
#if !UI_SETTINGMODEDETAILSETTING_COMBINED_BG_ENABLE
    bk_ui->settingmodedetailsetting_imageview4 = lv_image_create(bk_ui->settingmodedetailsetting);
    _img_set_src_timed(bk_ui->settingmodedetailsetting_imageview4, "/images/detail_temp_bt.png");
    lv_obj_set_pos(bk_ui->settingmodedetailsetting_imageview4, 16, 175);
    lv_obj_set_size(bk_ui->settingmodedetailsetting_imageview4, 176, 231);
#endif

    // Button: detail_temp_bt
    bk_ui->settingmodedetailsetting_detail_temp_bt = lv_button_create(bk_ui->settingmodedetailsetting);
    lv_obj_add_flag(bk_ui->settingmodedetailsetting_detail_temp_bt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->settingmodedetailsetting_detail_temp_bt, settingmodedetailsetting_detail_temp_bt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->settingmodedetailsetting_detail_temp_bt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->settingmodedetailsetting_detail_temp_bt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->settingmodedetailsetting_detail_temp_bt, 0, 0);
    lv_obj_set_pos(bk_ui->settingmodedetailsetting_detail_temp_bt, 16, 175);
    lv_obj_set_size(bk_ui->settingmodedetailsetting_detail_temp_bt, 176, 231);

    // ImageView: imageview6
#if !UI_SETTINGMODEDETAILSETTING_COMBINED_BG_ENABLE
    bk_ui->settingmodedetailsetting_imageview6 = lv_image_create(bk_ui->settingmodedetailsetting);
    _img_set_src_timed(bk_ui->settingmodedetailsetting_imageview6, "/images/detail_humidity_bt.png");
    lv_obj_set_pos(bk_ui->settingmodedetailsetting_imageview6, 220, 175);
    lv_obj_set_size(bk_ui->settingmodedetailsetting_imageview6, 176, 231);
#endif

    // Button: detail_humidity_bt
    bk_ui->settingmodedetailsetting_detail_humidity_bt = lv_button_create(bk_ui->settingmodedetailsetting);
    lv_obj_add_flag(bk_ui->settingmodedetailsetting_detail_humidity_bt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->settingmodedetailsetting_detail_humidity_bt, settingmodedetailsetting_detail_humidity_bt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->settingmodedetailsetting_detail_humidity_bt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->settingmodedetailsetting_detail_humidity_bt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->settingmodedetailsetting_detail_humidity_bt, 0, 0);
    lv_obj_set_pos(bk_ui->settingmodedetailsetting_detail_humidity_bt, 220, 175);
    lv_obj_set_size(bk_ui->settingmodedetailsetting_detail_humidity_bt, 176, 231);

    // ImageView: imageview8
#if !UI_SETTINGMODEDETAILSETTING_COMBINED_BG_ENABLE
    bk_ui->settingmodedetailsetting_imageview8 = lv_image_create(bk_ui->settingmodedetailsetting);
    _img_set_src_timed(bk_ui->settingmodedetailsetting_imageview8, "/images/detail_time_bt.png");
    lv_obj_set_pos(bk_ui->settingmodedetailsetting_imageview8, 424, 175);
    lv_obj_set_size(bk_ui->settingmodedetailsetting_imageview8, 176, 231);
#endif

    // Button: detail_time_bt
    bk_ui->settingmodedetailsetting_detail_time_bt = lv_button_create(bk_ui->settingmodedetailsetting);
    lv_obj_add_flag(bk_ui->settingmodedetailsetting_detail_time_bt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->settingmodedetailsetting_detail_time_bt, settingmodedetailsetting_detail_time_bt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->settingmodedetailsetting_detail_time_bt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->settingmodedetailsetting_detail_time_bt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->settingmodedetailsetting_detail_time_bt, 0, 0);
    lv_obj_set_pos(bk_ui->settingmodedetailsetting_detail_time_bt, 424, 175);
    lv_obj_set_size(bk_ui->settingmodedetailsetting_detail_time_bt, 176, 231);

    // ImageView: imageview10
#if !UI_SETTINGMODEDETAILSETTING_COMBINED_BG_ENABLE
    bk_ui->settingmodedetailsetting_imageview10 = lv_image_create(bk_ui->settingmodedetailsetting);
    _img_set_src_timed(bk_ui->settingmodedetailsetting_imageview10, "/images/detail_damper_bt.png");
    lv_obj_set_pos(bk_ui->settingmodedetailsetting_imageview10, 628, 175);
    lv_obj_set_size(bk_ui->settingmodedetailsetting_imageview10, 176, 231);
#endif

    // Button: detail_damper_bt
    bk_ui->settingmodedetailsetting_detail_damper_bt = lv_button_create(bk_ui->settingmodedetailsetting);
    lv_obj_add_flag(bk_ui->settingmodedetailsetting_detail_damper_bt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->settingmodedetailsetting_detail_damper_bt, settingmodedetailsetting_detail_damper_bt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->settingmodedetailsetting_detail_damper_bt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->settingmodedetailsetting_detail_damper_bt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->settingmodedetailsetting_detail_damper_bt, 0, 0);
    lv_obj_set_pos(bk_ui->settingmodedetailsetting_detail_damper_bt, 628, 175);
    lv_obj_set_size(bk_ui->settingmodedetailsetting_detail_damper_bt, 176, 231);

    // ImageView: imageview12
#if !UI_SETTINGMODEDETAILSETTING_COMBINED_BG_ENABLE
    bk_ui->settingmodedetailsetting_imageview12 = lv_image_create(bk_ui->settingmodedetailsetting);
    _img_set_src_timed(bk_ui->settingmodedetailsetting_imageview12, "/images/detail_defrost_bt.png");
    lv_obj_set_pos(bk_ui->settingmodedetailsetting_imageview12, 832, 175);
    lv_obj_set_size(bk_ui->settingmodedetailsetting_imageview12, 176, 231);
#endif

    // Button: detail_defrost_bt
    bk_ui->settingmodedetailsetting_detail_defrost_bt = lv_button_create(bk_ui->settingmodedetailsetting);
    lv_obj_add_flag(bk_ui->settingmodedetailsetting_detail_defrost_bt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->settingmodedetailsetting_detail_defrost_bt, settingmodedetailsetting_detail_defrost_bt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->settingmodedetailsetting_detail_defrost_bt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->settingmodedetailsetting_detail_defrost_bt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->settingmodedetailsetting_detail_defrost_bt, 0, 0);
    lv_obj_set_pos(bk_ui->settingmodedetailsetting_detail_defrost_bt, 832, 175);
    lv_obj_set_size(bk_ui->settingmodedetailsetting_detail_defrost_bt, 176, 231);

    // ImageView: imageview14
#if !UI_SETTINGMODEDETAILSETTING_COMBINED_BG_ENABLE
    bk_ui->settingmodedetailsetting_imageview14 = lv_image_create(bk_ui->settingmodedetailsetting);
    _img_set_src_timed(bk_ui->settingmodedetailsetting_imageview14, "/images/detail_reset_bt.png");
    lv_obj_set_pos(bk_ui->settingmodedetailsetting_imageview14, 825, 431);
    lv_obj_set_size(bk_ui->settingmodedetailsetting_imageview14, 179, 74);
#endif

    // Button: detail_reset_bt
    bk_ui->settingmodedetailsetting_detail_reset_bt = lv_button_create(bk_ui->settingmodedetailsetting);
    lv_obj_add_flag(bk_ui->settingmodedetailsetting_detail_reset_bt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->settingmodedetailsetting_detail_reset_bt, settingmodedetailsetting_detail_reset_bt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->settingmodedetailsetting_detail_reset_bt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->settingmodedetailsetting_detail_reset_bt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->settingmodedetailsetting_detail_reset_bt, 0, 0);
    lv_obj_set_pos(bk_ui->settingmodedetailsetting_detail_reset_bt, 825, 431);
    lv_obj_set_size(bk_ui->settingmodedetailsetting_detail_reset_bt, 179, 74);

}

rendererFuncStatus_t init_page_settingmodedetailsetting_with_step(bk_lv_ui_t *bk_ui)
{
    static uint32_t renderStartTick = 0;

    if(preRenderPageState[PAGE_SETTINGMODEDETAILSETTING].isRendered)
    {
        return RENDERER_FUNC_DONE;
    }

    switch(currentStep)
    {
        case RENDER_STEP_CREATE_PAGE:
        {
            renderStartTick = lv_tick_get();
            bk_printf(TAG "[RENDER][SETTINGMODEDETAILSETTING] start tick=%lu\n", (unsigned long)renderStartTick);

            if(bk_ui == NULL)
            {
                return RENDERER_FUNC_FAILED;
            }

            stepInitMode = true;
            init_page_settingmodedetailsetting(bk_ui);
            stepInitMode = false;
            if(bk_ui->settingmodedetailsetting == NULL || !lv_obj_is_valid(bk_ui->settingmodedetailsetting))
            {
                bk_printf(TAG "[RENDER][SETTINGMODEDETAILSETTING] CREATE_PAGE failed\n");
                return RENDERER_FUNC_FAILED;
            }

#if UI_PRENDERING_ENABLE
            lv_obj_add_flag(bk_ui->settingmodedetailsetting, LV_OBJ_FLAG_HIDDEN);
#endif
            currentStep = RENDER_STEP_CREATE_CHILD;
            return RENDERER_FUNC_NOT_DONE;
        }

        case RENDER_STEP_CREATE_CHILD:
        {
            stepInitMode = true;
            init_page_settingmodedetailsetting(bk_ui);
            stepInitMode = false;
            currentStep = RENDER_STEP_CACHE_BACKGROUND;
            return RENDERER_FUNC_NOT_DONE;
        }

        case RENDER_STEP_CACHE_BACKGROUND:
        {
            if(preRenderPageConfig[PAGE_SETTINGMODEDETAILSETTING].backgroundImageAssetId != SHARED_IMAGE_NONE)
            {
                const sharedImageAssetId_t assetId =
                    preRenderPageConfig[PAGE_SETTINGMODEDETAILSETTING].backgroundImageAssetId;
                if(set_shared_image_asset(bk_ui->settingmodedetailsetting_bg, assetId) != RENDERER_FUNC_DONE)
                {
                    return RENDERER_FUNC_FAILED;
                }
            }

            currentStep = RENDER_STEP_CACHE_IMAGE;
            return RENDERER_FUNC_NOT_DONE;
        }

        case RENDER_STEP_CACHE_IMAGE:
        {
            const uint32_t imageCount = preRenderPageConfig[PAGE_SETTINGMODEDETAILSETTING].preRenderImageCount;
            if(currentImageStep < imageCount)
            {
                const preRenderImageInfo_t *imageInfo =
                    &preRenderPageConfig[PAGE_SETTINGMODEDETAILSETTING].preRenderImageInfo[currentImageStep];
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
                    bk_printf(TAG "[PREWARM][SETTINGMODEDETAILSETTING] image %lu/%lu failed: %s (%lu ms)\n",
                              (unsigned long)(currentImageStep + 1),
                              (unsigned long)imageCount,
                              imagePath,
                              (unsigned long)lv_tick_elaps(imageStartTick));
                    return RENDERER_FUNC_FAILED;
                }

                bk_printf(TAG "[PREWARM][SETTINGMODEDETAILSETTING] image %lu/%lu done: %s (%lu ms)\n",
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
            /* init_page_settingmodedetailsetting() also attaches the page and control callbacks. */
            currentStep = RENDER_STEP_DONE;
            return RENDERER_FUNC_NOT_DONE;
        }

        case RENDER_STEP_DONE:
        {
            bk_printf(TAG "[RENDER][SETTINGMODEDETAILSETTING] done total=%lu ms\n",
                      (unsigned long)lv_tick_elaps(renderStartTick));

            currentStep = RENDER_STEP_CREATE_PAGE;
            currentImageStep = 0;
            renderStartTick = 0;
            preRenderPageState[PAGE_SETTINGMODEDETAILSETTING].isRendered = true;
            return RENDERER_FUNC_DONE;
        }

        default:
        {
            bk_printf(TAG "[RENDER][SETTINGMODEDETAILSETTING] invalid step=%lu\n",
                      (unsigned long)currentStep);
            return RENDERER_FUNC_FAILED;
        }
    }
}
