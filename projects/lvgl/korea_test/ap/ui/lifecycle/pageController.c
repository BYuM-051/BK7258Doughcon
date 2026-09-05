#include "lvgl.h"
#include <stdio.h>
#include "pageController.h"
#include "preRenderer.h"
#include "preRenderInfo.h"
#include "ui_config.h"
#include "queue.h"
#include "FreeRTOS.h"

#define TAG "[pageController.c] "
// #define bk_printf(fmt, ...) do {if(0) bk_printf(fmt, ##__VA_ARGS__); } while(0) // disable printf

lv_obj_t *preRenderRoot = NULL;

static bool pageControllerInitialized = false;

static lv_obj_t *currentPage = NULL;
static lv_obj_t *currentScreen = NULL;
static pageId_t currentPageID = PAGE_NONE;
static void setCurrentPageID(pageId_t pageID)
{
    currentPageID = pageID;
}
static void setCurrentPage(lv_obj_t* page)
{
    currentPage = page;
}
static void setCurrentScreen(lv_obj_t* screen)
{
    currentScreen = screen;
}

void pageControllerInit(void)
{
    if(pageControllerInitialized)
        {return;}

    preRenderRoot = lv_obj_create(NULL);
    lv_obj_set_scrollbar_mode(preRenderRoot, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_border_width(preRenderRoot, 0, 0);
    lv_obj_set_style_radius(preRenderRoot, 0, 0);
    lv_obj_set_style_pad_all(preRenderRoot, 0, 0);
    lv_obj_set_style_bg_opa(preRenderRoot, LV_OPA_TRANSP, 0);
    currentPage = NULL;
    currentPageID = PAGE_NONE;
    currentScreen = preRenderRoot;

#if UI_PRENDERING_ENABLE
    UI_EVENT_PAGE_SHOW_START = lv_event_register_id();
    UI_EVENT_PAGE_SHOWN = lv_event_register_id();
    UI_EVENT_PAGE_HIDE_START = lv_event_register_id();
    UI_EVENT_PAGE_HIDDEN = lv_event_register_id();
#endif
    pageControllerInitialized = true;
}

void ui_page_init(pageId_t pageID)
{
    ui_page_init_ShowOption(pageID, true);
}
void ui_page_init_ScreenOption(pageId_t pageID, bool screenOption)
{
    // change screen here first
    ui_page_init(pageID);
}
void ui_page_init_ShowOption(pageId_t pageID, bool showOption)
{
    if(!pageControllerInitialized)
    {
        bk_printf(TAG "UI screen event not initialized!\n");
        goto fatal;
    }
    if(!isPageIdValid(pageID))
    {
        bk_printf(TAG "Invalid page ID: %d\n", pageID);
        goto fatal;
    }

    lv_obj_t* *newPage = preRenderPageState[pageID].page;

    uiResetPreprocessQueue();

    if(!isPagePreloaded(pageID))
    {
        uiPreloadPageForce(pageID);
    }

    for(int i = 0 ; i < preRenderPageConfig[pageID].preRenderTargetPageCount ; i++)
    {
        uiPreloadPageForce(preRenderPageConfig[pageID].preRenderTargetPages[i]);
    }

    lv_obj_send_event(*newPage, UI_EVENT_PAGE_SHOW_START, NULL);
    setCurrentPageID(pageID);
    lv_obj_move_to_index(*newPage, -1);    
    if(showOption)
    {
        lv_obj_remove_flag(*newPage, LV_OBJ_FLAG_HIDDEN);
        lv_refr_now(NULL);
    }
    setCurrentPage(*newPage);
    setCurrentScreen(preRenderRoot);
    lv_obj_send_event(*newPage, UI_EVENT_PAGE_SHOWN, NULL);
    return;

fatal :
    lv_delay_ms(2000);
    LV_ASSERT(0);
    return;
}

void ui_page_change(pageId_t pageID)
{
    ui_page_change_ShowOption(pageID, true);
    return;
}
void ui_page_change_ShowOption(pageId_t newPageID, bool showOption)
{
    bk_printf(TAG "[SCREEN] ui_page_change called Tick : %d\n", (unsigned long)lv_tick_get());

    if(!pageControllerInitialized)
    {
        bk_printf(TAG "[SCREEN] ui_page_change() called before ui_screen_event_init()\n");
        return;
    }
    if(!isPageIdValid(newPageID))
    {
        bk_printf(TAG "[SCREEN] Invalid newPageID: %d\n", newPageID);
        goto fatal;
    }
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

    // 걍 다 비워.
    uiResetPreprocessQueue();

    // check if newPageID is already rendered and if not, call its init function
    if(preRenderPageState[newPageID].isRendered)
    {
        bk_printf(TAG "[SCREEN] newPageID %d is already rendered, skipping init\n", newPageID);
    }
    else
    {
        bk_printf(TAG "[SCREEN] newPageID %d is not rendered, calling init function Tick = %d\n", newPageID, (unsigned long)lv_tick_get());
        pageLifecycleFuncWithStep_t initFunc = getPageInitFunc(newPageID);
        if(initFunc != NULL)
        {
            bk_printf(TAG "[SCREEN] Initializing newPageID %d\n", newPageID);
            while(true)
            {
                rendererFuncStatus_t result = initFunc(&bk_lv_tool_ui);
                if(result == RENDERER_FUNC_DONE)
                {
                    break;
                }
                else if(result == RENDERER_FUNC_FAILED)
                {
                    bk_printf(TAG "[BOOT] initFunc failed\n");
                    lv_delay_ms(2000);
                    LV_ASSERT(0);
                }
            }
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
#if MINIMIZE_HIDDEN_ALGORITHM
            // for(int i = 0 ; i < PAGE_COUNT ; i++)
            // {
            //     lv_obj_t *page = *(preRenderPageState[i].page);
            //     if(page == NULL || !lv_obj_is_valid(page) || i == newPageID)
            //     {
            //         bk_printf(TAG "[SCREEN] Skipping hide for pageId: %d\n", i);
            //         continue;
            //     }
            //     lv_obj_add_flag(page, LV_OBJ_FLAG_HIDDEN);
            //     bk_printf
            //     (
            //         TAG "[PTR] main=%p auto=%p manual=%p autodry=%p root=%p\n",
            //         bk_lv_tool_ui.main,
            //         bk_lv_tool_ui.automode,
            //         bk_lv_tool_ui.manualmode,
            //         bk_lv_tool_ui.autodrymode,
            //         preRenderRoot
            //     );
            // }
#else
            lv_obj_add_flag(oldPage, LV_OBJ_FLAG_HIDDEN);
#endif
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
            lv_image_cache_dump();
            lv_obj_remove_flag(*newPage, LV_OBJ_FLAG_HIDDEN);
            lv_obj_move_to_index(*newPage, -1);
            lv_refr_now(NULL);
            bk_printf(TAG "[SCREEN] after show Tick : %d\n", (unsigned long)lv_tick_get());
            lv_image_cache_dump();
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
    // pageState 순회하면서 newPageID와 그 preRenderTargets를 제외한 나머지 페이지들 deinit
    for(int i = PAGE_MAIN ; i < PAGE_COUNT ; i++)
    {
        bool isPreRenderTarget = false;
        if(i == newPageID)
        {
            continue;
        }
        for(int j = 0 ; j < preRenderPageConfig[newPageID].preRenderTargetPageCount ; j++)
        {

            if(preRenderPageConfig[newPageID].preRenderTargetPages[j] == i)
            {
                isPreRenderTarget = true;
            }
        }
        if(isPreRenderTarget)
        {
            continue;
        }

        if(preRenderPageState[i].isRendered)
        {
            pageLifecycleFunc_t deinitFunc = preRenderPageConfig[i].deinit_func;
            if(deinitFunc != NULL)
            {
                bk_printf(TAG "[SCREEN] Deinitializing pageId: %d\n", i);
                deinitFunc(&bk_lv_tool_ui);
            }
        }
    }

    // enqueing preRender targets for newPageID ======================================================
    uiEnqueuePreloadTargets(newPageID);
    // page change logic end =======================================================================

    // update currentPageID
    currentPageID = newPageID;
    currentPage = *newPage;
    bk_printf(TAG "[SCREEN] ui_page_change completed Tick : %d\n", (unsigned long)lv_tick_get());
    return;
fatal :
    lv_delay_ms(2000);
    LV_ASSERT(0);
}

void ui_screen_change(lv_obj_t *newScreen)
{
    return;
}
void ui_screen_change_ShowOption(lv_obj_t *newScreen, bool showOption)
{

    return;
}

lv_obj_t* ui_get_current_page(void)
{
    return currentPage;
}
pageId_t ui_get_current_page_id(void)
{
    return currentPageID;
}