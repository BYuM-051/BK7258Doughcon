#include "lvgl.h"
#include "beken_ui.h"
#include "custom_func.h"
#include "event_runtime.h"
#include <stdio.h>
#include <string.h>

#include "beken_ui.h"
#include "device_state.h"
#include "hardware_hal.h"
#include "ui_lang.h"

extern bk_lv_ui_t bk_lv_tool_ui;
extern void destroy_page_main(bk_lv_ui_t *bk_ui);
extern void destroy_page_manualmode(bk_lv_ui_t *bk_ui);
extern int settings_get_int(const char *key);
extern void automode_mm_prewarm_start(void);
extern void automode_mm_prewarm_cancel(void);

static uint32_t last_click_time = 0;

/* ── 공유 이미지 캐시(LV_CACHE_DEF_SIZE) 주기적 강제 정리 ─────────────────
 * lodepng/JPEG 디코더가 내부적으로 realloc하며 커지는 임시 버퍼는 우리가 소유한
 * 고정 크기 캔버스가 아니라서 미리 선점해둘 수 없다 — 대신 장시간 사용 시
 * PSRAM 단편화가 누적되면서 이 임시 realloc 자체가 실패해 하드크래시로 이어짐
 * (teraterm 로그, autodrymode 진입 시 lodepng ensureBits32→lv_realloc 크래시로 확인).
 * main 화면(허브)으로 돌아올 때마다, 일정 간격(3분)마다 한 번씩 공유 캐시를
 * 통째로 비워(lv_image_cache_drop(NULL)) heap_4가 인접한 빈 블록들을 병합할
 * 기회를 자주 만들어준다 — 매번 비우면 화면 prewarm 효과가 사라지므로 간격을 둠. */
#define _CACHE_DROP_INTERVAL_MS  (3 * 60 * 1000)
static uint32_t s_last_cache_drop_tick = 0;
static bool     s_cache_drop_done_once = false;

static void _periodic_cache_drop_if_due(void)
{
    if (!s_cache_drop_done_once || lv_tick_elaps(s_last_cache_drop_tick) >= _CACHE_DROP_INTERVAL_MS) {
        lv_image_cache_drop(NULL);
        s_last_cache_drop_tick  = lv_tick_get();
        s_cache_drop_done_once  = true;
        printf("[MEM] shared image cache dropped (periodic defrag)\n");
    }
}

/* Settingmode image prewarm — loads image(s) one per tick while user is on main screen.
 * UI_SETTINGMODE_COMBINED_BG_ENABLE=1: settingmode is now a single combined
 * JPEG (feature-setting.jpg) — the 8 individual PNGs below no longer exist on
 * disk (moved to combinednimus/), so prewarm just that one file instead. */
#if UI_SETTINGMODE_COMBINED_BG_ENABLE
static const char * const s_sm_bases[] = {
    "/images/feature-setting",
};
#define _SM_IMG_EXT ".jpg"
#define _SM_IMG_COUNT 1
#else
static const char * const s_sm_bases[] = {
    "/images/setting_title",
    "/images/setting_mode_detailsetting",
    "/images/setting_mode_degree",
    "/images/setting_mode_record",
    "/images/setting_mode_test",
    "/images/setting_mode_time",
    "/images/setting_mode_language",
    "/images/exit_bt",
};
#define _SM_IMG_EXT ".png"
#define _SM_IMG_COUNT 8
#endif

static lv_timer_t *s_sm_prewarm_timer = NULL;
static lv_obj_t   *s_sm_prewarm_dummy = NULL;
static int         s_sm_prewarm_idx   = 0;

static void _sm_prewarm_tick(lv_timer_t *t)
{
    if (s_sm_prewarm_idx >= _SM_IMG_COUNT || !s_sm_prewarm_dummy) {
        lv_timer_delete(t); s_sm_prewarm_timer = NULL;
        if (s_sm_prewarm_dummy) { lv_obj_del(s_sm_prewarm_dummy); s_sm_prewarm_dummy = NULL; }
        printf("[PERF] settingmode prewarm done (%d imgs)\n", _SM_IMG_COUNT);
        return;
    }
    int lang = settings_get_int("LANGUAGE");
    const char *lsuf = (lang == 1) ? "_china" : (lang == 2) ? "_english" : "";
    char path[128];
    snprintf(path, sizeof(path), "%s%s%s", s_sm_bases[s_sm_prewarm_idx], lsuf, _SM_IMG_EXT);
    _img_set_src_timed(s_sm_prewarm_dummy, path);
    s_sm_prewarm_idx++;
}

static void _sm_prewarm_start(void)
{
#if !UI_PREWARM_ENABLE
    return;
#endif
    if (s_sm_prewarm_timer) return;   /* already running or done */
    if (s_sm_prewarm_idx >= _SM_IMG_COUNT) return;  /* already done */
    s_sm_prewarm_dummy = lv_image_create(lv_layer_top());
    lv_obj_add_flag(s_sm_prewarm_dummy, LV_OBJ_FLAG_HIDDEN);
    s_sm_prewarm_timer = lv_timer_create(_sm_prewarm_tick, 10, NULL);
}

void main_settingmode_prewarm_reset(void)
{
    if (s_sm_prewarm_timer) { lv_timer_delete(s_sm_prewarm_timer); s_sm_prewarm_timer = NULL; }
    if (s_sm_prewarm_dummy) { lv_obj_del(s_sm_prewarm_dummy); s_sm_prewarm_dummy = NULL; }
    s_sm_prewarm_idx = 0;
}

/* Automode(자동운전 설정 화면) image prewarm — loads image(s) one per tick
 * while user is on main screen, same pattern as settingmode prewarm above.
 * 목록은 ui_lang.c의 ui_lang_apply_automode()와 동일(언어 접미사 포함). */
static const char * const s_am_bases[] = {
    "/images/automode_title",
    "/images/exit_bt",
    "/images/start_bt",
    "/images/auto_mode_start_box_time",
    "/images/load_bt",
    "/images/save_bt",
    "/images/auto_mode_freeze_board",
    "/images/auto_mode_defrost_board",
    "/images/auto_mode_fermentation1_board",
    "/images/auto_mode_fermentation2_board",
    "/images/defrost_auto_time_box",
    "/images/keypad",
};
#define _AM_IMG_COUNT (int)(sizeof(s_am_bases) / sizeof(s_am_bases[0]))

static lv_timer_t *s_am_prewarm_timer = NULL;
static lv_obj_t   *s_am_prewarm_dummy = NULL;
static int         s_am_prewarm_idx   = 0;

static void _am_prewarm_tick(lv_timer_t *t)
{
    if (s_am_prewarm_idx >= _AM_IMG_COUNT || !s_am_prewarm_dummy) {
        lv_timer_delete(t); s_am_prewarm_timer = NULL;
        if (s_am_prewarm_dummy) { lv_obj_del(s_am_prewarm_dummy); s_am_prewarm_dummy = NULL; }
        printf("[PERF] automode prewarm done (%d imgs)\n", _AM_IMG_COUNT);
        return;
    }
    int lang = settings_get_int("LANGUAGE");
    const char *lsuf = (lang == 1) ? "_china" : (lang == 2) ? "_english" : "";
    char path[128];
    snprintf(path, sizeof(path), "%s%s.png", s_am_bases[s_am_prewarm_idx], lsuf);
    _img_set_src_timed(s_am_prewarm_dummy, path);
    s_am_prewarm_idx++;
}

static void _am_prewarm_start(void)
{
#if !UI_PREWARM_ENABLE
    return;
#endif
    if (s_am_prewarm_timer) return;   /* already running or done */
    if (s_am_prewarm_idx >= _AM_IMG_COUNT) return;  /* already done */
    s_am_prewarm_dummy = lv_image_create(lv_layer_top());
    lv_obj_add_flag(s_am_prewarm_dummy, LV_OBJ_FLAG_HIDDEN);
    s_am_prewarm_timer = lv_timer_create(_am_prewarm_tick, 10, NULL);
}

void main_automode_prewarm_reset(void)
{
    if (s_am_prewarm_timer) { lv_timer_delete(s_am_prewarm_timer); s_am_prewarm_timer = NULL; }
    if (s_am_prewarm_dummy) { lv_obj_del(s_am_prewarm_dummy); s_am_prewarm_dummy = NULL; }
    s_am_prewarm_idx = 0;
}

void main_automode_event_cb(lv_event_t *e);
void main_manualmode_event_cb(lv_event_t *e);
void main_autodrymode_event_cb(lv_event_t *e);
void main_memorymode_event_cb(lv_event_t *e);
void main_settingmode_event_cb(lv_event_t *e);
void main_load_event_cb(lv_event_t *e);

void main_automode_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    device_state_t *state = &g_device_state;
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    if (state->lock || state->hard_lock) return;
    if (lv_tick_elaps(last_click_time) < 250) return;
    last_click_time = lv_tick_get();
    hal_buzzer_beep();

    uint32_t _t0 = lv_tick_get();
    printf("[SCREEN] ── main→automode ──────────────────\n");
    if (bk_ui->automode == NULL || !lv_obj_is_valid(bk_ui->automode))
        init_page_automode(bk_ui);
    printf("[SCREEN] init_page_automode: %lu ms\n", lv_tick_elaps(_t0));
    _t0 = lv_tick_get();
    lv_scr_load(bk_ui->automode);
    printf("[SCREEN] lv_scr_load       : %lu ms\n", lv_tick_elaps(_t0));
    /* lv_scr_load는 화면 포인터만 바꾸고 실제 렌더링/플러시는 다음 lv_timer_handler
     * 틱에서 일어남 — 여기서 강제로 즉시 그려서 "실제 화면이 보이는 시점"을 측정. */
    _t0 = lv_tick_get();
    lv_refr_now(NULL);
    printf("[SCREEN] lv_refr_now(render): %lu ms\n", lv_tick_elaps(_t0));
    _t0 = lv_tick_get();
    destroy_page_main(bk_ui);
    printf("[SCREEN] destroy_page_main : %lu ms\n", lv_tick_elaps(_t0));
}

void main_manualmode_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    device_state_t *state = &g_device_state;
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    if (state->lock || state->hard_lock) return;
    if (lv_tick_elaps(last_click_time) < 250) return;
    last_click_time = lv_tick_get();
    hal_buzzer_beep();

    uint32_t _t0 = lv_tick_get();
    printf("[SCREEN] ── main→manualmode ─────────────────\n");
    if (bk_ui->manualmode == NULL || !lv_obj_is_valid(bk_ui->manualmode))
        init_page_manualmode(bk_ui);
    printf("[SCREEN] init_page_manualmode: %lu ms\n", lv_tick_elaps(_t0));
    _t0 = lv_tick_get();
    lv_scr_load(bk_ui->manualmode);
    printf("[SCREEN] lv_scr_load          : %lu ms\n", lv_tick_elaps(_t0));
    _t0 = lv_tick_get();
    lv_refr_now(NULL);
    printf("[SCREEN] lv_refr_now(render)  : %lu ms\n", lv_tick_elaps(_t0));
    _t0 = lv_tick_get();
    destroy_page_main(bk_ui);
    printf("[SCREEN] destroy_page_main    : %lu ms\n", lv_tick_elaps(_t0));
}

void main_autodrymode_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    device_state_t *state = &g_device_state;
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    if (state->lock || state->hard_lock) return;
    if (lv_tick_elaps(last_click_time) < 250) return;
    last_click_time = lv_tick_get();
    hal_buzzer_beep();

    uint32_t _t0 = lv_tick_get();
    printf("[SCREEN] ── main→autodrymode ───────────────\n");
    init_page_autodrymode(bk_ui);
    printf("[SCREEN] init_page_autodrymode: %lu ms\n", lv_tick_elaps(_t0));
    _t0 = lv_tick_get();
    lv_scr_load(bk_ui->autodrymode);
    printf("[SCREEN] lv_scr_load          : %lu ms\n", lv_tick_elaps(_t0));
    _t0 = lv_tick_get();
    lv_refr_now(NULL);
    printf("[SCREEN] lv_refr_now(render)  : %lu ms\n", lv_tick_elaps(_t0));
    _t0 = lv_tick_get();
    destroy_page_main(bk_ui);
    printf("[SCREEN] destroy_page_main    : %lu ms\n", lv_tick_elaps(_t0));
}

void main_memorymode_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    device_state_t *state = &g_device_state;
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    if (state->lock || state->hard_lock) return;
    if (lv_tick_elaps(last_click_time) < 250) return;
    last_click_time = lv_tick_get();
    hal_buzzer_beep();

    /* 메뉴에서 직접 진입 — 자동설정 불러오기/저장에서 남은 값이 있으면
     * 삭제 버튼 표시 로직(memorymode_load_event_cb)이 잘못 판단하므로 초기화 */
    state->memory_mode_check = MEMORY_MODE_NONE;
    uint32_t _t0 = lv_tick_get();
    printf("[SCREEN] ── main→memorymode ────────────────\n");
    if (bk_ui->memorymode == NULL || !lv_obj_is_valid(bk_ui->memorymode))
        init_page_memorymode(bk_ui);
    printf("[SCREEN] init_page_memorymode : %lu ms\n", lv_tick_elaps(_t0));
    _t0 = lv_tick_get();
    lv_scr_load(bk_ui->memorymode);
    printf("[SCREEN] lv_scr_load          : %lu ms\n", lv_tick_elaps(_t0));
    _t0 = lv_tick_get();
    lv_refr_now(NULL);
    printf("[SCREEN] lv_refr_now(render)  : %lu ms\n", lv_tick_elaps(_t0));
    _t0 = lv_tick_get();
    destroy_page_main(bk_ui);
    printf("[SCREEN] destroy_page_main    : %lu ms\n", lv_tick_elaps(_t0));
}

void main_settingmode_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    device_state_t *state = &g_device_state;
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    if (state->lock || state->hard_lock) return;
    if (lv_tick_elaps(last_click_time) < 250) return;
    last_click_time = lv_tick_get();
    hal_buzzer_beep();

    uint32_t _t0 = lv_tick_get();
    printf("[SCREEN] ── main→settingmode ───────────────\n");
    if (bk_ui->settingmode == NULL || !lv_obj_is_valid(bk_ui->settingmode))
        init_page_settingmode(bk_ui);
    printf("[SCREEN] init_page_settingmode: %lu ms\n", lv_tick_elaps(_t0));
    _t0 = lv_tick_get();
    lv_scr_load(bk_ui->settingmode);
    printf("[SCREEN] lv_scr_load          : %lu ms\n", lv_tick_elaps(_t0));
    _t0 = lv_tick_get();
    lv_refr_now(NULL);
    printf("[SCREEN] lv_refr_now(render)  : %lu ms\n", lv_tick_elaps(_t0));
    _t0 = lv_tick_get();
    destroy_page_main(bk_ui);
    printf("[SCREEN] destroy_page_main    : %lu ms\n", lv_tick_elaps(_t0));
}

void main_load_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    device_state_t *state = &g_device_state;

    if (code == LV_EVENT_SCREEN_UNLOAD_START) {
        if (s_sm_prewarm_timer) { lv_timer_delete(s_sm_prewarm_timer); s_sm_prewarm_timer = NULL; }
        if (s_sm_prewarm_dummy) { lv_obj_del(s_sm_prewarm_dummy);      s_sm_prewarm_dummy = NULL; }
        s_sm_prewarm_idx = 0;
        automode_mm_prewarm_cancel();
        main_automode_prewarm_reset();
        return;
    }
    if (code == LV_EVENT_SCREEN_LOAD_START) {
        state->auto_mode = false;
        ui_lang_apply_main(bk_ui);
        return;
    }
    if (code == LV_EVENT_SCREEN_LOADED) {
        _periodic_cache_drop_if_due();
        _sm_prewarm_start();
        automode_mm_prewarm_start();
        _am_prewarm_start();
        return;
    }
}
