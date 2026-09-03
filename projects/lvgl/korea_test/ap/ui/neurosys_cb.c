#include "lvgl.h"
#include "beken_ui.h"
#include "custom_func.h"
#include "event_runtime.h"
#include <stdio.h>
#include <string.h>
#include "hardware_hal.h"

#define TAG "[neurosys_cb.c] "
// #define bk_printf(fmt, ...) do {if(0) printf(fmt, ##__VA_ARGS__); } while(0) // disable printf

extern bk_lv_ui_t bk_lv_tool_ui;
/* ── Forward Declarations ──────────────────────────────── */
void neurosys_allbt_event_cb(lv_event_t *e);
void neurosys_onebt_event_cb(lv_event_t *e);
void neurosys_twobt_event_cb(lv_event_t *e);
void neurosys_threebt_event_cb(lv_event_t *e);
void neurosys_fourbt_event_cb(lv_event_t *e);

/* ── Callback Functions ────────────────────────────────── */
void neurosys_allbt_event_cb(lv_event_t * e) {
    bk_lv_ui_t * bk_ui = &bk_lv_tool_ui;
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        hal_buzzer_beep();
        bk_printf(TAG "neurosys_allbt clicked\n");
    }
}

void neurosys_onebt_event_cb(lv_event_t * e) {
    bk_lv_ui_t * bk_ui = &bk_lv_tool_ui;
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        hal_buzzer_beep();
        bk_printf(TAG "neurosys_onebt clicked\n");
    }
}

void neurosys_twobt_event_cb(lv_event_t * e) {
    bk_lv_ui_t * bk_ui = &bk_lv_tool_ui;
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        hal_buzzer_beep();
        bk_printf(TAG "neurosys_twobt clicked\n");
    }
}

void neurosys_threebt_event_cb(lv_event_t * e) {
    bk_lv_ui_t * bk_ui = &bk_lv_tool_ui;
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        hal_buzzer_beep();
        bk_printf(TAG "neurosys_threebt clicked\n");
    }
}

void neurosys_fourbt_event_cb(lv_event_t * e) {
    bk_lv_ui_t * bk_ui = &bk_lv_tool_ui;
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        hal_buzzer_beep();
        bk_printf(TAG "neurosys_fourbt clicked\n");
    }
}
