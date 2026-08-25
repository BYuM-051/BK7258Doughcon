#include "lvgl.h"

#include "preRenderer.h"
#include "ui_config.h"

#define TAG "[preRenderer.c] "
// #define bk_printf(fmt, ...) do {if(0) bk_printf(fmt, ##__VA_ARGS__); } while(0) // disable printf

extern bk_lv_ui_t bk_lv_tool_ui;
lv_obj_t *preRenderRoot = NULL;
lv_obj_t *currentPage = NULL;

#if !UI_PRENDERING_ENABLE
lv_event_code_t UI_EVENT_PAGE_SHOW_START = LV_EVENT_SCREEN_LOAD_START;
lv_event_code_t UI_EVENT_PAGE_SHOWN = LV_EVENT_SCREEN_LOADED;
lv_event_code_t UI_EVENT_PAGE_HIDE_START = LV_EVENT_SCREEN_UNLOAD_START;
lv_event_code_t UI_EVENT_PAGE_HIDDEN = LV_EVENT_SCREEN_UNLOADED;
#else
lv_event_code_t UI_EVENT_PAGE_SHOW_START;
lv_event_code_t UI_EVENT_PAGE_SHOWN;
lv_event_code_t UI_EVENT_PAGE_HIDE_START;
lv_event_code_t UI_EVENT_PAGE_HIDDEN;
#endif

static bool uiScreenEventInitialized = false;

void ui_screen_event_init()
{
    if(uiScreenEventInitialized) {return;}
    uiScreenEventInitialized = true;

#if UI_PRENDERING_ENABLE
    UI_EVENT_PAGE_SHOW_START = lv_event_register_id();
    UI_EVENT_PAGE_SHOWN = lv_event_register_id();
    UI_EVENT_PAGE_HIDE_START = lv_event_register_id();
    UI_EVENT_PAGE_HIDDEN = lv_event_register_id();
#endif
}

bool ui_screen_event_initialized(void)
{
    return uiScreenEventInitialized;
}

void ui_screen_change(lv_obj_t *newScreen)
{
    bk_printf(TAG "[SCREEN] ui_screen_change() called\n");
    if(newScreen == NULL || !lv_obj_is_valid(newScreen))
    {
        bk_printf(TAG "[SCREEN] newScreen is NULL or invalid\n");
        return;
    }
    if(currentPage == newScreen)
    {
        bk_printf(TAG "[SCREEN] newScreen is the same as currentPage\n");
        return;
    }

    bk_printf(TAG "[SCREEN] valid screen change.\n");
    if(currentPage != NULL && lv_obj_is_valid(currentPage))
    {
        lv_obj_send_event(currentPage, UI_EVENT_PAGE_HIDE_START, NULL);
        lv_obj_add_flag(currentPage, LV_OBJ_FLAG_HIDDEN);
    }

    lv_obj_send_event(newScreen, UI_EVENT_PAGE_SHOW_START, NULL);

    lv_obj_move_to_index(newScreen, -1);
    lv_obj_remove_flag(newScreen, LV_OBJ_FLAG_HIDDEN);

    lv_refr_now(NULL);

    if(currentPage != NULL && lv_obj_is_valid(currentPage))
    {
        lv_obj_send_event(currentPage, UI_EVENT_PAGE_HIDDEN, NULL);
    }

    currentPage = newScreen;

    lv_obj_send_event(newScreen, UI_EVENT_PAGE_SHOWN, NULL);
}