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

#define TAG "[main_init.c] "
// #define bk_printf(fmt, ...) do {if(0) bk_printf(fmt, ##__VA_ARGS__); } while(0) // disable printf

extern bk_lv_ui_t bk_lv_tool_ui;
extern lv_obj_t *preRenderRoot;
static uint32_t currentStep = 0;
static uint32_t currentImageStep = 0;

extern void lv_digital_clock_register(lv_obj_t *label, int show_second, int use_ampm, int hour, int minute, int second);
extern void lv_digital_date_register(lv_obj_t *label);
extern void lv_digital_clock_unregister(lv_obj_t *label);
extern void lv_digital_date_unregister(lv_obj_t *label);
extern void main_automode_event_cb(lv_event_t *e);
extern void main_manualmode_event_cb(lv_event_t *e);
extern void main_autodrymode_event_cb(lv_event_t *e);
extern void main_memorymode_event_cb(lv_event_t *e);
extern void main_settingmode_event_cb(lv_event_t *e);
extern void main_load_start_event_cb(lv_event_t *e);
extern void main_loaded_event_cb(lv_event_t *e);
extern void main_unload_start_event_cb(lv_event_t *e);
extern void main_unloaded_event_cb(lv_event_t *e);

// void destroy_page_main(bk_lv_ui_t *bk_ui)
// {
//     if (bk_ui == NULL) {
//         return;
//     }
//     if (bk_ui->main != NULL) {
//         lv_obj_del(bk_ui->main);
//         bk_ui->main = NULL;
//     }
// }

void destroy_page_main(bk_lv_ui_t *bk_ui) {
    bk_printf(TAG "[SCREEN] destroy_page_main() called\n");
    if (bk_ui->main != NULL) {
        lv_digital_clock_unregister(bk_ui->main_dclock_1);
        lv_digital_date_unregister(bk_ui->main_timebar_date);
        lv_obj_del(bk_ui->main);
        bk_ui->main = NULL;
    }
    bk_printf(TAG "[SCREEN] destroyed\n");
    
    currentStep = 0;
    currentImageStep = 0;
    preRenderPageState[PAGE_MAIN].isRendered = false;
}

void init_page_main(bk_lv_ui_t * bk_ui) 
{
    uint32_t _t_start = lv_tick_get();
    bk_printf(TAG "[SCREEN] init_page_main() called\n");

    if (bk_ui->main != NULL && lv_obj_is_valid(bk_ui->main)) 
    {
        destroy_page_main(bk_ui);
    }

    // bk_ui->main = lv_obj_create(NULL);
    // lv_obj_set_size(bk_ui->main, 1024, 600);
    // lv_obj_set_scrollbar_mode(bk_ui->main, LV_SCROLLBAR_MODE_OFF);
    // lv_obj_set_style_bg_color(bk_ui->main, lv_color_hex(0xD5D5D5), 0);
    // lv_obj_set_style_bg_opa(bk_ui->main, LV_OPA_COVER, 0);
    // lv_obj_add_event_cb(bk_ui->main, main_load_event_cb, LV_EVENT_SCREEN_LOAD_START, NULL);
    ui_lang_reset_main_cache();

#if UI_PRENDERING_ENABLE
    bk_ui->main = lv_obj_create(preRenderRoot);
    lv_obj_remove_style_all(bk_ui->main);
    lv_obj_set_size(bk_ui->main, 1024, 600);
    lv_obj_set_pos(bk_ui->main, 0, 0);
    lv_obj_set_style_bg_opa(bk_ui->main, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_scrollbar_mode(bk_ui->main, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_event_cb(bk_ui->main, main_load_start_event_cb,   UI_EVENT_PAGE_SHOW_START,  NULL);
    lv_obj_add_event_cb(bk_ui->main, main_loaded_event_cb,       UI_EVENT_PAGE_SHOWN,       NULL);
    lv_obj_add_event_cb(bk_ui->main, main_unload_start_event_cb, UI_EVENT_PAGE_HIDE_START,  NULL);
    lv_obj_add_event_cb(bk_ui->main, main_unloaded_event_cb,     UI_EVENT_PAGE_HIDDEN,      NULL);
#else
    bk_ui->main = lv_obj_create(NULL);
    lv_obj_set_size(bk_ui->main, 1024, 600);
    lv_obj_set_scrollbar_mode(bk_ui->main, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_event_cb(bk_ui->main, main_load_start_event_cb,   LV_EVENT_SCREEN_LOAD_START,   NULL);
    lv_obj_add_event_cb(bk_ui->main, main_loaded_event_cb,       LV_EVENT_SCREEN_LOADED,       NULL);
    lv_obj_add_event_cb(bk_ui->main, main_unload_start_event_cb, LV_EVENT_SCREEN_UNLOAD_START, NULL);
    lv_obj_add_event_cb(bk_ui->main, main_unloaded_event_cb,     LV_EVENT_SCREEN_UNLOADED,     NULL);
#endif
    bk_ui->main_bg = lv_image_create(bk_ui->main);
    
#if !UI_MAIN_COMBINED_BG_ENABLE
    _bg_set(bk_ui->main_bg);
#endif
    lv_obj_set_pos(bk_ui->main_bg, 0, 0);



    // Button: automode
    bk_ui->main_automode = lv_button_create(bk_ui->main);
    lv_obj_add_flag(bk_ui->main_automode, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->main_automode, main_automode_event_cb, LV_EVENT_PRESSED, NULL);
    lv_obj_set_style_bg_opa(bk_ui->main_automode, 0, 0);
    lv_obj_set_style_border_width(bk_ui->main_automode, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->main_automode, 0, 0);
    lv_obj_set_pos(bk_ui->main_automode, 33, 108);
    lv_obj_set_size(bk_ui->main_automode, 288, 158);

    // ImageView: imageview2
#if !UI_MAIN_COMBINED_BG_ENABLE
    bk_ui->main_imageview2 = lv_image_create(bk_ui->main);
    lv_obj_set_pos(bk_ui->main_imageview2, 33, 108);
    lv_obj_set_size(bk_ui->main_imageview2, 288, 158);
#endif

    // Button: manualmode
    bk_ui->main_manualmode = lv_button_create(bk_ui->main);
    lv_obj_add_flag(bk_ui->main_manualmode, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->main_manualmode, main_manualmode_event_cb, LV_EVENT_PRESSED, NULL);
    lv_obj_set_style_bg_opa(bk_ui->main_manualmode, 0, 0);
    lv_obj_set_style_border_width(bk_ui->main_manualmode, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->main_manualmode, 0, 0);
    lv_obj_set_pos(bk_ui->main_manualmode, 368, 108);
    lv_obj_set_size(bk_ui->main_manualmode, 288, 158);

    // ImageView: imageview4
#if !UI_MAIN_COMBINED_BG_ENABLE
    bk_ui->main_imageview4 = lv_image_create(bk_ui->main);
    lv_obj_set_pos(bk_ui->main_imageview4, 368, 108);
    lv_obj_set_size(bk_ui->main_imageview4, 288, 158);
#endif

    // Button: autodrymode
    bk_ui->main_autodrymode = lv_button_create(bk_ui->main);
    lv_obj_add_flag(bk_ui->main_autodrymode, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->main_autodrymode, main_autodrymode_event_cb, LV_EVENT_PRESSED, NULL);
    lv_obj_set_style_bg_opa(bk_ui->main_autodrymode, 0, 0);
    lv_obj_set_style_border_width(bk_ui->main_autodrymode, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->main_autodrymode, 0, 0);
    lv_obj_set_pos(bk_ui->main_autodrymode, 703, 108);
    lv_obj_set_size(bk_ui->main_autodrymode, 288, 158);

    // ImageView: imageview6
#if !UI_MAIN_COMBINED_BG_ENABLE
    bk_ui->main_imageview6 = lv_image_create(bk_ui->main);
    lv_obj_set_pos(bk_ui->main_imageview6, 703, 108);
    lv_obj_set_size(bk_ui->main_imageview6, 288, 158);
#endif

    // Button: memorymode
    bk_ui->main_memorymode = lv_button_create(bk_ui->main);
    lv_obj_add_flag(bk_ui->main_memorymode, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->main_memorymode, main_memorymode_event_cb, LV_EVENT_PRESSED, NULL);
    lv_obj_set_style_bg_opa(bk_ui->main_memorymode, 0, 0);
    lv_obj_set_style_border_width(bk_ui->main_memorymode, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->main_memorymode, 0, 0);
    lv_obj_set_pos(bk_ui->main_memorymode, 200, 323);
    lv_obj_set_size(bk_ui->main_memorymode, 288, 158);

    // ImageView: imageview8
#if !UI_MAIN_COMBINED_BG_ENABLE
    bk_ui->main_imageview8 = lv_image_create(bk_ui->main);
    lv_obj_set_pos(bk_ui->main_imageview8, 200, 323);
    lv_obj_set_size(bk_ui->main_imageview8, 288, 158);
#endif

    // Button: settingmode
    bk_ui->main_settingmode = lv_button_create(bk_ui->main);
    lv_obj_add_flag(bk_ui->main_settingmode, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->main_settingmode, main_settingmode_event_cb, LV_EVENT_PRESSED, NULL);
    lv_obj_set_style_bg_opa(bk_ui->main_settingmode, 0, 0);
    lv_obj_set_style_border_width(bk_ui->main_settingmode, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->main_settingmode, 0, 0);
    lv_obj_set_pos(bk_ui->main_settingmode, 536, 323);
    lv_obj_set_size(bk_ui->main_settingmode, 288, 158);

    // ImageView: imageview10
#if !UI_MAIN_COMBINED_BG_ENABLE
    bk_ui->main_imageview10 = lv_image_create(bk_ui->main);
    lv_obj_set_pos(bk_ui->main_imageview10, 536, 323);
    lv_obj_set_size(bk_ui->main_imageview10, 288, 158);
#endif

    // // TextView: main_timebar_date  (날짜 YYYY.MM.DD)
    // bk_ui->main_timebar_date = lv_label_create(bk_ui->main);
    // lv_label_set_text(bk_ui->main_timebar_date, "");
    // lv_label_set_long_mode(bk_ui->main_timebar_date, LV_LABEL_LONG_MODE_WRAP);
    // lv_obj_set_x(bk_ui->main_timebar_date, 881);
    // lv_obj_set_y(bk_ui->main_timebar_date, 544);
    // lv_obj_set_width(bk_ui->main_timebar_date, 140);
    // lv_obj_set_height(bk_ui->main_timebar_date, 25);
    // lv_obj_set_style_text_color(bk_ui->main_timebar_date, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_set_style_text_opa(bk_ui->main_timebar_date, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_set_style_text_font(bk_ui->main_timebar_date, &lv_font_scdream_regular_22, LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_set_style_text_align(bk_ui->main_timebar_date, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_digital_date_register(bk_ui->main_timebar_date);

    ui_lang_apply_main(bk_ui);

    // // TextView: main_dclock_1  (시간 HH:MM)
    // bk_ui->main_dclock_1 = lv_label_create(bk_ui->main);
    // lv_label_set_text(bk_ui->main_dclock_1, "");
    // lv_digital_clock_register(bk_ui->main_dclock_1, 0, 0, 11, 25, 50);
    // lv_obj_set_x(bk_ui->main_dclock_1, 891);
    // lv_obj_set_y(bk_ui->main_dclock_1, 570);
    // lv_obj_set_width(bk_ui->main_dclock_1, 114);
    // lv_obj_set_height(bk_ui->main_dclock_1, 30);
    // lv_obj_set_style_text_color(bk_ui->main_dclock_1, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_set_style_text_opa(bk_ui->main_dclock_1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_set_style_text_font(bk_ui->main_dclock_1, &lv_font_scdream_regular_22, LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_set_style_text_align(bk_ui->main_dclock_1, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
}

rendererFuncStatus_t init_page_main_with_step(bk_lv_ui_t *bk_ui)
{
    static uint32_t renderStartTick = 0;
    if(preRenderPageState[PAGE_MAIN].isRendered)
    {
        return RENDERER_FUNC_DONE;
    }

    switch (currentStep)
    {
        case RENDER_STEP_CREATE_PAGE :
        {
            uint32_t stepStartTick = lv_tick_get();
            renderStartTick = stepStartTick;

            bk_printf(TAG "[RENDER][MAIN] start tick=%lu\n", (unsigned long)renderStartTick);

            ui_lang_reset_main_cache();

            bk_ui->main = lv_obj_create(preRenderRoot);
            lv_obj_remove_style_all(bk_ui->main);
            lv_obj_add_flag(bk_ui->main, LV_OBJ_FLAG_HIDDEN);
            lv_obj_set_size(bk_ui->main, 1024, 600);
            lv_obj_set_pos(bk_ui->main, 0, 0);
            lv_obj_set_style_bg_opa(bk_ui->main, LV_OPA_TRANSP, LV_PART_MAIN);
            lv_obj_set_style_radius(bk_ui->main, 0, LV_PART_MAIN);
            lv_obj_set_scrollbar_mode(bk_ui->main, LV_SCROLLBAR_MODE_OFF);

            bk_printf(TAG "[RENDER][MAIN] CREATE_PAGE done elapsed=%lu ms\n", (unsigned long)lv_tick_elaps(stepStartTick));

            currentStep = RENDER_STEP_CREATE_CHILD;
            return RENDERER_FUNC_NOT_DONE;
        }
        case RENDER_STEP_CREATE_CHILD :
        {
            uint32_t stepStartTick = lv_tick_get();

            bk_printf(TAG "[RENDER][MAIN] CREATE_CHILD start\n");

            bk_ui->main_bg = lv_image_create(bk_ui->main);
            lv_obj_set_pos(bk_ui->main_bg, 0, 0);

            // Button: automode
            bk_ui->main_automode = lv_button_create(bk_ui->main);
            lv_obj_add_flag(bk_ui->main_automode, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_set_style_bg_opa(bk_ui->main_automode, 0, 0);
            lv_obj_set_style_border_width(bk_ui->main_automode, 0, 0);
            lv_obj_set_style_shadow_width(bk_ui->main_automode, 0, 0);
            lv_obj_set_pos(bk_ui->main_automode, 33, 108);
            lv_obj_set_size(bk_ui->main_automode, 288, 158);

            // ImageView: imageview2
            bk_ui->main_imageview2 = lv_image_create(bk_ui->main);
            _img_set_src_timed(bk_ui->main_imageview2, "/images/automode.png");
            lv_obj_set_pos(bk_ui->main_imageview2, 33, 108);
            lv_obj_set_size(bk_ui->main_imageview2, 288, 158);

            // Button: manualmode
            bk_ui->main_manualmode = lv_button_create(bk_ui->main);
            lv_obj_add_flag(bk_ui->main_manualmode, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_set_style_bg_opa(bk_ui->main_manualmode, 0, 0);
            lv_obj_set_style_border_width(bk_ui->main_manualmode, 0, 0);
            lv_obj_set_style_shadow_width(bk_ui->main_manualmode, 0, 0);
            lv_obj_set_pos(bk_ui->main_manualmode, 368, 108);
            lv_obj_set_size(bk_ui->main_manualmode, 288, 158);

            // ImageView: imageview4
            bk_ui->main_imageview4 = lv_image_create(bk_ui->main);
            _img_set_src_timed(bk_ui->main_imageview4, "/images/manualmode.png");
            lv_obj_set_pos(bk_ui->main_imageview4, 368, 108);
            lv_obj_set_size(bk_ui->main_imageview4, 288, 158);

            // Button: autodrymode
            bk_ui->main_autodrymode = lv_button_create(bk_ui->main);
            lv_obj_add_flag(bk_ui->main_autodrymode, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_set_style_bg_opa(bk_ui->main_autodrymode, 0, 0);
            lv_obj_set_style_border_width(bk_ui->main_autodrymode, 0, 0);
            lv_obj_set_style_shadow_width(bk_ui->main_autodrymode, 0, 0);
            lv_obj_set_pos(bk_ui->main_autodrymode, 703, 108);
            lv_obj_set_size(bk_ui->main_autodrymode, 288, 158);

            // ImageView: imageview6
            bk_ui->main_imageview6 = lv_image_create(bk_ui->main);
            _img_set_src_timed(bk_ui->main_imageview6, "/images/autodrymode.png");
            lv_obj_set_pos(bk_ui->main_imageview6, 703, 108);
            lv_obj_set_size(bk_ui->main_imageview6, 288, 158);

            // Button: memorymode
            bk_ui->main_memorymode = lv_button_create(bk_ui->main);
            lv_obj_add_flag(bk_ui->main_memorymode, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_set_style_bg_opa(bk_ui->main_memorymode, 0, 0);
            lv_obj_set_style_border_width(bk_ui->main_memorymode, 0, 0);
            lv_obj_set_style_shadow_width(bk_ui->main_memorymode, 0, 0);
            lv_obj_set_pos(bk_ui->main_memorymode, 200, 323);
            lv_obj_set_size(bk_ui->main_memorymode, 288, 158);

            // ImageView: imageview8
            bk_ui->main_imageview8 = lv_image_create(bk_ui->main);
            _img_set_src_timed(bk_ui->main_imageview8, "/images/memorymode.png");
            lv_obj_set_pos(bk_ui->main_imageview8, 200, 323);
            lv_obj_set_size(bk_ui->main_imageview8, 288, 158);

            // Button: settingmode
            bk_ui->main_settingmode = lv_button_create(bk_ui->main);
            lv_obj_add_flag(bk_ui->main_settingmode, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_set_style_bg_opa(bk_ui->main_settingmode, 0, 0);
            lv_obj_set_style_border_width(bk_ui->main_settingmode, 0, 0);
            lv_obj_set_style_shadow_width(bk_ui->main_settingmode, 0, 0);
            lv_obj_set_pos(bk_ui->main_settingmode, 536, 323);
            lv_obj_set_size(bk_ui->main_settingmode, 288, 158);

            // ImageView: imageview10
            bk_ui->main_imageview10 = lv_image_create(bk_ui->main);
            _img_set_src_timed(bk_ui->main_imageview10, "/images/settingmode.png");
            lv_obj_set_pos(bk_ui->main_imageview10, 536, 323);
            lv_obj_set_size(bk_ui->main_imageview10, 288, 158);

            ui_lang_apply_main(bk_ui);

            bk_printf(TAG "[RENDER][MAIN] CREATE_CHILD done elapsed=%lu ms total=%lu ms\n",
                      (unsigned long)lv_tick_elaps(stepStartTick),
                      (unsigned long)lv_tick_elaps(renderStartTick));

            currentStep = RENDER_STEP_CACHE_BACKGROUND;
            return RENDERER_FUNC_NOT_DONE;
        }
        case RENDER_STEP_CACHE_BACKGROUND :
        {
            if(preRenderPageConfig[PAGE_MAIN].backgroundImageAssetId != SHARED_IMAGE_NONE)
            {
                const sharedImageAssetId_t bgImageId = preRenderPageConfig[PAGE_MAIN].backgroundImageAssetId;
                set_shared_image_asset(bk_ui->main_bg, bgImageId);
            }
            currentStep = RENDER_STEP_CACHE_IMAGE;
            return RENDERER_FUNC_NOT_DONE;
        }
        case RENDER_STEP_CACHE_IMAGE :
        {
            const uint32_t imageCount = preRenderPageConfig[PAGE_MAIN].preRenderImageCount;

            bk_printf(TAG "[PREWARM][MAIN] CACHE_IMAGE start (%lu images)\n", (unsigned long)imageCount);

            if(currentImageStep < imageCount)
            {
                const preRenderImageInfo_t *imageInfo = &preRenderPageConfig[PAGE_MAIN].preRenderImageInfo[currentImageStep];
                char imagePath[128] = {0};
                const bool hasVariant = imageInfo->hasLanguageVariant;
                uint32_t imageStartTick = lv_tick_get();
                if(hasVariant)
                {
                    const char* lang = settings_get_int("LANGUAGE") == 1 ? "_china.png" : settings_get_int("LANGUAGE") == 2 ? "_english.png" : ".png"; 
                    snprintf(imagePath, sizeof(imagePath), "%s%s", imageInfo->imagePath, lang);
                }
                else
                {
                    snprintf(imagePath, sizeof(imagePath), "%s.png", imageInfo->imagePath);
                }


                bk_printf(TAG "[PREWARM][MAIN] image %lu/%lu start: %s\n",
                          (unsigned long)(currentImageStep + 1),
                          (unsigned long)imageCount,
                          imagePath);

                lv_result_t res = lv_image_decoder_prewarm(imagePath);
                uint32_t imageElapsed = lv_tick_elaps(imageStartTick);

                if(res != LV_RESULT_OK)
                {
                    bk_printf(TAG "[PREWARM][MAIN] image %lu/%lu FAILED res=%d elapsed=%lu ms path=%s\n",
                              (unsigned long)(currentImageStep + 1),
                              (unsigned long)imageCount,
                              (int)res,
                              (unsigned long)imageElapsed,
                              imagePath);
                    return RENDERER_FUNC_FAILED;
                }

                bk_printf(TAG "[PREWARM][MAIN] image %lu/%lu OK elapsed=%lu ms path=%s\n",
                          (unsigned long)(currentImageStep + 1),
                          (unsigned long)imageCount,
                          (unsigned long)imageElapsed,
                          imagePath);

                currentImageStep++;
                return RENDERER_FUNC_NOT_DONE;
            }

            bk_printf(TAG "[PREWARM][MAIN] all images done count=%lu total=%lu ms\n",
                      (unsigned long)imageCount,
                      (unsigned long)lv_tick_elaps(renderStartTick));

            currentImageStep = 0;
            currentStep = RENDER_STEP_ATTACH_EVENT;
            return RENDERER_FUNC_NOT_DONE;
        }
        case RENDER_STEP_ATTACH_EVENT :
        {
            uint32_t stepStartTick = lv_tick_get();

            bk_printf(TAG "[RENDER][MAIN] ATTACH_EVENT start\n");

            lv_obj_add_event_cb(bk_ui->main, main_load_start_event_cb, UI_EVENT_PAGE_SHOW_START, NULL);
            lv_obj_add_event_cb(bk_ui->main, main_loaded_event_cb, UI_EVENT_PAGE_SHOWN, NULL);
            lv_obj_add_event_cb(bk_ui->main, main_unload_start_event_cb, UI_EVENT_PAGE_HIDE_START, NULL);
            lv_obj_add_event_cb(bk_ui->main, main_unloaded_event_cb, UI_EVENT_PAGE_HIDDEN, NULL);

            lv_obj_add_event_cb(bk_ui->main_automode, main_automode_event_cb, LV_EVENT_PRESSED, NULL);
            lv_obj_add_event_cb(bk_ui->main_manualmode, main_manualmode_event_cb, LV_EVENT_PRESSED, NULL);
            lv_obj_add_event_cb(bk_ui->main_autodrymode, main_autodrymode_event_cb, LV_EVENT_PRESSED, NULL);
            lv_obj_add_event_cb(bk_ui->main_memorymode, main_memorymode_event_cb, LV_EVENT_PRESSED, NULL);
            lv_obj_add_event_cb(bk_ui->main_settingmode, main_settingmode_event_cb, LV_EVENT_PRESSED, NULL);

            bk_printf(TAG "[RENDER][MAIN] ATTACH_EVENT done elapsed=%lu ms\n",
                      (unsigned long)lv_tick_elaps(stepStartTick));

            currentStep = RENDER_STEP_DONE;
            return RENDERER_FUNC_NOT_DONE;
        }
        case RENDER_STEP_DONE :
        {
            uint32_t totalElapsed = lv_tick_elaps(renderStartTick);

            bk_printf(TAG "[RENDER][MAIN] DONE total=%lu ms\n", (unsigned long)totalElapsed);

            currentStep = 0;
            currentImageStep = 0;
            renderStartTick = 0;
            preRenderPageState[PAGE_MAIN].isRendered = true;
            return RENDERER_FUNC_DONE;
        }
        default :
        {
            bk_printf(TAG "[RENDER][MAIN] INVALID STEP: %lu\n", (unsigned long)currentStep);
            return RENDERER_FUNC_FAILED;
        }
    }
}
