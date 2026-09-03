#include "lvgl.h"
#include "beken_ui.h"
#include "custom_func.h"
#include "event_runtime.h"
#include "ui_lang.h"
#include <stdio.h>
#include <string.h>

#include "pageManager.h"
#define TAG "[popupdelete_init.c] "
// #define bk_printf(fmt, ...) do {if(0) printf(fmt, ##__VA_ARGS__); } while(0) // disable printf

extern bk_lv_ui_t bk_lv_tool_ui;
extern void popupdelete_yesbt_event_cb(lv_event_t *e);
extern void popupdelete_nobt_event_cb(lv_event_t *e);

void destroy_page_popupdelete(bk_lv_ui_t *bk_ui)
{
    if (bk_ui == NULL) return;
    if (bk_ui->popupdelete != NULL && lv_obj_is_valid(bk_ui->popupdelete)) {
        lv_obj_del(bk_ui->popupdelete);
        bk_ui->popupdelete = NULL;
    }
}

void init_page_popupdelete(bk_lv_ui_t *bk_ui)
{
    if (bk_ui->popupdelete != NULL && lv_obj_is_valid(bk_ui->popupdelete))
        destroy_page_popupdelete(bk_ui);

    /* memorymode 위에 오버레이 — lv_scr_load 없이 동작 */
    ui_lang_reset_popupdelete_cache();
    bk_ui->popupdelete = lv_obj_create(bk_ui->memorymode);
    lv_obj_set_pos(bk_ui->popupdelete, 0, 0);
    lv_obj_set_size(bk_ui->popupdelete, 1024, 600);
    lv_obj_set_scrollbar_mode(bk_ui->popupdelete, LV_SCROLLBAR_MODE_OFF);
#if POPUP_OPAQUE_BG
    lv_obj_set_style_bg_color(bk_ui->popupdelete, lv_color_hex(0x1C1C1C), 0);
    lv_obj_set_style_bg_opa(bk_ui->popupdelete, LV_OPA_COVER, 0);
#else
    lv_obj_set_style_bg_color(bk_ui->popupdelete, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(bk_ui->popupdelete, LV_OPA_50, 0);
#endif
    lv_obj_set_style_border_width(bk_ui->popupdelete, 0, 0);
    lv_obj_set_style_radius(bk_ui->popupdelete, 0, 0);
    lv_obj_set_style_pad_all(bk_ui->popupdelete, 0, 0);
    lv_obj_add_flag(bk_ui->popupdelete, LV_OBJ_FLAG_CLICKABLE);  /* 하위 클릭 차단 */

    // ImageView: imageview1
    bk_ui->popupdelete_imageview1 = lv_image_create(bk_ui->popupdelete);
#if UI_POPUP_DIALOG_JPG_ENABLE
    /* jpg 에셋이 원본 png 대비 상하좌우 11px씩 trim되어 있으므로 배치 좌표를
     * +11,+11 보정해 화면상 위치가 기존 png(239,82)와 동일하게 유지되게 함 */
    lv_obj_set_pos(bk_ui->popupdelete_imageview1, 239 + 11, 82 + 11);
#else
    lv_obj_set_pos(bk_ui->popupdelete_imageview1, 239, 82);
#endif
    lv_obj_set_size(bk_ui->popupdelete_imageview1, 0, 0);

    // Button: yesbt
    bk_ui->popupdelete_yesbt = lv_button_create(bk_ui->popupdelete);
    lv_obj_add_flag(bk_ui->popupdelete_yesbt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->popupdelete_yesbt, popupdelete_yesbt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->popupdelete_yesbt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->popupdelete_yesbt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->popupdelete_yesbt, 0, 0);
    lv_obj_set_pos(bk_ui->popupdelete_yesbt, 496, 348);
    lv_obj_set_size(bk_ui->popupdelete_yesbt, 255, 75);

    // Button: nobt
    bk_ui->popupdelete_nobt = lv_button_create(bk_ui->popupdelete);
    lv_obj_add_flag(bk_ui->popupdelete_nobt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->popupdelete_nobt, popupdelete_nobt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->popupdelete_nobt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->popupdelete_nobt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->popupdelete_nobt, 0, 0);
    lv_obj_set_pos(bk_ui->popupdelete_nobt, 244, 348);
    lv_obj_set_size(bk_ui->popupdelete_nobt, 255, 75);

    ui_lang_apply_popupdelete(bk_ui);
    lv_obj_set_size(bk_ui->popupdelete_imageview1, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
}

rendererFuncStatus_t init_page_popupdelete_with_step(bk_lv_ui_t *bk_ui)
{
    return RENDERER_FUNC_FAILED;
}