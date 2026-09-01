#include "lvgl.h"
#include "beken_ui.h"
#include "custom_func.h"
#include "event_runtime.h"
#include <stdio.h>
#include <string.h>

#include "ui_lang.h"

#include "preRenderer.h"
#include "preRenderInfo.h"
#define TAG "[popupcaution_init.c] "
// #define bk_printf(fmt, ...) do {if(0) printf(fmt, ##__VA_ARGS__); } while(0) // disable printf

extern bk_lv_ui_t bk_lv_tool_ui;
extern void popupcaution_dismissbt_event_cb(lv_event_t *e);
extern void popupcaution_load_event_cb(lv_event_t *e);

void destroy_page_popupcaution(bk_lv_ui_t *bk_ui)
{
    if (bk_ui == NULL) {
        return;
    }
    if (bk_ui->popupcaution != NULL) {
        lv_obj_del(bk_ui->popupcaution);
        bk_ui->popupcaution = NULL;
    }
}

void init_page_popupcaution(bk_lv_ui_t * bk_ui) {
    if (bk_ui->popupcaution != NULL && lv_obj_is_valid(bk_ui->popupcaution)) {
        destroy_page_popupcaution(bk_ui);
    }

    ui_lang_reset_popupcaution_cache();
    bk_ui->popupcaution = lv_obj_create(lv_scr_act());
    lv_obj_set_pos(bk_ui->popupcaution, 0, 0);
    lv_obj_set_size(bk_ui->popupcaution, 1024, 600);
#if POPUP_OPAQUE_BG
    lv_obj_set_style_bg_color(bk_ui->popupcaution, lv_color_hex(0x1C1C1C), 0);
    lv_obj_set_style_bg_opa(bk_ui->popupcaution, LV_OPA_COVER, 0);
#else
    lv_obj_set_style_bg_color(bk_ui->popupcaution, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(bk_ui->popupcaution, LV_OPA_50, 0);
#endif
    lv_obj_set_style_border_width(bk_ui->popupcaution, 0, 0);
    lv_obj_set_style_pad_all(bk_ui->popupcaution, 0, 0);
    lv_obj_remove_flag(bk_ui->popupcaution, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(bk_ui->popupcaution, LV_OBJ_FLAG_CLICKABLE);

    // ImageView: imageview1
    bk_ui->popupcaution_imageview1 = lv_image_create(bk_ui->popupcaution);
#if UI_POPUP_DIALOG_JPG_ENABLE
    /* jpg 에셋이 원본 png 대비 상하좌우 10px씩 trim되어 있으므로 배치 좌표를
     * +10,+10 보정해 화면상 위치가 기존 png(239,82)와 동일하게 유지되게 함 */
    lv_obj_set_pos(bk_ui->popupcaution_imageview1, 239 + 10, 82 + 10);
#else
    lv_obj_set_pos(bk_ui->popupcaution_imageview1, 239, 82);
#endif
    lv_obj_set_size(bk_ui->popupcaution_imageview1, 0, 0);

    // Button: dismissbt
    bk_ui->popupcaution_dismissbt = lv_button_create(bk_ui->popupcaution);
    lv_obj_add_flag(bk_ui->popupcaution_dismissbt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->popupcaution_dismissbt, popupcaution_dismissbt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->popupcaution_dismissbt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->popupcaution_dismissbt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->popupcaution_dismissbt, 0, 0);
    lv_obj_set_pos(bk_ui->popupcaution_dismissbt, 0, 0);
    lv_obj_set_size(bk_ui->popupcaution_dismissbt, 1024, 600);

    ui_lang_apply_popupcaution(bk_ui);
    lv_obj_set_size(bk_ui->popupcaution_imageview1, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
}

rendererFuncStatus_t init_page_popupcaution_with_step(bk_lv_ui_t *bk_ui)
{
    return RENDERER_FUNC_FAILED;
}