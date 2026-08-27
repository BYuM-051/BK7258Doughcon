#include "lvgl.h"
#include "beken_ui.h"
#include "custom_func.h"
#include "event_runtime.h"
#include <stdio.h>
#include <string.h>

#include "ui_config.h"
#include "preRenderer.h"

extern bk_lv_ui_t bk_lv_tool_ui;
extern lv_obj_t *preRenderRoot;
extern void neurosys_allbt_event_cb(lv_event_t *e);
extern void neurosys_onebt_event_cb(lv_event_t *e);
extern void neurosys_twobt_event_cb(lv_event_t *e);
extern void neurosys_threebt_event_cb(lv_event_t *e);
extern void neurosys_fourbt_event_cb(lv_event_t *e);
void destroy_page_neurosys(bk_lv_ui_t *bk_ui)
{
    if (bk_ui == NULL) {
        return;
    }

    if (bk_ui->neurosys != NULL) {
        lv_obj_del(bk_ui->neurosys);
        bk_ui->neurosys = NULL;
    }
}

void init_page_neurosys(bk_lv_ui_t * bk_ui) {
    if (bk_ui->neurosys != NULL && lv_obj_is_valid(bk_ui->neurosys)) {
        destroy_page_neurosys(bk_ui);
    }

#if UI_PRENDERING_ENABLE
    bk_ui->neurosys = lv_obj_create(preRenderRoot);
    lv_obj_remove_style_all(bk_ui->neurosys);
    lv_obj_set_size(bk_ui->neurosys, 1024, 600);
    lv_obj_set_pos(bk_ui->neurosys, 0, 0);
    lv_obj_set_style_radius(bk_ui->neurosys, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(bk_ui->neurosys, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_scrollbar_mode(bk_ui->neurosys, LV_SCROLLBAR_MODE_OFF);
#else
    bk_ui->neurosys = lv_obj_create(NULL);
    lv_obj_set_size(bk_ui->neurosys, 1024, 600);
    lv_obj_set_scrollbar_mode(bk_ui->neurosys, LV_SCROLLBAR_MODE_OFF);
#endif /* UI_PRENDERING_ENABLE */
    // ImageView: allim
    bk_ui->neurosys_allim = lv_image_create(bk_ui->neurosys);
    ui_page_build_set_image_src(bk_ui->neurosys_allim, "/images/red.png");
    lv_obj_set_pos(bk_ui->neurosys_allim, 0, 0);
    lv_obj_set_size(bk_ui->neurosys_allim, LV_PCT(100), LV_PCT(100));

    // Button: allbt
    bk_ui->neurosys_allbt = lv_button_create(bk_ui->neurosys);
    lv_obj_add_flag(bk_ui->neurosys_allbt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->neurosys_allbt, neurosys_allbt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->neurosys_allbt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->neurosys_allbt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->neurosys_allbt, 0, 0);
    lv_obj_set_pos(bk_ui->neurosys_allbt, 0, 0);
    lv_obj_set_size(bk_ui->neurosys_allbt, LV_PCT(100), LV_PCT(100));

    // Button: onebt
    bk_ui->neurosys_onebt = lv_button_create(bk_ui->neurosys);
    lv_obj_add_flag(bk_ui->neurosys_onebt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->neurosys_onebt, neurosys_onebt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->neurosys_onebt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->neurosys_onebt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->neurosys_onebt, 0, 0);
    lv_obj_set_pos(bk_ui->neurosys_onebt, 132, 124);
    lv_obj_set_size(bk_ui->neurosys_onebt, 100, 100);

    // Button: twobt
    bk_ui->neurosys_twobt = lv_button_create(bk_ui->neurosys);
    lv_obj_add_flag(bk_ui->neurosys_twobt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->neurosys_twobt, neurosys_twobt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->neurosys_twobt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->neurosys_twobt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->neurosys_twobt, 0, 0);
    lv_obj_set_pos(bk_ui->neurosys_twobt, 132, 384);
    lv_obj_set_size(bk_ui->neurosys_twobt, 100, 100);

    // Button: threebt
    bk_ui->neurosys_threebt = lv_button_create(bk_ui->neurosys);
    lv_obj_add_flag(bk_ui->neurosys_threebt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->neurosys_threebt, neurosys_threebt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->neurosys_threebt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->neurosys_threebt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->neurosys_threebt, 0, 0);
    lv_obj_set_pos(bk_ui->neurosys_threebt, 782, 124);
    lv_obj_set_size(bk_ui->neurosys_threebt, 100, 100);

    // Button: fourbt
    bk_ui->neurosys_fourbt = lv_button_create(bk_ui->neurosys);
    lv_obj_add_flag(bk_ui->neurosys_fourbt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->neurosys_fourbt, neurosys_fourbt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->neurosys_fourbt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->neurosys_fourbt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->neurosys_fourbt, 0, 0);
    lv_obj_set_pos(bk_ui->neurosys_fourbt, 782, 384);
    lv_obj_set_size(bk_ui->neurosys_fourbt, 100, 100);

}
