#include "lvgl.h"
#include "beken_ui.h"
#include "custom_func.h"
#include "event_runtime.h"
#include <stdio.h>
#include <string.h>

#include "beken_ui.h"
#include "device_state.h"
#include "ui_lang.h"
#include "hardware_hal.h"

extern bk_lv_ui_t bk_lv_tool_ui;
extern void init_page_popuperror(bk_lv_ui_t *bk_ui);

static uint32_t last_click_time  = 0;
static bool     s_shown          = false;
static bool     s_dismissed      = false;
static bool     s_prev_error     = false;   /* 에러 상승 엣지 감지용 */

#define _EIMG(obj, flag, n) \
    _img_set_src_timed((obj), (flag) ? "/images/e" #n "_on.png" : "/images/e" #n "_off.png")

static void _update_error_codes(bk_lv_ui_t *bk_ui)
{
    /* MCU 0x43 프로토콜 문서 기준 실제 비트 배정 (순서대로 1:1이 아님):
     * Error Code 1 (err8=saveoperation[8]): bit0=E1(F), bit1=E2(제상), bit3=E3(RT),
     *              bit4=E4(습도), bit6=E6(EEPROM)  — bit2,5,7 미사용
     * Error Code 2 (err9=saveoperation[9]): bit0=E7(설정시간), bit1=E8(제상),
     *              bit2=E9(COMP 동작이상), bit3=E10(사이클 동작이상), bit4=E5(히터동작이상) */
    uint8_t err8 = g_device_state.error_flags[2];
    uint8_t err9 = g_device_state.error_flags[3];

    _EIMG(bk_ui->popuperror_e1,  err8 & (1<<0), 1);
    _EIMG(bk_ui->popuperror_e2,  err8 & (1<<1), 2);
    _EIMG(bk_ui->popuperror_e3,  err8 & (1<<3), 3);
    _EIMG(bk_ui->popuperror_e4,  err8 & (1<<4), 4);
    _EIMG(bk_ui->popuperror_e5,  err9 & (1<<4), 5);
    _EIMG(bk_ui->popuperror_e6,  err8 & (1<<6), 6);
    _EIMG(bk_ui->popuperror_e7,  err9 & (1<<0), 7);
    _EIMG(bk_ui->popuperror_e8,  err9 & (1<<1), 8);
    _EIMG(bk_ui->popuperror_e9,  err9 & (1<<2), 9);
    _EIMG(bk_ui->popuperror_e10, err9 & (1<<3), 10);
}

#undef _EIMG

static void _show_overlay(bk_lv_ui_t *bk_ui)
{
    if (bk_ui->popuperror != NULL && lv_obj_is_valid(bk_ui->popuperror)) {
        if (lv_obj_get_parent(bk_ui->popuperror) != lv_scr_act()) {
            lv_obj_del(bk_ui->popuperror);
            bk_ui->popuperror = NULL;
        }
    }
    if (bk_ui->popuperror == NULL || !lv_obj_is_valid(bk_ui->popuperror))
        init_page_popuperror(bk_ui);

    ui_lang_apply_popuperror(bk_ui);
    _update_error_codes(bk_ui);
    _img_ensure_src(bk_ui->popuperror);
    lv_obj_clear_flag(bk_ui->popuperror, LV_OBJ_FLAG_HIDDEN);
    s_shown = true;
}

static void _hide_overlay(bk_lv_ui_t *bk_ui)
{
    if (bk_ui->popuperror != NULL && lv_obj_is_valid(bk_ui->popuperror)) {
        lv_obj_del(bk_ui->popuperror);
        bk_ui->popuperror = NULL;
    }
    s_shown = false;
}

void popuperror_tick(bk_lv_ui_t *bk_ui)
{
    bool cur = g_device_state.error_popup;

    /* 에러 새 발생 (false→true): 이전 dismiss 초기화 → 팝업 재표시 */
    if (cur && !s_prev_error)
        s_dismissed = false;
    s_prev_error = cur;

    /* 에러 발생 중이고 아직 표시 안 됐으면 표시 */
    if (cur && !s_shown && !s_dismissed) {
        _show_overlay(bk_ui);
        return;
    }

    /* 표시 중: 화면 전환 시 오버레이 재생성, 에러 코드 갱신 */
    if (s_shown) {
        if (bk_ui->popuperror == NULL || !lv_obj_is_valid(bk_ui->popuperror)) {
            s_shown = false;
            if (!s_dismissed) _show_overlay(bk_ui);
        } else if (lv_obj_get_parent(bk_ui->popuperror) != lv_scr_act()) {
            _hide_overlay(bk_ui);
            if (!s_dismissed) _show_overlay(bk_ui);
        } else {
            _update_error_codes(bk_ui);
        }
    }
}

/* 팝업 클릭 시 dismiss */
void popuperror_dismiss(bk_lv_ui_t *bk_ui)
{
    s_dismissed = true;
    _hide_overlay(bk_ui);
}

/* 에러 버튼: 표시 중이면 닫고, 닫혀 있으면 표시 */
void popuperror_toggle(bk_lv_ui_t *bk_ui)
{
    if (s_shown) {
        s_dismissed = true;
        _hide_overlay(bk_ui);
    } else {
        s_dismissed = false;
        _show_overlay(bk_ui);
    }
}

void popuperror_popup_dismiss_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    if (lv_tick_elaps(last_click_time) < 250) return;
    last_click_time = lv_tick_get();
    hal_buzzer_beep();
    popuperror_dismiss(bk_ui);
}
