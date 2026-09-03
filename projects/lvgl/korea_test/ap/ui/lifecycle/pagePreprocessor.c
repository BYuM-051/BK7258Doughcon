#include "lvgl.h"
#include <stdio.h>
#include <string.h>
#include "preRenderer.h"
#include "queue.h"
#include "FreeRTOS.h"
#include "settings.h"

#define TAG "[pagePreprocessor.c] "
// #define bk_printf(fmt, ...) do {if(0) bk_printf(fmt, ##__VA_ARGS__); } while(0) // disable printf

extern bk_lv_ui_t bk_lv_tool_ui;
extern lv_obj_t *preRenderRoot;

#include "lv_conf.h"

static void uiPagePreloadTask(lv_timer_t *timer);

static QueueHandle_t preRendererQueue = NULL;
static lv_timer_t* preRendererTimer = NULL;

static bool uiScreenEventInitialized = false;

void uiPreprocessorInit(void)
{
    if(uiPreprocessorInitialized()) {return;}

    if(init_shared_image_asset() != RENDERER_FUNC_DONE)
    {
        bk_printf(TAG "[SCREEN] init_shared_image_asset() failed\n");
        lv_delay_ms(2000);
        LV_ASSERT(0);
    }

    preRendererQueue = xQueueCreate(16, sizeof(pageId_t));
    if(preRendererQueue == NULL)
    {
        bk_printf(TAG "[SCREEN] Failed to create preRendererQueue\n");
        lv_delay_ms(2000);
        LV_ASSERT(0);
    }

    preRendererTimer = lv_timer_create(uiPagePreloadTask, _PREPROCESSOR_YIELD_DELAY, NULL);
    if(preRendererTimer == NULL)
    {
        bk_printf(TAG "[SCREEN] Failed to create preRendererTimer\n");
        lv_delay_ms(2000);
        LV_ASSERT(0);
    }
    uiScreenEventInitialized = true;
}

bool uiPreprocessorInitialized(void)
{
    return uiScreenEventInitialized;
}

void uiEnqueuePreloadTargets(pageId_t newPageID)
{
    for(uint32_t i = 0 ; i < preRenderPageConfig[newPageID].preRenderTargetPageCount ; i++)
    {
        pageId_t targetPageID = preRenderPageConfig[newPageID].preRenderTargetPages[i];
        xQueueSendToBack(preRendererQueue, &targetPageID, 0);
    }
    if(lv_timer_get_paused(preRendererTimer))
    {
        lv_timer_reset(preRendererTimer);
        lv_timer_resume(preRendererTimer);
    }
}
void uiResetPreprocessQueue(void)
{
    xQueueReset(preRendererQueue);
    lv_timer_pause(preRendererTimer);
}
static void uiPagePreloadTask(lv_timer_t *timer)
{
    pageId_t pageId;
    if(preRendererQueue == NULL)
    {
        goto fatal;
    }
    if(xQueuePeek(preRendererQueue, &pageId, 0) == pdPASS)
    {
        if((pageId < PAGE_MAIN || pageId >= PAGE_COUNT) || 
            (preRenderPageState[pageId].config == NULL || preRenderPageState[pageId].config->init_func_with_step == NULL))
        {
            bk_printf(TAG "[SCREEN] Invalid pre-loading page function: %d\n", pageId);
            goto fatal;
        }

        rendererFuncStatus_t result =
            preRenderPageState[pageId].config->init_func_with_step(&bk_lv_tool_ui);
        if(result == RENDERER_FUNC_NOT_DONE)
        {
            bk_printf(TAG "[SCREEN] Pre-loading page %d is not finished yet\n", pageId);
        }
        else if(result == RENDERER_FUNC_FAILED)
        {
            bk_printf(TAG "[SCREEN] Pre-loading page %d failed\n", pageId);
            goto fatal;
        }
        else
        {
            xQueueReceive(preRendererQueue, &pageId, 0);
        }
    }
    else
    {
        bk_printf(TAG "[SCREEN] Preloading queue is empty\n");
        lv_timer_pause(timer);
    }

    lv_timer_reset(timer); // yeild처럼 쓰려면 이러면 됨. period만큼 무조건 쉰다.
    return;

fatal:
    bk_printf(TAG "[SCREEN] Fatal error in uiPagePreloadTask\n");
    lv_delay_ms(2000);
    LV_ASSERT(0);
    return;
}

bool isPagePreloaded(pageId_t pageID)
{
    return preRenderPageState[pageID].isRendered;
}
void uiPreloadPageForce(pageId_t pageID)
{
    pageLifecycleFuncWithStep_t initFunc;
    initFunc = getPageInitFunc(pageID);
    if(initFunc == NULL)
    {
        bk_printf(TAG "[SCREEN] Preloading pageId: %d is NULL\n", pageID);
        goto fatal;
    }
    while(true)
    {
        rendererFuncStatus_t ret = initFunc(&bk_lv_tool_ui);
        bk_printf(TAG "[SCREEN] Preloading pageId: %d\n", pageID);
        if(ret == RENDERER_FUNC_DONE)
        {
            break;
        }
        else if(ret == RENDERER_FUNC_FAILED)
        {
            bk_printf(TAG "[SCREEN] Preloading pageId: %d encountered an error\n", pageID);
            goto fatal;
        }
    }
    bk_printf(TAG "[SCREEN] Preloading pageId: %d completed\n", pageID);
    return;
    
fatal:
    lv_delay_ms(2000);
    LV_ASSERT(0);
    return;
}

rendererFuncStatus_t init_shared_image_asset()
{
    for(int i = 0 ; i < SHARED_IMAGE_COUNT ; i++)
    {
        if(i == SHARED_IMAGE_NONE)
        {
            continue;
        }
        sharedImageAssetState[i].imageInfo = &sharedImageAssetInfo[i];
        sharedImageAssetState[i].imageBuffer = NULL;
    }
    for(int i = 0 ; i < SHARED_IMAGE_COUNT ; i++)
    {
        if(i == SHARED_IMAGE_NONE)
        {
            continue;
        }
        bool hasLanguageVariant = sharedImageAssetInfo[i].hasLanguageVariant;
        bool hasDegreeVariant = sharedImageAssetInfo[i].hasDegreeVariant;
        const char *extension = sharedImageAssetInfo[i].fileExtension != NULL ?
                                sharedImageAssetInfo[i].fileExtension : ".png";
        const char *degreeSuffix = hasDegreeVariant &&
                                   strcmp(settings_get_str("Degree"), "\xc2\xb0""F") == 0 ? "_f" : "";
        char variantFilePath[128];
        lv_draw_buf_t *imageBuffer;
        if(hasLanguageVariant)
        {
            const char *languageSuffix = settings_get_int("LANGUAGE") == 1 ? "_china" :
                                         settings_get_int("LANGUAGE") == 2 ? "_english" : "";
            snprintf(variantFilePath, sizeof(variantFilePath), "%s%s%s%s",
                     sharedImageAssetInfo[i].imagePath, degreeSuffix, languageSuffix, extension);
            bk_printf(TAG "[SHARED_IMAGE] init_shared_image_asset: Loading image for assetId %d with language variant: [%s]\n", i, variantFilePath);
        }
        else
        {
            snprintf(variantFilePath, sizeof(variantFilePath), "%s%s%s",
                     sharedImageAssetInfo[i].imagePath, degreeSuffix, extension);
            bk_printf(TAG "[SHARED_IMAGE] init_shared_image_asset: Loading image for assetId %d without language variant: [%s]\n", i, variantFilePath);
        }

        imageBuffer = lv_image_decoder_prewarm_to_buffer(variantFilePath);
        if(!imageBuffer)
        {
            bk_printf(TAG "[SHARED_IMAGE] init_shared_image_asset: Failed to load image for assetId %d from path: %s\n", i, variantFilePath);
            goto failed;
        }
        sharedImageAssetState[i].imageBuffer = imageBuffer;
        bk_printf(TAG "[SHARED_IMAGE] init_shared_image_asset: Successfully loaded image for assetId %d from path: %s\n", i, variantFilePath);
    }
    return RENDERER_FUNC_DONE;

    failed:
    return RENDERER_FUNC_FAILED;
}
