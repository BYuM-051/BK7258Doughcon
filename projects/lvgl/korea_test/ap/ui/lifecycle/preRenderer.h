#ifndef __PRE_RENDERER_H__
#define __PRE_RENDERER_H__

#include "lvgl.h"
#include "beken_ui.h"
#include "preRenderInfo.h"

#ifdef __cplusplus
extern "C" {
#endif

extern lv_obj_t *preRenderRoot;
extern lv_obj_t *currentPage;
extern lv_event_code_t UI_EVENT_PAGE_SHOW_START;
extern lv_event_code_t UI_EVENT_PAGE_SHOWN;
extern lv_event_code_t UI_EVENT_PAGE_HIDE_START;
extern lv_event_code_t UI_EVENT_PAGE_HIDDEN;

extern void ui_screen_event_init(void);
extern bool ui_screen_event_initialized(void);
extern void ui_page_change(pageId_t newPageID);
extern lv_obj_t *ui_get_current_page(void);

#ifdef __cplusplus
}
#endif
#endif