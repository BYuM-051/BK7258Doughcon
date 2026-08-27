#include "lvgl.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "preRenderer.h"
#include "preRenderInfo.h"
#include "ui_config.h"

#define TAG "[preRenderer.c] "
#define bk_printf(fmt, ...) do {if(0) bk_printf(fmt, ##__VA_ARGS__); } while(0) // disable printf

extern bk_lv_ui_t bk_lv_tool_ui;
extern lv_obj_t *preRenderRoot;
lv_obj_t *currentPage = NULL;
pageId_t currentPageID = PAGE_NONE; // 첫 init을 main으로하면 uiChange에서 return맞고 assert됨

#define PAGE_BUILD_INITIAL_CAPACITY  8U

typedef enum
{
    PAGE_BUILD_OP_IMAGE,
    PAGE_BUILD_OP_TASK
} pageBuildOpType_t;

typedef struct
{
    pageBuildOpType_t type;
    lv_obj_t *obj;
    char *src;
    const void *expectedValue;
    char *expectedText;
    lv_image_src_t expectedType;
    void (*task)(void);
    const char *taskName;
} pageBuildOp_t;

typedef struct
{
    pageBuildOp_t *ops;
    uint32_t count;
    uint32_t capacity;
    uint32_t next;
} pageBuildPageState_t;

typedef struct
{
    lv_timer_t *timer;
    pageId_t pageId;
    uint32_t step;
    uint32_t generation;
    bool running;
} pageBuildJob_t;

static pageBuildPageState_t pageBuildStates[PAGE_COUNT];
static lv_obj_t *pageBuildObservedPages[PAGE_COUNT];
static pageBuildJob_t pageBuildJob = {.pageId = PAGE_NONE};
static bool pageBuildCaptureActive = false;
static pageId_t pageBuildCapturePage = PAGE_NONE;
static pageId_t pageBuildTouchPausedPage = PAGE_NONE;

static void pageBuildTimerCb(lv_timer_t *timer);
static void pageBuildTouchEventCb(lv_event_t *e);
static void pageBuildPageDeleteEventCb(lv_event_t *e);
static void pageBuildStart(pageId_t pageId);
static void pageBuildDiscard(pageId_t pageId);
static void pageBuildObservePage(pageId_t pageId, lv_obj_t *page);
static void pageBuildRemovePendingForObject(lv_obj_t *obj);
static bool pageBuildSourceStillExpected(const pageBuildOp_t *op);
static pageBuildOp_t *pageBuildAppendOp(pageBuildPageState_t *state);
static bool pageBuildHasPending(pageId_t pageId);
static void pageBuildCaptureBegin(pageId_t pageId);
static void pageBuildCaptureEnd(void);

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

static inline bool isObjInRoot(lv_obj_t *obj);
static void uiPagePrepare(pageId_t pageId);
static void uiPageShow(pageId_t pageId);
static void uiPageHide(pageId_t pageId);
static void uiPageDestroy(pageId_t pageId);
static void uiPagePreRender(pageId_t pageId);
static void uiPagePreRenderFlush(pageId_t pageId);
static void uiPagePreRenderRegister(pageId_t pageId);

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

    /* Create the one reusable worker before page-specific prewarm timers. */
    pageBuildJob.timer = lv_timer_create(pageBuildTimerCb, 1, NULL);
    if(pageBuildJob.timer != NULL)
    {
        lv_timer_pause(pageBuildJob.timer);
    }

    /* Input-device events are delivered before the same event is sent to the
     * hit object. Cancel only the renderer; never stop event processing. */
    lv_indev_t *indev = NULL;
    while((indev = lv_indev_get_next(indev)) != NULL)
    {
        if(lv_indev_get_type(indev) != LV_INDEV_TYPE_POINTER)
        {
            continue;
        }

        lv_indev_add_event_cb(indev, pageBuildTouchEventCb, LV_EVENT_ALL, NULL);
        lv_timer_t *readTimer = lv_indev_get_read_timer(indev);
        if(readTimer != NULL)
        {
            lv_timer_set_period(readTimer, UI_PAGE_BUILD_TOUCH_PERIOD_MS);
        }
    }
#endif
}

bool ui_screen_event_initialized(void)
{
    return uiScreenEventInitialized;
}

static bool pageBuildHasPending(pageId_t pageId)
{
    if(pageId < 0 || pageId >= PAGE_COUNT)
    {
        return false;
    }

    const pageBuildPageState_t *state = &pageBuildStates[pageId];
    return state->next < state->count;
}

static void pageBuildDiscard(pageId_t pageId)
{
    if(pageId < 0 || pageId >= PAGE_COUNT)
    {
        return;
    }

    pageBuildPageState_t *state = &pageBuildStates[pageId];
    for(uint32_t i = state->next; i < state->count; i++)
    {
        lv_free(state->ops[i].src);
        lv_free(state->ops[i].expectedText);
    }
    lv_free(state->ops);
    *state = (pageBuildPageState_t){0};
}

static void pageBuildObservePage(pageId_t pageId, lv_obj_t *page)
{
    if(pageId < 0 || pageId >= PAGE_COUNT ||
       page == NULL || !lv_obj_is_valid(page) ||
       pageBuildObservedPages[pageId] == page)
    {
        return;
    }

    pageBuildObservedPages[pageId] = page;
    lv_obj_add_event_cb(page, pageBuildPageDeleteEventCb, LV_EVENT_DELETE,
                        (void *)(uintptr_t)pageId);
}

static void pageBuildRemovePendingForObject(lv_obj_t *obj)
{
    for(uint32_t page = 0; page < PAGE_COUNT; page++)
    {
        pageBuildPageState_t *state = &pageBuildStates[page];
        uint32_t write = state->next;
        for(uint32_t read = state->next; read < state->count; read++)
        {
            if(state->ops[read].obj == obj)
            {
                lv_free(state->ops[read].src);
                lv_free(state->ops[read].expectedText);
                continue;
            }

            if(write != read)
            {
                state->ops[write] = state->ops[read];
            }
            write++;
        }
        state->count = write;
    }
}

static bool pageBuildSourceStillExpected(const pageBuildOp_t *op)
{
    const void *current = lv_image_get_src(op->obj);
    lv_image_src_t currentType = lv_image_src_get_type(current);
    if(currentType != op->expectedType)
    {
        return false;
    }

    if(currentType == LV_IMAGE_SRC_FILE || currentType == LV_IMAGE_SRC_SYMBOL)
    {
        return current != NULL && op->expectedText != NULL &&
               strcmp((const char *)current, op->expectedText) == 0;
    }
    return current == op->expectedValue;
}

static pageBuildOp_t *pageBuildAppendOp(pageBuildPageState_t *state)
{
    if(state->count == state->capacity)
    {
        uint32_t newCapacity = state->capacity == 0
                             ? PAGE_BUILD_INITIAL_CAPACITY
                             : state->capacity * 2U;
        pageBuildOp_t *newOps = lv_realloc(
            state->ops, sizeof(pageBuildOp_t) * newCapacity);
        if(newOps == NULL)
        {
            return NULL;
        }
        state->ops = newOps;
        state->capacity = newCapacity;
    }

    pageBuildOp_t *op = &state->ops[state->count++];
    *op = (pageBuildOp_t){0};
    return op;
}

static void pageBuildCaptureBegin(pageId_t pageId)
{
    pageBuildCapturePage = pageId;
    pageBuildCaptureActive = true;
}

static void pageBuildCaptureEnd(void)
{
    pageBuildCaptureActive = false;
    pageBuildCapturePage = PAGE_NONE;
}

void ui_page_build_set_image_src(lv_obj_t *obj, const void *src)
{
    if(obj == NULL || src == NULL || !lv_obj_is_valid(obj))
    {
        return;
    }

    if(!pageBuildCaptureActive ||
       pageBuildCapturePage < 0 || pageBuildCapturePage >= PAGE_COUNT ||
       lv_image_src_get_type(src) != LV_IMAGE_SRC_FILE)
    {
        /* An event can update an image while an older captured path is paused.
         * Remove that stale step before applying the newer source. Variable
         * image descriptors are always applied synchronously. */
        pageBuildRemovePendingForObject(obj);
        lv_image_set_src(obj, src);
        return;
    }

    pageBuildPageState_t *state = &pageBuildStates[pageBuildCapturePage];
    const char *path = (const char *)src;
    char *srcCopy = lv_strdup(path);
    if(srcCopy == NULL)
    {
        bk_printf(TAG "[BUILD] path allocation failed; loading synchronously: %s\n", path);
        pageBuildRemovePendingForObject(obj);
        lv_image_set_src(obj, src);
        return;
    }

    /* initBase and the language callback often target the same object. Keep
     * only the last path so the default asset is not decoded immediately before
     * its localized replacement. */
    for(uint32_t i = state->next; i < state->count; i++)
    {
        if(state->ops[i].type == PAGE_BUILD_OP_IMAGE &&
           state->ops[i].obj == obj)
        {
            lv_free(state->ops[i].src);
            state->ops[i].src = srcCopy;
            return;
        }
    }

    const void *expectedValue = lv_image_get_src(obj);
    lv_image_src_t expectedType = lv_image_src_get_type(expectedValue);
    char *expectedText = NULL;
    if(expectedType == LV_IMAGE_SRC_FILE || expectedType == LV_IMAGE_SRC_SYMBOL)
    {
        expectedText = lv_strdup((const char *)expectedValue);
        if(expectedText == NULL)
        {
            lv_free(srcCopy);
            bk_printf(TAG "[BUILD] source guard allocation failed; loading synchronously: %s\n", path);
            pageBuildRemovePendingForObject(obj);
            lv_image_set_src(obj, src);
            return;
        }
    }

    pageBuildOp_t *op = pageBuildAppendOp(state);
    if(op == NULL)
    {
        lv_free(srcCopy);
        lv_free(expectedText);
        bk_printf(TAG "[BUILD] step allocation failed; loading synchronously: %s\n", path);
        pageBuildRemovePendingForObject(obj);
        lv_image_set_src(obj, src);
        return;
    }

    op->type = PAGE_BUILD_OP_IMAGE;
    op->obj = obj;
    op->src = srcCopy;
    op->expectedValue = expectedValue;
    op->expectedText = expectedText;
    op->expectedType = expectedType;
}

void ui_page_build_enqueue_task(void (*task)(void), const char *name)
{
    if(task == NULL)
    {
        return;
    }

    if(!pageBuildCaptureActive ||
       pageBuildCapturePage < 0 || pageBuildCapturePage >= PAGE_COUNT)
    {
        task();
        return;
    }

    pageBuildOp_t *op =
        pageBuildAppendOp(&pageBuildStates[pageBuildCapturePage]);
    if(op == NULL)
    {
        bk_printf(TAG "[BUILD] task allocation failed; running synchronously: %s\n",
                  name != NULL ? name : "<unnamed>");
        task();
        return;
    }

    op->type = PAGE_BUILD_OP_TASK;
    op->task = task;
    op->taskName = name;
}

bool ui_page_build_step(uint32_t step)
{
    pageId_t pageId = pageBuildJob.pageId;
    if(pageId < 0 || pageId >= PAGE_COUNT)
    {
        return true;
    }

    pageBuildPageState_t *state = &pageBuildStates[pageId];
    if(step < state->next)
    {
        step = state->next;
    }
    if(step >= state->count)
    {
        pageBuildDiscard(pageId);
        return true;
    }

    pageBuildOp_t *op = &state->ops[step];
    if(op->type == PAGE_BUILD_OP_TASK)
    {
        bk_printf(TAG "[BUILD] task=%s\n",
                  op->taskName != NULL ? op->taskName : "<unnamed>");
        if(op->task != NULL)
        {
            op->task();
        }
    }
    else
    {
        /* A button/state callback may have changed this image without going
         * through the page-build API. Only apply the captured path if the
         * source is still the one observed when the step was queued. */
        if(op->obj != NULL && lv_obj_is_valid(op->obj) &&
           pageBuildSourceStillExpected(op))
        {
            lv_image_set_src(op->obj, op->src);
        }
    }

    lv_free(op->src);
    lv_free(op->expectedText);
    op->src = NULL;
    op->expectedText = NULL;
    op->obj = NULL;
    state->next = step + 1U;

    if(state->next >= state->count)
    {
        pageBuildDiscard(pageId);
        return true;
    }
    return false;
}

void ui_page_build_cancel(void)
{
    pageBuildJob.running = false;
    pageBuildJob.generation++;
    pageBuildJob.pageId = PAGE_NONE;
    pageBuildJob.step = 0;
    if(pageBuildJob.timer != NULL)
    {
        lv_timer_pause(pageBuildJob.timer);
    }
}

static void pageBuildStart(pageId_t pageId)
{
    if(pageId < 0 || pageId >= PAGE_COUNT || !pageBuildHasPending(pageId))
    {
        return;
    }

    pageInitStepFunc_t initStep = getPageInitStepFunc(pageId);
    if(initStep == NULL)
    {
        pageBuildDiscard(pageId);
        return;
    }

    if(pageBuildJob.timer == NULL)
    {
        pageBuildJob.timer = lv_timer_create(pageBuildTimerCb, 1, NULL);
        if(pageBuildJob.timer == NULL)
        {
            bk_printf(TAG "[BUILD] worker timer allocation failed\n");
            return;
        }
    }

    pageBuildJob.generation++;
    pageBuildJob.pageId = pageId;
    pageBuildJob.step = pageBuildStates[pageId].next;
    pageBuildJob.running = true;
    lv_timer_set_user_data(pageBuildJob.timer,
                           (void *)(uintptr_t)pageBuildJob.generation);
    lv_timer_set_period(pageBuildJob.timer, 1);
    lv_timer_resume(pageBuildJob.timer);
    lv_timer_ready(pageBuildJob.timer);
    bk_printf(TAG "[BUILD] start page=%d step=%lu remaining=%lu generation=%lu\n",
              pageId,
              (unsigned long)pageBuildJob.step,
              (unsigned long)(pageBuildStates[pageId].count - pageBuildJob.step),
              (unsigned long)pageBuildJob.generation);
}

static void pageBuildTimerCb(lv_timer_t *timer)
{
    uint32_t callbackGeneration =
        (uint32_t)(uintptr_t)lv_timer_get_user_data(timer);
    if(!pageBuildJob.running ||
       callbackGeneration != pageBuildJob.generation ||
       pageBuildJob.pageId != currentPageID)
    {
        lv_timer_pause(timer);
        return;
    }

    pageInitStepFunc_t initStep = getPageInitStepFunc(pageBuildJob.pageId);
    if(initStep == NULL)
    {
        ui_page_build_cancel();
        return;
    }

    pageId_t pageId = pageBuildJob.pageId;
    uint32_t step = pageBuildJob.step;
    uint32_t initStart = lv_tick_get();
    bool completed = initStep(step);
    uint32_t initElapsed = lv_tick_elaps(initStart);

    if(callbackGeneration != pageBuildJob.generation ||
       pageId != currentPageID)
    {
        lv_timer_pause(timer);
        return;
    }

    uint32_t refreshStart = lv_tick_get();
    lv_refr_now(NULL);
    uint32_t refreshElapsed = lv_tick_elaps(refreshStart);
    bk_printf(TAG "[BUILD] page=%d step=%lu init=%lu refr=%lu total=%lu\n",
              pageId,
              (unsigned long)step,
              (unsigned long)initElapsed,
              (unsigned long)refreshElapsed,
              (unsigned long)(initElapsed + refreshElapsed));

    if(completed)
    {
        pageBuildJob.running = false;
        pageBuildJob.pageId = PAGE_NONE;
        pageBuildJob.step = 0;
        lv_timer_pause(timer);
        bk_printf(TAG "[BUILD] completed page=%d generation=%lu\n",
                  pageId, (unsigned long)callbackGeneration);
        return;
    }

    pageBuildJob.step++;
    /* Return now. lv_timer_handler() can service the input/display timers. */
}

static void pageBuildTouchEventCb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_PRESSED)
    {
        if(pageBuildJob.running)
        {
            pageBuildTouchPausedPage = pageBuildJob.pageId;
            ui_page_build_cancel();
        }
        return;
    }

    if(code == LV_EVENT_RELEASED || code == LV_EVENT_CANCEL)
    {
        pageId_t pausedPage = pageBuildTouchPausedPage;
        pageBuildTouchPausedPage = PAGE_NONE;
        if(pausedPage == currentPageID && pageBuildHasPending(pausedPage))
        {
            pageBuildStart(pausedPage);
        }
    }
}

static void pageBuildPageDeleteEventCb(lv_event_t *e)
{
    pageId_t pageId = (pageId_t)(uintptr_t)lv_event_get_user_data(e);
    if(pageBuildJob.pageId == pageId)
    {
        ui_page_build_cancel();
    }
    if(pageBuildTouchPausedPage == pageId)
    {
        pageBuildTouchPausedPage = PAGE_NONE;
    }
    if(pageId >= 0 && pageId < PAGE_COUNT &&
       pageBuildObservedPages[pageId] == lv_event_get_target(e))
    {
        pageBuildObservedPages[pageId] = NULL;
    }
    pageBuildDiscard(pageId);
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
#define USE_OLD_PAGE_CHANGE_BEFORE_REFACTOR 1
#if USE_OLD_PAGE_CHANGE_BEFORE_REFACTOR
void ui_page_change(pageId_t newPageID)
{
    if(!uiScreenEventInitialized)
    {
        printf(TAG "[SCREEN] ui_page_change() called before ui_screen_event_init()\n");
        return;
    }

    if(newPageID < 0 || newPageID >= PAGE_COUNT)
    {
        bk_printf(TAG "[SCREEN] invalid page id: %d\n", newPageID);
        return;
    }

    const preRendererPageInfo_t *newPageInfo = &preRenderPageInfo[newPageID];
    if(newPageInfo->page == NULL)
    {
        bk_printf(TAG "[SCREEN] page descriptor has no page pointer: %d\n", newPageID);
        return;
    }

    uint32_t startTick = lv_tick_get();
    lv_obj_t *oldPage = NULL;
    if(currentPageID >= 0 && currentPageID < PAGE_COUNT &&
       preRenderPageInfo[currentPageID].page != NULL)
    {
        oldPage = *(preRenderPageInfo[currentPageID].page);
        oldPageID = currentPageID;
    }
    lv_obj_t **newPage = newPageInfo->page;

    bk_printf(TAG "[SCREEN] ui_page_change(%p -> %p) called\n", oldPage, *newPage);

    /* Pages created during boot bypass initBase here. Observe every page once
     * so its deferred queue is still released if another owner deletes it. */
    pageBuildObservePage(newPageID, *newPage);

    if(oldPage != NULL && oldPage == *newPage && lv_obj_is_valid(oldPage))
    {
        bk_printf(TAG "[SCREEN] newPage is the same as currentPage\n");
        return;
    }

    /* A transition supersedes the old worker. Pending steps remain attached to
     * their page state and can resume if that page is visited again. */
    ui_page_build_cancel();
    pageBuildTouchPausedPage = PAGE_NONE;

    if(*newPage == NULL || !lv_obj_is_valid(*newPage))
    {
        bk_printf(TAG "[SCREEN] newPage is NULL or invalid\n");
        void *caller = __builtin_return_address(0);
        bk_printf(TAG "[CALLER] %p\n", caller); // Print returning addr
        pageBuildDiscard(newPageID);
        if(newPageInfo->initBase != NULL)
        {
            bk_printf(TAG "[SCREEN] calling initBase for page=%d\n", newPageID);
            pageBuildCaptureBegin(newPageID);
            newPageInfo->initBase(&bk_lv_tool_ui);
            pageBuildCaptureEnd();

            if(*newPage != NULL && lv_obj_is_valid(*newPage))
            {
                pageBuildObservePage(newPageID, *newPage);
            }
        }
        else
        {
            bk_printf(TAG "[SCREEN] newPage has no initBase function\n");
            return;
        }
    }

    if(*newPage == NULL || !lv_obj_is_valid(*newPage))
    {
        bk_printf(TAG "[SCREEN] initBase did not create a valid page: %d\n", newPageID);
        pageBuildDiscard(newPageID);
        return;
    }

    /* The real background image is a deferred step. Guarantee an opaque first
     * frame so the newly interactive page never exposes stale pixels beneath a
     * transparent root while its images are still loading. */
    if(lv_obj_get_style_bg_opa(*newPage, LV_PART_MAIN) < LV_OPA_COVER)
    {
        lv_obj_set_style_bg_color(*newPage, lv_color_hex(UI_BG_COLOR_HEX),
                                  LV_PART_MAIN);
        lv_obj_set_style_bg_opa(*newPage, LV_OPA_COVER, LV_PART_MAIN);
    }

    bool newPageInRoot = isObjInRoot(*newPage);
    bool newPageIsScreenItself = !newPageInRoot && lv_obj_get_parent(*newPage) == NULL;

    if(newPageInRoot && lv_obj_get_parent(*newPage) != preRenderRoot)
    {
        bk_printf(TAG "[SCREEN] newPage is inside a page, but is not a direct child of preRenderRoot\n");
        pageBuildDiscard(newPageID);
        return;
    }
    if(!newPageInRoot && !newPageIsScreenItself)
    {
        bk_printf(TAG "[SCREEN] newPage is neither a root page nor a standalone screen\n");
        pageBuildDiscard(newPageID);
        return;
    }
    if(*newPage == preRenderRoot)
    {
        bk_printf(TAG "[SCREEN] pass a child page instead of preRenderRoot itself\n");
        pageBuildDiscard(newPageID);
        return;
    }
    if(newPageInRoot && (preRenderRoot == NULL || !lv_obj_is_valid(preRenderRoot) || lv_obj_get_parent(preRenderRoot) != NULL))
    {
        bk_printf(TAG "[SCREEN] preRenderRoot is NULL, invalid, or not a screen\n");
        pageBuildDiscard(newPageID);
        return;
    }

    bool oldPageValid = oldPage != NULL && lv_obj_is_valid(oldPage);
    bool oldPageInRoot = oldPageValid && isObjInRoot(oldPage);

    /* Load-start callbacks can now enqueue language/state-specific images for
     * the new page without decoding them inside the transition. */
    currentPageID = newPageID;

    /* 전환 시작 이벤트: 기존 page가 아직 유효하고 새 page는 아직 표시되기 전이다. */
    if(oldPageValid && oldPage != *newPage)
    {
        lv_obj_send_event(oldPage, UI_EVENT_PAGE_HIDE_START, NULL);
        oldPageValid = lv_obj_is_valid(oldPage);
    }

    pageBuildCaptureBegin(newPageID);
    lv_obj_send_event(*newPage, UI_EVENT_PAGE_SHOW_START, NULL);
    pageBuildCaptureEnd();

    if(!lv_obj_is_valid(*newPage))
    {
        bk_printf(TAG "[SCREEN] newPage became invalid during UI_EVENT_PAGE_SHOW_START\n");
        currentPageID = PAGE_NONE;
        currentPage = NULL;
        pageBuildDiscard(newPageID);
        return;
    }

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
        /* Loaded callbacks may request deferred static images. Capture those
         * requests as more steps before the worker is started. */
        pageBuildCaptureBegin(newPageID);
        lv_obj_send_event(*newPage, UI_EVENT_PAGE_SHOWN, NULL);
        pageBuildCaptureEnd();
        if(!lv_obj_is_valid(*newPage))
        {
            bk_printf(TAG "[SCREEN] newPage became invalid after UI_EVENT_PAGE_SHOWN\n");
            currentPage = NULL;
            currentPageID = PAGE_NONE;
            pageBuildDiscard(newPageID);
            return;
        }
        if(oldPageID != PAGE_NONE)
        {
            uiPagePreRenderFlush(oldPageID);
        }
        uiPagePreRenderRegister(newPageID);
        pageBuildStart(newPageID);
    }
    else
    {
        currentPage = NULL;
        currentPageID = PAGE_NONE;
        pageBuildDiscard(newPageID);
    }

    bk_printf(TAG "[SCREEN] ui_page_change() completed. [elapsed: %lu]\n", (unsigned long)lv_tick_elaps(startTick));
}
#else
void ui_page_change(pageId_t newPageID)
{

}

static void uiPagePrepare(pageId_t pageId)
{

}

static void uiPageShow(pageId_t pageId)
{

}

static void uiPageHide(pageId_t pageId)
{

}

static void uiPageDestroy(pageId_t pageId)
{

}

static void uiPagePreRender(pageId_t pageId)
{

}
#endif /* USE_OLD_PAGE_CHANGE_BEFORE_REFACTOR */
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

lv_obj_t *ui_get_current_page(void)
{
    return currentPage;
}
