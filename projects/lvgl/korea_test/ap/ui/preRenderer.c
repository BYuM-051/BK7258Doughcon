#include "lvgl.h"
#include <stdio.h>
#include "preRenderer.h"
#include "preRenderInfo.h"
#include "ui_config.h"

#define TAG "[preRenderer.c] "
// #define bk_printf(fmt, ...) do {if(0) bk_printf(fmt, ##__VA_ARGS__); } while(0) // disable printf

extern bk_lv_ui_t bk_lv_tool_ui;
extern lv_obj_t *preRenderRoot;
lv_obj_t *currentPage = NULL;
lv_obj_t *currentScreen = NULL;
pageId_t currentPageID = PAGE_NONE; // 첫 init을 main으로하면 uiChange에서 return맞고 assert됨

//TODO : preRenderer Queue구현 여기말고

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

static void uiPagePrepare(pageId_t pageId);
static void uiPageShow(pageId_t pageId);
static void uiPageHide(pageId_t pageId);
static void uiPageDestroy(pageId_t pageId);
static void uiPagePreRender(pageId_t pageId);
static void uiPagePreRenderFlush(pageId_t pageId);
static void uiPagePreRenderRegister(pageId_t pageId);

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

// REFACTOR : 첫 init 분기가 진행되는데, 아예 main으로 전환하는건 이걸 안 타게 하는게 나을지도 모르겠다. 그러면 branch 줄어서 in-order에서 latency가 줄겠지
#define USE_OLD_PAGE_CHANGE_BEFORE_REFACTOR 0
#if USE_OLD_PAGE_CHANGE_BEFORE_REFACTOR
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

void ui_page_change(pageId_t newPageID)
{
    if(!uiScreenEventInitialized)
    {
        printf(TAG "[SCREEN] ui_page_change() called before ui_screen_event_init()\n");
        return;
    }

    uint32_t startTick = lv_tick_get();
    lv_obj_t *oldPage = NULL;
    pageId_t oldPageID = PAGE_NONE;
    if(currentPageID != PAGE_NONE)
    {
        oldPage = *(preRenderPageInfo[currentPageID].page);
        oldPageID = currentPageID;
    }
    lv_obj_t **newPage = preRenderPageInfo[newPageID].page;

    bk_printf(TAG "[SCREEN] ui_page_change(%p -> %p) called\n", oldPage, *newPage);

    if(newPage == NULL || !lv_obj_is_valid(*newPage))
    {
        bk_printf(TAG "[SCREEN] newPage is NULL or invalid\n");
        void *caller = __builtin_return_address(0);
        bk_printf(TAG "[CALLER] %p\n", caller); // Print returning addr
        pageLifecycleFunc_t initFunc = getPageInitFunc(newPageID);
        if(initFunc != NULL)
        {
            bk_printf(TAG "[SCREEN] newPage has an init function, calling it\n");
            initFunc(&bk_lv_tool_ui);
            newPage = preRenderPageInfo[newPageID].page;
        }
        else
        {
            bk_printf(TAG "[SCREEN] newPage has no init function\n");
            return;
        }
    }
    if(oldPage == *newPage)
    {
        bk_printf(TAG "[SCREEN] newPage is the same as currentPage\n");
        return;
    }
    currentPageID = newPageID;

    bool newPageInRoot = isObjInRoot(*newPage);
    bool newPageIsScreenItself = !newPageInRoot && lv_obj_get_parent(*newPage) == NULL;

    if(newPageInRoot && lv_obj_get_parent(*newPage) != preRenderRoot)
    {
        bk_printf(TAG "[SCREEN] newPage is inside a page, but is not a direct child of preRenderRoot\n");
        return;
    }
    if(!newPageInRoot && !newPageIsScreenItself)
    {
        bk_printf(TAG "[SCREEN] newPage is neither a root page nor a standalone screen\n");
        return;
    }
    if(*newPage == preRenderRoot)
    {
        bk_printf(TAG "[SCREEN] pass a child page instead of preRenderRoot itself\n");
        return;
    }
    if(newPageInRoot && (preRenderRoot == NULL || !lv_obj_is_valid(preRenderRoot) || lv_obj_get_parent(preRenderRoot) != NULL))
    {
        bk_printf(TAG "[SCREEN] preRenderRoot is NULL, invalid, or not a screen\n");
        return;
    }

    lv_obj_t *targetScreen = newPageInRoot ? preRenderRoot : *newPage;

    if(oldPage == *newPage && lv_scr_act() == targetScreen)
    {
        bk_printf(TAG "[SCREEN] newPage is already active\n");
        return;
    }

    bool oldPageValid = oldPage != NULL && lv_obj_is_valid(oldPage);
    bool oldPageInRoot = oldPageValid && isObjInRoot(oldPage);

    /* 전환 시작 이벤트: 기존 page가 아직 유효하고 새 page는 아직 표시되기 전이다. */
    if(oldPageValid && oldPage != *newPage)
    {
        lv_obj_send_event(oldPage, UI_EVENT_PAGE_HIDE_START, NULL);
        oldPageValid = lv_obj_is_valid(oldPage);
    }

    lv_obj_send_event(*newPage, UI_EVENT_PAGE_SHOW_START, NULL);

    /* 합성 page만 직접 숨긴다. 독립 screen은 lv_scr_load()가 비활성화한다. */
    if(oldPageValid && oldPage != *newPage && oldPageInRoot)
    {
        lv_obj_add_flag(oldPage, LV_OBJ_FLAG_HIDDEN);
    }

    if(newPageInRoot)
    {
        bk_printf(TAG "[SCREEN] -정상적인 루트를 밟았다 이 말임.-Switching to new page in preRenderRoot\n");
        /* root 내부 page 전환: 새 page를 최상단에 놓고 root를 active screen으로 만든다. */
        lv_obj_move_to_index(*newPage, -1);
        lv_obj_remove_flag(*newPage, LV_OBJ_FLAG_HIDDEN);

        lv_scr_load(preRenderRoot);
        lv_refr_now(NULL);
        bk_printf(TAG "[SCREEN] -정상적인 루트를 밟았다 이 말임.-Switching to new page in preRenderRoot completed [TICK : %d]\n", (unsigned long)lv_tick_get());
    }
    else
    {
        /* 독립 screen 전환: screen에는 z-order 조작을 하지 않는다. */
        lv_obj_remove_flag(*newPage, LV_OBJ_FLAG_HIDDEN);
        if(lv_scr_act() != *newPage)
        {
            lv_scr_load(*newPage);
        }
    }

    /* refresh 중 실행되는 timer/callback도 새 page를 현재 page로 보게 한다. */
    currentPage = *newPage;
    lv_refr_now(NULL);

    /* 전환 완료 이벤트는 실제 active screen 교체 및 즉시 refresh 뒤에 보낸다. */
    if(oldPageValid && oldPage != *newPage && lv_obj_is_valid(oldPage))
    {
        lv_obj_send_event(oldPage, UI_EVENT_PAGE_HIDDEN, NULL);
    }

    if(lv_obj_is_valid(*newPage))
    {
        lv_obj_send_event(*newPage, UI_EVENT_PAGE_SHOWN, NULL);
        if(!lv_obj_is_valid(*newPage))
        {
            bk_printf(TAG "[SCREEN] newPage became invalid after UI_EVENT_PAGE_SHOWN\n");
            currentPage = NULL;
            LV_ASSERT(0);
        }
        if(oldPageID != PAGE_NONE)
        {
            uiPagePreRenderFlush(oldPageID);
        }
        uiPagePreRenderRegister(newPageID);
    }
    else
    {
        currentPage = NULL;
    }

    bk_printf(TAG "[SCREEN] ui_page_change() completed. [elapsed: %lu]\n", (unsigned long)lv_tick_elaps(startTick));
}
#else
inline bool isPageIdValid(pageId_t pageId)
{
    return (pageId >= 0 && pageId < PAGE_COUNT);
}

void ui_page_change(pageId_t newPageID)
{
    bk_printf(TAG "[SCREEN] ui_page_change called Tick : %d\n", (unsigned long)lv_tick_get());
    // check if ui_screen_event_init() has been called before
    if(!uiScreenEventInitialized)
    {
        bk_printf(TAG "[SCREEN] ui_page_change() called before ui_screen_event_init()\n");
        return;
    }
    // check if newPageID is valid
    if(!isPageIdValid(newPageID))
    {
        bk_printf(TAG "[SCREEN] Invalid newPageID: %d\n", newPageID);
        goto fatal;
    }
    // check if oldPageID is valid [PAGE_NONE으로 처음 초기화하면 안됨.]
    if(!isPageIdValid(currentPageID))
    {
        bk_printf(TAG "[SCREEN] Invalid currentPageID: %d\n", currentPageID);
        goto fatal;
    }

    pageId_t oldPageID = currentPageID;
    lv_obj_t* oldPage = *(preRenderPageState[oldPageID].page);
    lv_obj_t* *newPage = preRenderPageState[newPageID].page; // lv_obj_t*를 가리키는 포인터

    // page change logic start =======================================================================
    bk_printf(TAG "[SCREEN] page change logic start Tick : %d\n", (unsigned long)lv_tick_get());

    // TODO : halt + empty preRendering queue + deinit page except newPageID and its preRenderTargets
    // 큐만 비우면 될 것 같기도 하고?

    // check if newPageID is already rendered and if not, call its init function
    if(preRenderPageState[newPageID].isRendered)
    {
        bk_printf(TAG "[SCREEN] newPageID %d is already rendered, skipping init\n", newPageID);
    }
    else
    {
        bk_printf(TAG "[SCREEN] newPageID %d is not rendered, calling init function Tick = %d\n", newPageID, (unsigned long)lv_tick_get());
        pageLifecycleFunc_t initFunc = getPageInitFunc(newPageID);
        if(initFunc != NULL)
        {
            bk_printf(TAG "[SCREEN] Initializing newPageID %d\n", newPageID);
            initFunc(&bk_lv_tool_ui);
            if(newPage == NULL || !lv_obj_is_valid(*newPage))
            {
                bk_printf(TAG "[SCREEN] newPage is invalid after init function\n");
                goto fatal;
            }
            preRenderPageState[newPageID].isRendered = true;
        }
        else
        {
            bk_printf(TAG "[SCREEN] No init function for newPageID %d\n", newPageID);
            goto fatal;
        }
        bk_printf(TAG "[SCREEN] newPageID %d initialized successfully Tick = %d\n", newPageID, (unsigned long)lv_tick_get());
    }
    // FIXME : popup 처리하는 부분인데, 로직 괜찮은지 모르겠음. 좀 더 봐야돼.
    if(currentScreen != preRenderRoot) 
    {
        bk_printf(TAG "[SCREEN] currentScreen is not preRenderRoot, switching to preRenderRoot\n");
        lv_obj_t *oldScreen = currentScreen;
        lv_scr_load(preRenderRoot);
        currentScreen = preRenderRoot;
        lv_refr_now(NULL);
        lv_obj_del(oldScreen);
    }
    // layer change and hide old page to minimize latency
    else 
    {
        bk_printf(TAG "[SCREEN] before hide start Tick : %d\n", (unsigned long)lv_tick_get());
        lv_obj_send_event(oldPage, UI_EVENT_PAGE_HIDE_START, NULL);
        if(oldPageID != PAGE_NONE && oldPageID != newPageID)
        {
            for(int i = 0 ; i < PAGE_COUNT ; i++)
            {
                lv_obj_t *page = *(preRenderPageState[i].page);
                if(page == NULL || !lv_obj_is_valid(page) || i == newPageID)
                {
                    // bk_printf(TAG "[SCREEN] Skipping hide for pageId: %d\n", i);
                    continue;
                }
                lv_obj_add_flag(page, LV_OBJ_FLAG_HIDDEN);
                bk_printf
                (
                    TAG "[PTR] main=%p auto=%p manual=%p autodry=%p root=%p\n",
                    bk_lv_tool_ui.main,
                    bk_lv_tool_ui.automode,
                    bk_lv_tool_ui.manualmode,
                    bk_lv_tool_ui.autodrymode,
                    preRenderRoot
                );
            }
        }
        else
        {
            bk_printf(TAG "[SCREEN] No old page to hide or same as new page\n");
            goto fatal;
        }
        bk_printf(TAG "[SCREEN] before show start Tick : %d\n", (unsigned long)lv_tick_get());
        lv_obj_send_event(*newPage, UI_EVENT_PAGE_SHOW_START, NULL);
        if(newPageID != PAGE_NONE)
        {
            bk_printf(TAG "[SCREEN] Showing newPageID %d\n", newPageID);
            lv_obj_remove_flag(*newPage, LV_OBJ_FLAG_HIDDEN);
            lv_obj_move_to_index(*newPage, -1);
            lv_refr_now(NULL);
            bk_printf(TAG "[SCREEN] after show Tick : %d\n", (unsigned long)lv_tick_get());
        }
        else
        {
            bk_printf(TAG "[SCREEN] newPageID is PAGE_NONE\n");
            goto fatal;
        }
        lv_obj_send_event(oldPage, UI_EVENT_PAGE_HIDDEN, NULL);
        lv_obj_send_event(*newPage, UI_EVENT_PAGE_SHOWN, NULL);
        bk_printf(TAG "[SCREEN] every event processed Tick : %d\n", (unsigned long)lv_tick_get());
    }
    // TODO : delete page except newPageID and its preRenderTargets and remove dangling pointer < if needed? <not now>

    // TODO : enqueing preRender targets for newPageID ======================================================
    bk_printf(TAG "[SCREEN] Start enqueuing preRender targets Tick : %d\n", (unsigned long)lv_tick_get());
    for(uint32_t i = 0 ; i < preRenderPageConfig[newPageID].preRenderTargetCount ; i++)
    {
        //TODO : enqueue preRender target pageId
        pageId_t targetPageID = preRenderPageConfig[newPageID].preRenderTargets[i];
    }
    // page change logic end =======================================================================

    // update currentPageID
    currentPageID = newPageID;
    currentPage = *newPage;
    bk_printf(TAG "[SCREEN] ui_page_change completed Tick : %d\n", (unsigned long)lv_tick_get());
    return;
    // NOTE : fatal error handling marker =================================================================
    fatal :
    lv_delay_ms(2000);
    LV_ASSERT(0);
}

void ui_screen_change(lv_obj_t *newScreen)
{
    return;
}

#endif /* USE_OLD_PAGE_CHANGE_BEFORE_REFACTOR */
#if USE_OLD_PAGE_CHANGE_BEFORE_REFACTOR
static void uiPagePreRenderFlush(pageId_t oldPageID)
{
    bk_printf(TAG "[SCREEN] Flushing pre-rendered pages\n");
    lv_image_cache_drop(NULL);

    for(uint32_t i = 0; i < preRenderPageInfo[oldPageID].preRenderTargetCount; i++)
    {
        if(preRenderPageInfo[oldPageID].preRenderTargets[i] == currentPageID)
        {
            bk_printf(TAG "[SCREEN] Skipping flush for current pageId: %d\n", currentPageID);
            continue;
        }
        pageId_t targetPageID = preRenderPageInfo[oldPageID].preRenderTargets[i];
        if(targetPageID >= PAGE_COUNT)
        {
            bk_printf(TAG "[SCREEN] Invalid preRender target pageId: %d\n", targetPageID);
            continue;
        }
        pageLifecycleFunc_t destroyFunc = preRenderPageInfo[targetPageID].deinit_func;
        if(destroyFunc != NULL)
        {
            bk_printf(TAG "[SCREEN] Destroying pre-rendered pageId: %d\n", targetPageID);
            destroyFunc(&bk_lv_tool_ui);
        }
    }
}

// TODO : preRender가 쌓이면 touch이벤트가 밀리는 문제를 해결해야하는데, 일단은 구현먼저 합시다.
static void uiPagePreRenderRegister(pageId_t newPageID)
{
    lv_obj_t **newPage = preRenderPageInfo[newPageID].page;
    bk_printf(TAG "[SCREEN] uiPagePreRenderis [%d]\n", newPageID != PAGE_NONE ? true : false);
    for(uint32_t i = 0; i < preRenderPageInfo[newPageID].preRenderTargetCount; i++)
    {
        pageId_t targetPageID = preRenderPageInfo[newPageID].preRenderTargets[i];
        lv_obj_t **targetPage = preRenderPageInfo[targetPageID].page;
        if(targetPageID >= PAGE_COUNT)
        {
            bk_printf(TAG "[SCREEN] Invalid preRender target pageId: %d\n", targetPageID);
            continue;
        }
        pageLifecycleFunc_t initFunc = getPageInitFunc(targetPageID);
        if(initFunc != NULL)
        {
            bk_printf(TAG "[SCREEN] Pre-rendering pageId: %d\n", targetPageID);
            initFunc(&bk_lv_tool_ui);
        }
    }
    lv_obj_move_to_index(*newPage, -1);
    lv_obj_remove_flag(*newPage, LV_OBJ_FLAG_HIDDEN);// REFACTOR : 일단은 위로 계속 쌓으면서 보이게 하는데, init 그 자체를 한번 더 체크합시다.
    lv_refr_now(NULL);
    for (size_t i = 0; i < preRenderPageInfo[newPageID].preRenderTargetCount; i++) // REFACTOR : 혹시 이거 먹으면 refresh에서 히든넣고 올리기로 다시 셋팅하자
    {
        pageId_t targetPageID = preRenderPageInfo[newPageID].preRenderTargets[i];
        lv_obj_t **targetPage = preRenderPageInfo[targetPageID].page;
        lv_obj_add_flag(*targetPage, LV_OBJ_FLAG_HIDDEN);
    }
    lv_refr_now(NULL);
    
    return;
}
#else



#endif /* USE_OLD_PAGE_CHANGE_BEFORE_REFACTOR */

lv_obj_t *ui_get_current_page(void)
{
    return currentPage;
}
