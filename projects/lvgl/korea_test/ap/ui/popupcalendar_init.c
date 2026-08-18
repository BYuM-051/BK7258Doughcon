#include "lvgl.h"
#include "beken_ui.h"
#include "custom_func.h"
#include "event_runtime.h"
#include <stdio.h>
#include <string.h>

extern bk_lv_ui_t bk_lv_tool_ui;
extern void popupcalendar_load_event_cb(lv_event_t *e);

void destroy_page_popupcalendar(bk_lv_ui_t *bk_ui)
{
    if (bk_ui == NULL) {
        return;
    }
    if (bk_ui->popupcalendar != NULL) {
        lv_obj_del(bk_ui->popupcalendar);
        bk_ui->popupcalendar = NULL;
    }
}

void init_page_popupcalendar(bk_lv_ui_t * bk_ui) {
    if (bk_ui->popupcalendar != NULL && lv_obj_is_valid(bk_ui->popupcalendar)) {
        destroy_page_popupcalendar(bk_ui);
    }

    bk_ui->popupcalendar = lv_obj_create(NULL);
    lv_obj_set_size(bk_ui->popupcalendar, 1024, 600);
    lv_obj_set_scrollbar_mode(bk_ui->popupcalendar, LV_SCROLLBAR_MODE_OFF);
    // lv_obj_add_event_cb(bk_ui->popupcalendar, popupcalendar_load_event_cb, LV_EVENT_SCREEN_LOAD_START, NULL);

    // Unsupported widget: com.prolificinteractive.materialcalendarview.MaterialCalendarView id=calendarpopup_bg
    bk_ui->popupcalendar_calendarpopup_bg = lv_obj_create(bk_ui->popupcalendar);
    lv_obj_set_style_bg_opa(bk_ui->popupcalendar_calendarpopup_bg, 0, 0);
    lv_obj_set_pos(bk_ui->popupcalendar_calendarpopup_bg, 0, 0);
    lv_obj_set_size(bk_ui->popupcalendar_calendarpopup_bg, 541, 501);

}
