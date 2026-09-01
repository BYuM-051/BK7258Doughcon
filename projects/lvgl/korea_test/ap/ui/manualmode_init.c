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

#define TAG "[manualmode_init.c] "
// #define bk_printf(fmt, ...) do {if(0) printf(fmt, ##__VA_ARGS__); } while(0) // disable printf

static uint32_t currentStep = 0;
static uint32_t currentImageStep = 0;

extern bk_lv_ui_t bk_lv_tool_ui;
extern lv_obj_t *preRenderRoot;
extern void manualmode_backbt_event_cb(lv_event_t *e);
extern void manualmode_manual_freezebt_event_cb(lv_event_t *e);
extern void manualmode_manual_defrostbt_event_cb(lv_event_t *e);
extern void manualmode_manual_fermentationbt_event_cb(lv_event_t *e);
extern void manualmode_load_start_event_cb(lv_event_t *e);
extern void manualmode_loaded_event_cb(lv_event_t *e);
extern void manualmode_unload_start_event_cb(lv_event_t *e);
extern void manualmode_unloaded_event_cb(lv_event_t *e);

void destroy_page_manualmode(bk_lv_ui_t *bk_ui)
{
    if (bk_ui == NULL) {
        return;
    }
    if (bk_ui->manualmode != NULL) {
        lv_obj_del(bk_ui->manualmode);
        bk_ui->manualmode = NULL;
    }

    currentStep = 0;
    currentImageStep = 0;
    preRenderPageState[PAGE_MANUALMODE].isRendered = false;
}

// NOTE : discontinued initialize method. check _with_step()
void init_page_manualmode(bk_lv_ui_t * bk_ui) 
{
    uint32_t _t_start = lv_tick_get();
    bk_printf(TAG "[SCREEN] init_page_manualmode start\n");

    if (bk_ui->manualmode != NULL && lv_obj_is_valid(bk_ui->manualmode)) 
    {
        destroy_page_manualmode(bk_ui);
    }
#if UI_PRENDERING_ENABLE
    ui_lang_reset_manualmode_cache();
    bk_ui->manualmode = lv_obj_create(preRenderRoot);
    lv_obj_remove_style_all(bk_ui->manualmode);
    lv_obj_set_size(bk_ui->manualmode, 1024, 600);
    lv_obj_set_scrollbar_mode(bk_ui->manualmode, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_event_cb(bk_ui->manualmode, manualmode_load_start_event_cb, UI_EVENT_PAGE_SHOW_START, NULL);
    lv_obj_add_event_cb(bk_ui->manualmode, manualmode_loaded_event_cb, UI_EVENT_PAGE_SHOWN,     NULL);
    lv_obj_add_event_cb(bk_ui->manualmode, manualmode_unload_start_event_cb, UI_EVENT_PAGE_HIDE_START, NULL);
    lv_obj_add_event_cb(bk_ui->manualmode, manualmode_unloaded_event_cb, UI_EVENT_PAGE_HIDDEN,     NULL);
    lv_obj_set_style_bg_color(bk_ui->manualmode, lv_color_hex(0xD9D9D9), 0);
    lv_obj_set_style_radius(bk_ui->manualmode, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(bk_ui->manualmode, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(bk_ui->manualmode, LV_OPA_COVER, LV_PART_MAIN);
#else
    ui_lang_reset_manualmode_cache();
    bk_ui->manualmode = lv_obj_create(NULL);
    lv_obj_set_size(bk_ui->manualmode, 1024, 600);
    lv_obj_set_scrollbar_mode(bk_ui->manualmode, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_event_cb(bk_ui->manualmode, manualmode_load_start_event_cb, LV_EVENT_SCREEN_LOAD_START, NULL);
    lv_obj_add_event_cb(bk_ui->manualmode, manualmode_loaded_event_cb, LV_EVENT_SCREEN_LOADED,     NULL);
    lv_obj_add_event_cb(bk_ui->manualmode, manualmode_unload_start_event_cb, LV_EVENT_SCREEN_UNLOAD_START, NULL);
    lv_obj_add_event_cb(bk_ui->manualmode, manualmode_unloaded_event_cb, LV_EVENT_SCREEN_UNLOADED,     NULL);
    lv_obj_set_style_bg_color(bk_ui->manualmode, lv_color_hex(0xD9D9D9), 0);
#endif /* UI_PRENDERING_ENABLE */

    // bk_ui->manualmode_bg = lv_image_create(bk_ui->manualmode);
    // _img_set_src_timed(bk_ui->manualmode_bg, "/images/manual_bg.jpg");
    // lv_obj_set_pos(bk_ui->manualmode_bg, 0, 0);

    // ImageView: title
    bk_ui->manualmode_title = lv_image_create(bk_ui->manualmode);
    _img_set_src_timed(bk_ui->manualmode_title, "/images/manualmode_title.png");
    lv_obj_set_pos(bk_ui->manualmode_title, 0, 10);
    lv_obj_set_size(bk_ui->manualmode_title, 380, 80);
    lv_image_set_inner_align(bk_ui->manualmode_title, LV_IMAGE_ALIGN_TOP_LEFT);

    // Button: backbt
    bk_ui->manualmode_backbt = lv_button_create(bk_ui->manualmode);
    lv_obj_add_flag(bk_ui->manualmode_backbt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->manualmode_backbt, manualmode_backbt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->manualmode_backbt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->manualmode_backbt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->manualmode_backbt, 0, 0);
    lv_obj_set_pos(bk_ui->manualmode_backbt, 13, 445);
    lv_obj_set_size(bk_ui->manualmode_backbt, 179, 74);

    // ImageView: imageview3
    bk_ui->manualmode_imageview3 = lv_image_create(bk_ui->manualmode);
    _img_set_src_timed(bk_ui->manualmode_imageview3, "/images/exit_bt.png");
    lv_obj_set_pos(bk_ui->manualmode_imageview3, 13, 445);
    lv_obj_set_size(bk_ui->manualmode_imageview3, 179, 74);

    // ImageView: imageview4
    bk_ui->manualmode_imageview4 = lv_image_create(bk_ui->manualmode);
    _img_set_src_timed(bk_ui->manualmode_imageview4, "/images/manual_menu.png");
    /* manual_menu.jpg 원본(1024x340, 상하 30px 투명)에서 상하 28px씩 크롭 후
     * 잔여 2px 투명 여백은 화면 배경색(0xD9D9D9)으로 flatten하여 1024x284로 재저장 —
     * 색상 블록 위치 유지 위해 y를 28px 내림 */
    lv_obj_set_pos(bk_ui->manualmode_imageview4, 0, 109+28);
    lv_obj_set_size(bk_ui->manualmode_imageview4, 1024, 284);

    // Button: manual_freezebt
    bk_ui->manualmode_manual_freezebt = lv_button_create(bk_ui->manualmode);
    lv_obj_add_flag(bk_ui->manualmode_manual_freezebt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->manualmode_manual_freezebt, manualmode_manual_freezebt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->manualmode_manual_freezebt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->manualmode_manual_freezebt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->manualmode_manual_freezebt, 0, 0);
    lv_obj_set_pos(bk_ui->manualmode_manual_freezebt, 0, 128);
    lv_obj_set_size(bk_ui->manualmode_manual_freezebt, 334, 290);

    // Button: manual_defrostbt
    bk_ui->manualmode_manual_defrostbt = lv_button_create(bk_ui->manualmode);
    lv_obj_add_flag(bk_ui->manualmode_manual_defrostbt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->manualmode_manual_defrostbt, manualmode_manual_defrostbt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->manualmode_manual_defrostbt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->manualmode_manual_defrostbt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->manualmode_manual_defrostbt, 0, 0);
    lv_obj_set_pos(bk_ui->manualmode_manual_defrostbt, 342, 128);
    lv_obj_set_size(bk_ui->manualmode_manual_defrostbt, 334, 290);

    // Button: manual_fermentationbt
    bk_ui->manualmode_manual_fermentationbt = lv_button_create(bk_ui->manualmode);
    lv_obj_add_flag(bk_ui->manualmode_manual_fermentationbt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->manualmode_manual_fermentationbt, manualmode_manual_fermentationbt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->manualmode_manual_fermentationbt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->manualmode_manual_fermentationbt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->manualmode_manual_fermentationbt, 0, 0);
    lv_obj_set_pos(bk_ui->manualmode_manual_fermentationbt, 685, 128);
    lv_obj_set_size(bk_ui->manualmode_manual_fermentationbt, 334, 290);

}

rendererFuncStatus_t init_page_manualmode_with_step(bk_lv_ui_t *bk_ui)
{
    static uint32_t renderStartTick = 0;

    if(preRenderPageState[PAGE_MANUALMODE].isRendered)
    {
        return RENDERER_FUNC_DONE;
    }

    switch (currentStep)
    {
        case RENDER_STEP_CREATE_PAGE :
        {
            uint32_t stepStartTick = lv_tick_get();
            renderStartTick = stepStartTick;

            bk_printf(TAG "[RENDER][MANUALMODE] start tick=%lu\n", (unsigned long)renderStartTick);

            ui_lang_reset_manualmode_cache();

            bk_ui->manualmode = lv_obj_create(preRenderRoot);
            lv_obj_add_flag(bk_ui->manualmode, LV_OBJ_FLAG_HIDDEN);
            lv_obj_remove_style_all(bk_ui->manualmode);
            lv_obj_set_size(bk_ui->manualmode, 1024, 600);
            lv_obj_set_scrollbar_mode(bk_ui->manualmode, LV_SCROLLBAR_MODE_OFF);
            lv_obj_set_style_bg_color(bk_ui->manualmode, lv_color_hex(0xD9D9D9), 0);
            lv_obj_set_style_radius(bk_ui->manualmode, 0, LV_PART_MAIN);
            lv_obj_set_style_border_width(bk_ui->manualmode, 0, LV_PART_MAIN);
            lv_obj_set_style_bg_opa(bk_ui->manualmode, LV_OPA_COVER, LV_PART_MAIN);

            bk_printf(TAG "[RENDER][MANUALMODE] CREATE_PAGE done elapsed=%lu ms\n",
                      (unsigned long)lv_tick_elaps(stepStartTick));

            currentStep = RENDER_STEP_CREATE_CHILD;
            return RENDERER_FUNC_NOT_DONE;
        }
        case RENDER_STEP_CREATE_CHILD :
        {
            uint32_t stepStartTick = lv_tick_get();

            bk_printf(TAG "[RENDER][MANUALMODE] CREATE_CHILD start\n");

            // ImageView: title
            bk_ui->manualmode_title = lv_image_create(bk_ui->manualmode);
            _img_set_src_timed(bk_ui->manualmode_title, "/images/manualmode_title.png");
            lv_obj_set_pos(bk_ui->manualmode_title, 0, 10);
            lv_obj_set_size(bk_ui->manualmode_title, 380, 80);
            lv_image_set_inner_align(bk_ui->manualmode_title, LV_IMAGE_ALIGN_TOP_LEFT);

            // Button: backbt
            bk_ui->manualmode_backbt = lv_button_create(bk_ui->manualmode);
            lv_obj_add_flag(bk_ui->manualmode_backbt, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_set_style_bg_opa(bk_ui->manualmode_backbt, 0, 0);
            lv_obj_set_style_border_width(bk_ui->manualmode_backbt, 0, 0);
            lv_obj_set_style_shadow_width(bk_ui->manualmode_backbt, 0, 0);
            lv_obj_set_pos(bk_ui->manualmode_backbt, 13, 445);
            lv_obj_set_size(bk_ui->manualmode_backbt, 179, 74);

            // ImageView: imageview3
            bk_ui->manualmode_imageview3 = lv_image_create(bk_ui->manualmode);
            _img_set_src_timed(bk_ui->manualmode_imageview3, "/images/exit_bt.png");
            lv_obj_set_pos(bk_ui->manualmode_imageview3, 13, 445);
            lv_obj_set_size(bk_ui->manualmode_imageview3, 179, 74);

            // ImageView: imageview4
            bk_ui->manualmode_imageview4 = lv_image_create(bk_ui->manualmode);
            _img_set_src_timed(bk_ui->manualmode_imageview4, "/images/manual_menu.png");
            /* manual_menu.jpg 원본(1024x340, 상하 30px 투명)에서 상하 28px씩 크롭 후
             * 잔여 2px 투명 여백은 화면 배경색(0xD9D9D9)으로 flatten하여 1024x284로 재저장 —
             * 색상 블록 위치 유지 위해 y를 28px 내림 */
            lv_obj_set_pos(bk_ui->manualmode_imageview4, 0, 109+28);
            lv_obj_set_size(bk_ui->manualmode_imageview4, 1024, 284);

            // Button: manual_freezebt
            bk_ui->manualmode_manual_freezebt = lv_button_create(bk_ui->manualmode);
            lv_obj_add_flag(bk_ui->manualmode_manual_freezebt, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_set_style_bg_opa(bk_ui->manualmode_manual_freezebt, 0, 0);
            lv_obj_set_style_border_width(bk_ui->manualmode_manual_freezebt, 0, 0);
            lv_obj_set_style_shadow_width(bk_ui->manualmode_manual_freezebt, 0, 0);
            lv_obj_set_pos(bk_ui->manualmode_manual_freezebt, 0, 128);
            lv_obj_set_size(bk_ui->manualmode_manual_freezebt, 334, 290);

            // Button: manual_defrostbt
            bk_ui->manualmode_manual_defrostbt = lv_button_create(bk_ui->manualmode);
            lv_obj_add_flag(bk_ui->manualmode_manual_defrostbt, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_set_style_bg_opa(bk_ui->manualmode_manual_defrostbt, 0, 0);
            lv_obj_set_style_border_width(bk_ui->manualmode_manual_defrostbt, 0, 0);
            lv_obj_set_style_shadow_width(bk_ui->manualmode_manual_defrostbt, 0, 0);
            lv_obj_set_pos(bk_ui->manualmode_manual_defrostbt, 342, 128);
            lv_obj_set_size(bk_ui->manualmode_manual_defrostbt, 334, 290);

            // Button: manual_fermentationbt
            bk_ui->manualmode_manual_fermentationbt = lv_button_create(bk_ui->manualmode);
            lv_obj_add_flag(bk_ui->manualmode_manual_fermentationbt, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_set_style_bg_opa(bk_ui->manualmode_manual_fermentationbt, 0, 0);
            lv_obj_set_style_border_width(bk_ui->manualmode_manual_fermentationbt, 0, 0);
            lv_obj_set_style_shadow_width(bk_ui->manualmode_manual_fermentationbt, 0, 0);
            lv_obj_set_pos(bk_ui->manualmode_manual_fermentationbt, 685, 128);
            lv_obj_set_size(bk_ui->manualmode_manual_fermentationbt, 334, 290);

            bk_printf(TAG "[RENDER][MANUALMODE] CREATE_CHILD done elapsed=%lu ms total=%lu ms\n",
                      (unsigned long)lv_tick_elaps(stepStartTick),
                      (unsigned long)lv_tick_elaps(renderStartTick));

            currentStep = RENDER_STEP_CACHE_BACKGROUND;
            return RENDERER_FUNC_NOT_DONE;
        }
        case RENDER_STEP_CACHE_BACKGROUND :
        {
            if(preRenderPageConfig[PAGE_MANUALMODE].backgroundImageAssetId != SHARED_IMAGE_NONE)
            {
                const sharedImageAssetId_t bgImageId = preRenderPageConfig[PAGE_MANUALMODE].backgroundImageAssetId;
                set_shared_image_asset(NULL, bgImageId);
            }
            currentStep = RENDER_STEP_CACHE_IMAGE;
            return RENDERER_FUNC_NOT_DONE;
        }
        case RENDER_STEP_CACHE_IMAGE :
        {
            const uint32_t imageCount = preRenderPageConfig[PAGE_MANUALMODE].preRenderImageCount;

            bk_printf(TAG "[PREWARM][MANUALMODE] CACHE_IMAGE start (%lu images)\n", (unsigned long)imageCount);

            if(currentImageStep < imageCount)
            {
                const preRenderImageInfo_t *imageInfo = &preRenderPageConfig[PAGE_MANUALMODE].preRenderImageInfo[currentImageStep];
                char imagePath[128] = {0};
                const bool hasVariant = imageInfo->hasLanguageVariant;
                uint32_t imageStartTick = lv_tick_get();
                if(hasVariant)
                {
                    const char *lang = settings_get_int("LANGUAGE") == 1 ? "_china.png" : settings_get_int("LANGUAGE") == 2 ? "_english.png" : ".png";
                    snprintf(imagePath, sizeof(imagePath), "%s%s", imageInfo->imagePath, lang);
                }
                else
                {
                    snprintf(imagePath, sizeof(imagePath), "%s.png", imageInfo->imagePath);
                }

                bk_printf(TAG "[PREWARM][MANUALMODE] image %lu/%lu start: %s\n",
                          (unsigned long)(currentImageStep + 1),
                          (unsigned long)imageCount,
                          imagePath);

                lv_result_t res = lv_image_decoder_prewarm(imagePath);
                uint32_t imageElapsed = lv_tick_elaps(imageStartTick);

                if(res != LV_RESULT_OK)
                {
                    bk_printf(TAG "[PREWARM][MANUALMODE] image %lu/%lu FAILED res=%d elapsed=%lu ms path=%s\n",
                              (unsigned long)(currentImageStep + 1),
                              (unsigned long)imageCount,
                              (int)res,
                              (unsigned long)imageElapsed,
                              imagePath);
                    return RENDERER_FUNC_FAILED;
                }

                bk_printf(TAG "[PREWARM][MANUALMODE] image %lu/%lu OK elapsed=%lu ms path=%s\n",
                          (unsigned long)(currentImageStep + 1),
                          (unsigned long)imageCount,
                          (unsigned long)imageElapsed,
                          imagePath);

                currentImageStep++;
                return RENDERER_FUNC_NOT_DONE;
            }

            bk_printf(TAG "[PREWARM][MANUALMODE] all images done count=%lu total=%lu ms\n",
                      (unsigned long)imageCount,
                      (unsigned long)lv_tick_elaps(renderStartTick));

            currentImageStep = 0;
            currentStep = RENDER_STEP_ATTACH_EVENT;
            return RENDERER_FUNC_NOT_DONE;
        }
        case RENDER_STEP_ATTACH_EVENT :
        {
            uint32_t stepStartTick = lv_tick_get();

            bk_printf(TAG "[RENDER][MANUALMODE] ATTACH_EVENT start\n");

            lv_obj_add_event_cb(bk_ui->manualmode, manualmode_load_start_event_cb, UI_EVENT_PAGE_SHOW_START, NULL);
            lv_obj_add_event_cb(bk_ui->manualmode, manualmode_loaded_event_cb, UI_EVENT_PAGE_SHOWN, NULL);
            lv_obj_add_event_cb(bk_ui->manualmode, manualmode_unload_start_event_cb, UI_EVENT_PAGE_HIDE_START, NULL);
            lv_obj_add_event_cb(bk_ui->manualmode, manualmode_unloaded_event_cb, UI_EVENT_PAGE_HIDDEN, NULL);

            lv_obj_add_event_cb(bk_ui->manualmode_backbt, manualmode_backbt_event_cb, LV_EVENT_PRESSED, NULL);
            lv_obj_add_event_cb(bk_ui->manualmode_manual_freezebt, manualmode_manual_freezebt_event_cb, LV_EVENT_PRESSED, NULL);
            lv_obj_add_event_cb(bk_ui->manualmode_manual_defrostbt, manualmode_manual_defrostbt_event_cb, LV_EVENT_PRESSED, NULL);
            lv_obj_add_event_cb(bk_ui->manualmode_manual_fermentationbt, manualmode_manual_fermentationbt_event_cb, LV_EVENT_PRESSED, NULL);

            bk_printf(TAG "[RENDER][MANUALMODE] ATTACH_EVENT done elapsed=%lu ms\n",
                      (unsigned long)lv_tick_elaps(stepStartTick));

            currentStep = RENDER_STEP_DONE;
            return RENDERER_FUNC_NOT_DONE;
        }
        case RENDER_STEP_DONE :
        {
            uint32_t totalElapsed = lv_tick_elaps(renderStartTick);

            bk_printf(TAG "[RENDER][MANUALMODE] DONE total=%lu ms\n", (unsigned long)totalElapsed);

            currentStep = 0;
            currentImageStep = 0;
            renderStartTick = 0;
            preRenderPageState[PAGE_MANUALMODE].isRendered = true;
            return RENDERER_FUNC_DONE;
        }
        default :
        {
            bk_printf(TAG "[RENDER][MANUALMODE] INVALID STEP: %lu\n", (unsigned long)currentStep);
            return RENDERER_FUNC_FAILED;
        }
    }
}
