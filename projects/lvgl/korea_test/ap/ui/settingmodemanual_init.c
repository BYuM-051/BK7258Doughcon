#include "lvgl.h"
#include "beken_ui.h"
#include "custom_func.h"
#include "event_runtime.h"
#include "ui_lang.h"
#include <stdio.h>
#include <string.h>

#include "ui_config.h"
#include "settings.h"

#define TAG "[settingmodemanual_init.c] "
#include "pageManager.h"
// #define bk_printf(fmt, ...) do {if(0) bk_printf(fmt, ##__VA_ARGS__); } while(0) // disable printf

static uint32_t currentStep = RENDER_STEP_CREATE_PAGE;
static uint32_t currentImageStep = 0;

extern bk_lv_ui_t bk_lv_tool_ui;
extern lv_obj_t *preRenderRoot;

static bool stepInitMode = false;
extern void settingmodemanual_backbt_event_cb(lv_event_t *e);
extern void settingmodemanual_setting_manual_autobt_event_cb(lv_event_t *e);
extern void settingmodemanual_setting_manual_manualbt_event_cb(lv_event_t *e);
extern void settingmodemanual_setting_manual_drybt_event_cb(lv_event_t *e);
extern void settingmodemanual_setting_manual_memorybt_event_cb(lv_event_t *e);
extern void settingmodemanual_setting_manual_settingbt_event_cb(lv_event_t *e);
extern void settingmodemanual_load_start_event_cb(lv_event_t *e);
extern void settingmodemanual_loaded_event_cb(lv_event_t *e);
extern void settingmodemanual_unload_start_event_cb(lv_event_t *e);
extern void settingmodemanual_unloaded_event_cb(lv_event_t *e);

void destroy_page_settingmodemanual(bk_lv_ui_t *bk_ui)
{
    if (bk_ui == NULL) {
        return;
    }
    if (bk_ui->settingmodemanual != NULL) {
        lv_obj_del(bk_ui->settingmodemanual);
        bk_ui->settingmodemanual = NULL;
    }

    currentStep = RENDER_STEP_CREATE_PAGE;
    currentImageStep = 0;
    preRenderPageState[PAGE_SETTINGMODEMANUAL].isRendered = false;

    const uint32_t imageCount = preRenderPageConfig[PAGE_SETTINGMODEMANUAL].preRenderImageCount;
    for(uint32_t i = 0; i < imageCount; i++)
    {
        const preRenderImageInfo_t *imageInfo = &preRenderPageConfig[PAGE_SETTINGMODEMANUAL].preRenderImageInfo[i];
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

void init_page_settingmodemanual(bk_lv_ui_t * bk_ui) {
    if(stepInitMode && currentStep == RENDER_STEP_CREATE_CHILD)
    {
        goto create_children;
    }

    if (bk_ui->settingmodemanual != NULL && lv_obj_is_valid(bk_ui->settingmodemanual)) {
        destroy_page_settingmodemanual(bk_ui);
    }

    ui_lang_reset_settingmodemanual_cache();

#if UI_PRENDERING_ENABLE
    bk_ui->settingmodemanual = lv_obj_create(preRenderRoot);
    lv_obj_remove_style_all(bk_ui->settingmodemanual);
    lv_obj_set_size(bk_ui->settingmodemanual, 1024, 600);
    lv_obj_set_pos(bk_ui->settingmodemanual, 0, 0);
    lv_obj_set_style_radius(bk_ui->settingmodemanual, 0, LV_PART_MAIN);
    lv_obj_set_scrollbar_mode(bk_ui->settingmodemanual, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_event_cb(bk_ui->settingmodemanual, settingmodemanual_load_start_event_cb, UI_EVENT_PAGE_SHOW_START, NULL);
    lv_obj_add_event_cb(bk_ui->settingmodemanual, settingmodemanual_loaded_event_cb, UI_EVENT_PAGE_SHOWN,     NULL);
    lv_obj_add_event_cb(bk_ui->settingmodemanual, settingmodemanual_unload_start_event_cb, UI_EVENT_PAGE_HIDE_START, NULL);
    lv_obj_add_event_cb(bk_ui->settingmodemanual, settingmodemanual_unloaded_event_cb, UI_EVENT_PAGE_HIDDEN,     NULL);
#else
    bk_ui->settingmodemanual = lv_obj_create(NULL);
    lv_obj_set_size(bk_ui->settingmodemanual, 1024, 600);
    lv_obj_set_scrollbar_mode(bk_ui->settingmodemanual, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_event_cb(bk_ui->settingmodemanual, settingmodemanual_load_start_event_cb, LV_EVENT_SCREEN_LOAD_START, NULL);
    lv_obj_add_event_cb(bk_ui->settingmodemanual, settingmodemanual_loaded_event_cb, LV_EVENT_SCREEN_LOADED,     NULL);
    lv_obj_add_event_cb(bk_ui->settingmodemanual, settingmodemanual_unload_start_event_cb, LV_EVENT_SCREEN_UNLOAD_START, NULL);
    lv_obj_add_event_cb(bk_ui->settingmodemanual, settingmodemanual_unloaded_event_cb, LV_EVENT_SCREEN_UNLOADED,     NULL);
#endif /* UI_PRENDERING_ENABLE */

    if(stepInitMode)
    {
        return;
    }

create_children:
    bk_ui->settingmodemanual_bg = lv_image_create(bk_ui->settingmodemanual);
    _bg_set(bk_ui->settingmodemanual_bg);
    lv_obj_set_pos(bk_ui->settingmodemanual_bg, 0, 0);

    // ImageView: title
    bk_ui->settingmodemanual_title = lv_image_create(bk_ui->settingmodemanual);
    _img_set_src_timed(bk_ui->settingmodemanual_title, "/images/usermanual_title.png");
    lv_obj_set_pos(bk_ui->settingmodemanual_title, 0, 10);
    lv_obj_set_size(bk_ui->settingmodemanual_title, 380, 80);
    lv_image_set_inner_align(bk_ui->settingmodemanual_title, LV_IMAGE_ALIGN_TOP_LEFT);

    // Button: backbt
    bk_ui->settingmodemanual_backbt = lv_button_create(bk_ui->settingmodemanual);
    lv_obj_add_flag(bk_ui->settingmodemanual_backbt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->settingmodemanual_backbt, settingmodemanual_backbt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->settingmodemanual_backbt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->settingmodemanual_backbt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->settingmodemanual_backbt, 0, 0);
    lv_obj_set_pos(bk_ui->settingmodemanual_backbt, 825, 13);
    lv_obj_set_size(bk_ui->settingmodemanual_backbt, 179, 74);

    // ImageView: imageview3
    bk_ui->settingmodemanual_imageview3 = lv_image_create(bk_ui->settingmodemanual);
    _img_set_src_timed(bk_ui->settingmodemanual_imageview3, "/images/exit_bt.png");
    lv_obj_set_pos(bk_ui->settingmodemanual_imageview3, 825, 13);
    lv_obj_set_size(bk_ui->settingmodemanual_imageview3, 179, 74);

    // ImageView: setting_manual_autoim
    bk_ui->settingmodemanual_setting_manual_autoim = lv_image_create(bk_ui->settingmodemanual);
    _img_set_src_timed(bk_ui->settingmodemanual_setting_manual_autoim, "/images/setting_manual_auto_off.png");
    lv_obj_set_pos(bk_ui->settingmodemanual_setting_manual_autoim, 23, 120);
    lv_obj_set_size(bk_ui->settingmodemanual_setting_manual_autoim, 445, 66);

    // Button: setting_manual_autobt
    bk_ui->settingmodemanual_setting_manual_autobt = lv_button_create(bk_ui->settingmodemanual);
    lv_obj_add_flag(bk_ui->settingmodemanual_setting_manual_autobt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->settingmodemanual_setting_manual_autobt, settingmodemanual_setting_manual_autobt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->settingmodemanual_setting_manual_autobt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->settingmodemanual_setting_manual_autobt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->settingmodemanual_setting_manual_autobt, 0, 0);
    lv_obj_set_pos(bk_ui->settingmodemanual_setting_manual_autobt, 23, 120);
    lv_obj_set_size(bk_ui->settingmodemanual_setting_manual_autobt, 445, 66);

    // ImageView: setting_manual_manualim
    bk_ui->settingmodemanual_setting_manual_manualim = lv_image_create(bk_ui->settingmodemanual);
    _img_set_src_timed(bk_ui->settingmodemanual_setting_manual_manualim, "/images/setting_manual_manual_off.png");
    lv_obj_set_pos(bk_ui->settingmodemanual_setting_manual_manualim, 23, 198);
    lv_obj_set_size(bk_ui->settingmodemanual_setting_manual_manualim, 445, 66);

    // Button: setting_manual_manualbt
    bk_ui->settingmodemanual_setting_manual_manualbt = lv_button_create(bk_ui->settingmodemanual);
    lv_obj_add_flag(bk_ui->settingmodemanual_setting_manual_manualbt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->settingmodemanual_setting_manual_manualbt, settingmodemanual_setting_manual_manualbt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->settingmodemanual_setting_manual_manualbt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->settingmodemanual_setting_manual_manualbt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->settingmodemanual_setting_manual_manualbt, 0, 0);
    lv_obj_set_pos(bk_ui->settingmodemanual_setting_manual_manualbt, 23, 198);
    lv_obj_set_size(bk_ui->settingmodemanual_setting_manual_manualbt, 445, 66);

    // ImageView: setting_manual_dryim
    bk_ui->settingmodemanual_setting_manual_dryim = lv_image_create(bk_ui->settingmodemanual);
    _img_set_src_timed(bk_ui->settingmodemanual_setting_manual_dryim, "/images/setting_manual_dry_off.png");
    lv_obj_set_pos(bk_ui->settingmodemanual_setting_manual_dryim, 23, 276);
    lv_obj_set_size(bk_ui->settingmodemanual_setting_manual_dryim, 445, 66);

    // Button: setting_manual_drybt
    bk_ui->settingmodemanual_setting_manual_drybt = lv_button_create(bk_ui->settingmodemanual);
    lv_obj_add_flag(bk_ui->settingmodemanual_setting_manual_drybt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->settingmodemanual_setting_manual_drybt, settingmodemanual_setting_manual_drybt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->settingmodemanual_setting_manual_drybt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->settingmodemanual_setting_manual_drybt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->settingmodemanual_setting_manual_drybt, 0, 0);
    lv_obj_set_pos(bk_ui->settingmodemanual_setting_manual_drybt, 23, 276);
    lv_obj_set_size(bk_ui->settingmodemanual_setting_manual_drybt, 445, 66);

    // ImageView: setting_manual_memoryim
    bk_ui->settingmodemanual_setting_manual_memoryim = lv_image_create(bk_ui->settingmodemanual);
    _img_set_src_timed(bk_ui->settingmodemanual_setting_manual_memoryim, "/images/setting_manual_memory_off.png");
    lv_obj_set_pos(bk_ui->settingmodemanual_setting_manual_memoryim, 23, 354);
    lv_obj_set_size(bk_ui->settingmodemanual_setting_manual_memoryim, 445, 66);

    // Button: setting_manual_memorybt
    bk_ui->settingmodemanual_setting_manual_memorybt = lv_button_create(bk_ui->settingmodemanual);
    lv_obj_add_flag(bk_ui->settingmodemanual_setting_manual_memorybt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->settingmodemanual_setting_manual_memorybt, settingmodemanual_setting_manual_memorybt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->settingmodemanual_setting_manual_memorybt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->settingmodemanual_setting_manual_memorybt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->settingmodemanual_setting_manual_memorybt, 0, 0);
    lv_obj_set_pos(bk_ui->settingmodemanual_setting_manual_memorybt, 23, 354);
    lv_obj_set_size(bk_ui->settingmodemanual_setting_manual_memorybt, 445, 66);

    // ImageView: setting_manual_settingim
    bk_ui->settingmodemanual_setting_manual_settingim = lv_image_create(bk_ui->settingmodemanual);
    _img_set_src_timed(bk_ui->settingmodemanual_setting_manual_settingim, "/images/setting_manual_setting_off.png");
    lv_obj_set_pos(bk_ui->settingmodemanual_setting_manual_settingim, 23, 432);
    lv_obj_set_size(bk_ui->settingmodemanual_setting_manual_settingim, 445, 66);

    // Button: setting_manual_settingbt
    bk_ui->settingmodemanual_setting_manual_settingbt = lv_button_create(bk_ui->settingmodemanual);
    lv_obj_add_flag(bk_ui->settingmodemanual_setting_manual_settingbt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->settingmodemanual_setting_manual_settingbt, settingmodemanual_setting_manual_settingbt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->settingmodemanual_setting_manual_settingbt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->settingmodemanual_setting_manual_settingbt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->settingmodemanual_setting_manual_settingbt, 0, 0);
    lv_obj_set_pos(bk_ui->settingmodemanual_setting_manual_settingbt, 23, 432);
    lv_obj_set_size(bk_ui->settingmodemanual_setting_manual_settingbt, 445, 66);

    // ImageView: setting_manual_boxim
    bk_ui->settingmodemanual_setting_manual_boxim = lv_image_create(bk_ui->settingmodemanual);
    _img_set_src_deferred(bk_ui->settingmodemanual_setting_manual_boxim, "/images/setting_manual_box.png");
    lv_obj_add_flag(bk_ui->settingmodemanual_setting_manual_boxim, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_pos(bk_ui->settingmodemanual_setting_manual_boxim, 498, 128);
    lv_obj_set_size(bk_ui->settingmodemanual_setting_manual_boxim, 506, 378);

    // TextView: setting_manual_title
    bk_ui->settingmodemanual_setting_manual_title = lv_label_create(bk_ui->settingmodemanual);
    lv_label_set_text(bk_ui->settingmodemanual_setting_manual_title, "자동설정");
    lv_obj_set_style_bg_opa(bk_ui->settingmodemanual_setting_manual_title, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmodemanual_setting_manual_title, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmodemanual_setting_manual_title, &lv_font_scdream_regular_32, 0);
    lv_obj_add_flag(bk_ui->settingmodemanual_setting_manual_title, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_pos(bk_ui->settingmodemanual_setting_manual_title, 529, 138);
    lv_obj_set_size(bk_ui->settingmodemanual_setting_manual_title, 200, 40);

    // TextView: setting_manual_value
    bk_ui->settingmodemanual_setting_manual_value = lv_label_create(bk_ui->settingmodemanual);
    lv_label_set_text(bk_ui->settingmodemanual_setting_manual_value, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmodemanual_setting_manual_value, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmodemanual_setting_manual_value, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmodemanual_setting_manual_value, &lv_font_scdream_regular_22, 0);
    lv_obj_add_flag(bk_ui->settingmodemanual_setting_manual_value, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_pos(bk_ui->settingmodemanual_setting_manual_value, 535, 195);
    lv_obj_set_size(bk_ui->settingmodemanual_setting_manual_value, 430, 280);

}

rendererFuncStatus_t init_page_settingmodemanual_with_step(bk_lv_ui_t *bk_ui)
{
    static uint32_t renderStartTick = 0;

    if(preRenderPageState[PAGE_SETTINGMODEMANUAL].isRendered)
    {
        return RENDERER_FUNC_DONE;
    }

    switch(currentStep)
    {
        case RENDER_STEP_CREATE_PAGE:
        {
            renderStartTick = lv_tick_get();
            bk_printf(TAG "[RENDER][SETTINGMODEMANUAL] start tick=%lu\n", (unsigned long)renderStartTick);

            if(bk_ui == NULL)
            {
                return RENDERER_FUNC_FAILED;
            }

            stepInitMode = true;
            init_page_settingmodemanual(bk_ui);
            stepInitMode = false;
            if(bk_ui->settingmodemanual == NULL || !lv_obj_is_valid(bk_ui->settingmodemanual))
            {
                bk_printf(TAG "[RENDER][SETTINGMODEMANUAL] CREATE_PAGE failed\n");
                return RENDERER_FUNC_FAILED;
            }

#if UI_PRENDERING_ENABLE
            lv_obj_add_flag(bk_ui->settingmodemanual, LV_OBJ_FLAG_HIDDEN);
#endif
            currentStep = RENDER_STEP_CREATE_CHILD;
            return RENDERER_FUNC_NOT_DONE;
        }

        case RENDER_STEP_CREATE_CHILD:
        {
            stepInitMode = true;
            init_page_settingmodemanual(bk_ui);
            stepInitMode = false;
            currentStep = RENDER_STEP_CACHE_BACKGROUND;
            return RENDERER_FUNC_NOT_DONE;
        }

        case RENDER_STEP_CACHE_BACKGROUND:
        {
            if(preRenderPageConfig[PAGE_SETTINGMODEMANUAL].backgroundImageAssetId != SHARED_IMAGE_NONE)
            {
                const sharedImageAssetId_t assetId =
                    preRenderPageConfig[PAGE_SETTINGMODEMANUAL].backgroundImageAssetId;
                if(set_shared_image_asset(bk_ui->settingmodemanual_bg, assetId) != RENDERER_FUNC_DONE)
                {
                    return RENDERER_FUNC_FAILED;
                }
            }

            currentStep = RENDER_STEP_CACHE_IMAGE;
            return RENDERER_FUNC_NOT_DONE;
        }

        case RENDER_STEP_CACHE_IMAGE:
        {
            const uint32_t imageCount = preRenderPageConfig[PAGE_SETTINGMODEMANUAL].preRenderImageCount;
            if(currentImageStep < imageCount)
            {
                const preRenderImageInfo_t *imageInfo =
                    &preRenderPageConfig[PAGE_SETTINGMODEMANUAL].preRenderImageInfo[currentImageStep];
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
                    bk_printf(TAG "[PREWARM][SETTINGMODEMANUAL] image %lu/%lu failed: %s (%lu ms)\n",
                              (unsigned long)(currentImageStep + 1),
                              (unsigned long)imageCount,
                              imagePath,
                              (unsigned long)lv_tick_elaps(imageStartTick));
                    return RENDERER_FUNC_FAILED;
                }

                bk_printf(TAG "[PREWARM][SETTINGMODEMANUAL] image %lu/%lu done: %s (%lu ms)\n",
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
            /* init_page_settingmodemanual() also attaches the page and control callbacks. */
            currentStep = RENDER_STEP_DONE;
            return RENDERER_FUNC_NOT_DONE;
        }

        case RENDER_STEP_DONE:
        {
            bk_printf(TAG "[RENDER][SETTINGMODEMANUAL] done total=%lu ms\n",
                      (unsigned long)lv_tick_elaps(renderStartTick));

            currentStep = RENDER_STEP_CREATE_PAGE;
            currentImageStep = 0;
            renderStartTick = 0;
            preRenderPageState[PAGE_SETTINGMODEMANUAL].isRendered = true;
            return RENDERER_FUNC_DONE;
        }

        default:
        {
            bk_printf(TAG "[RENDER][SETTINGMODEMANUAL] invalid step=%lu\n",
                      (unsigned long)currentStep);
            return RENDERER_FUNC_FAILED;
        }
    }
}
