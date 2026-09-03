#ifndef __PRE_RENDERER_H__
#define __PRE_RENDERER_H__

#include "lvgl.h"
#include "beken_ui.h"
#include "preRenderInfo.h"
#include "ui_config.h"

#ifdef __cplusplus
extern "C" {
#endif

#define _PREPROCESSOR_YIELD_DELAY 1

extern lv_event_code_t UI_EVENT_PAGE_SHOW_START;
extern lv_event_code_t UI_EVENT_PAGE_SHOWN;
extern lv_event_code_t UI_EVENT_PAGE_HIDE_START;
extern lv_event_code_t UI_EVENT_PAGE_HIDDEN;

extern void uiPreprocessorInit(void);
extern bool uiPreprocessorInitialized(void);
extern bool isPagePreloaded(pageId_t pageID);

extern void uiEnqueuePreloadTargets(pageId_t newPageID);
extern void uiPreloadPageForce(pageId_t pageID);
extern void uiResetPreprocessQueue(void);

#ifdef __cplusplus
}
#endif
#endif