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
#include "settings.h"

#define TAG "[settingmodelanguage_init.c] "
#include "preRenderer.h"
// #define bk_printf(fmt, ...) do {if(0) bk_printf(fmt, ##__VA_ARGS__); } while(0) // disable printf

static uint32_t currentStep = RENDER_STEP_CREATE_PAGE;
static uint32_t currentImageStep = 0;

extern bk_lv_ui_t bk_lv_tool_ui;
extern lv_obj_t *preRenderRoot;

static bool stepInitMode = false;
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

    currentStep = RENDER_STEP_CREATE_PAGE;
    currentImageStep = 0;
    preRenderPageState[PAGE_SETTINGMODELANGUAGE].isRendered = false;

    const uint32_t imageCount = preRenderPageConfig[PAGE_SETTINGMODELANGUAGE].preRenderImageCount;
    for(uint32_t i = 0; i < imageCount; i++)
    {
        const preRenderImageInfo_t *imageInfo = &preRenderPageConfig[PAGE_SETTINGMODELANGUAGE].preRenderImageInfo[i];
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

void init_page_settingmodelanguage(bk_lv_ui_t * bk_ui) {
    if(stepInitMode && currentStep == RENDER_STEP_CREATE_CHILD)
    {
        goto create_children;
    }

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

    if(stepInitMode)
    {
        return;
    }

create_children:
    /* 배경 — bg.jpg 대신 단색(0xd9d9d9) */
    bk_ui->settingmodelanguage_bg = lv_image_create(bk_ui->settingmodelanguage);
    lv_obj_add_flag(bk_ui->settingmodelanguage_bg, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_style_bg_color(bk_ui->settingmodelanguage, lv_color_hex(0xd9d9d9), 0);
    lv_obj_set_style_bg_opa(bk_ui->settingmodelanguage, LV_OPA_COVER, 0);
    lv_obj_set_pos(bk_ui->settingmodelanguage_bg, 0, 0);

    // ImageView: title
    bk_ui->settingmodelanguage_title = lv_image_create(bk_ui->settingmodelanguage);
    _img_set_src_timed(bk_ui->settingmodelanguage_title, "/images/language_title.png");
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
    _img_set_src_timed(bk_ui->settingmodelanguage_exitim, "/images/exit_bt.png");
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
    _img_set_src_timed(bk_ui->settingmodelanguage_koreanim, "/images/language_korean_off.png");
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
    _img_set_src_timed(bk_ui->settingmodelanguage_englishim, "/images/language_english_off.png");
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
    _img_set_src_timed(bk_ui->settingmodelanguage_chinaim, "/images/language_china_off.png");
    lv_obj_set_pos(bk_ui->settingmodelanguage_chinaim, 19, 192);
    lv_obj_set_size(bk_ui->settingmodelanguage_chinaim, 296, 156);
    _update_language_ui();

}

rendererFuncStatus_t init_page_settingmodelanguage_with_step(bk_lv_ui_t *bk_ui)
{
    static uint32_t renderStartTick = 0;

    if(preRenderPageState[PAGE_SETTINGMODELANGUAGE].isRendered)
    {
        return RENDERER_FUNC_DONE;
    }

    switch(currentStep)
    {
        case RENDER_STEP_CREATE_PAGE:
        {
            renderStartTick = lv_tick_get();
            bk_printf(TAG "[RENDER][SETTINGMODELANGUAGE] start tick=%lu\n", (unsigned long)renderStartTick);

            if(bk_ui == NULL)
            {
                return RENDERER_FUNC_FAILED;
            }

            stepInitMode = true;
            init_page_settingmodelanguage(bk_ui);
            stepInitMode = false;
            if(bk_ui->settingmodelanguage == NULL || !lv_obj_is_valid(bk_ui->settingmodelanguage))
            {
                bk_printf(TAG "[RENDER][SETTINGMODELANGUAGE] CREATE_PAGE failed\n");
                return RENDERER_FUNC_FAILED;
            }

#if UI_PRENDERING_ENABLE
            lv_obj_add_flag(bk_ui->settingmodelanguage, LV_OBJ_FLAG_HIDDEN);
#endif
            currentStep = RENDER_STEP_CREATE_CHILD;
            return RENDERER_FUNC_NOT_DONE;
        }

        case RENDER_STEP_CREATE_CHILD:
        {
            stepInitMode = true;
            init_page_settingmodelanguage(bk_ui);
            stepInitMode = false;
            currentStep = RENDER_STEP_CACHE_BACKGROUND;
            return RENDERER_FUNC_NOT_DONE;
        }

        case RENDER_STEP_CACHE_BACKGROUND:
        {
            if(preRenderPageConfig[PAGE_SETTINGMODELANGUAGE].backgroundImageAssetId != SHARED_IMAGE_NONE)
            {
                const sharedImageAssetId_t assetId =
                    preRenderPageConfig[PAGE_SETTINGMODELANGUAGE].backgroundImageAssetId;
                if(set_shared_image_asset(bk_ui->settingmodelanguage_bg, assetId) != RENDERER_FUNC_DONE)
                {
                    return RENDERER_FUNC_FAILED;
                }
            }

            currentStep = RENDER_STEP_CACHE_IMAGE;
            return RENDERER_FUNC_NOT_DONE;
        }

        case RENDER_STEP_CACHE_IMAGE:
        {
            const uint32_t imageCount = preRenderPageConfig[PAGE_SETTINGMODELANGUAGE].preRenderImageCount;
            if(currentImageStep < imageCount)
            {
                const preRenderImageInfo_t *imageInfo =
                    &preRenderPageConfig[PAGE_SETTINGMODELANGUAGE].preRenderImageInfo[currentImageStep];
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
                    bk_printf(TAG "[PREWARM][SETTINGMODELANGUAGE] image %lu/%lu failed: %s (%lu ms)\n",
                              (unsigned long)(currentImageStep + 1),
                              (unsigned long)imageCount,
                              imagePath,
                              (unsigned long)lv_tick_elaps(imageStartTick));
                    return RENDERER_FUNC_FAILED;
                }

                bk_printf(TAG "[PREWARM][SETTINGMODELANGUAGE] image %lu/%lu done: %s (%lu ms)\n",
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
            /* init_page_settingmodelanguage() also attaches the page and control callbacks. */
            currentStep = RENDER_STEP_DONE;
            return RENDERER_FUNC_NOT_DONE;
        }

        case RENDER_STEP_DONE:
        {
            bk_printf(TAG "[RENDER][SETTINGMODELANGUAGE] done total=%lu ms\n",
                      (unsigned long)lv_tick_elaps(renderStartTick));

            currentStep = RENDER_STEP_CREATE_PAGE;
            currentImageStep = 0;
            renderStartTick = 0;
            preRenderPageState[PAGE_SETTINGMODELANGUAGE].isRendered = true;
            return RENDERER_FUNC_DONE;
        }

        default:
        {
            bk_printf(TAG "[RENDER][SETTINGMODELANGUAGE] invalid step=%lu\n",
                      (unsigned long)currentStep);
            return RENDERER_FUNC_FAILED;
        }
    }
}
