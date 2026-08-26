#include "lvgl.h"
#include "src/misc/cache/instance/lv_image_cache.h"
#include "beken_ui.h"
#include "custom_func.h"
#include "event_runtime.h"
#include <stdio.h>
#include <string.h>
#include <os/os.h>

#include "beken_ui.h"
#include "ui_animations.h"
#include "ui_lang.h"
#include "settings.h"
#include "device_state.h"
#include "hardware_hal.h"

#define TAG "[settingmode_cb.c] "
#define bk_printf(fmt, ...) do {if(0) printf(fmt, ##__VA_ARGS__); } while(0) // disable printf

extern bk_lv_ui_t bk_lv_tool_ui;
extern void popuppassword_open(void);
extern void popuppassword_bg_preload(void);


static uint32_t last_click_time = 0;

/* testmode bg prewarm — kept for cancel API but NOT started on SCREEN_LOADED.
 * Starting it alongside pp_pw caused LRU overflow (testmode 2.4 MB + pp_pw
 * 2.66 MB > 4 MB cache), which evicted password_popup before popup render. */
static lv_timer_t *s_tm_bg_timer = NULL;
static lv_obj_t   *s_tm_bg_dummy = NULL;
static int         s_tm_bg_idx   = 0;

void settingmode_testmode_prewarm_reset(void)
{
    if (s_tm_bg_timer) { lv_timer_delete(s_tm_bg_timer); s_tm_bg_timer = NULL; }
    if (s_tm_bg_dummy) { lv_obj_del(s_tm_bg_dummy); s_tm_bg_dummy = NULL; }
    s_tm_bg_idx = 0;
}

/* Popup-password prewarm — caches ONLY the two large popup backgrounds.
 *
 * Why only 2 images?
 *   password_popup.png (~1.33 MB) and popup_caution.png (~1.33 MB) are the
 *   images that trigger psram_malloc OOM at render time if not already decoded.
 *   Caching the 12 keypad images (12 × ~64 KB = 0.77 MB) offered no OOM
 *   protection and actually CAUSED it: the keypad images pushed
 *   password_popup to the LRU tail, where the next incoming image evicted it
 *   before swdraw could render the popup.  Removing them from the prewarm
 *   leaves both critical images at MRU positions and keeps total cache usage
 *   well inside the 4 MB limit.
 *
 *   Keypad images decode lazily on first key-touch (~83 ms each).  This is a
 *   minor UX delay compared to the risk of a hard crash. */
static const char * const s_pp_pw_lang_bases[2] = {
    "/images/password_popup",
    "/images/popup_caution",
};
#define _PP_PW_TOTAL 2

static lv_timer_t *s_pp_pw_timer = NULL;
static lv_obj_t   *s_pp_pw_dummy = NULL;
static int         s_pp_pw_idx   = 0;

static void _pp_pw_tick(lv_timer_t *t)
{
    if (s_pp_pw_idx >= _PP_PW_TOTAL || !s_pp_pw_dummy) {
        lv_timer_delete(t); s_pp_pw_timer = NULL;
        if (s_pp_pw_dummy) { lv_obj_del(s_pp_pw_dummy); s_pp_pw_dummy = NULL; }
        bk_printf(TAG "[PERF] popuppassword prewarm done (%d imgs)\n", _PP_PW_TOTAL);
        return;
    }
    int lang = settings_get_int("LANGUAGE");
    const char *lsuf = (lang == 1) ? "_china" : (lang == 2) ? "_english" : "";
    char path[128];
    snprintf(path, sizeof(path), "%s%s.png", s_pp_pw_lang_bases[s_pp_pw_idx], lsuf);
    _img_set_src_timed(s_pp_pw_dummy, path);
    s_pp_pw_idx++;
}

static void _pp_pw_start(void)
{
#if !UI_PREWARM_ENABLE
    return;
#endif
    if (s_pp_pw_timer) return;
    if (s_pp_pw_idx >= _PP_PW_TOTAL) return;
    s_pp_pw_dummy = lv_image_create(lv_layer_top());
    lv_obj_add_flag(s_pp_pw_dummy, LV_OBJ_FLAG_HIDDEN);
    s_pp_pw_timer = lv_timer_create(_pp_pw_tick, 10, NULL);
}

static void _pp_pw_cancel(void)
{
    if (s_pp_pw_timer) { lv_timer_delete(s_pp_pw_timer); s_pp_pw_timer = NULL; }
    if (s_pp_pw_dummy) { lv_obj_del(s_pp_pw_dummy);      s_pp_pw_dummy = NULL; }
    s_pp_pw_idx = 0;
}

void settingmode_popuppassword_prewarm_reset(void)
{
    _pp_pw_cancel();
}

void settingmode_setting_detailsettingbt_event_cb(lv_event_t *e);
void settingmode_setting_degreebt_event_cb(lv_event_t *e);
void settingmode_setting_recordbt_event_cb(lv_event_t *e);
void settingmode_setting_testbt_event_cb(lv_event_t *e);
void settingmode_setting_timebt_event_cb(lv_event_t *e);
void settingmode_setting_languagebt_event_cb(lv_event_t *e);
void settingmode_backbt_event_cb(lv_event_t *e);
void settingmode_load_event_cb(lv_event_t *e);

void settingmode_setting_detailsettingbt_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    if (lv_tick_elaps(last_click_time) < 250) return;
    last_click_time = lv_tick_get();
    hal_buzzer_beep();

    /* canvas 버퍼 방식: lv_image_cache_drop 불필요 (canvas는 캐시 외부)
     * 팝업 열릴 때 raw 픽셀 직접 사용 → decode 없음 */
    bk_printf(TAG "[MEM] popup open: psram_free=%u min=%u\n",
           (unsigned)rtos_get_psram_free_heap_size(),
           (unsigned)rtos_get_psram_minimum_free_heap_size());
    popuppassword_open();
    init_page_popuppassword(bk_ui);
    /* lv_layer_top() 오버레이 방식 — lv_scr_load 불필요 */
}

void settingmode_setting_degreebt_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    if (lv_tick_elaps(last_click_time) < 250) return;
    last_click_time = lv_tick_get();
    hal_buzzer_beep();

    if (bk_ui->settingmodedegree == NULL || !lv_obj_is_valid(bk_ui->settingmodedegree))
        init_page_settingmodedegree(bk_ui);
    lv_scr_load(bk_ui->settingmodedegree);
}

void settingmode_setting_recordbt_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    if (lv_tick_elaps(last_click_time) < 250) return;
    last_click_time = lv_tick_get();
    hal_buzzer_beep();

    if (bk_ui->settingmoderecord == NULL || !lv_obj_is_valid(bk_ui->settingmoderecord))
        init_page_settingmoderecord(bk_ui);
    lv_scr_load(bk_ui->settingmoderecord);
}

void settingmode_setting_testbt_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    device_state_t *state = &g_device_state;
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    if (lv_tick_elaps(last_click_time) < 250) return;
    last_click_time = lv_tick_get();
    hal_buzzer_beep();

    state->test_mode = true;
    {
        uint32_t _pt = lv_tick_get();
        bk_printf(TAG "[SCREEN] ── settingmode→settingmodetest(고장진단) ──\n");
        if (bk_ui->settingmodetest == NULL || !lv_obj_is_valid(bk_ui->settingmodetest)) {
            init_page_settingmodetest(bk_ui);
            bk_printf(TAG "[SCREEN] init_page_settingmodetest: %lu ms\n", (unsigned long)lv_tick_elaps(_pt));
        } else {
            bk_printf(TAG "[SCREEN] settingmodetest cached (init skipped)\n");
        }
        _pt = lv_tick_get();
        lv_scr_load(bk_ui->settingmodetest);
        bk_printf(TAG "[SCREEN] lv_scr_load             : %lu ms\n", (unsigned long)lv_tick_elaps(_pt));
        _pt = lv_tick_get();
        lv_refr_now(NULL);
        bk_printf(TAG "[SCREEN] lv_refr_now(render)     : %lu ms\n", (unsigned long)lv_tick_elaps(_pt));
    }
}

void settingmode_setting_timebt_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    if (lv_tick_elaps(last_click_time) < 250) return;
    last_click_time = lv_tick_get();
    hal_buzzer_beep();

    if (bk_ui->settingmodetime == NULL || !lv_obj_is_valid(bk_ui->settingmodetime))
        init_page_settingmodetime(bk_ui);
    lv_scr_load(bk_ui->settingmodetime);
}

void settingmode_setting_languagebt_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    if (lv_tick_elaps(last_click_time) < 250) return;
    last_click_time = lv_tick_get();
    hal_buzzer_beep();

    if (bk_ui->settingmodelanguage == NULL || !lv_obj_is_valid(bk_ui->settingmodelanguage))
        init_page_settingmodelanguage(bk_ui);
    lv_scr_load(bk_ui->settingmodelanguage);
}

void settingmode_backbt_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    if (lv_tick_elaps(last_click_time) < 250) return;
    last_click_time = lv_tick_get();
    hal_buzzer_beep();

    init_page_main(bk_ui);
    lv_scr_load(bk_ui->main);
}

void settingmode_load_start_event_cb(lv_event_t *e)
{
    (void)e;

    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;

    ui_lang_apply_settingmode(bk_ui);
}

void settingmode_loaded_event_cb(lv_event_t *e)
{
    (void)e;

    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;

    ui_title_anim(bk_ui->settingmode_title);

    /* password_popup.png를 canvas 영구 버퍼에 1회 decode
     * (lv_image_cache_drop과 무관 — 팝업 열릴 때 즉시 렌더) */
    popuppassword_bg_preload();

#if UI_POPUPPASSWORD_KEYPAD_COMBINED_ENABLE
    /* 키패드 12장을 스프라이트 canvas에 1회 decode */
    popuppassword_keypad_preload();
#endif

    /* testmode_box.jpg를 전용 canvas에 1회 decode
     * — 고장진단 진입 시 즉시 렌더 */
    settingmodetest_bg_preload();
}

void settingmode_unload_start_event_cb(lv_event_t *e)
{
    (void)e;

    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;

    /* Stop all prewarming — dummies on lv_layer_top() must not change
     * images while another screen is active (dirty-region corruption).
     * Also reset indices so next visit re-prewarms (LRU may have evicted). */
    if (s_tm_bg_timer) {
        lv_timer_delete(s_tm_bg_timer);
        s_tm_bg_timer = NULL;
    }

    if (s_tm_bg_dummy) {
        lv_obj_del(s_tm_bg_dummy);
        s_tm_bg_dummy = NULL;
    }

    s_tm_bg_idx = 0;
    _pp_pw_cancel();

    /* 팝업이 열린 채 뒤로가기 시 imageview1->src가 canvas->draw_buf를 참조중.
     * canvas_free() 전에 팝업을 먼저 닫아야 dangling pointer 크래시 방지.
     *
     * UI_POPUPPASSWORD_COMBINED_BG_ENABLE=1일 때 팝업은 독립 화면이라
     * init_page_popuppassword()의 lv_scr_load()가 지금 이 SCREEN_UNLOAD_START를
     * 동기적으로 유발한 것일 수 있다(settingmode를 "완전히 떠난" 게 아니라
     * "팝업을 여는 중"). popuppassword_is_screen_opening()으로 이 경우를
     * 걸러내지 않으면, 방금 만든 팝업을 그 자리에서 바로 파괴해버리게 된다. */
#if UI_POPUPPASSWORD_COMBINED_BG_ENABLE
    if (!popuppassword_is_screen_opening()) {
#endif

    if (bk_ui->popuppassword && lv_obj_is_valid(bk_ui->popuppassword)) {
        destroy_page_popuppassword(bk_ui);
    }

#if !UI_POPUPPASSWORD_CANVAS_PERSIST_ENABLE
    /* 602 KB 반납 — 재진입 시 SCREEN_LOADED에서 재할당 */
    popuppassword_canvas_free();
#endif

#if UI_POPUPPASSWORD_COMBINED_BG_ENABLE
    }
#endif

#if UI_POPUPPASSWORD_KEYPAD_COMBINED_ENABLE
    /* 키패드 스프라이트 canvas 반납 */
    popuppassword_keypad_canvas_free();
#endif
}

void settingmode_unloaded_event_cb(lv_event_t *e)
{
    (void)e;
}