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

#define TAG "[detailsettingdefrost_init.c] "
#include "preRenderer.h"
// #define bk_printf(fmt, ...) do {if(0) bk_printf(fmt, ##__VA_ARGS__); } while(0) // disable printf

static uint32_t currentStep = RENDER_STEP_CREATE_PAGE;
static uint32_t currentImageStep = 0;

extern bk_lv_ui_t bk_lv_tool_ui;
extern lv_obj_t *preRenderRoot;

static bool stepInitMode = false;
extern void detailsettingdefrost_backbt_event_cb(lv_event_t *e);
extern void detailsettingdefrost_settingbt1_event_cb(lv_event_t *e);
extern void detailsettingdefrost_settingbt2_event_cb(lv_event_t *e);
extern void detailsettingdefrost_settingbt3_event_cb(lv_event_t *e);
extern void detailsettingdefrost_changebt_event_cb(lv_event_t *e);
extern void detailsettingdefrost_roller_event_cb(lv_event_t *e);
extern void detailsettingdefrost_load_start_event_cb(lv_event_t *e);
extern void detailsettingdefrost_loaded_event_cb(lv_event_t *e);
extern void detailsettingdefrost_unload_start_event_cb(lv_event_t *e);
extern void detailsettingdefrost_unloaded_event_cb(lv_event_t *e);

void destroy_page_detailsettingdefrost(bk_lv_ui_t *bk_ui)
{
    if (bk_ui == NULL) {
        return;
    }
    if (bk_ui->detailsettingdefrost != NULL) {
        lv_obj_del(bk_ui->detailsettingdefrost);
        bk_ui->detailsettingdefrost = NULL;
    }

    currentStep = RENDER_STEP_CREATE_PAGE;
    currentImageStep = 0;
    preRenderPageState[PAGE_DETAILSETTINGDEFROST].isRendered = false;

    const uint32_t imageCount = preRenderPageConfig[PAGE_DETAILSETTINGDEFROST].preRenderImageCount;
    for(uint32_t i = 0; i < imageCount; i++)
    {
        const preRenderImageInfo_t *imageInfo = &preRenderPageConfig[PAGE_DETAILSETTINGDEFROST].preRenderImageInfo[i];
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

void init_page_detailsettingdefrost(bk_lv_ui_t * bk_ui) {
    if(stepInitMode && currentStep == RENDER_STEP_CREATE_CHILD)
    {
        goto create_children;
    }

    if (bk_ui->detailsettingdefrost != NULL && lv_obj_is_valid(bk_ui->detailsettingdefrost)) {
        destroy_page_detailsettingdefrost(bk_ui);
    }

    ui_lang_reset_detailsettingdefrost_cache();

#if UI_PRENDERING_ENABLE
    bk_ui->detailsettingdefrost = lv_obj_create(preRenderRoot);
    lv_obj_remove_style_all(bk_ui->detailsettingdefrost);
    lv_obj_set_size(bk_ui->detailsettingdefrost, 1024, 600);
    lv_obj_set_pos(bk_ui->detailsettingdefrost, 0, 0);
    lv_obj_set_style_radius(bk_ui->detailsettingdefrost, 0, LV_PART_MAIN);
    lv_obj_set_scrollbar_mode(bk_ui->detailsettingdefrost, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_event_cb(bk_ui->detailsettingdefrost, detailsettingdefrost_load_start_event_cb, UI_EVENT_PAGE_SHOW_START, NULL);
    lv_obj_add_event_cb(bk_ui->detailsettingdefrost, detailsettingdefrost_loaded_event_cb, UI_EVENT_PAGE_SHOWN,     NULL);
    lv_obj_add_event_cb(bk_ui->detailsettingdefrost, detailsettingdefrost_unload_start_event_cb, UI_EVENT_PAGE_HIDE_START, NULL);
    lv_obj_add_event_cb(bk_ui->detailsettingdefrost, detailsettingdefrost_unloaded_event_cb, UI_EVENT_PAGE_HIDDEN,     NULL);
#else
    bk_ui->detailsettingdefrost = lv_obj_create(NULL);
    lv_obj_set_size(bk_ui->detailsettingdefrost, 1024, 600);
    lv_obj_set_scrollbar_mode(bk_ui->detailsettingdefrost, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_event_cb(bk_ui->detailsettingdefrost, detailsettingdefrost_load_start_event_cb, LV_EVENT_SCREEN_LOAD_START, NULL);
    lv_obj_add_event_cb(bk_ui->detailsettingdefrost, detailsettingdefrost_loaded_event_cb, LV_EVENT_SCREEN_LOADED,     NULL);
    lv_obj_add_event_cb(bk_ui->detailsettingdefrost, detailsettingdefrost_unload_start_event_cb, LV_EVENT_SCREEN_UNLOAD_START, NULL);
    lv_obj_add_event_cb(bk_ui->detailsettingdefrost, detailsettingdefrost_unloaded_event_cb, LV_EVENT_SCREEN_UNLOADED,     NULL);
#endif /* UI_PRENDERING_ENABLE */

    if(stepInitMode)
    {
        return;
    }

create_children:
    bk_ui->detailsettingdefrost_bg = lv_image_create(bk_ui->detailsettingdefrost);
    lv_obj_add_flag(bk_ui->detailsettingdefrost_bg, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_style_bg_color(bk_ui->detailsettingdefrost, lv_color_hex(0xd9d9d9), 0);
    lv_obj_set_style_bg_opa(bk_ui->detailsettingdefrost, LV_OPA_COVER, 0);
    lv_obj_set_pos(bk_ui->detailsettingdefrost_bg, 0, 0);

    // ImageView: title
    bk_ui->detailsettingdefrost_title = lv_image_create(bk_ui->detailsettingdefrost);
    lv_obj_set_pos(bk_ui->detailsettingdefrost_title, 0, 10);
    lv_obj_set_size(bk_ui->detailsettingdefrost_title, 380, 80);
    lv_image_set_inner_align(bk_ui->detailsettingdefrost_title, LV_IMAGE_ALIGN_TOP_LEFT);

    // Button: backbt
    bk_ui->detailsettingdefrost_backbt = lv_button_create(bk_ui->detailsettingdefrost);
    lv_obj_add_flag(bk_ui->detailsettingdefrost_backbt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->detailsettingdefrost_backbt, detailsettingdefrost_backbt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->detailsettingdefrost_backbt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->detailsettingdefrost_backbt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->detailsettingdefrost_backbt, 0, 0);
    lv_obj_set_pos(bk_ui->detailsettingdefrost_backbt, 825, 13);
    lv_obj_set_size(bk_ui->detailsettingdefrost_backbt, 179, 74);

    // ImageView: imageview3
    bk_ui->detailsettingdefrost_imageview3 = lv_image_create(bk_ui->detailsettingdefrost);
    lv_obj_set_pos(bk_ui->detailsettingdefrost_imageview3, 825, 13);
    lv_obj_set_size(bk_ui->detailsettingdefrost_imageview3, 179, 74);

    // ImageView: settingim1
    bk_ui->detailsettingdefrost_settingim1 = lv_image_create(bk_ui->detailsettingdefrost);
    lv_obj_set_pos(bk_ui->detailsettingdefrost_settingim1, 23, 120);
    lv_obj_set_size(bk_ui->detailsettingdefrost_settingim1, 538, 66);

    // TextView: settingtxt1
    bk_ui->detailsettingdefrost_settingtxt1 = lv_label_create(bk_ui->detailsettingdefrost);
    lv_label_set_text(bk_ui->detailsettingdefrost_settingtxt1, "");
    lv_obj_set_style_bg_opa(bk_ui->detailsettingdefrost_settingtxt1, 0, 0);
    lv_obj_set_style_text_color(bk_ui->detailsettingdefrost_settingtxt1, lv_color_hex(0xA6A6A6), 0);
    lv_obj_set_style_text_font(bk_ui->detailsettingdefrost_settingtxt1, &lv_font_scdream_regular_32, 0);
    lv_obj_set_pos(bk_ui->detailsettingdefrost_settingtxt1, 270, 135);
    lv_obj_set_size(bk_ui->detailsettingdefrost_settingtxt1, 200, 42);
    lv_obj_set_style_text_align(bk_ui->detailsettingdefrost_settingtxt1, LV_TEXT_ALIGN_RIGHT, 0);

    // Button: settingbt1
    bk_ui->detailsettingdefrost_settingbt1 = lv_button_create(bk_ui->detailsettingdefrost);
    lv_obj_add_flag(bk_ui->detailsettingdefrost_settingbt1, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->detailsettingdefrost_settingbt1, detailsettingdefrost_settingbt1_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->detailsettingdefrost_settingbt1, 0, 0);
    lv_obj_set_style_border_width(bk_ui->detailsettingdefrost_settingbt1, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->detailsettingdefrost_settingbt1, 0, 0);
    lv_obj_set_pos(bk_ui->detailsettingdefrost_settingbt1, 23, 120);
    lv_obj_set_size(bk_ui->detailsettingdefrost_settingbt1, 538, 66);

    // ImageView: settingim2
    bk_ui->detailsettingdefrost_settingim2 = lv_image_create(bk_ui->detailsettingdefrost);
    lv_obj_set_pos(bk_ui->detailsettingdefrost_settingim2, 23, 198);
    lv_obj_set_size(bk_ui->detailsettingdefrost_settingim2, 538, 66);

    // TextView: settingtxt2
    bk_ui->detailsettingdefrost_settingtxt2 = lv_label_create(bk_ui->detailsettingdefrost);
    lv_label_set_text(bk_ui->detailsettingdefrost_settingtxt2, "");
    lv_obj_set_style_bg_opa(bk_ui->detailsettingdefrost_settingtxt2, 0, 0);
    lv_obj_set_style_text_color(bk_ui->detailsettingdefrost_settingtxt2, lv_color_hex(0xA6A6A6), 0);
    lv_obj_set_style_text_font(bk_ui->detailsettingdefrost_settingtxt2, &lv_font_scdream_regular_32, 0);
    lv_obj_set_pos(bk_ui->detailsettingdefrost_settingtxt2, 400, 213);
    lv_obj_set_size(bk_ui->detailsettingdefrost_settingtxt2, 70, 42);
    lv_obj_set_style_text_align(bk_ui->detailsettingdefrost_settingtxt2, LV_TEXT_ALIGN_RIGHT, 0);

    // Button: settingbt2
    bk_ui->detailsettingdefrost_settingbt2 = lv_button_create(bk_ui->detailsettingdefrost);
    lv_obj_add_flag(bk_ui->detailsettingdefrost_settingbt2, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->detailsettingdefrost_settingbt2, detailsettingdefrost_settingbt2_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->detailsettingdefrost_settingbt2, 0, 0);
    lv_obj_set_style_border_width(bk_ui->detailsettingdefrost_settingbt2, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->detailsettingdefrost_settingbt2, 0, 0);
    lv_obj_set_pos(bk_ui->detailsettingdefrost_settingbt2, 23, 198);
    lv_obj_set_size(bk_ui->detailsettingdefrost_settingbt2, 538, 66);

    // ImageView: settingim3
    bk_ui->detailsettingdefrost_settingim3 = lv_image_create(bk_ui->detailsettingdefrost);
    lv_obj_set_pos(bk_ui->detailsettingdefrost_settingim3, 23, 276);
    lv_obj_set_size(bk_ui->detailsettingdefrost_settingim3, 538, 66);

    // TextView: settingtxt3
    bk_ui->detailsettingdefrost_settingtxt3 = lv_label_create(bk_ui->detailsettingdefrost);
    lv_label_set_text(bk_ui->detailsettingdefrost_settingtxt3, "");
    lv_obj_set_style_bg_opa(bk_ui->detailsettingdefrost_settingtxt3, 0, 0);
    lv_obj_set_style_text_color(bk_ui->detailsettingdefrost_settingtxt3, lv_color_hex(0xA6A6A6), 0);
    lv_obj_set_style_text_font(bk_ui->detailsettingdefrost_settingtxt3, &lv_font_scdream_regular_32, 0);
    lv_obj_set_pos(bk_ui->detailsettingdefrost_settingtxt3, 420, 291);
    lv_obj_set_size(bk_ui->detailsettingdefrost_settingtxt3, 50, 42);
    lv_obj_set_style_text_align(bk_ui->detailsettingdefrost_settingtxt3, LV_TEXT_ALIGN_RIGHT, 0);

    // Button: settingbt3
    bk_ui->detailsettingdefrost_settingbt3 = lv_button_create(bk_ui->detailsettingdefrost);
    lv_obj_add_flag(bk_ui->detailsettingdefrost_settingbt3, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->detailsettingdefrost_settingbt3, detailsettingdefrost_settingbt3_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->detailsettingdefrost_settingbt3, 0, 0);
    lv_obj_set_style_border_width(bk_ui->detailsettingdefrost_settingbt3, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->detailsettingdefrost_settingbt3, 0, 0);
    lv_obj_set_pos(bk_ui->detailsettingdefrost_settingbt3, 20, 276);
    lv_obj_set_size(bk_ui->detailsettingdefrost_settingbt3, 538, 66);

    // ImageView: pickerbox
    bk_ui->detailsettingdefrost_pickerbox = lv_image_create(bk_ui->detailsettingdefrost);
    ui_lang_apply_picker(bk_ui->detailsettingdefrost_pickerbox, 1);
    lv_obj_set_pos(bk_ui->detailsettingdefrost_pickerbox, 624, 120);
    lv_obj_set_size(bk_ui->detailsettingdefrost_pickerbox, 376, 376);
    lv_obj_add_flag(bk_ui->detailsettingdefrost_pickerbox, LV_OBJ_FLAG_HIDDEN);


    // NumberPicker: settemp_setn1 (lv_roller)
    bk_ui->detailsettingdefrost_settemp_setn1 = lv_roller_create(bk_ui->detailsettingdefrost);
    lv_roller_set_options(bk_ui->detailsettingdefrost_settemp_setn1, "0", LV_ROLLER_MODE_NORMAL);
    /* 폰트 스타일을 먼저 적용 — visible_row_count는 현재 폰트의 line height로 높이를
     * 계산하므로 폰트 적용 전에 호출하면 롤러 높이가 작게 계산된다 */
    lv_obj_set_style_bg_opa(bk_ui->detailsettingdefrost_settemp_setn1, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(bk_ui->detailsettingdefrost_settemp_setn1, 0, LV_PART_MAIN);
    lv_obj_set_style_text_color(bk_ui->detailsettingdefrost_settemp_setn1, lv_color_hex(0xCCCCCC), LV_PART_MAIN);
    lv_obj_set_style_text_font(bk_ui->detailsettingdefrost_settemp_setn1, &lv_font_scdream_regular_72, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(bk_ui->detailsettingdefrost_settemp_setn1, LV_OPA_TRANSP, LV_PART_SELECTED);
    lv_obj_set_style_text_color(bk_ui->detailsettingdefrost_settemp_setn1, lv_color_hex(0x333333), LV_PART_SELECTED);
    lv_obj_set_style_text_font(bk_ui->detailsettingdefrost_settemp_setn1, &lv_font_scdream_regular_90, LV_PART_SELECTED);
    lv_roller_set_visible_row_count(bk_ui->detailsettingdefrost_settemp_setn1, 3);
    lv_obj_add_event_cb(bk_ui->detailsettingdefrost_settemp_setn1, detailsettingdefrost_roller_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_add_flag(bk_ui->detailsettingdefrost_settemp_setn1, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_pos(bk_ui->detailsettingdefrost_settemp_setn1,  700-10, 136-5-2-1);
    lv_obj_set_width(bk_ui->detailsettingdefrost_settemp_setn1, 240);

    // Button: changebt
    bk_ui->detailsettingdefrost_changebt = lv_button_create(bk_ui->detailsettingdefrost);
    lv_obj_add_flag(bk_ui->detailsettingdefrost_changebt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->detailsettingdefrost_changebt, detailsettingdefrost_changebt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->detailsettingdefrost_changebt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->detailsettingdefrost_changebt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->detailsettingdefrost_changebt, 0, 0);
    lv_obj_set_pos(bk_ui->detailsettingdefrost_changebt, 590, 420);
    lv_obj_set_size(bk_ui->detailsettingdefrost_changebt, 410, 80);

    ui_lang_apply_detailsettingdefrost(bk_ui);
}

rendererFuncStatus_t init_page_detailsettingdefrost_with_step(bk_lv_ui_t *bk_ui)
{
    static uint32_t renderStartTick = 0;

    if(preRenderPageState[PAGE_DETAILSETTINGDEFROST].isRendered)
    {
        return RENDERER_FUNC_DONE;
    }

    switch(currentStep)
    {
        case RENDER_STEP_CREATE_PAGE:
        {
            renderStartTick = lv_tick_get();
            bk_printf(TAG "[RENDER][DETAILSETTINGDEFROST] start tick=%lu\n", (unsigned long)renderStartTick);

            if(bk_ui == NULL)
            {
                return RENDERER_FUNC_FAILED;
            }

            stepInitMode = true;
            init_page_detailsettingdefrost(bk_ui);
            stepInitMode = false;
            if(bk_ui->detailsettingdefrost == NULL || !lv_obj_is_valid(bk_ui->detailsettingdefrost))
            {
                bk_printf(TAG "[RENDER][DETAILSETTINGDEFROST] CREATE_PAGE failed\n");
                return RENDERER_FUNC_FAILED;
            }

#if UI_PRENDERING_ENABLE
            lv_obj_add_flag(bk_ui->detailsettingdefrost, LV_OBJ_FLAG_HIDDEN);
#endif
            currentStep = RENDER_STEP_CREATE_CHILD;
            return RENDERER_FUNC_NOT_DONE;
        }

        case RENDER_STEP_CREATE_CHILD:
        {
            stepInitMode = true;
            init_page_detailsettingdefrost(bk_ui);
            stepInitMode = false;
            currentStep = RENDER_STEP_CACHE_BACKGROUND;
            return RENDERER_FUNC_NOT_DONE;
        }

        case RENDER_STEP_CACHE_BACKGROUND:
        {
            if(preRenderPageConfig[PAGE_DETAILSETTINGDEFROST].backgroundImageAssetId != SHARED_IMAGE_NONE)
            {
                const sharedImageAssetId_t assetId =
                    preRenderPageConfig[PAGE_DETAILSETTINGDEFROST].backgroundImageAssetId;
                if(set_shared_image_asset(bk_ui->detailsettingdefrost_bg, assetId) != RENDERER_FUNC_DONE)
                {
                    return RENDERER_FUNC_FAILED;
                }
            }

            currentStep = RENDER_STEP_CACHE_IMAGE;
            return RENDERER_FUNC_NOT_DONE;
        }

        case RENDER_STEP_CACHE_IMAGE:
        {
            const uint32_t imageCount = preRenderPageConfig[PAGE_DETAILSETTINGDEFROST].preRenderImageCount;
            if(currentImageStep < imageCount)
            {
                const preRenderImageInfo_t *imageInfo =
                    &preRenderPageConfig[PAGE_DETAILSETTINGDEFROST].preRenderImageInfo[currentImageStep];
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
                    bk_printf(TAG "[PREWARM][DETAILSETTINGDEFROST] image %lu/%lu failed: %s (%lu ms)\n",
                              (unsigned long)(currentImageStep + 1),
                              (unsigned long)imageCount,
                              imagePath,
                              (unsigned long)lv_tick_elaps(imageStartTick));
                    return RENDERER_FUNC_FAILED;
                }

                bk_printf(TAG "[PREWARM][DETAILSETTINGDEFROST] image %lu/%lu done: %s (%lu ms)\n",
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
            /* init_page_detailsettingdefrost() also attaches the page and control callbacks. */
            currentStep = RENDER_STEP_DONE;
            return RENDERER_FUNC_NOT_DONE;
        }

        case RENDER_STEP_DONE:
        {
            bk_printf(TAG "[RENDER][DETAILSETTINGDEFROST] done total=%lu ms\n",
                      (unsigned long)lv_tick_elaps(renderStartTick));

            currentStep = RENDER_STEP_CREATE_PAGE;
            currentImageStep = 0;
            renderStartTick = 0;
            preRenderPageState[PAGE_DETAILSETTINGDEFROST].isRendered = true;
            return RENDERER_FUNC_DONE;
        }

        default:
        {
            bk_printf(TAG "[RENDER][DETAILSETTINGDEFROST] invalid step=%lu\n",
                      (unsigned long)currentStep);
            return RENDERER_FUNC_FAILED;
        }
    }
}
