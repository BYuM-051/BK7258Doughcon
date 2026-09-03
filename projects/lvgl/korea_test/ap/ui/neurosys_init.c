#include "lvgl.h"
#include "beken_ui.h"
#include "custom_func.h"
#include "event_runtime.h"
#include <stdio.h>
#include <string.h>

#include "ui_config.h"
#include "settings.h"

#define TAG "[neurosys_init.c] "
#include "pageManager.h"
// #define bk_printf(fmt, ...) do {if(0) bk_printf(fmt, ##__VA_ARGS__); } while(0) // disable printf

static uint32_t currentStep = RENDER_STEP_CREATE_PAGE;
static uint32_t currentImageStep = 0;

extern bk_lv_ui_t bk_lv_tool_ui;
extern lv_obj_t *preRenderRoot;

static bool stepInitMode = false;
extern void neurosys_allbt_event_cb(lv_event_t *e);
extern void neurosys_onebt_event_cb(lv_event_t *e);
extern void neurosys_twobt_event_cb(lv_event_t *e);
extern void neurosys_threebt_event_cb(lv_event_t *e);
extern void neurosys_fourbt_event_cb(lv_event_t *e);
void destroy_page_neurosys(bk_lv_ui_t *bk_ui)
{
    if (bk_ui == NULL) {
        return;
    }

    if (bk_ui->neurosys != NULL) {
        lv_obj_del(bk_ui->neurosys);
        bk_ui->neurosys = NULL;
    }

    currentStep = RENDER_STEP_CREATE_PAGE;
    currentImageStep = 0;
    preRenderPageState[PAGE_NEUROSYS].isRendered = false;

    const uint32_t imageCount = preRenderPageConfig[PAGE_NEUROSYS].preRenderImageCount;
    for(uint32_t i = 0; i < imageCount; i++)
    {
        const preRenderImageInfo_t *imageInfo = &preRenderPageConfig[PAGE_NEUROSYS].preRenderImageInfo[i];
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

void init_page_neurosys(bk_lv_ui_t * bk_ui) {
    if(stepInitMode && currentStep == RENDER_STEP_CREATE_CHILD)
    {
        goto create_children;
    }

    if (bk_ui->neurosys != NULL && lv_obj_is_valid(bk_ui->neurosys)) {
        destroy_page_neurosys(bk_ui);
    }

#if UI_PRENDERING_ENABLE
    bk_ui->neurosys = lv_obj_create(preRenderRoot);
    lv_obj_remove_style_all(bk_ui->neurosys);
    lv_obj_set_size(bk_ui->neurosys, 1024, 600);
    lv_obj_set_pos(bk_ui->neurosys, 0, 0);
    lv_obj_set_style_radius(bk_ui->neurosys, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(bk_ui->neurosys, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_scrollbar_mode(bk_ui->neurosys, LV_SCROLLBAR_MODE_OFF);
#else
    bk_ui->neurosys = lv_obj_create(NULL);
    lv_obj_set_size(bk_ui->neurosys, 1024, 600);
    lv_obj_set_scrollbar_mode(bk_ui->neurosys, LV_SCROLLBAR_MODE_OFF);
#endif /* UI_PRENDERING_ENABLE */

    if(stepInitMode)
    {
        return;
    }

create_children:
    // ImageView: allim
    bk_ui->neurosys_allim = lv_image_create(bk_ui->neurosys);
    _img_set_src_timed(bk_ui->neurosys_allim, "/images/red.png");
    lv_obj_set_pos(bk_ui->neurosys_allim, 0, 0);
    lv_obj_set_size(bk_ui->neurosys_allim, LV_PCT(100), LV_PCT(100));

    // Button: allbt
    bk_ui->neurosys_allbt = lv_button_create(bk_ui->neurosys);
    lv_obj_add_flag(bk_ui->neurosys_allbt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->neurosys_allbt, neurosys_allbt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->neurosys_allbt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->neurosys_allbt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->neurosys_allbt, 0, 0);
    lv_obj_set_pos(bk_ui->neurosys_allbt, 0, 0);
    lv_obj_set_size(bk_ui->neurosys_allbt, LV_PCT(100), LV_PCT(100));

    // Button: onebt
    bk_ui->neurosys_onebt = lv_button_create(bk_ui->neurosys);
    lv_obj_add_flag(bk_ui->neurosys_onebt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->neurosys_onebt, neurosys_onebt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->neurosys_onebt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->neurosys_onebt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->neurosys_onebt, 0, 0);
    lv_obj_set_pos(bk_ui->neurosys_onebt, 132, 124);
    lv_obj_set_size(bk_ui->neurosys_onebt, 100, 100);

    // Button: twobt
    bk_ui->neurosys_twobt = lv_button_create(bk_ui->neurosys);
    lv_obj_add_flag(bk_ui->neurosys_twobt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->neurosys_twobt, neurosys_twobt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->neurosys_twobt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->neurosys_twobt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->neurosys_twobt, 0, 0);
    lv_obj_set_pos(bk_ui->neurosys_twobt, 132, 384);
    lv_obj_set_size(bk_ui->neurosys_twobt, 100, 100);

    // Button: threebt
    bk_ui->neurosys_threebt = lv_button_create(bk_ui->neurosys);
    lv_obj_add_flag(bk_ui->neurosys_threebt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->neurosys_threebt, neurosys_threebt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->neurosys_threebt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->neurosys_threebt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->neurosys_threebt, 0, 0);
    lv_obj_set_pos(bk_ui->neurosys_threebt, 782, 124);
    lv_obj_set_size(bk_ui->neurosys_threebt, 100, 100);

    // Button: fourbt
    bk_ui->neurosys_fourbt = lv_button_create(bk_ui->neurosys);
    lv_obj_add_flag(bk_ui->neurosys_fourbt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->neurosys_fourbt, neurosys_fourbt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->neurosys_fourbt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->neurosys_fourbt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->neurosys_fourbt, 0, 0);
    lv_obj_set_pos(bk_ui->neurosys_fourbt, 782, 384);
    lv_obj_set_size(bk_ui->neurosys_fourbt, 100, 100);

}

rendererFuncStatus_t init_page_neurosys_with_step(bk_lv_ui_t *bk_ui)
{
    static uint32_t renderStartTick = 0;

    if(preRenderPageState[PAGE_NEUROSYS].isRendered)
    {
        return RENDERER_FUNC_DONE;
    }

    switch(currentStep)
    {
        case RENDER_STEP_CREATE_PAGE:
        {
            renderStartTick = lv_tick_get();
            bk_printf(TAG "[RENDER][NEUROSYS] start tick=%lu\n", (unsigned long)renderStartTick);

            if(bk_ui == NULL)
            {
                return RENDERER_FUNC_FAILED;
            }

            stepInitMode = true;
            init_page_neurosys(bk_ui);
            stepInitMode = false;
            if(bk_ui->neurosys == NULL || !lv_obj_is_valid(bk_ui->neurosys))
            {
                bk_printf(TAG "[RENDER][NEUROSYS] CREATE_PAGE failed\n");
                return RENDERER_FUNC_FAILED;
            }

#if UI_PRENDERING_ENABLE
            lv_obj_add_flag(bk_ui->neurosys, LV_OBJ_FLAG_HIDDEN);
#endif
            currentStep = RENDER_STEP_CREATE_CHILD;
            return RENDERER_FUNC_NOT_DONE;
        }

        case RENDER_STEP_CREATE_CHILD:
        {
            stepInitMode = true;
            init_page_neurosys(bk_ui);
            stepInitMode = false;
            currentStep = RENDER_STEP_CACHE_BACKGROUND;
            return RENDERER_FUNC_NOT_DONE;
        }

        case RENDER_STEP_CACHE_BACKGROUND:
        {
            if(preRenderPageConfig[PAGE_NEUROSYS].backgroundImageAssetId != SHARED_IMAGE_NONE)
            {
                const sharedImageAssetId_t assetId =
                    preRenderPageConfig[PAGE_NEUROSYS].backgroundImageAssetId;
                if(set_shared_image_asset(bk_ui->neurosys_allim, assetId) != RENDERER_FUNC_DONE)
                {
                    return RENDERER_FUNC_FAILED;
                }
            }

            currentStep = RENDER_STEP_CACHE_IMAGE;
            return RENDERER_FUNC_NOT_DONE;
        }

        case RENDER_STEP_CACHE_IMAGE:
        {
            const uint32_t imageCount = preRenderPageConfig[PAGE_NEUROSYS].preRenderImageCount;
            if(currentImageStep < imageCount)
            {
                const preRenderImageInfo_t *imageInfo =
                    &preRenderPageConfig[PAGE_NEUROSYS].preRenderImageInfo[currentImageStep];
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
                    bk_printf(TAG "[PREWARM][NEUROSYS] image %lu/%lu failed: %s (%lu ms)\n",
                              (unsigned long)(currentImageStep + 1),
                              (unsigned long)imageCount,
                              imagePath,
                              (unsigned long)lv_tick_elaps(imageStartTick));
                    return RENDERER_FUNC_FAILED;
                }

                bk_printf(TAG "[PREWARM][NEUROSYS] image %lu/%lu done: %s (%lu ms)\n",
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
            /* init_page_neurosys() also attaches the page and control callbacks. */
            currentStep = RENDER_STEP_DONE;
            return RENDERER_FUNC_NOT_DONE;
        }

        case RENDER_STEP_DONE:
        {
            bk_printf(TAG "[RENDER][NEUROSYS] done total=%lu ms\n",
                      (unsigned long)lv_tick_elaps(renderStartTick));

            currentStep = RENDER_STEP_CREATE_PAGE;
            currentImageStep = 0;
            renderStartTick = 0;
            preRenderPageState[PAGE_NEUROSYS].isRendered = true;
            return RENDERER_FUNC_DONE;
        }

        default:
        {
            bk_printf(TAG "[RENDER][NEUROSYS] invalid step=%lu\n",
                      (unsigned long)currentStep);
            return RENDERER_FUNC_FAILED;
        }
    }
}
