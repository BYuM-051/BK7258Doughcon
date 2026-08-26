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

static inline bool isObjInRoot(lv_obj_t *obj)
{
    if(obj == NULL || preRenderRoot == NULL ||
       !lv_obj_is_valid(obj) || !lv_obj_is_valid(preRenderRoot))
    {
        return false;
    }

    lv_obj_t *parent = lv_obj_get_parent(obj);
    while(parent != NULL)
    {
        if(parent == preRenderRoot)
        {
            return true;
        }
        parent = lv_obj_get_parent(parent);
    }
    return false;
}
#endif

static bool uiScreenEventInitialized = false;

void ui_screen_event_init(void)
{
    if(uiScreenEventInitialized)
    {
        return;
    }
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
void ui_page_change(lv_obj_t *newPage)
{
    if(!uiScreenEventInitialized)
    {
        printf(TAG "[SCREEN] ui_page_change() called before ui_screen_event_init()\n");
        return;
    }

    uint32_t startTick = lv_tick_get();
    lv_obj_t *oldPage = currentPage;

    bk_printf(TAG "[SCREEN] ui_page_change(%p -> %p) called\n", oldPage, newPage);

    if(newPage == NULL || !lv_obj_is_valid(newPage))
    {
        bk_printf(TAG "[SCREEN] newPage is NULL or invalid\n");
        void *caller = __builtin_return_address(0);
        bk_printf(TAG "[CALLER] %p\n", caller); // Print returning addr
        return;
    }
    if(oldPage == newPage)
    {
        bk_printf(TAG "[SCREEN] newPage is the same as currentPage\n");
        return;
    }

    bool newPageInRoot = isObjInRoot(newPage);
    bool newPageIsScreenItself = !newPageInRoot && lv_obj_get_parent(newPage) == NULL;

    if(newPageInRoot && lv_obj_get_parent(newPage) != preRenderRoot)
    {
        bk_printf(TAG "[SCREEN] newPage is inside a page, but is not a direct child of preRenderRoot\n");
        return;
    }
    if(!newPageInRoot && !newPageIsScreenItself)
    {
        bk_printf(TAG "[SCREEN] newPage is neither a root page nor a standalone screen\n");
        return;
    }
    if(newPage == preRenderRoot)
    {
        bk_printf(TAG "[SCREEN] pass a child page instead of preRenderRoot itself\n");
        return;
    }
    if(newPageInRoot && (preRenderRoot == NULL || !lv_obj_is_valid(preRenderRoot) || lv_obj_get_parent(preRenderRoot) != NULL))
    {
        bk_printf(TAG "[SCREEN] preRenderRoot is NULL, invalid, or not a screen\n");
        return;
    }

    lv_obj_t *targetScreen = newPageInRoot ? preRenderRoot : newPage;

    if(oldPage == newPage && lv_scr_act() == targetScreen)
    {
        bk_printf(TAG "[SCREEN] newPage is already active\n");
        return;
    }

    bool oldPageValid = oldPage != NULL && lv_obj_is_valid(oldPage);
    bool oldPageInRoot = oldPageValid && isObjInRoot(oldPage);

    /* 전환 시작 이벤트: 기존 page가 아직 유효하고 새 page는 아직 표시되기 전이다. */
    if(oldPageValid && oldPage != newPage)
    {
        lv_obj_send_event(oldPage, UI_EVENT_PAGE_HIDE_START, NULL);
        oldPageValid = lv_obj_is_valid(oldPage);
    }

    lv_obj_send_event(newPage, UI_EVENT_PAGE_SHOW_START, NULL);

    /* 합성 page만 직접 숨긴다. 독립 screen은 lv_scr_load()가 비활성화한다. */
    if(oldPageValid && oldPage != newPage && oldPageInRoot)
    {
        lv_obj_add_flag(oldPage, LV_OBJ_FLAG_HIDDEN);
    }

    if(newPageInRoot)
    {
        /* root 내부 page 전환: 새 page를 최상단에 놓고 root를 active screen으로 만든다. */
        lv_obj_move_to_index(newPage, -1);
        lv_obj_remove_flag(newPage, LV_OBJ_FLAG_HIDDEN);

        if(lv_scr_act() != preRenderRoot)
        {
            lv_scr_load(preRenderRoot);
        }
    }
    else
    {
        /* 독립 screen 전환: screen에는 z-order 조작을 하지 않는다. */
        lv_obj_remove_flag(newPage, LV_OBJ_FLAG_HIDDEN);
        if(lv_scr_act() != newPage)
        {
            lv_scr_load(newPage);
        }
    }

    /* refresh 중 실행되는 timer/callback도 새 page를 현재 page로 보게 한다. */
    currentPage = newPage;
    lv_refr_now(NULL);

    /* 전환 완료 이벤트는 실제 active screen 교체 및 즉시 refresh 뒤에 보낸다. */
    if(oldPageValid && oldPage != newPage && lv_obj_is_valid(oldPage))
    {
        lv_obj_send_event(oldPage, UI_EVENT_PAGE_HIDDEN, NULL);
    }

    if(lv_obj_is_valid(newPage))
    {
        lv_obj_send_event(newPage, UI_EVENT_PAGE_SHOWN, NULL);
        if(!lv_obj_is_valid(newPage))
        {
            currentPage = NULL;
        }
    }
    else
    {
        currentPage = NULL;
    }

    bk_printf(TAG "[SCREEN] ui_page_change() completed. [elapsed: %lu]\n", (unsigned long)lv_tick_elaps(startTick));
}