#include "lvgl.h"
#include "beken_ui.h"
#include "custom_func.h"
#include "event_runtime.h"
#include <stdio.h>
#include <string.h>

#include "beken_ui.h"
#include "device_state.h"

#define TAG "[popupconnectionerror_cb.c] "
// #define bk_printf(fmt, ...) do {if(0) printf(fmt, ##__VA_ARGS__); } while(0) // disable printf

extern bk_lv_ui_t bk_lv_tool_ui;
extern void init_page_popupconnectionerror(bk_lv_ui_t *bk_ui);

static bool s_conn_shown = false;

static void _conn_show_overlay(bk_lv_ui_t *bk_ui)
{
    if (bk_ui->popupconnectionerror != NULL && lv_obj_is_valid(bk_ui->popupconnectionerror)) {
        if (lv_obj_get_parent(bk_ui->popupconnectionerror) != lv_scr_act()) {
            lv_obj_del(bk_ui->popupconnectionerror);
            bk_ui->popupconnectionerror = NULL;
        }
    }
    if (bk_ui->popupconnectionerror == NULL || !lv_obj_is_valid(bk_ui->popupconnectionerror))
        init_page_popupconnectionerror(bk_ui);

    _img_ensure_src(bk_ui->popupconnectionerror);
    lv_obj_clear_flag(bk_ui->popupconnectionerror, LV_OBJ_FLAG_HIDDEN);
    s_conn_shown = true;
}

static void _conn_hide_overlay(bk_lv_ui_t *bk_ui)
{
    if (bk_ui->popupconnectionerror != NULL && lv_obj_is_valid(bk_ui->popupconnectionerror)) {
        lv_obj_del(bk_ui->popupconnectionerror);
        bk_ui->popupconnectionerror = NULL;
    }
    s_conn_shown = false;
}

void popupconnectionerror_tick(bk_lv_ui_t *bk_ui)
{
    device_state_t *state = &g_device_state;

    if (!state->error_cut) {
        if (s_conn_shown) _conn_hide_overlay(bk_ui);
        return;
    }

    /* error_cut=true: force stop all operations */
    state->operation           = false;
    state->auto_mode_start     = false;
    state->auto_dry_mode_start = false;

    if (!s_conn_shown) {
        bk_printf(TAG "[CONN_ERR] connection error detected, showing overlay\n");
        _conn_show_overlay(bk_ui);
        return;
    }

    /* Shown: handle screen change */
    if (bk_ui->popupconnectionerror == NULL || !lv_obj_is_valid(bk_ui->popupconnectionerror)) {
        s_conn_shown = false;
        _conn_show_overlay(bk_ui);
    } else if (lv_obj_get_parent(bk_ui->popupconnectionerror) != lv_scr_act()) {
        _conn_hide_overlay(bk_ui);
        _conn_show_overlay(bk_ui);
    }
}
