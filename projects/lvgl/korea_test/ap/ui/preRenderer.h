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

/* During initBase and page show lifecycle callbacks, file-backed image paths
 * are queued instead of decoded immediately. The single page-build worker
 * applies one path per timer callback and refreshes before returning control to
 * LVGL. Outside a page-build capture this behaves like lv_image_set_src(). */
extern void ui_page_build_set_image_src(lv_obj_t *obj, const void *src);
extern void ui_page_build_enqueue_task(void (*task)(void), const char *name);
extern bool ui_page_build_step(uint32_t step);
extern void ui_page_build_cancel(void);

#ifdef __cplusplus
}
#endif
#endif
