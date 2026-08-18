#include "lvgl.h"
#include "beken_ui.h"
#include "custom_func.h"
#include "event_runtime.h"
#include <stdio.h>
#include <string.h>

#include "ui_lang.h"

extern bk_lv_ui_t bk_lv_tool_ui;

void destroy_page_popupconnectionerror(bk_lv_ui_t *bk_ui)
{
    if (bk_ui == NULL) return;
    if (bk_ui->popupconnectionerror != NULL && lv_obj_is_valid(bk_ui->popupconnectionerror)) {
        lv_obj_del(bk_ui->popupconnectionerror);
        bk_ui->popupconnectionerror = NULL;
    }
}

/* 현재 활성 화면 위에 오버레이로 생성 — 닫기 버튼 없음 (통신 에러 해제 시 자동 제거) */
void init_page_popupconnectionerror(bk_lv_ui_t *bk_ui)
{
    if (bk_ui->popupconnectionerror != NULL && lv_obj_is_valid(bk_ui->popupconnectionerror))
        destroy_page_popupconnectionerror(bk_ui);

    ui_lang_reset_popupconnectionerror_cache();
    bk_ui->popupconnectionerror = lv_obj_create(lv_scr_act());
    lv_obj_set_pos(bk_ui->popupconnectionerror, 0, 0);
    lv_obj_set_size(bk_ui->popupconnectionerror, 1024, 600);
    lv_obj_set_scrollbar_mode(bk_ui->popupconnectionerror, LV_SCROLLBAR_MODE_OFF);
#if POPUP_OPAQUE_BG
    lv_obj_set_style_bg_color(bk_ui->popupconnectionerror, lv_color_hex(0x1C1C1C), 0);
    lv_obj_set_style_bg_opa(bk_ui->popupconnectionerror, LV_OPA_COVER, 0);
#else
    lv_obj_set_style_bg_color(bk_ui->popupconnectionerror, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(bk_ui->popupconnectionerror, LV_OPA_50, 0);
#endif
    lv_obj_set_style_border_width(bk_ui->popupconnectionerror, 0, 0);
    lv_obj_set_style_radius(bk_ui->popupconnectionerror, 0, 0);
    lv_obj_set_style_pad_all(bk_ui->popupconnectionerror, 0, 0);
    lv_obj_add_flag(bk_ui->popupconnectionerror, LV_OBJ_FLAG_CLICKABLE);

    bk_ui->popupconnectionerror_popup = lv_image_create(bk_ui->popupconnectionerror);
    _img_set_src_timed(bk_ui->popupconnectionerror_popup, "/images/popup_connection_error.png");
    lv_obj_set_pos(bk_ui->popupconnectionerror_popup, 0, 0);
    lv_obj_set_size(bk_ui->popupconnectionerror_popup, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_center(bk_ui->popupconnectionerror_popup);

    ui_lang_apply_popupconnectionerror(bk_ui);
}
