#include "lvgl.h"
#include <stdio.h>
#include "preRenderer.h"
#include "ui_config.h"

#define TAG "[preRenderer.c] "
// #define bk_printf(fmt, ...) do {if(0) bk_printf(fmt, ##__VA_ARGS__); } while(0) // disable printf

extern bk_lv_ui_t bk_lv_tool_ui;
extern lv_obj_t *preRenderRoot;
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

// 여기가 ui_screen_change()의 핵심 로직임. 기존 화면을 숨기고 새 화면을 보여주는 것까지 처리함.
// if (bk_ui->automode != NULL && lv_obj_is_valid(bk_ui->automode)) 
// {   
//     uint32_t elapsed;
//     bk_printf(TAG "[SCREEN] automode already exists, moving to top\n");
//     /*
//     * 여기는 프리힛이 이미 됐다는 소리잖아. 프리힛 된거에 다시 init했다는거는 다른애들 지우고 refresh하면 되는거잖아
//     */
//     bk_ui->main != NULL ? lv_obj_add_flag(bk_ui->main, LV_OBJ_FLAG_HIDDEN) : (void)0;
//     bk_ui->automode != NULL ? lv_obj_add_flag(bk_ui->automode, LV_OBJ_FLAG_HIDDEN) : (void)0;
//     bk_ui->manualmode != NULL ? lv_obj_add_flag(bk_ui->manualmode, LV_OBJ_FLAG_HIDDEN) : (void)0;
//     bk_ui->autodrymode != NULL ? lv_obj_add_flag(bk_ui->autodrymode, LV_OBJ_FLAG_HIDDEN) : (void)0;
//     elapsed = lv_tick_get() - _t_start;
//     bk_printf(TAG "[SCREEN] preRenderCleared elapsed: %u\n", elapsed);
//     lv_obj_move_to_index(bk_ui->automode, -1);
//     lv_obj_remove_flag(bk_ui->automode, LV_OBJ_FLAG_HIDDEN);
//     elapsed = lv_tick_get() - _t_start;
//     bk_printf(TAG "[SCREEN] automode unhidden elapsed: %u\n", elapsed);
//     lv_refr_now(NULL);
//     elapsed = lv_tick_get() - _t_start;
//     bk_printf(TAG "[SCREEN] automode moved to top elapsed: %u\n", elapsed);
//     return;
// }
// screen 변경에 따른 생명주기 관리 함수
// currentPage는 현재 화면을 가리키는 전역 변수로, 이전 화면을 추적하는 데 사용됩니다.
// newScreen은 새로 표시할 화면을 나타내는 매개변수입니다.
// SHOW_START, SHOWN, HIDE_START, HIDDEN 이벤트 발생으로 기존 functions의 event_cb를 대체합니다.
// 화면 전환 자체도 ui_screen_change()에서 처리합니다. (lv_scr_load() 호출)
void ui_page_change(lv_obj_t *newScreen)
{
    if(!uiScreenEventInitialized)
    {
        printf(TAG "[SCREEN] ui_page_change() called before ui_screen_event_init()\n");
        return;
    }

    // TODO : oldPage가 root에 있는 page가 아닐 경우 escape해야됨.

    lv_obj_t *oldPage = currentPage;
    currentPage = newScreen;
    uint32_t _t_start = lv_tick_get();
    uint32_t elapsed;
    bk_printf(TAG
    "[SCREEN] current=%p new=%p main=%p auto=%p manual=%p autodry=%p setting=%p\n",
    oldPage,
    newScreen,
    bk_lv_tool_ui.main,
    bk_lv_tool_ui.automode,
    bk_lv_tool_ui.manualmode,
    bk_lv_tool_ui.autodrymode,
    bk_lv_tool_ui.settingmode
    );
    bk_printf(TAG "[SCREEN] ui_page_change() called\n");
    if(newScreen == NULL || !lv_obj_is_valid(newScreen))
    {
        bk_printf(TAG "[SCREEN] newScreen is NULL or invalid\n");
        //print stack trace
        return;
    }
    if(oldPage == newScreen)
    {
        bk_printf(TAG "[SCREEN] newScreen is the same as currentPage\n");
        return;
    }

    bk_printf(TAG "[SCREEN] valid screen change.\n");
    if(oldPage != NULL && lv_obj_is_valid(oldPage))
    {
        lv_obj_send_event(oldPage, UI_EVENT_PAGE_HIDE_START, NULL);
        lv_obj_add_flag(oldPage, LV_OBJ_FLAG_HIDDEN);
    }
    elapsed = lv_tick_get() - _t_start;
    bk_printf(TAG "[SCREEN] oldPage hidden. [elapsed : %u]\n", elapsed);
    if(oldPage != NULL && lv_obj_is_valid(oldPage))
    {
        lv_obj_send_event(oldPage, UI_EVENT_PAGE_HIDDEN, NULL);
    }

    lv_obj_send_event(newScreen, UI_EVENT_PAGE_SHOW_START, NULL);

    lv_obj_move_to_index(newScreen, -1);
    lv_obj_remove_flag(newScreen, LV_OBJ_FLAG_HIDDEN);

    lv_refr_now(NULL);
    
    elapsed = lv_tick_get() - _t_start;
    bk_printf(TAG "[SCREEN] newScreen shown. [elapsed : %u]\n", elapsed);
    currentPage = newScreen;    

    lv_obj_send_event(newScreen, UI_EVENT_PAGE_SHOWN, NULL);
    elapsed = lv_tick_get() - _t_start;
    bk_printf(TAG "[SCREEN] ui_page_change() completed. [elapsed : %u]\n", elapsed);
}