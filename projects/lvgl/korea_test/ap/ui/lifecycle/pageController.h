#ifndef PAGE_CONTROLLER_H
#define PAGE_CONTROLLER_H

#include "preRenderInfo.h"

extern bk_lv_ui_t bk_lv_tool_ui;
extern lv_obj_t *preRenderRoot;

extern void pageControllerInit(void);
extern void ui_page_init(pageId_t pageID);
extern void ui_page_init_ShowOption(pageId_t pageID, bool showOption);
extern void ui_page_change(pageId_t pageID);
extern void ui_page_change_ShowOption(pageId_t pageID, bool showOption);

extern lv_obj_t* ui_get_current_page(void);
extern pageId_t ui_get_current_page_id(void);

#endif /* PAGE_CONTROLLER_H */