#include "lvgl.h"
#include "beken_ui.h"
#include "custom_func.h"
#include "event_runtime.h"
#include "ui_lang.h"
#include <stdio.h>
#include <string.h>

#include "ui_config.h"
#include "settings.h"

#define TAG "[settingmodedegree_init.c] "
#include "pageManager.h"
// #define bk_printf(fmt, ...) do {if(0) bk_printf(fmt, ##__VA_ARGS__); } while(0) // disable printf

extern bk_lv_ui_t bk_lv_tool_ui;
extern lv_obj_t *preRenderRoot;
static uint32_t currentStep = RENDER_STEP_CREATE_PAGE;
static uint32_t currentImageStep = 0;
static bool stepInitMode = false;
extern void settingmodedegree_backbt_event_cb(lv_event_t *e);
extern void settingmodedegree_degree_c_bt_event_cb(lv_event_t *e);
extern void settingmodedegree_degree_f_bt_event_cb(lv_event_t *e);
extern void settingmodedegree_load_start_event_cb(lv_event_t *e);
extern void settingmodedegree_loaded_event_cb(lv_event_t *e);
extern void settingmodedegree_unload_start_event_cb(lv_event_t *e);
extern void settingmodedegree_unloaded_event_cb(lv_event_t *e);

void destroy_page_settingmodedegree(bk_lv_ui_t *bk_ui)
{
    if (bk_ui == NULL) {
        return;
    }
    if (bk_ui->settingmodedegree != NULL) {
        lv_obj_del(bk_ui->settingmodedegree);
        bk_ui->settingmodedegree = NULL;
    }

    currentStep = RENDER_STEP_CREATE_PAGE;
    currentImageStep = 0;
    preRenderPageState[PAGE_SETTINGMODEDEGREE].isRendered = false;

    const uint32_t imageCount = preRenderPageConfig[PAGE_SETTINGMODEDEGREE].preRenderImageCount;
    for(uint32_t i = 0; i < imageCount; i++)
    {
        const preRenderImageInfo_t *imageInfo = &preRenderPageConfig[PAGE_SETTINGMODEDEGREE].preRenderImageInfo[i];
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

void init_page_settingmodedegree(bk_lv_ui_t * bk_ui) {
    if(stepInitMode && currentStep == RENDER_STEP_CREATE_CHILD)
    {
        goto create_children;
    }

    if (bk_ui->settingmodedegree != NULL && lv_obj_is_valid(bk_ui->settingmodedegree)) {
        destroy_page_settingmodedegree(bk_ui);
    }

    ui_lang_reset_settingmodedegree_cache();

#if UI_PRENDERING_ENABLE
    bk_ui->settingmodedegree = lv_obj_create(preRenderRoot);
    lv_obj_remove_style_all(bk_ui->settingmodedegree);
    lv_obj_set_size(bk_ui->settingmodedegree, 1024, 600);
    lv_obj_set_pos(bk_ui->settingmodedegree, 0, 0);
    lv_obj_set_style_radius(bk_ui->settingmodedegree, 0, LV_PART_MAIN);
    lv_obj_set_scrollbar_mode(bk_ui->settingmodedegree, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_event_cb(bk_ui->settingmodedegree, settingmodedegree_load_start_event_cb, UI_EVENT_PAGE_SHOW_START, NULL);
    lv_obj_add_event_cb(bk_ui->settingmodedegree, settingmodedegree_loaded_event_cb, UI_EVENT_PAGE_SHOWN,     NULL);
    lv_obj_add_event_cb(bk_ui->settingmodedegree, settingmodedegree_unload_start_event_cb, UI_EVENT_PAGE_HIDE_START, NULL);
    lv_obj_add_event_cb(bk_ui->settingmodedegree, settingmodedegree_unloaded_event_cb, UI_EVENT_PAGE_HIDDEN,     NULL);
#else
    bk_ui->settingmodedegree = lv_obj_create(NULL);
    lv_obj_set_size(bk_ui->settingmodedegree, 1024, 600);
    lv_obj_set_scrollbar_mode(bk_ui->settingmodedegree, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_event_cb(bk_ui->settingmodedegree, settingmodedegree_load_start_event_cb, LV_EVENT_SCREEN_LOAD_START, NULL);
    lv_obj_add_event_cb(bk_ui->settingmodedegree, settingmodedegree_loaded_event_cb, LV_EVENT_SCREEN_LOADED,     NULL);
    lv_obj_add_event_cb(bk_ui->settingmodedegree, settingmodedegree_unload_start_event_cb, LV_EVENT_SCREEN_UNLOAD_START, NULL);
    lv_obj_add_event_cb(bk_ui->settingmodedegree, settingmodedegree_unloaded_event_cb, LV_EVENT_SCREEN_UNLOADED,     NULL);
#endif /* UI_PRENDERING_ENABLE */

    if(stepInitMode)
    {
        return;
    }

create_children:
    /* 배경 — bg.jpg 대신 단색(0xd9d9d9) */
    bk_ui->settingmodedegree_bg = lv_image_create(bk_ui->settingmodedegree);
    lv_obj_add_flag(bk_ui->settingmodedegree_bg, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_style_bg_color(bk_ui->settingmodedegree, lv_color_hex(0xd9d9d9), 0);
    lv_obj_set_style_bg_opa(bk_ui->settingmodedegree, LV_OPA_COVER, 0);
    lv_obj_set_pos(bk_ui->settingmodedegree_bg, 0, 0);

    // ImageView: title
    bk_ui->settingmodedegree_title = lv_image_create(bk_ui->settingmodedegree);
    _img_set_src_timed(bk_ui->settingmodedegree_title, "/images/symbol_title.png");
    lv_obj_set_pos(bk_ui->settingmodedegree_title, 0, 10);
    lv_obj_set_size(bk_ui->settingmodedegree_title, 380, 80);
    lv_image_set_inner_align(bk_ui->settingmodedegree_title, LV_IMAGE_ALIGN_TOP_LEFT);

    // Button: backbt
    bk_ui->settingmodedegree_backbt = lv_button_create(bk_ui->settingmodedegree);
    lv_obj_add_flag(bk_ui->settingmodedegree_backbt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->settingmodedegree_backbt, settingmodedegree_backbt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->settingmodedegree_backbt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->settingmodedegree_backbt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->settingmodedegree_backbt, 0, 0);
    lv_obj_set_pos(bk_ui->settingmodedegree_backbt, 825, 13);
    lv_obj_set_size(bk_ui->settingmodedegree_backbt, 179, 74);

    // ImageView: imageview3
    bk_ui->settingmodedegree_imageview3 = lv_image_create(bk_ui->settingmodedegree);
    _img_set_src_timed(bk_ui->settingmodedegree_imageview3, "/images/exit_bt.png");
    lv_obj_set_pos(bk_ui->settingmodedegree_imageview3, 825, 13);
    lv_obj_set_size(bk_ui->settingmodedegree_imageview3, 179, 74);

    // Button: degree_c_bt
    bk_ui->settingmodedegree_degree_c_bt = lv_button_create(bk_ui->settingmodedegree);
    lv_obj_add_flag(bk_ui->settingmodedegree_degree_c_bt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->settingmodedegree_degree_c_bt, settingmodedegree_degree_c_bt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->settingmodedegree_degree_c_bt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->settingmodedegree_degree_c_bt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->settingmodedegree_degree_c_bt, 0, 0);
    lv_obj_set_pos(bk_ui->settingmodedegree_degree_c_bt, 148, 192);
    lv_obj_set_size(bk_ui->settingmodedegree_degree_c_bt, 296, 156);

    // ImageView: degree_c_im
    bk_ui->settingmodedegree_degree_c_im = lv_image_create(bk_ui->settingmodedegree);
    _img_set_src_timed(bk_ui->settingmodedegree_degree_c_im, "/images/degree_bt_c_off.png");
    lv_obj_set_pos(bk_ui->settingmodedegree_degree_c_im, 148, 192);
    lv_obj_set_size(bk_ui->settingmodedegree_degree_c_im, 296, 156);

    // Button: degree_f_bt
    bk_ui->settingmodedegree_degree_f_bt = lv_button_create(bk_ui->settingmodedegree);
    lv_obj_add_flag(bk_ui->settingmodedegree_degree_f_bt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->settingmodedegree_degree_f_bt, settingmodedegree_degree_f_bt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->settingmodedegree_degree_f_bt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->settingmodedegree_degree_f_bt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->settingmodedegree_degree_f_bt, 0, 0);
    lv_obj_set_pos(bk_ui->settingmodedegree_degree_f_bt, 580, 192);
    lv_obj_set_size(bk_ui->settingmodedegree_degree_f_bt, 296, 156);

    // ImageView: degree_f_im
    bk_ui->settingmodedegree_degree_f_im = lv_image_create(bk_ui->settingmodedegree);
    _img_set_src_timed(bk_ui->settingmodedegree_degree_f_im, "/images/degree_bt_f_off.png");
    lv_obj_set_pos(bk_ui->settingmodedegree_degree_f_im, 580, 192);
    lv_obj_set_size(bk_ui->settingmodedegree_degree_f_im, 296, 156);

}

rendererFuncStatus_t init_page_settingmodedegree_with_step(bk_lv_ui_t *bk_ui)
{
    static uint32_t renderStartTick = 0;

    if(preRenderPageState[PAGE_SETTINGMODEDEGREE].isRendered)
    {
        return RENDERER_FUNC_DONE;
    }

    switch(currentStep)
    {
        case RENDER_STEP_CREATE_PAGE:
        {
            renderStartTick = lv_tick_get();
            bk_printf(TAG "[RENDER][SETTINGMODEDEGREE] start tick=%lu\n", (unsigned long)renderStartTick);

            if(bk_ui == NULL)
            {
                return RENDERER_FUNC_FAILED;
            }

            stepInitMode = true;
            init_page_settingmodedegree(bk_ui);
            stepInitMode = false;
            if(bk_ui->settingmodedegree == NULL || !lv_obj_is_valid(bk_ui->settingmodedegree))
            {
                bk_printf(TAG "[RENDER][SETTINGMODEDEGREE] CREATE_PAGE failed\n");
                return RENDERER_FUNC_FAILED;
            }

#if UI_PRENDERING_ENABLE
            lv_obj_add_flag(bk_ui->settingmodedegree, LV_OBJ_FLAG_HIDDEN);
#endif
            currentStep = RENDER_STEP_CREATE_CHILD;
            return RENDERER_FUNC_NOT_DONE;
        }

        case RENDER_STEP_CREATE_CHILD:
        {
            stepInitMode = true;
            init_page_settingmodedegree(bk_ui);
            stepInitMode = false;
            currentStep = RENDER_STEP_CACHE_BACKGROUND;
            return RENDERER_FUNC_NOT_DONE;
        }

        case RENDER_STEP_CACHE_BACKGROUND:
        {
            if(preRenderPageConfig[PAGE_SETTINGMODEDEGREE].backgroundImageAssetId != SHARED_IMAGE_NONE)
            {
                const sharedImageAssetId_t assetId =
                    preRenderPageConfig[PAGE_SETTINGMODEDEGREE].backgroundImageAssetId;
                if(set_shared_image_asset(bk_ui->settingmodedegree_bg, assetId) != RENDERER_FUNC_DONE)
                {
                    return RENDERER_FUNC_FAILED;
                }
            }

            currentStep = RENDER_STEP_CACHE_IMAGE;
            return RENDERER_FUNC_NOT_DONE;
        }

        case RENDER_STEP_CACHE_IMAGE:
        {
            const uint32_t imageCount = preRenderPageConfig[PAGE_SETTINGMODEDEGREE].preRenderImageCount;
            if(currentImageStep < imageCount)
            {
                const preRenderImageInfo_t *imageInfo =
                    &preRenderPageConfig[PAGE_SETTINGMODEDEGREE].preRenderImageInfo[currentImageStep];
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
                    bk_printf(TAG "[PREWARM][SETTINGMODEDEGREE] image %lu/%lu failed: %s (%lu ms)\n",
                              (unsigned long)(currentImageStep + 1),
                              (unsigned long)imageCount,
                              imagePath,
                              (unsigned long)lv_tick_elaps(imageStartTick));
                    return RENDERER_FUNC_FAILED;
                }

                bk_printf(TAG "[PREWARM][SETTINGMODEDEGREE] image %lu/%lu done: %s (%lu ms)\n",
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
            /* init_page_settingmodedegree() also attaches the page and control callbacks. */
            currentStep = RENDER_STEP_DONE;
            return RENDERER_FUNC_NOT_DONE;
        }

        case RENDER_STEP_DONE:
        {
            bk_printf(TAG "[RENDER][SETTINGMODEDEGREE] done total=%lu ms\n",
                      (unsigned long)lv_tick_elaps(renderStartTick));

            currentStep = RENDER_STEP_CREATE_PAGE;
            currentImageStep = 0;
            renderStartTick = 0;
            preRenderPageState[PAGE_SETTINGMODEDEGREE].isRendered = true;
            return RENDERER_FUNC_DONE;
        }

        default:
        {
            bk_printf(TAG "[RENDER][SETTINGMODEDEGREE] invalid step=%lu\n",
                      (unsigned long)currentStep);
            return RENDERER_FUNC_FAILED;
        }
    }
}
