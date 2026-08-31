#include "lvgl.h"
#include "beken_ui.h"
#include "custom_func.h"
#include "event_runtime.h"
#include "ui_lang.h"
#include <stdio.h>
#include <string.h>

#include "preRenderer.h"
#include "preRenderInfo.h"
#define TAG "[popuperror_init.c] "
#define bk_printf(fmt, ...) do {if(0) printf(fmt, ##__VA_ARGS__); } while(0) // disable printf

extern bk_lv_ui_t bk_lv_tool_ui;
extern void popuperror_popup_dismiss_event_cb(lv_event_t *e);

void destroy_page_popuperror(bk_lv_ui_t *bk_ui)
{
    if (bk_ui == NULL) return;
    if (bk_ui->popuperror != NULL && lv_obj_is_valid(bk_ui->popuperror)) {
        lv_obj_del(bk_ui->popuperror);
        bk_ui->popuperror = NULL;
    }
}

/* 현재 활성 화면 위에 오버레이로 생성 */
void init_page_popuperror(bk_lv_ui_t *bk_ui)
{
    if (bk_ui->popuperror != NULL && lv_obj_is_valid(bk_ui->popuperror))
        destroy_page_popuperror(bk_ui);

    ui_lang_reset_popuperror_cache();
    bk_ui->popuperror = lv_obj_create(lv_scr_act());
    lv_obj_set_pos(bk_ui->popuperror, 0, 0);
    lv_obj_set_size(bk_ui->popuperror, 1024, 600);
    lv_obj_set_scrollbar_mode(bk_ui->popuperror, LV_SCROLLBAR_MODE_OFF);
#if POPUP_OPAQUE_BG
    lv_obj_set_style_bg_color(bk_ui->popuperror, lv_color_hex(0x1C1C1C), 0);
    lv_obj_set_style_bg_opa(bk_ui->popuperror, LV_OPA_COVER, 0);
#else
    lv_obj_set_style_bg_color(bk_ui->popuperror, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(bk_ui->popuperror, LV_OPA_50, 0);
#endif
    lv_obj_set_style_border_width(bk_ui->popuperror, 0, 0);
    lv_obj_set_style_radius(bk_ui->popuperror, 0, 0);
    lv_obj_set_style_pad_all(bk_ui->popuperror, 0, 0);
    lv_obj_add_flag(bk_ui->popuperror, LV_OBJ_FLAG_CLICKABLE);

    // 전체 클릭 → dismiss (에러 팝업은 클릭으로 닫기 가능)
    bk_ui->popuperror_popup_dismiss = lv_button_create(bk_ui->popuperror);
    lv_obj_add_flag(bk_ui->popuperror_popup_dismiss, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->popuperror_popup_dismiss, popuperror_popup_dismiss_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->popuperror_popup_dismiss, 0, 0);
    lv_obj_set_style_border_width(bk_ui->popuperror_popup_dismiss, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->popuperror_popup_dismiss, 0, 0);
    lv_obj_set_pos(bk_ui->popuperror_popup_dismiss, 0, 0);
    lv_obj_set_size(bk_ui->popuperror_popup_dismiss, 1024, 600);

    // ImageView: popup 배경 이미지
    bk_ui->popuperror_imageview2 = lv_image_create(bk_ui->popuperror);
    lv_obj_set_pos(bk_ui->popuperror_imageview2, 257, 145);
    lv_obj_set_size(bk_ui->popuperror_imageview2, LV_SIZE_CONTENT, LV_SIZE_CONTENT);

    // ImageView: e1~e10 (에러 코드 아이콘)
    bk_ui->popuperror_e1  = lv_image_create(bk_ui->popuperror);
    lv_obj_set_pos(bk_ui->popuperror_e1,  302, 253);
    lv_obj_set_size(bk_ui->popuperror_e1,  LV_SIZE_CONTENT, LV_SIZE_CONTENT);

    bk_ui->popuperror_e2  = lv_image_create(bk_ui->popuperror);
    lv_obj_set_pos(bk_ui->popuperror_e2,  390, 253);
    lv_obj_set_size(bk_ui->popuperror_e2,  LV_SIZE_CONTENT, LV_SIZE_CONTENT);

    bk_ui->popuperror_e3  = lv_image_create(bk_ui->popuperror);
    lv_obj_set_pos(bk_ui->popuperror_e3,  478, 253);
    lv_obj_set_size(bk_ui->popuperror_e3,  LV_SIZE_CONTENT, LV_SIZE_CONTENT);

    bk_ui->popuperror_e4  = lv_image_create(bk_ui->popuperror);
    lv_obj_set_pos(bk_ui->popuperror_e4,  566, 253);
    lv_obj_set_size(bk_ui->popuperror_e4,  LV_SIZE_CONTENT, LV_SIZE_CONTENT);

    bk_ui->popuperror_e5  = lv_image_create(bk_ui->popuperror);
    lv_obj_set_pos(bk_ui->popuperror_e5,  654, 253);
    lv_obj_set_size(bk_ui->popuperror_e5,  LV_SIZE_CONTENT, LV_SIZE_CONTENT);

    bk_ui->popuperror_e6  = lv_image_create(bk_ui->popuperror);
    lv_obj_set_pos(bk_ui->popuperror_e6,  302, 308);
    lv_obj_set_size(bk_ui->popuperror_e6,  LV_SIZE_CONTENT, LV_SIZE_CONTENT);

    bk_ui->popuperror_e7  = lv_image_create(bk_ui->popuperror);
    lv_obj_set_pos(bk_ui->popuperror_e7,  390, 308);
    lv_obj_set_size(bk_ui->popuperror_e7,  LV_SIZE_CONTENT, LV_SIZE_CONTENT);

    bk_ui->popuperror_e8  = lv_image_create(bk_ui->popuperror);
    lv_obj_set_pos(bk_ui->popuperror_e8,  478, 308);
    lv_obj_set_size(bk_ui->popuperror_e8,  LV_SIZE_CONTENT, LV_SIZE_CONTENT);

    bk_ui->popuperror_e9  = lv_image_create(bk_ui->popuperror);
    lv_obj_set_pos(bk_ui->popuperror_e9,  566, 308);
    lv_obj_set_size(bk_ui->popuperror_e9,  LV_SIZE_CONTENT, LV_SIZE_CONTENT);

    bk_ui->popuperror_e10 = lv_image_create(bk_ui->popuperror);
    lv_obj_set_pos(bk_ui->popuperror_e10, 654, 308);
    lv_obj_set_size(bk_ui->popuperror_e10, LV_SIZE_CONTENT, LV_SIZE_CONTENT);

    ui_lang_apply_popuperror(bk_ui);
}

rendererFuncStatus_t init_page_popuperror_with_step(bk_lv_ui_t *bk_ui)
{
    return RENDERER_FUNC_FAILED;
}