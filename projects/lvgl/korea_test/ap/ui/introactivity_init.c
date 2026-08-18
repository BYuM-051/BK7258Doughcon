#include "lvgl.h"
#include "beken_ui.h"
#include "custom_func.h"
#include "event_runtime.h"
#include <stdio.h>
#include <string.h>

extern bk_lv_ui_t bk_lv_tool_ui;

void destroy_page_introactivity(bk_lv_ui_t *bk_ui)
{
    if (bk_ui == NULL) {
        return;
    }

    if (bk_ui->introactivity != NULL) {
        lv_obj_del(bk_ui->introactivity);
        bk_ui->introactivity = NULL;
    }
}

void init_page_introactivity(bk_lv_ui_t * bk_ui) {
    if (bk_ui->introactivity != NULL && lv_obj_is_valid(bk_ui->introactivity)) {
        destroy_page_introactivity(bk_ui);
    }

    bk_ui->introactivity = lv_obj_create(NULL);
    lv_obj_set_size(bk_ui->introactivity, 1024, 600);
    lv_obj_set_scrollbar_mode(bk_ui->introactivity, LV_SCROLLBAR_MODE_OFF);
    // ImageView: intro
    bk_ui->introactivity_intro = lv_image_create(bk_ui->introactivity);
    lv_obj_set_pos(bk_ui->introactivity_intro, 0, 0);
    lv_obj_set_size(bk_ui->introactivity_intro, LV_PCT(100), LV_PCT(100));
    lv_image_set_src(bk_ui->introactivity_intro, "/images/intro.jpg");

    lv_obj_t *label = lv_label_create(bk_ui->introactivity);
    lv_label_set_text(label, "BOOTING...");
    lv_obj_align(label, LV_ALIGN_BOTTOM_MID, 0, -50);
    lv_obj_set_style_text_color(label, lv_color_white(), 0);
}