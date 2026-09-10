#ifndef __PAGE_PREPROCESSOR_H__
#define __PAGE_PREPROCESSOR_H__

#include "preRenderInfo.h"

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
extern void uiResetPreprocessedPages(pageId_t exceptionPage);
extern void uiPageUnloadImage(pageId_t pageID);

#ifdef __cplusplus
}
#endif
#endif /* __PAGE_PREPROCESSOR_H__ */
