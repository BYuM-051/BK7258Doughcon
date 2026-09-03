#include "lvgl.h"
#include "beken_ui.h"
#include "custom_func.h"
#include "event_runtime.h"
#include "ui_lang.h"
#include <stdio.h>
#include <string.h>

#include "ui_config.h"
#include "settings.h"

#define TAG "[settingmodedefrost_init.c] "
#include "pageManager.h"
// #define bk_printf(fmt, ...) do {if(0) bk_printf(fmt, ##__VA_ARGS__); } while(0) // disable printf

static uint32_t currentStep = RENDER_STEP_CREATE_PAGE;
static uint32_t currentImageStep = 0;

extern bk_lv_ui_t bk_lv_tool_ui;
extern lv_obj_t *preRenderRoot;

static bool stepInitMode = false;
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

    currentStep = RENDER_STEP_CREATE_PAGE;
    currentImageStep = 0;
    preRenderPageState[PAGE_SETTINGMODEDEFROST].isRendered = false;

    const uint32_t imageCount = preRenderPageConfig[PAGE_SETTINGMODEDEFROST].preRenderImageCount;
    for(uint32_t i = 0; i < imageCount; i++)
    {
        const preRenderImageInfo_t *imageInfo = &preRenderPageConfig[PAGE_SETTINGMODEDEFROST].preRenderImageInfo[i];
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

void init_page_settingmodedefrost(bk_lv_ui_t * bk_ui) {
    if(stepInitMode && currentStep == RENDER_STEP_CREATE_CHILD)
    {
        goto create_children;
    }

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

    if(stepInitMode)
    {
        return;
    }

create_children:
    bk_ui->settingmodedefrost_bg = lv_image_create(bk_ui->settingmodedefrost);
    _bg_set(bk_ui->settingmodedefrost_bg);
    lv_obj_set_pos(bk_ui->settingmodedefrost_bg, 0, 0);

    // ImageView: title
    bk_ui->settingmodedefrost_title = lv_image_create(bk_ui->settingmodedefrost);
    _img_set_src_timed(bk_ui->settingmodedefrost_title, "/images/setting_title.png");
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
    _img_set_src_timed(bk_ui->settingmodedefrost_imageview3, "/images/exit_bt.png");
    lv_obj_set_pos(bk_ui->settingmodedefrost_imageview3, 13, 445);
    lv_obj_set_size(bk_ui->settingmodedefrost_imageview3, 179, 74);

}

rendererFuncStatus_t init_page_settingmodedefrost_with_step(bk_lv_ui_t *bk_ui)
{
    static uint32_t renderStartTick = 0;

    if(preRenderPageState[PAGE_SETTINGMODEDEFROST].isRendered)
    {
        return RENDERER_FUNC_DONE;
    }

    switch(currentStep)
    {
        case RENDER_STEP_CREATE_PAGE:
        {
            renderStartTick = lv_tick_get();
            bk_printf(TAG "[RENDER][SETTINGMODEDEFROST] start tick=%lu\n", (unsigned long)renderStartTick);

            if(bk_ui == NULL)
            {
                return RENDERER_FUNC_FAILED;
            }

            stepInitMode = true;
            init_page_settingmodedefrost(bk_ui);
            stepInitMode = false;
            if(bk_ui->settingmodedefrost == NULL || !lv_obj_is_valid(bk_ui->settingmodedefrost))
            {
                bk_printf(TAG "[RENDER][SETTINGMODEDEFROST] CREATE_PAGE failed\n");
                return RENDERER_FUNC_FAILED;
            }

#if UI_PRENDERING_ENABLE
            lv_obj_add_flag(bk_ui->settingmodedefrost, LV_OBJ_FLAG_HIDDEN);
#endif
            currentStep = RENDER_STEP_CREATE_CHILD;
            return RENDERER_FUNC_NOT_DONE;
        }

        case RENDER_STEP_CREATE_CHILD:
        {
            stepInitMode = true;
            init_page_settingmodedefrost(bk_ui);
            stepInitMode = false;
            currentStep = RENDER_STEP_CACHE_BACKGROUND;
            return RENDERER_FUNC_NOT_DONE;
        }

        case RENDER_STEP_CACHE_BACKGROUND:
        {
            if(preRenderPageConfig[PAGE_SETTINGMODEDEFROST].backgroundImageAssetId != SHARED_IMAGE_NONE)
            {
                const sharedImageAssetId_t assetId =
                    preRenderPageConfig[PAGE_SETTINGMODEDEFROST].backgroundImageAssetId;
                if(set_shared_image_asset(bk_ui->settingmodedefrost_bg, assetId) != RENDERER_FUNC_DONE)
                {
                    return RENDERER_FUNC_FAILED;
                }
            }

            currentStep = RENDER_STEP_CACHE_IMAGE;
            return RENDERER_FUNC_NOT_DONE;
        }

        case RENDER_STEP_CACHE_IMAGE:
        {
            const uint32_t imageCount = preRenderPageConfig[PAGE_SETTINGMODEDEFROST].preRenderImageCount;
            if(currentImageStep < imageCount)
            {
                const preRenderImageInfo_t *imageInfo =
                    &preRenderPageConfig[PAGE_SETTINGMODEDEFROST].preRenderImageInfo[currentImageStep];
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
                    bk_printf(TAG "[PREWARM][SETTINGMODEDEFROST] image %lu/%lu failed: %s (%lu ms)\n",
                              (unsigned long)(currentImageStep + 1),
                              (unsigned long)imageCount,
                              imagePath,
                              (unsigned long)lv_tick_elaps(imageStartTick));
                    return RENDERER_FUNC_FAILED;
                }

                bk_printf(TAG "[PREWARM][SETTINGMODEDEFROST] image %lu/%lu done: %s (%lu ms)\n",
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
            /* init_page_settingmodedefrost() also attaches the page and control callbacks. */
            currentStep = RENDER_STEP_DONE;
            return RENDERER_FUNC_NOT_DONE;
        }

        case RENDER_STEP_DONE:
        {
            bk_printf(TAG "[RENDER][SETTINGMODEDEFROST] done total=%lu ms\n",
                      (unsigned long)lv_tick_elaps(renderStartTick));

            currentStep = RENDER_STEP_CREATE_PAGE;
            currentImageStep = 0;
            renderStartTick = 0;
            preRenderPageState[PAGE_SETTINGMODEDEFROST].isRendered = true;
            return RENDERER_FUNC_DONE;
        }

        default:
        {
            bk_printf(TAG "[RENDER][SETTINGMODEDEFROST] invalid step=%lu\n",
                      (unsigned long)currentStep);
            return RENDERER_FUNC_FAILED;
        }
    }
}
