#include "lvgl.h"
#include "beken_ui.h"
#include "custom_func.h"
#include "event_runtime.h"
#include <stdio.h>
#include <string.h>
#include <os/os.h>

#include "beken_ui.h"
#include "ui_animations.h"
#include "ui_lang.h"
#include "hardware_hal.h"
#include "preRenderer.h"

#define TAG "[settingmodedetailsetting_cb.c] "
#define bk_printf(fmt, ...) do {if(0) printf(fmt, ##__VA_ARGS__); } while(0) // disable printf

extern bk_lv_ui_t bk_lv_tool_ui;

static uint32_t last_click_time = 0;

/* Pre-cache reset_popup.png (729 KB, 알파 있는 RGBA라 ARGB8888 decode 필요)
 * on every entry to settingmodedetailsetting — popupreset_bg_preload()가
 * 전용 canvas에 decode하므로 클릭 시점에 PSRAM 단편화로 malloc 실패하는
 * 문제(psram_malloc_cm:738 크래시 이력)가 없다. One tick = one decode. */
static lv_timer_t *s_rst_timer = NULL;

static void _rst_prewarm_tick(lv_timer_t *t)
{
    lv_timer_delete(t); s_rst_timer = NULL;
    popupreset_bg_preload();
}

static void _rst_prewarm_start(void)
{
#if !UI_PREWARM_ENABLE
    return;
#endif
    if (s_rst_timer) return;
    s_rst_timer = lv_timer_create(_rst_prewarm_tick, 20, NULL);
    /* tick fires once then deletes itself — no repeat count needed */
}

static void _rst_prewarm_cancel(void)
{
    if (s_rst_timer) { lv_timer_delete(s_rst_timer); s_rst_timer = NULL; }
}

/* Performance logging — measure time from tap to screen load start */
static uint32_t _pt0 = 0;
#define PERF_START(tag)   do { _pt0 = lv_tick_get(); bk_printf(TAG "[PERF] %s tap\n", tag); } while(0)
#define PERF_MARK(tag)    bk_printf(TAG "[PERF]   %s +%lu ms\n", (tag), (unsigned long)lv_tick_elaps(_pt0))

void settingmodedetailsetting_backbt_event_cb(lv_event_t *e);
void settingmodedetailsetting_detail_temp_bt_event_cb(lv_event_t *e);
void settingmodedetailsetting_detail_humidity_bt_event_cb(lv_event_t *e);
void settingmodedetailsetting_detail_time_bt_event_cb(lv_event_t *e);
void settingmodedetailsetting_detail_damper_bt_event_cb(lv_event_t *e);
void settingmodedetailsetting_detail_defrost_bt_event_cb(lv_event_t *e);
void settingmodedetailsetting_detail_reset_bt_event_cb(lv_event_t *e);
void settingmodedetailsetting_load_event_cb(lv_event_t *e);

void settingmodedetailsetting_backbt_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    if (lv_tick_elaps(last_click_time) < 250) return;
    last_click_time = lv_tick_get();
    hal_buzzer_beep();

#if UI_PRENDERING_ENABLE
    ui_page_change(PAGE_SETTINGMODE);
#else
    if (bk_ui->settingmode == NULL || !lv_obj_is_valid(bk_ui->settingmode))
        init_page_settingmode(bk_ui);
    lv_scr_load(bk_ui->settingmode);
#endif /* UI_PRENDERING_ENABLE */
}

void settingmodedetailsetting_detail_temp_bt_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    if (lv_tick_elaps(last_click_time) < 250) return;
    last_click_time = lv_tick_get();
    hal_buzzer_beep();

    PERF_START("detailtemp");
#if UI_PRENDERING_ENABLE
    ui_page_change(PAGE_DETAILSETTINGTEMP);
    PERF_MARK("ui_page_change");
#else
    if (bk_ui->detailsettingtemp == NULL || !lv_obj_is_valid(bk_ui->detailsettingtemp)) {
        init_page_detailsettingtemp(bk_ui);
        PERF_MARK("init_page_detailsettingtemp");
    } else {
        PERF_MARK("detailtemp cached (no init)");
    }
    lv_scr_load(bk_ui->detailsettingtemp);
    PERF_MARK("lv_scr_load");
#endif /* UI_PRENDERING_ENABLE */
}

void settingmodedetailsetting_detail_humidity_bt_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    if (lv_tick_elaps(last_click_time) < 250) return;
    last_click_time = lv_tick_get();
    hal_buzzer_beep();

    PERF_START("detailhumidity");
#if UI_PRENDERING_ENABLE
    ui_page_change(PAGE_DETAILSETTINGHUMIDITY);
    PERF_MARK("ui_page_change");
#else
    if (bk_ui->detailsettinghumidity == NULL || !lv_obj_is_valid(bk_ui->detailsettinghumidity)) {
        init_page_detailsettinghumidity(bk_ui);
        PERF_MARK("init_page_detailsettinghumidity");
    } else {
        PERF_MARK("detailhumidity cached (no init)");
    }
    lv_scr_load(bk_ui->detailsettinghumidity);
    PERF_MARK("lv_scr_load");
#endif /* UI_PRENDERING_ENABLE */
}

void settingmodedetailsetting_detail_time_bt_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    if (lv_tick_elaps(last_click_time) < 250) return;
    last_click_time = lv_tick_get();
    hal_buzzer_beep();

    PERF_START("detailtime");
#if UI_PRENDERING_ENABLE
    ui_page_change(PAGE_DETAILSETTINGTIME);
    PERF_MARK("ui_page_change");
#else
    if (bk_ui->detailsettingtime == NULL || !lv_obj_is_valid(bk_ui->detailsettingtime)) {
        init_page_detailsettingtime(bk_ui);
        PERF_MARK("init_page_detailsettingtime");
    } else {
        PERF_MARK("detailtime cached (no init)");
    }
    lv_scr_load(bk_ui->detailsettingtime);
    PERF_MARK("lv_scr_load");
#endif /* UI_PRENDERING_ENABLE */
}

void settingmodedetailsetting_detail_damper_bt_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    if (lv_tick_elaps(last_click_time) < 250) return;
    last_click_time = lv_tick_get();
    hal_buzzer_beep();

    PERF_START("detaildamper");
#if UI_PRENDERING_ENABLE
    ui_page_change(PAGE_DETAILSETTINGDAMPER);
    PERF_MARK("ui_page_change");
#else
    if (bk_ui->detailsettingdamper == NULL || !lv_obj_is_valid(bk_ui->detailsettingdamper)) {
        init_page_detailsettingdamper(bk_ui);
        PERF_MARK("init_page_detailsettingdamper");
    } else {
        PERF_MARK("detaildamper cached (no init)");
    }
    lv_scr_load(bk_ui->detailsettingdamper);
    PERF_MARK("lv_scr_load");
#endif /* UI_PRENDERING_ENABLE */
}

void settingmodedetailsetting_detail_defrost_bt_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    if (lv_tick_elaps(last_click_time) < 250) return;
    last_click_time = lv_tick_get();
    hal_buzzer_beep();

    PERF_START("detaildefrost");
#if UI_PRENDERING_ENABLE
    ui_page_change(PAGE_DETAILSETTINGDEFROST);
    PERF_MARK("ui_page_change");
#else
    if (bk_ui->detailsettingdefrost == NULL || !lv_obj_is_valid(bk_ui->detailsettingdefrost)) {
        init_page_detailsettingdefrost(bk_ui);
        PERF_MARK("init_page_detailsettingdefrost");
    } else {
        PERF_MARK("detaildefrost cached (no init)");
    }
    lv_scr_load(bk_ui->detailsettingdefrost);
    PERF_MARK("lv_scr_load");
#endif /* UI_PRENDERING_ENABLE */
}

void settingmodedetailsetting_detail_reset_bt_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    if (lv_tick_elaps(last_click_time) < 250) return;
    last_click_time = lv_tick_get();

    _rst_prewarm_cancel();   /* stop timer before popup renders */
    hal_buzzer_beep();
    bk_printf(TAG "[MEM] reset popup: psram_free=%u min=%u\n",
           (unsigned)rtos_get_psram_free_heap_size(),
           (unsigned)rtos_get_psram_minimum_free_heap_size());

    /* 확인 팝업 — lv_layer_top() 오버레이로 표시 (배경 화면 유지) */
    init_page_popupreset(bk_ui);
}

void settingmodedetailsetting_load_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;

    if (code == LV_EVENT_SCREEN_UNLOAD_START) {
        _rst_prewarm_cancel();
        /* popupreset_imageview1->src가 canvas->draw_buf를 참조 중일 수 있으므로,
         * canvas_free() 전에 팝업이 열려있다면 먼저 닫아 dangling pointer 방지
         * (popuppassword/settingmode_cb.c와 동일한 방어 패턴). */
        if (bk_ui->popupreset && lv_obj_is_valid(bk_ui->popupreset))
            destroy_page_popupreset(bk_ui);
        popupreset_canvas_free();
        return;
    }
    if (code == LV_EVENT_SCREEN_LOAD_START) {
        ui_lang_apply_settingmodedetailsetting(bk_ui);
        return;
    }
    if (code == LV_EVENT_SCREEN_LOADED) {
        ui_title_anim(bk_ui->settingmodedetailsetting_title);
        /* Re-cache reset_popup.png on every entry (including returns from sub-screens).
         * 20 ms delay lets the screen render complete before the decode starts. */
        _rst_prewarm_start();
        return;
    }
}

void settingmodedetailsetting_unload_start_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;

    _rst_prewarm_cancel();

    /* popupreset_imageview1->src가 canvas->draw_buf를 참조 중일 수 있으므로,
     * canvas_free() 전에 팝업이 열려있다면 먼저 닫아 dangling pointer 방지
     * (popuppassword/settingmode_cb.c와 동일한 방어 패턴). */
    if (bk_ui->popupreset && lv_obj_is_valid(bk_ui->popupreset))
    {
        destroy_page_popupreset(bk_ui);
    }

    popupreset_canvas_free();
}

void settingmodedetailsetting_load_start_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;

    ui_lang_apply_settingmodedetailsetting(bk_ui);
}

void settingmodedetailsetting_loaded_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;

    ui_title_anim(bk_ui->settingmodedetailsetting_title);

    /* Re-cache reset_popup.png on every entry (including returns from sub-screens).
     * 20 ms delay lets the screen render complete before the decode starts. */
    _rst_prewarm_start();
}

void settingmodedetailsetting_unloaded_event_cb(lv_event_t *e)
{
}
