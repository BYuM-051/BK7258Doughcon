#include "lvgl.h"
#include "beken_ui.h"
#include "custom_func.h"
#include "event_runtime.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <os/os.h>

#include "beken_ui.h"
#include "settings.h"
#include "device_state.h"
#include "ui_animations.h"
#include "ui_lang.h"
#include "hardware_hal.h"
#include "uart_comm.h"
#include "preRenderer.h"

#define TAG "[automodestart_cb.c] "
#define bk_printf(fmt, ...) do {if(0) printf(fmt, ##__VA_ARGS__); } while(0) // disable printf


extern bk_lv_ui_t bk_lv_tool_ui;
extern void automodeend_save_record_and_buzzer(device_state_t *state);

static uint32_t last_click_time = 0;
static lv_timer_t *s_ui_timer = NULL;
static bool s_over_ferm_triggered = false;  /* 과발효방지 1회 트리거 방지 */

/* GIF 애니메이션 상태 */
static int       s_ams_current_mode   = -1;

static bool        s_anim_enabled   = true;

/* 수동 lv_timer 기반 애니메이션 — 30fps (33ms/tick) */
static lv_timer_t *s_anim_timer_ams = NULL;
static int32_t     s_anim_pos       = 0;


static lv_obj_t   *s_anim_toggle_btn   = NULL;
static lv_obj_t   *s_anim_toggle_label = NULL;

/* FERM2 완료 감지 statics — 파일 범위로 승격 (SCREEN_LOADED 에서 명시적 리셋 필요) */
static bool     s_ferm2_nonzero    = false;
static bool     s_end_triggered    = false;
static uint32_t s_ferm2_start_tick = 0;
static bool     s_ferm2_tick_valid = false;

/* 모드 전환 감지용 */
static int      s_arc_seen_mode    = -1;

/* _refresh_running_ui 중복 dirty 마킹 방지 캐시 */
static int      s_last_show_hum  = -1;
static int      s_last_ui_mode   = -1;
static int      s_last_arc_mode  = -1;
static uint32_t s_last_arc_tick  = 0;
static int      s_last_pct[4]    = {-1, -1, -1, -1};
static int      s_last_temp_val  = 0x7FFFFFFF;
static int      s_last_hum_val   = 0x7FFFFFFF;
static int      s_last_remain_h  = -1;
static int      s_last_remain_m  = -1;
/* AP fallback 단계 전환용 */
static uint32_t s_mode_start_tick  = 0;
/* 이 단계 진입 이후 MCU STATUS를 아직 한 번도 못 받았을 때(uart 통신 전) 쓸
 * rx_seq 기준값 — g_uart_rx_seq가 이 값과 같으면 saveoperation[10/11]이
 * 아직 이전 단계/초기값(0)이라는 뜻이므로 MCU raw 대신 자체 계산값을 표시한다. */
static uint32_t s_rx_seq_at_mode_start = 0;

/* ── bg JPEG 영구 canvas — 최초 1회 decode 후 raw RGB565 재사용 ─
 * 버퍼(s_ams_canvas_buf)는 main_activity_on_create()에서 부팅 극초반(단편화 전)
 * 1회만 malloc되고 이후 절대 free되지 않는다 — 이전엔 화면 나갈 때마다
 * free하고 들어올 때마다 재malloc해서, psram_malloc_cm이 실패 시 NULL 대신
 * 즉시 assert 크래시하는 이 프로젝트 특성상 단편화가 쌓이면 자동운전 시작
 * 화면 진입 시 하드크래시로 이어졌다(reset_popup과 동일한 근본 원인). */
static void     *s_ams_canvas_buf = NULL;
static lv_obj_t *s_ams_bg_canvas  = NULL;
static int       s_ams_buf_lang   = -1;  /* lang*10 + is_f */

void automodestart_canvas_buf_alloc(void)
{
    if (s_ams_canvas_buf) return;
    uint32_t buf_sz = LV_CANVAS_BUF_SIZE(1024, 540, 16, LV_DRAW_BUF_ALIGN);
    s_ams_canvas_buf = lv_malloc(buf_sz);
    if (s_ams_canvas_buf)
        bk_printf(TAG "[AMS] bg buf alloc ok (%lu B)\n", (unsigned long)buf_sz);
    else
        bk_printf(TAG "[AMS] bg buf alloc FAILED\n");
}

/* ── 서클 PNG 사전 캐싱 canvas (PSRAM 힙 동적 할당 360KB — 모드 전환 시 재렌더) ─ */
#define CIRCLE_CANVAS_W  300
#define CIRCLE_CANVAS_H  300
static uint8_t  *s_cc_buf    = NULL;  /* malloc() → PSRAM 힙 */
static lv_obj_t *s_cc_canvas = NULL;

static uint8_t  *s_arc_buf    = NULL;  /* 진도 아크 canvas 버퍼 (360KB, PSRAM 영구) */
static lv_obj_t *s_arc_canvas = NULL;
static int       s_cc_mode   = -1;   /* canvas에 현재 렌더된 모드 */

#if AUTO_MODE_TEST
/* TEST_MODE: MCU remain 무시, lv_tick 기반 진도 및 자동 모드 전환 */
static uint32_t  s_test_mode_tick     = 0;   /* 현재 단계 시작 tick */
static int       s_test_op_mode       = OP_MODE_FREEZE;
#endif

/* 해동 낙하 클립 */
static lv_obj_t *s_ams_defrost_clip   = NULL;
static lv_obj_t *s_ams_defrost_img    = NULL;

/* 발효1 중첩 클립 */
static lv_obj_t *s_ams_ferm1_outer    = NULL;
static lv_obj_t *s_ams_ferm1_inner    = NULL;
static lv_obj_t *s_ams_ferm1_top      = NULL;
static lv_obj_t *s_ams_ferm1_btm      = NULL;
static lv_obj_t *s_ams_ferm1_btm_img  = NULL;

/* 발효2 중첩 클립 */
static lv_obj_t *s_ams_ferm2_outer    = NULL;
static lv_obj_t *s_ams_ferm2_inner    = NULL;
static lv_obj_t *s_ams_ferm2_top      = NULL;
static lv_obj_t *s_ams_ferm2_btm      = NULL;
static lv_obj_t *s_ams_ferm2_btm_img  = NULL;

void automodestart_startbt_event_cb(lv_event_t *e);
void automodestart_load_start_event_cb(lv_event_t *e);
void automodestart_loaded_event_cb(lv_event_t *e);
void automodestart_unload_start_event_cb(lv_event_t *e);
void automodestart_unloaded_event_cb(lv_event_t *e);

/* ── GIF 애니메이션 헬퍼 (수동 lv_timer, 33ms/30fps) */

#define AMS_ANIM_PERIOD_MS 33

static void _anim_tick_ams(lv_timer_t *t)
{
    bk_lv_ui_t *bk_ui = (bk_lv_ui_t *)lv_timer_get_user_data(t);
    switch (s_ams_current_mode) {
        case OP_MODE_FREEZE:
            /* 3600 / 2500ms × 33ms ≈ 48 units/tick → ~2.5s/회전 */
            s_anim_pos = (s_anim_pos + 48) % 3600;
            lv_image_set_rotation(bk_ui->automodestart_freeze_gif, (uint16_t)s_anim_pos);
            break;
        case OP_MODE_DEFROST:
            if (!s_ams_defrost_img) break;
            /* -45→45 / 33ms×1px/tick → ~2.7s/사이클 */
            s_anim_pos += 1;
            if (s_anim_pos > 45) s_anim_pos = -45;
            lv_obj_set_y(s_ams_defrost_img, (lv_coord_t)s_anim_pos);
            break;
        case OP_MODE_FERM1:
            if (!s_ams_ferm1_inner) break;
            /* 20→-20 / 33ms×1px/tick → ~1.3s/사이클 */
            s_anim_pos -= 1;
            if (s_anim_pos < -20) s_anim_pos = 20;
            lv_obj_set_y(s_ams_ferm1_inner, (lv_coord_t)s_anim_pos);
            break;
        case OP_MODE_FERM2:
            if (!s_ams_ferm2_inner) break;
            s_anim_pos -= 1;
            if (s_anim_pos < -20) s_anim_pos = 20;
            lv_obj_set_y(s_ams_ferm2_inner, (lv_coord_t)s_anim_pos);
            break;
        default: break;
    }
}

/* ── GIF 애니메이션 중지 (모든 모드 정리 + gif_basic 복원) ─────────── */
static void _gif_anim_stop_ams(bk_lv_ui_t *bk_ui)
{
    if (s_anim_timer_ams) { lv_timer_delete(s_anim_timer_ams); s_anim_timer_ams = NULL; }

    lv_image_set_rotation(bk_ui->automodestart_freeze_gif, 0);
    lv_obj_set_style_opa(bk_ui->automodestart_freeze_gif, LV_OPA_COVER, 0);
    lv_obj_add_flag(bk_ui->automodestart_freeze_gif, LV_OBJ_FLAG_HIDDEN);

    if (s_ams_defrost_clip && lv_obj_is_valid(s_ams_defrost_clip)) {
        if (s_ams_defrost_img) lv_obj_set_y(s_ams_defrost_img, 0);
        lv_obj_add_flag(s_ams_defrost_clip, LV_OBJ_FLAG_HIDDEN);
    }

    if (s_ams_ferm1_outer && lv_obj_is_valid(s_ams_ferm1_outer)) {
        if (s_ams_ferm1_inner) lv_obj_set_y(s_ams_ferm1_inner, 0);
        lv_obj_add_flag(s_ams_ferm1_outer, LV_OBJ_FLAG_HIDDEN);
    }
    if (s_ams_ferm1_btm && lv_obj_is_valid(s_ams_ferm1_btm))
        lv_obj_add_flag(s_ams_ferm1_btm, LV_OBJ_FLAG_HIDDEN);

    if (s_ams_ferm2_outer && lv_obj_is_valid(s_ams_ferm2_outer)) {
        if (s_ams_ferm2_inner) lv_obj_set_y(s_ams_ferm2_inner, 0);
        lv_obj_add_flag(s_ams_ferm2_outer, LV_OBJ_FLAG_HIDDEN);
    }
    if (s_ams_ferm2_btm && lv_obj_is_valid(s_ams_ferm2_btm))
        lv_obj_add_flag(s_ams_ferm2_btm, LV_OBJ_FLAG_HIDDEN);

    _img_ensure_src(bk_ui->automodestart_freeze_gif_basic);
    lv_obj_clear_flag(bk_ui->automodestart_freeze_gif_basic,        LV_OBJ_FLAG_HIDDEN);
    _img_ensure_src(bk_ui->automodestart_defrost_gif_basic);
    lv_obj_clear_flag(bk_ui->automodestart_defrost_gif_basic,       LV_OBJ_FLAG_HIDDEN);
    _img_ensure_src(bk_ui->automodestart_fermentation1_gif_basic);
    lv_obj_clear_flag(bk_ui->automodestart_fermentation1_gif_basic, LV_OBJ_FLAG_HIDDEN);
    _img_ensure_src(bk_ui->automodestart_fermentation2_gif_basic);
    lv_obj_clear_flag(bk_ui->automodestart_fermentation2_gif_basic, LV_OBJ_FLAG_HIDDEN);

    s_ams_current_mode = -1;
}



/* ── GIF 애니메이션 시작 — 수동 lv_timer (100ms/10fps)
 * 속도: freeze=144units/tick(2.5s/회전), defrost=3px/tick(3.7s/낙하), ferm=1px/tick(4.8s/부상) */
static void _gif_anim_start_ams(bk_lv_ui_t *bk_ui, int op_mode)
{
    if (s_anim_timer_ams) { lv_timer_delete(s_anim_timer_ams); s_anim_timer_ams = NULL; }

    switch (op_mode) {
        case OP_MODE_FREEZE: {
            lv_obj_t *gif = bk_ui->automodestart_freeze_gif;
            _img_ensure_src(gif);
            lv_obj_clear_flag(gif, LV_OBJ_FLAG_HIDDEN);
            lv_image_set_rotation(gif, 0);
            lv_obj_set_style_opa(gif, LV_OPA_COVER, 0);
            lv_obj_add_flag(bk_ui->automodestart_freeze_gif_basic, LV_OBJ_FLAG_HIDDEN);
            s_anim_pos = 0;
            break;
        }
        case OP_MODE_DEFROST: {
            if (!s_ams_defrost_clip || !lv_obj_is_valid(s_ams_defrost_clip)) break;
            _img_ensure_src(s_ams_defrost_clip);
            lv_obj_clear_flag(s_ams_defrost_clip, LV_OBJ_FLAG_HIDDEN);
            lv_obj_set_y(s_ams_defrost_img, -55);
            lv_obj_add_flag(bk_ui->automodestart_defrost_gif_basic, LV_OBJ_FLAG_HIDDEN);
            s_anim_pos = -55;
            break;
        }
        case OP_MODE_FERM1: {
            if (!s_ams_ferm1_outer || !lv_obj_is_valid(s_ams_ferm1_outer)) break;
            if (s_ams_ferm1_top)     _img_ensure_src(s_ams_ferm1_top);
            if (s_ams_ferm1_btm_img) _img_ensure_src(s_ams_ferm1_btm_img);
            lv_obj_clear_flag(s_ams_ferm1_outer, LV_OBJ_FLAG_HIDDEN);
            if (s_ams_ferm1_btm && lv_obj_is_valid(s_ams_ferm1_btm))
                lv_obj_clear_flag(s_ams_ferm1_btm, LV_OBJ_FLAG_HIDDEN);
            lv_obj_set_y(s_ams_ferm1_inner, 24);
            lv_obj_add_flag(bk_ui->automodestart_fermentation1_gif_basic, LV_OBJ_FLAG_HIDDEN);
            s_anim_pos = 24;
            break;
        }
        case OP_MODE_FERM2: {
            if (!s_ams_ferm2_outer || !lv_obj_is_valid(s_ams_ferm2_outer)) break;
            if (s_ams_ferm2_top)     _img_ensure_src(s_ams_ferm2_top);
            if (s_ams_ferm2_btm_img) _img_ensure_src(s_ams_ferm2_btm_img);
            lv_obj_clear_flag(s_ams_ferm2_outer, LV_OBJ_FLAG_HIDDEN);
            if (s_ams_ferm2_btm && lv_obj_is_valid(s_ams_ferm2_btm))
                lv_obj_clear_flag(s_ams_ferm2_btm, LV_OBJ_FLAG_HIDDEN);
            lv_obj_set_y(s_ams_ferm2_inner, 24);
            lv_obj_add_flag(bk_ui->automodestart_fermentation2_gif_basic, LV_OBJ_FLAG_HIDDEN);
            s_anim_pos = 24;
            break;
        }
        default: break;
    }

    s_ams_current_mode = op_mode;

    if (s_anim_enabled)
        s_anim_timer_ams = lv_timer_create(_anim_tick_ams, AMS_ANIM_PERIOD_MS, bk_ui);
}


/* ── 진도 아크 업데이트 ──────────────────────────────────────────────
 * 경과시간 / 설정시간 → canvas 전체 재렌더 (lv_draw_arc 직접 호출)
 * ────────────────────────────────────────────────────────────────── */
/* 각 모드별 전체 설정시간 (분)
 * 정전 복구 중: send_*_hour/min은 정전 시 잔여분이므로 bo_*_total_min(원래 전체 시간)을 우선 사용.
 * 이렇게 해야 arc가 "처음부터"가 아닌 "정전 전 경과분"부터 이어서 그려진다. */
static int _total_min_for_mode(device_state_t *state, int op_mode)
{
    if (state->black_out_checking) {
        switch (op_mode) {
            case OP_MODE_FREEZE:  if (state->bo_freeze_total_min  > 0) return state->bo_freeze_total_min;  break;
            case OP_MODE_DEFROST: if (state->bo_defrost_total_min > 0) return state->bo_defrost_total_min; break;
            case OP_MODE_FERM1:   if (state->bo_ferm1_total_min   > 0) return state->bo_ferm1_total_min;   break;
            case OP_MODE_FERM2:   if (state->bo_ferm2_total_min   > 0) return state->bo_ferm2_total_min;   break;
            default:                                                                                         break;
        }
    }
    switch (op_mode) {
        case OP_MODE_FREEZE:  return state->send_freeze_hour  * 60 + state->send_freeze_min;
        case OP_MODE_DEFROST: return state->send_defreeze_hour * 60 + state->send_defreeze_min;
        case OP_MODE_FERM1:   return state->send_ferm1_hour   * 60 + state->send_ferm1_min;
        case OP_MODE_FERM2:   return state->send_ferm2_hour   * 60 + state->send_ferm2_min;
        default:              return 0;
    }
}

/* 진도 아크 업데이트 — canvas 전체 재렌더
 * - 완료 단계: 100% 컬러 호 + 배경 트랙
 * - 현재 단계: 경과% 컬러 호 + 배경 트랙
 * - 미진행 단계: 건너뜀 (투명) */
static void _update_arc_ams(bk_lv_ui_t *bk_ui)
{
    (void)bk_ui;
    if (!s_arc_canvas) return;
    device_state_t *state = &g_device_state;
#if AUTO_MODE_TEST
    int cur = s_test_op_mode;
#else
    int cur = (int)state->current_op_mode;
#endif

    static const struct { int mode, sa, ea; uint32_t rgb; } k[4] = {
        { OP_MODE_FREEZE,  180, 270, 0x283593 },
        { OP_MODE_DEFROST, 270, 360, 0x4AACE8 },
        { OP_MODE_FERM1,     0,  90, 0xE07820 },
        { OP_MODE_FERM2,    90, 180, 0xB71C1C },
    };

    lv_canvas_fill_bg(s_arc_canvas, lv_color_hex(0x000000), LV_OPA_TRANSP);
    lv_layer_t layer;
    lv_canvas_init_layer(s_arc_canvas, &layer);

    for (int i = 0; i < 4; i++) {
        int mode = k[i].mode;
        if (mode > cur) continue;

        lv_draw_arc_dsc_t bg;
        lv_draw_arc_dsc_init(&bg);
        bg.color       = lv_color_hex(0xD8D8D8);
        bg.width       = 25;
        bg.center.x    = 150;
        bg.center.y    = 150;
        bg.radius      = 142;
        bg.start_angle = k[i].sa;
        bg.end_angle   = k[i].ea;
        bg.rounded     = 0;
        bg.opa         = LV_OPA_COVER;
        // lv_draw_arc(&layer, &bg);

        int pct;
        if (mode < cur) {
            pct = 1000;
        } else {
#if AUTO_MODE_TEST
            int total_ms   = AUTO_MODE_TEST_MIN * 60 * 1000;
            int elapsed_ms = (int)lv_tick_elaps(s_test_mode_tick);
            if (elapsed_ms < 0) elapsed_ms = 0;
            pct = (total_ms > 0) ? (elapsed_ms * 1000 / total_ms) : 0;
            if (pct > 1000) pct = 1000;
#else
            static int      s_arc_mcu_ref_min  = -1;
            static uint32_t s_arc_mcu_ref_tick = 0;

            int total_min       = _total_min_for_mode(state, mode);
            int mcu_elapsed_min = (int)(uint8_t)state->saveoperation[12] * 60
                                + (int)(uint8_t)state->saveoperation[13];
            if (mcu_elapsed_min != s_arc_mcu_ref_min) {
                s_arc_mcu_ref_min  = mcu_elapsed_min;
                s_arc_mcu_ref_tick = lv_tick_get();
            }
            int interp_sec  = (int)(lv_tick_elaps(s_arc_mcu_ref_tick) / 1000u);
            int elapsed_sec = mcu_elapsed_min * 60 + interp_sec;
            int total_sec   = total_min * 60;
            if (elapsed_sec < 0) elapsed_sec = 0;
            if (total_sec > 0 && elapsed_sec > total_sec) elapsed_sec = total_sec;
            pct = (total_sec > 0) ? (elapsed_sec * 1000 / total_sec) : 0;
            if (pct > 1000) pct = 1000;
#endif
        }
        if (pct <= 0) continue;

        int ind_ea = k[i].sa + (int)((uint64_t)pct * (k[i].ea - k[i].sa) / 1000);
        if (ind_ea <= k[i].sa) continue;

        lv_draw_arc_dsc_t ind;
        lv_draw_arc_dsc_init(&ind);
        ind.color       = lv_color_hex(k[i].rgb);
        ind.width       = 15;
        ind.center.x    = 150;
        ind.center.y    = 150;
        ind.radius      = 137;
        ind.start_angle = k[i].sa;
        ind.end_angle   = ind_ea;
        ind.rounded     = 0;
        ind.opa         = LV_OPA_COVER;
        lv_draw_arc(&layer, &ind);
    }

    lv_canvas_finish_layer(s_arc_canvas, &layer);
}

/* ── 자동모드 완료 → automodeend 화면 전환 ───────────────────────── */
static void _go_to_automodeend(bk_lv_ui_t *bk_ui)
{
    device_state_t *state = &g_device_state;

    // s_over_ferm_triggered  = false;
    // state->over_ferm_active = false;

    if (s_ui_timer) {
        lv_timer_delete(s_ui_timer);
        s_ui_timer = NULL;
    }
    _gif_anim_stop_ams(bk_ui);

    state->operation            = true;


    state->auto_mode_start      = true;
    state->first_freeze         = false;
    state->first_defrost        = false;
    /* 정전 복구 플래그(black_out_checking, bo_*_total_min) 초기화는
     * automodeend_load_event_cb 의 운전기록 저장(_record_save_slot0) 완료 후 수행.
     * 여기서 미리 클리어하면 _record_save_slot0이 bo_*_total_min(원래 설정 시간)을
     * 참조하지 못해 정전복구 후 완료된 기록에 잔여분(0)이 저장되는 버그 발생. */

#if UI_PRENDERING_ENABLE
    ui_page_change(PAGE_AUTOMODEEND);
    lv_label_set_text(bk_ui->automodeend_AutoModeCompleteYear,  settings_get_str("CurrentCompleteYear"));
    lv_label_set_text(bk_ui->automodeend_AutoModeCompleteMonth, settings_get_str("CurrentCompleteMonth"));
    lv_label_set_text(bk_ui->automodeend_AutoModeCompleteDay,   settings_get_str("CurrentCompleteDay"));
    lv_label_set_text(bk_ui->automodeend_AutoModeCompleteHour,  settings_get_str("CurrentCompleteHour"));
    lv_label_set_text(bk_ui->automodeend_AutoModeCompleteMin,   settings_get_str("CurrentCompleteMin"));
#else
    init_page_automodeend(bk_ui);
    lv_label_set_text(bk_ui->automodeend_AutoModeCompleteYear,  settings_get_str("CurrentCompleteYear"));
    lv_label_set_text(bk_ui->automodeend_AutoModeCompleteMonth, settings_get_str("CurrentCompleteMonth"));
    lv_label_set_text(bk_ui->automodeend_AutoModeCompleteDay,   settings_get_str("CurrentCompleteDay"));
    lv_label_set_text(bk_ui->automodeend_AutoModeCompleteHour,  settings_get_str("CurrentCompleteHour"));
    lv_label_set_text(bk_ui->automodeend_AutoModeCompleteMin,   settings_get_str("CurrentCompleteMin"));
    lv_scr_load(bk_ui->automodeend);
    destroy_page_automodestart(bk_ui);
#endif /* UI_PRENDERING_ENABLE */

    settings_set_str("saveChecking", "0");
    settings_save_dirty();
}

/* ── 자동모드 완료 → automodeend 화면 전환 ───────────────────────── */
static void _go_to_automodeend1(bk_lv_ui_t *bk_ui)
{
    device_state_t *state = &g_device_state;

    // s_over_ferm_triggered  = false;
    // state->over_ferm_active = false;

    if (s_ui_timer) {
        lv_timer_delete(s_ui_timer);
        s_ui_timer = NULL;
    }
    _gif_anim_stop_ams(bk_ui);

    state->operation            = false;
    state->auto_mode_start      = false;
    state->first_freeze         = false;
    state->first_defrost        = false;
    /* 정전 복구 플래그(black_out_checking, bo_*_total_min) 초기화는
     * automodeend_load_event_cb 의 운전기록 저장(_record_save_slot0) 완료 후 수행.
     * 여기서 미리 클리어하면 _record_save_slot0이 bo_*_total_min(원래 설정 시간)을
     * 참조하지 못해 정전복구 후 완료된 기록에 잔여분(0)이 저장되는 버그 발생. */

#if UI_PRENDERING_ENABLE
    ui_page_change(PAGE_AUTOMODEEND);
    lv_label_set_text(bk_ui->automodeend_AutoModeCompleteYear,  settings_get_str("CurrentCompleteYear"));
    lv_label_set_text(bk_ui->automodeend_AutoModeCompleteMonth, settings_get_str("CurrentCompleteMonth"));
    lv_label_set_text(bk_ui->automodeend_AutoModeCompleteDay,   settings_get_str("CurrentCompleteDay"));
    lv_label_set_text(bk_ui->automodeend_AutoModeCompleteHour,  settings_get_str("CurrentCompleteHour"));
    lv_label_set_text(bk_ui->automodeend_AutoModeCompleteMin,   settings_get_str("CurrentCompleteMin"));
#else
    init_page_automodeend(bk_ui);
    lv_label_set_text(bk_ui->automodeend_AutoModeCompleteYear,  settings_get_str("CurrentCompleteYear"));
    lv_label_set_text(bk_ui->automodeend_AutoModeCompleteMonth, settings_get_str("CurrentCompleteMonth"));
    lv_label_set_text(bk_ui->automodeend_AutoModeCompleteDay,   settings_get_str("CurrentCompleteDay"));
    lv_label_set_text(bk_ui->automodeend_AutoModeCompleteHour,  settings_get_str("CurrentCompleteHour"));
    lv_label_set_text(bk_ui->automodeend_AutoModeCompleteMin,   settings_get_str("CurrentCompleteMin"));
    lv_scr_load(bk_ui->automodeend);
    destroy_page_automodestart(bk_ui);
#endif /* UI_PRENDERING_ENABLE */

    settings_set_str("saveChecking", "0");
    settings_save_dirty();
}

/* ── 운전 중 실시간 UI 갱신 ─────────────────────────────────────────
 * - 현재 온도/습도: state->current_temp / current_humidity
 * - 운전 모드 서클: current_op_mode 에 맞는 1개만 표시
 * - 진도 아크: 경과시간 % 로 업데이트
 * - 운전 모드 GIF : 모드 변경 시에만 stop/start
 * 모드 전환(냉동→해동→저온→고온)은 MCU가 saveoperation[5]를
 * 0x10→0x20→0x30→0x40으로 변경하면 current_op_mode가 자동 갱신됨
 * ────────────────────────────────────────────────────────────────── */
static const char *_circle_png_for_mode(int op_mode);
static void        _load_circle_canvas_ams(int op_mode);

static void _refresh_running_ui(bk_lv_ui_t *bk_ui)
{
    device_state_t *state = &g_device_state;
    char buf[16];

    /* ── TEST_MODE: lv_tick 기반 단계 자동 전환 ──────────────────── */
#if AUTO_MODE_TEST
    if (state->auto_mode_start) {
        uint32_t elapsed_ms = lv_tick_elaps(s_test_mode_tick);
        uint32_t stage_ms   = (uint32_t)AUTO_MODE_TEST_MIN * 60u * 1000u;
        if (elapsed_ms >= stage_ms) {
            s_test_mode_tick = lv_tick_get();
            if (s_test_op_mode < OP_MODE_FERM2) {
                s_test_op_mode++;
                bk_printf(TAG "[TEST] mode -> %d\n", s_test_op_mode);
            } else {
                /* FERM2 완료 → automodeend */
                _go_to_automodeend1(bk_ui);
                return;
            }
        }
    }
#endif

//     /* ── 자동모드 완료 감지 ───────────────────────────────────────────────
//      * 0x42=FERM2완료, 0x43=과발효방지 포함 완료.
//      * 과발효방지(DetailOverFermentationOnOff=ON) 시:
//      *   0x42 수신 → over_ferm_active=true → TX가 0x43 지속 전송
//      *   MCU가 0x34(저온발효 실행)로 응답하면 → over_ferm_active=false, TX가 0x34 에코
//      *   MCU 완료 후 0x43 송신 → automodeend.
//      * 과발효방지 OFF: 0x42 수신 즉시 automodeend (기존 동작).
//      * Android와 동일: senddrivemode=0x43 고정 → MCU 응답 에코로 자동 전환. */
// #if !AUTO_MODE_TEST
//     if (state->auto_mode_start) {
//         uint8_t _op5 = (uint8_t)state->saveoperation[5];
//         if (_op5 == 0x43 && !state->over_ferm_active) {
//             /* 저온발효 완료(MCU 0x43) 또는 일반 완료.
//              * over_ferm_active=true 동안은 MCU early-ACK(0x43 에코) 가능 → 무시.
//              * over_ferm_active=false(0x34 후 해제)이면 진짜 완료 신호 → end 화면. */
//             _go_to_automodeend(bk_ui);
//             return;
//         }
//         if (_op5 == 0x42) {
//             bool over_on  = (strcmp(settings_get_str("DetailOverFermentationOnOff"), "ON") == 0);
//             int  over_min = atoi(settings_get_str("DetailOverFermentation"));
//             if (over_on && over_min > 0) {
//                 /* 과발효방지 ON: over_ferm_active로 TX 0x43 지속 전송.
//                  * first_send(1회) 아닌 영구 override → MCU가 0x34 응답할 때 RX에서 자동 해제. */
//                 if (!s_over_ferm_triggered) {
//                     s_over_ferm_triggered   = true;
//                     state->over_ferm_active = true;
//                     settings_set_int("saveOperationTemp", 8);

//                     /* 완료시각 재계산: 현재시각 + DetailOverFermentation분
//                      * → payload[14/15]로 MCU에 과발효방지 종료 목표시각 전달 */
//                     int oy, om, od, oh, omn, os;
//                     hal_rtc_get(&oy, &om, &od, &oh, &omn, &os);
//                     int total_min = oh * 60 + omn + over_min;
//                     while (total_min >= 24 * 60) {
//                         total_min -= 24 * 60;
//                         od++;
//                         /* 월말 처리 (간략: 각 달 정확 일수 대신 안전값 28 사용) */
//                         int _dim = (om==2)?28:(om==4||om==6||om==9||om==11)?30:31;
//                         if (od > _dim) { od = 1; if (++om > 12) { om = 1; oy++; } }
//                     }
//                     state->send_complete_year  = oy;
//                     state->send_complete_month = om;
//                     state->send_complete_day   = od;
//                     state->send_complete_hour  = total_min / 60;
//                     state->send_complete_min   = total_min % 60;
//                     settings_save_dirty();
//                     printf("[OVER_FERM] 0x42 ON → over_ferm target=%04d-%02d-%02d %02d:%02d (+%dmin)\n",
//                            oy, om, od, total_min/60, total_min%60, over_min);
//                 }
//                 /* MCU가 아직 0x42: over_ferm_active 유지, automodeend 보류 */
//             } else {
//                 /* 과발효방지 OFF: 즉시 

//                 _go_to_automodeend(bk_ui);
//                 return;
//             }
//         }
//     }
// #endif

#if !AUTO_MODE_TEST
    {
        int cur_op = (int)state->current_op_mode;

        if (!state->auto_mode_start) {
            s_ferm2_nonzero       = false;
            s_end_triggered       = false;
            s_ferm2_tick_valid    = false;
            s_over_ferm_triggered = false;
        }

        if (state->auto_mode_start && !s_end_triggered) {
            /* MCU remain 기반 완료 감지 (MCU가 remain>0을 보낸 적 있으면 활성화) */
            if (cur_op == OP_MODE_FERM2 &&
                (state->remain_hour > 0 || state->remain_min > 0))
                s_ferm2_nonzero = true;

            /* 벽시계 기반 FERM2 완료 감지 (remain이 항상 0인 경우 폴백) */
            if (cur_op == OP_MODE_FERM2 && !s_ferm2_tick_valid) {
                s_ferm2_start_tick = lv_tick_get();
                s_ferm2_tick_valid = true;
            }
            bool ferm2_wall_done = false;
            if (cur_op == OP_MODE_FERM2 && s_ferm2_tick_valid) {
                int ferm2_min = state->send_ferm2_hour * 60 + state->send_ferm2_min;
                uint32_t ferm2_ms = (uint32_t)ferm2_min * 60u * 1000u;
                if (ferm2_ms > 0 && lv_tick_elaps(s_ferm2_start_tick) >= ferm2_ms)
                    ferm2_wall_done = true;
            }

            bool mcu_done   = (cur_op >= 7);
            bool ferm2_done = (cur_op == OP_MODE_FERM2 && s_ferm2_nonzero &&
                               state->remain_hour == 0 && state->remain_min == 0);

            if (mcu_done || ferm2_done || ferm2_wall_done) {
                /* Android AutoModeEndFragment 와 동일:
                 * 조건: DetailOverFermentationOnOff==ON AND day_period>1 AND over_min>0 AND 1회만 */
                bool over_on  = (strcmp(settings_get_str("DetailOverFermentationOnOff"), "ON") == 0);
                int  over_min = atoi(settings_get_str("DetailOverFermentation"));
                bk_printf(TAG "[OVER_FERM] DONE CHECK: cur_op=%d over_on=%d day=%d min=%d triggered=%d\n",
                       cur_op, over_on, state->day_period, over_min, (int)s_over_ferm_triggered);
                if (over_on && state->day_period > 1 && over_min > 0 && !s_over_ferm_triggered) {
                    s_over_ferm_triggered   = true;
                    state->over_ferm_active = true;
                    /* comp_time 갱신: current_time + over_min → MCU payload[11-16] 기준
                     * MCU가 0x43 echo와 이 comp_time을 보고 0x34(저온발효) 전환 시점 결정 */
                    {
                        int oy, om, od, oh, omn, os;
                        hal_rtc_get(&oy, &om, &od, &oh, &omn, &os);
                        int total_min = oh * 60 + omn + over_min;
                        while (total_min >= 24 * 60) {
                            total_min -= 24 * 60;
                            od++;
                            int _dim = (om==2)?28:(om==4||om==6||om==9||om==11)?30:31;
                            if (od > _dim) { od = 1; if (++om > 12) { om = 1; oy++; } }
                        }
                        state->send_complete_year  = oy;
                        state->send_complete_month = om;
                        state->send_complete_day   = od;
                        state->send_complete_hour  = total_min / 60;
                        state->send_complete_min   = total_min % 60;
                        bk_printf(TAG "[OVER_FERM] comp_time → %04d-%02d-%02d %02d:%02d (+%dmin)\n",
                               oy, om, od, total_min/60, total_min%60, over_min);
                    }
                    bk_printf(TAG "[OVER_FERM] FERM2 done -> automodeend 화면 (day_period=%d, over_min=%d)\n",
                           state->day_period, over_min);
                    /* 부저+기록저장: lv_scr_load 전에 처리 (SCREEN_LOAD_START 콜백 밖에서)
                     * uart_comm.c RX 0x34 → over_ferm_jeon_started → manualmodestart 전환 */
                    automodeend_save_record_and_buzzer(state);
#if UI_PRENDERING_ENABLE
                    ui_page_change(PAGE_AUTOMODEEND);
                    lv_label_set_text(bk_ui->automodeend_AutoModeCompleteYear,  settings_get_str("CurrentCompleteYear"));
                    lv_label_set_text(bk_ui->automodeend_AutoModeCompleteMonth, settings_get_str("CurrentCompleteMonth"));
                    lv_label_set_text(bk_ui->automodeend_AutoModeCompleteDay,   settings_get_str("CurrentCompleteDay"));
                    lv_label_set_text(bk_ui->automodeend_AutoModeCompleteHour,  settings_get_str("CurrentCompleteHour"));
                    lv_label_set_text(bk_ui->automodeend_AutoModeCompleteMin,   settings_get_str("CurrentCompleteMin"));
#else
                    init_page_automodeend(bk_ui);
                    lv_label_set_text(bk_ui->automodeend_AutoModeCompleteYear,  settings_get_str("CurrentCompleteYear"));
                    lv_label_set_text(bk_ui->automodeend_AutoModeCompleteMonth, settings_get_str("CurrentCompleteMonth"));
                    lv_label_set_text(bk_ui->automodeend_AutoModeCompleteDay,   settings_get_str("CurrentCompleteDay"));
                    lv_label_set_text(bk_ui->automodeend_AutoModeCompleteHour,  settings_get_str("CurrentCompleteHour"));
                    lv_label_set_text(bk_ui->automodeend_AutoModeCompleteMin,   settings_get_str("CurrentCompleteMin"));
                    lv_scr_load(bk_ui->automodeend);
#endif /* UI_PRENDERING_ENABLE */
                    return;
                }
                /* 과발효방지 진행 중(over_ferm_active=true): MCU 완료 신호 대기 */
                if (state->over_ferm_active) return;

                /* 과발효방지 발동 후 MCU 0x34(저온발효) 진행 중 → 0x43(완료) 대기 */
                if (s_over_ferm_triggered &&
                    ((uint8_t)state->saveoperation[5] == 0x34 ||
                     (uint8_t)state->saveoperation[5] == 0x44)) return;

                s_end_triggered = true;
                _go_to_automodeend(bk_ui);
                return;
            }
        }
    }
#endif


    /* TEST_MODE: 표시 기준 모드는 s_test_op_mode, 아니면 state->current_op_mode */
#if AUTO_MODE_TEST
    int _ui_mode = s_test_op_mode;
#else
    int _ui_mode = (int)state->current_op_mode;
#endif
    /* 저온발효(op_mode 9=0x34, 10=0x44): Android AutoModeOver와 동일하게 FERM1 에셋으로 표시 */
    if (_ui_mode == 9 || _ui_mode == 10) _ui_mode = OP_MODE_FERM1;

    /* 모드 전환 즉시 s_mode_start_tick 리셋 — 남은시간 계산보다 먼저 처리해야 함.
     * _update_arc_ams 내부에서 리셋하면 남은시간 레이블이 한 프레임 동안
     * '신모드_total - 구모드_elapsed' 로 잘못 계산됨(예: 1분 표시 버그). */
    if (_ui_mode != s_arc_seen_mode) {
        int prev_mode = s_arc_seen_mode;   /* 갱신 전 이전 모드 보존 */

        /* ── 이전 단계 완료 → 해당 단계 잔여시간 "00:00" 확정 ─────────────────
         * MCU는 remain=1에서 곧바로 다음 단계로 전환하므로 AP가 remain=0을
         * 수신할 기회가 없음. 전환 감지 시점에 이전 단계 라벨을 강제로 00:00으로
         * 설정해야 "각 행정 완료 후 00:01 잔류" 표시 버그를 해소할 수 있다. */
        switch (prev_mode) {
            case OP_MODE_FREEZE:
                lv_label_set_text(bk_ui->automodestart_AutoFreezeTimeHourTxt, "00");
                lv_label_set_text(bk_ui->automodestart_AutoFreezeTimeMinTxt,  "00");
                break;
            case OP_MODE_DEFROST:
                lv_label_set_text(bk_ui->automodestart_AutoDefrostTimeHourTxt, "00");
                lv_label_set_text(bk_ui->automodestart_AutoDefrostTimeMinTxt,  "00");
                break;
            case OP_MODE_FERM1:
                lv_label_set_text(bk_ui->automodestart_AutoFermentation1TimeHourTxt, "00");
                lv_label_set_text(bk_ui->automodestart_AutoFermentation1TimeMinTxt,  "00");
                break;
            default: break;   /* -1(초기) 또는 FERM2(3): 처리 불필요 */
        }

        s_arc_seen_mode   = _ui_mode;
        s_mode_start_tick = lv_tick_get();
        s_last_remain_h   = -1;
        s_last_remain_m   = -1;
        s_rx_seq_at_mode_start = g_uart_rx_seq;
    }

    /* 현재 온도 / 습도 — 값 변화 시에만 set (동일값 dirty 마킹 방지) */
    if (state->current_temp != s_last_temp_val) {
        s_last_temp_val = state->current_temp;
        {
            int _is_f = (strcmp(settings_get_str("Degree"), "\xc2\xb0""F") == 0);
            int _t = _is_f ? (state->current_temp * 9 / 5 + 32) : state->current_temp;
            snprintf(buf, sizeof(buf), "%d", _t);
        }
        lv_label_set_text(bk_ui->automodestart_tempbox_current_temp, buf);
    }
    if (state->current_humidity != s_last_hum_val) {
        s_last_hum_val = state->current_humidity;
        snprintf(buf, sizeof(buf), "%d", state->current_humidity);
        lv_label_set_text(bk_ui->automodestart_tempbox_current_humidity, buf);
    }

    /* 잔여시간: MCU UART saveoperation[10/11]을 기본으로 쓰되, 이 단계 진입 후
     * 아직 MCU STATUS를 한 번도 못 받았을 때(g_uart_rx_seq가 단계 진입 시점과
     * 동일)는 saveoperation[10/11]이 이전 단계/초기값(0)이라 "0:00"이 1-2초간
     * 잘못 표시된다 — 그 짧은 구간만 이 단계의 총 소요시간(_total_min_for_mode,
     * automode_cb.c가 시작 시 이미 계산해 둔 send_*_hour/min 기반)을 "아직 하나도
     * 경과 안 함" 상태로 대체 표시한다. day_period 계산 버그(24시간 부족)를 이미
     * 수정했으므로, 이 자체 계산값은 MCU가 실제로 보내올 값과 일치해 전환 시
     * 숫자가 급변하지 않는다. state->remain_hour/min(첫 단계에서만 유효)이 아니라
     * _total_min_for_mode를 쓰는 이유: 해동/발효1/발효2로 단계가 넘어갈 때도
     * (냉동 단계의 잔류값이 아니라) 그 단계 자신의 총 시간이 나와야 하기 때문. */
    {
        int _rh, _rm;
        {
            int mcu_rem_h = (int)(uint8_t)state->saveoperation[10];
            int mcu_rem_m = (int)(uint8_t)state->saveoperation[11];
            if (state->black_out_checking) {
                int total_min   = _total_min_for_mode(state, _ui_mode);
                int elapsed_min = (int)(uint8_t)state->saveoperation[12] * 60
                                + (int)(uint8_t)state->saveoperation[13];
                int remain_min  = total_min - elapsed_min;
                if (remain_min < 0) remain_min = 0;
                _rh = remain_min / 60;
                _rm = remain_min % 60;
            } else if (g_uart_rx_seq == s_rx_seq_at_mode_start) {
                /* 이 단계에서 아직 MCU STATUS 미수신 — 이 단계의 총 시간 표시 */
                int total_min = _total_min_for_mode(state, _ui_mode);
                _rh = total_min / 60;
                _rm = total_min % 60;
            } else {
                _rh = mcu_rem_h;
                _rm = mcu_rem_m;
            }
        }
        if (_rh != s_last_remain_h || _rm != s_last_remain_m) {
            s_last_remain_h = _rh;
            s_last_remain_m = _rm;
            char _rhs[8], _rms[8];
            snprintf(_rhs, sizeof(_rhs), "%02d", _rh);
            snprintf(_rms, sizeof(_rms), "%02d", _rm);
            switch (_ui_mode) {
                case OP_MODE_FREEZE:
                    lv_label_set_text(bk_ui->automodestart_AutoFreezeTimeHourTxt, _rhs);
                    lv_label_set_text(bk_ui->automodestart_AutoFreezeTimeMinTxt,  _rms);
                    break;
                case OP_MODE_DEFROST:
                    lv_label_set_text(bk_ui->automodestart_AutoDefrostTimeHourTxt, _rhs);
                    lv_label_set_text(bk_ui->automodestart_AutoDefrostTimeMinTxt,  _rms);
                    break;
                case OP_MODE_FERM1:
                    lv_label_set_text(bk_ui->automodestart_AutoFermentation1TimeHourTxt, _rhs);
                    lv_label_set_text(bk_ui->automodestart_AutoFermentation1TimeMinTxt,  _rms);
                    break;
                case OP_MODE_FERM2:
                    lv_label_set_text(bk_ui->automodestart_AutoFermentation2TimeHourTxt, _rhs);
                    lv_label_set_text(bk_ui->automodestart_AutoFermentation2TimeMinTxt,  _rms);
                    break;
                default: break;
            }
            /* 정전복구용 flash 저장 — 분 변화 시 즉시
             * black_out_checking=true(복구 중) 시 저장 금지:
             *   _rh = (bo_*_total_min - MCU_elapsed) / 60
             *       = (원래전체시간 - 0) / 60  ← MCU elapsed 버그로 항상 0
             *       = 전체시간  →  이전 세션의 올바른 잔여시간을 덮어씌움.
             * 복구 중 flash 저장은 uart_comm.c (벽시계 기반, 30s 주기)가 전담. */
            if (state->auto_mode_start && !state->black_out_checking) {
                settings_set_int("saveCurrentRemainHour", _rh);
                settings_set_int("saveCurrentRemainMin",  _rm);
                settings_save_dirty();
            }
        }
    }

    /* tempbox src + 습도 가시성 — 모드 전환 시에만 (매 초 0ms 타이머 생성 방지) */
    int _show_hum = (_ui_mode == OP_MODE_FERM1 || _ui_mode == OP_MODE_FERM2) ? 1 : 0;
    if (_show_hum != s_last_show_hum) {
        s_last_show_hum = _show_hum;
        {
            int _lang = settings_get_int("LANGUAGE");
            int _is_f = (strcmp(settings_get_str("Degree"), "\xc2\xb0""F") == 0) ? 1 : 0;
            const char *_fsuf = _is_f ? "_f" : "";
            const char *_lsuf = (_lang == 1) ? "_china" : (_lang == 2) ? "_english" : "";
            char _tb[128];
            if (_show_hum)
                snprintf(_tb, sizeof(_tb), "/images/tempbox%s%s.png", _fsuf, _lsuf);
            else
                snprintf(_tb, sizeof(_tb), "/images/tempbox%s_zero%s.png", _fsuf, _lsuf);
            _img_set_src_timed(bk_ui->automodestart_auto_tempbox, _tb);
        }
        if (_show_hum) {
            _img_ensure_src(bk_ui->automodestart_tempbox_current_humidity);
            lv_obj_clear_flag(bk_ui->automodestart_tempbox_current_humidity, LV_OBJ_FLAG_HIDDEN);
        } else
            lv_obj_add_flag(bk_ui->automodestart_tempbox_current_humidity,   LV_OBJ_FLAG_HIDDEN);
    }

    /* 모드 서클 (canvas 1개) — 모드 전환 시 PNG 재렌더 후 표시 */
    if (_ui_mode != s_last_ui_mode) {
        s_last_ui_mode = _ui_mode;
        if (_circle_png_for_mode(_ui_mode)) {
            // _load_circle_canvas_ams(_ui_mode);
            if (s_cc_canvas) lv_obj_clear_flag(s_cc_canvas, LV_OBJ_FLAG_HIDDEN);
        } else {
            if (s_cc_canvas) lv_obj_add_flag(s_cc_canvas, LV_OBJ_FLAG_HIDDEN);
        }
    }

    /* GIF 애니메이션: 모드 변경 시에만 stop/start */
    if (_ui_mode != s_ams_current_mode) {
        _gif_anim_stop_ams(bk_ui);
        _gif_anim_start_ams(bk_ui, _ui_mode);
    }

    /* 진도 아크: 5초마다 + 모드 전환 시 즉시.
     * arc canvas 소프트웨어 렌더링 비용이 크므로 갱신 주기를 연장.
     * 운전 시간 수십분~수시간 대비 5초 해상도는 시각적으로 충분. */
    if (_ui_mode != s_last_arc_mode || lv_tick_elaps(s_last_arc_tick) >= 1000) {
        s_last_arc_mode = _ui_mode;
        s_last_arc_tick = lv_tick_get();
        _update_arc_ams(bk_ui);
    }
}

static void _ui_timer_cb(lv_timer_t *timer)
{
    (void)timer;
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    if (!bk_ui->automodestart || !lv_obj_is_valid(bk_ui->automodestart)) return;
    if (lv_scr_act() != bk_ui->automodestart) return;
    _refresh_running_ui(bk_ui);
}

void automodestart_startbt_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    device_state_t *state = &g_device_state;
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    if (lv_tick_elaps(last_click_time) < 250) return;
    last_click_time = lv_tick_get();

    /* 과발효방지 상태 강제 해제 (Android AutoModeOver=false 와 동일) */
    state->over_ferm_active       = false;
    state->over_ferm_jeon_started = false;
    state->auto_mode_over         = false;
    state->manual_current_mode    = 0;
    s_over_ferm_triggered         = false;

    /* 타이머 정지 */
    if (s_ui_timer) {
        lv_timer_delete(s_ui_timer);
        s_ui_timer = NULL;
    }

    hal_buzzer_beep();

    /* automode SCREEN_LOAD_START의 완료시간/day_period 리셋 로직이
     * !state->operation을 조건으로 하므로, 화면 전환(lv_scr_load) 전에
     * operation을 먼저 false로 내려야 한다. 순서가 바뀌면 리셋이 스킵되어
     * 운전 중 누적된 day_period가 그대로 남아 완료일이 실제보다 며칠
     * 뒤로 표시되는 버그가 발생한다. */
    state->operation          = false;
    state->start_run          = true;
    state->first_start        = false;
    state->auto_mode_start    = false;
    state->black_out_checking = false;
    settings_set_str("saveChecking", "0");
    settings_save_dirty();

#if UI_PRENDERING_ENABLE
    ui_page_change(PAGE_AUTOMODE);
#else
    if (bk_ui->automode == NULL || !lv_obj_is_valid(bk_ui->automode))
        init_page_automode(bk_ui);
    lv_scr_load(bk_ui->automode);
#endif /* UI_PRENDERING_ENABLE */
}

static void _anim_toggle_event_cb(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    s_anim_enabled = !s_anim_enabled;
    if (s_anim_enabled) {
        if (s_ams_current_mode >= 0)
            _gif_anim_start_ams(bk_ui, s_ams_current_mode);
    } else {
        if (s_anim_timer_ams) { lv_timer_delete(s_anim_timer_ams); s_anim_timer_ams = NULL; }
    }
    if (s_anim_toggle_label)
        lv_label_set_text(s_anim_toggle_label, s_anim_enabled ? "ANIM:ON" : "ANIM:OFF");
}

/* ── 투명 클립 컨테이너 생성 헬퍼 ──────────────────────────────────── */
static lv_obj_t *_make_clip_ams(lv_obj_t *parent, int x, int y, int w, int h, bool hidden)
{
    lv_obj_t *obj = lv_obj_create(parent);
    lv_obj_set_pos(obj, x, y);
    lv_obj_set_size(obj, w, h);
    lv_obj_set_style_bg_opa(obj, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(obj, 0, 0);
    lv_obj_set_style_pad_all(obj, 0, 0);
    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE);
    if (hidden) lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN);
    return obj;
}

/* ── bg JPEG 1회 decode → canvas 영구 buffer, auto_bg src에 직접 적용 ─ */
static void _ams_bg_load(bk_lv_ui_t *bk_ui)
{
    int lang  = settings_get_int("LANGUAGE");
    int is_f  = (strcmp(settings_get_str("Degree"), "\xc2\xb0""F") == 0) ? 1 : 0;
    int key   = lang * 10 + is_f;
    const char *fsuf = is_f     ? "_f"      : "";
    const char *lsuf = (lang == 1) ? "_china"
                     : (lang == 2) ? "_english"
                     :               "";
    char bg_path[128];
    snprintf(bg_path, sizeof(bg_path), "/images/auto_mode_start_bgi%s%s.jpg", fsuf, lsuf);


    /* buf는 부팅 시 automodestart_canvas_buf_alloc()에서 이미 확보되어 있음 —
     * 여기서는 JPEG SW decode용 임시 스크래치(~1.06MB)만 여유 있으면 됨.
     * (예전엔 매 진입마다 buf_sz만큼을 추가로 malloc해야 해서 need가 더 컸고,
     * 그 malloc 자체가 단편화 시 하드크래시의 원인이었음 — 이제 buf는 고정.) */
    size_t need = 1300 * 1024;
    size_t _free = rtos_get_psram_free_heap_size();
    bk_printf(TAG "[AMS] psram free = %u B (need %u B)\n", (unsigned)_free, (unsigned)need);
    if (_free < need) {
        bk_printf(TAG "[AMS] psram low, skip bg load\n");
        _img_set_src_timed(bk_ui->automodestart_auto_bg, bg_path);
        return;
    }

    if (!s_ams_canvas_buf) automodestart_canvas_buf_alloc();  /* 안전망 */
    if (!s_ams_canvas_buf) {
        _img_set_src_timed(bk_ui->automodestart_auto_bg, bg_path);
        return;
    }

    s_ams_bg_canvas = lv_canvas_create(bk_ui->automodestart);
    lv_canvas_set_buffer(s_ams_bg_canvas, s_ams_canvas_buf, 1024, 540, LV_COLOR_FORMAT_RGB565);
    lv_obj_add_flag(s_ams_bg_canvas, LV_OBJ_FLAG_HIDDEN);

    if (s_ams_buf_lang != key) {
        lv_layer_t layer;
        lv_canvas_init_layer(s_ams_bg_canvas, &layer);
        lv_draw_image_dsc_t img_dsc;
        lv_draw_image_dsc_init(&img_dsc);
        img_dsc.src = bg_path;
        lv_area_t area = {0, 0, 1023, 539};
        lv_draw_image(&layer, &img_dsc, &area);
        lv_canvas_finish_layer(s_ams_bg_canvas, &layer);
        s_ams_buf_lang = key;
    }

    lv_image_set_src(bk_ui->automodestart_auto_bg, lv_canvas_get_image(s_ams_bg_canvas));
    lv_obj_move_background(bk_ui->automodestart_auto_bg);
}

/* ── PNG를 canvas RAM 버퍼에 1회 디코딩 — 이후 렌더는 memcpy (~5ms) ──── */
static const char *_circle_png_for_mode(int op_mode)
{
    switch (op_mode) {
        // case OP_MODE_FREEZE:  return "/images/freeze_gif.png";
        // case OP_MODE_DEFROST: return "/images/defrost_gif.png";
        // case OP_MODE_FERM1:   return "/images/fermentation1_gif.png";
        // case OP_MODE_FERM2:   return "/images/fermentation2_gif.png";
        default:              return NULL;
    }
}

/* 모드 전환 시 canvas 버퍼에 해당 PNG를 1회 디코딩 */
static void _load_circle_canvas_ams(int op_mode)
{
    if (!s_cc_canvas || s_cc_mode == op_mode) return;
    const char *src = _circle_png_for_mode(op_mode);
    if (!src) return;

    lv_canvas_fill_bg(s_cc_canvas, lv_color_hex(0x000000), LV_OPA_TRANSP);
    lv_layer_t layer;
    lv_canvas_init_layer(s_cc_canvas, &layer);
    lv_draw_image_dsc_t img_dsc;
    lv_draw_image_dsc_init(&img_dsc);
    img_dsc.src = src;
    lv_area_t area = {0, 0, CIRCLE_CANVAS_W - 1, CIRCLE_CANVAS_H - 1};
    lv_draw_image(&layer, &img_dsc, &area);
    lv_canvas_finish_layer(s_cc_canvas, &layer);
    s_cc_mode = op_mode;
    bk_printf(TAG "[CANVAS] mode=%d %s\n", op_mode, src);
}

/* ui_lang_invalidate_cached_screens 에서 lv_obj_del이 SCREEN_UNLOAD_START를 우회하므로
 * 언어/온도 변경 시 이 함수로 명시적 정리 — destroy 호출 전에 반드시 먼저 호출할 것. */
void automodestart_lang_invalidate(bk_lv_ui_t *bk_ui)
{
    if (!bk_ui->automodestart || !lv_obj_is_valid(bk_ui->automodestart)) return;
    if (s_ui_timer) { lv_timer_delete(s_ui_timer); s_ui_timer = NULL; }
    _gif_anim_stop_ams(bk_ui);
    s_ams_defrost_clip  = s_ams_defrost_img  = NULL;
    s_ams_ferm1_outer   = s_ams_ferm1_inner  = NULL;
    s_ams_ferm1_top     = s_ams_ferm1_btm    = s_ams_ferm1_btm_img = NULL;
    s_ams_ferm2_outer   = s_ams_ferm2_inner  = NULL;
    s_ams_ferm2_top     = s_ams_ferm2_btm    = s_ams_ferm2_btm_img = NULL;
    s_anim_toggle_btn   = NULL;
    s_anim_toggle_label = NULL;
    s_arc_canvas        = NULL;
    /* s_ams_canvas_buf는 절대 free하지 않음(부팅 시 1회 malloc, 파일 상단 주석
     * 참고) — canvas 위젯(s_ams_bg_canvas)만 정리하고 버퍼는 재사용한다.
     * UI_CANVAS_BUF_PERMANENT_ENABLE=0이면 예전처럼 화면 이탈 시 버퍼도 반납. */
    s_ams_bg_canvas     = NULL;
#if !UI_CANVAS_BUF_PERMANENT_ENABLE
    lv_free(s_ams_canvas_buf);
    s_ams_canvas_buf    = NULL;
#endif
    s_ams_buf_lang      = -1;
}

void automodestart_unload_start_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    (void)e;
    automodestart_lang_invalidate(bk_ui);
}

/* SCREEN_LOADED: lv_task_handler에서 호출 — 스택 깊이 얕음
 * PNG 디코딩(_img_ensure_src)이 여기서 안전하게 실행됨.
 * (SCREEN_LOAD_START는 이벤트 콜백 내부에서 호출 → 스택 오버플로우 위험) */
void automodestart_loaded_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    (void)e;

    ui_title_anim(bk_ui->automodestart_title);
    /* 완료 감지 statics 리셋: 이전 운전(day=1→s_end_triggered=true 등) 잔류 방지
     * auto_mode_start가 이미 true로 설정된 채 로드되면 _refresh_running_ui의
     * !auto_mode_start 리셋 분기가 실행되지 않으므로 여기서 명시적으로 초기화 */
    s_ferm2_nonzero    = false;
    s_end_triggered    = false;
    s_ferm2_start_tick = 0;
    s_ferm2_tick_valid = false;
    s_over_ferm_triggered = false;
    /* 상태 캐시 초기화 — 매 로드 시 강제 적용 */
    s_ams_current_mode = -1;
    s_arc_seen_mode    = -1;
    s_mode_start_tick  = lv_tick_get();
    s_last_show_hum = -1;
    s_last_ui_mode  = -1;
    s_last_arc_mode = -1;
    s_last_arc_tick = 0;
    s_last_pct[0] = s_last_pct[1] = s_last_pct[2] = s_last_pct[3] = -1;
    s_last_temp_val = 0x7FFFFFFF;
    s_last_hum_val  = 0x7FFFFFFF;
    s_last_remain_h = -1;
    s_last_remain_m = -1;
#if AUTO_MODE_TEST
    s_test_mode_tick = lv_tick_get();
    s_test_op_mode   = OP_MODE_FREEZE;
    g_device_state.current_op_mode = OP_MODE_FREEZE;
    bk_printf(TAG "[TEST] automodestart loaded, timer reset\n");
#endif
    _refresh_running_ui(bk_ui);
    if (s_ui_timer) { lv_timer_delete(s_ui_timer); s_ui_timer = NULL; }
    s_ui_timer = lv_timer_create(_ui_timer_cb, 1000, NULL);
}

void automodestart_unloaded_event_cb(lv_event_t *e)
{
    (void)e;
}

/* SCREEN_LOAD_START: 레이블 갱신 + 클립 컨테이너 생성 — PNG 디코딩 없음
 * (fermentation 이미지는 deferred로 처리됨) */
void automodestart_load_start_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    (void)e;

    /* 원본 image 객체 영구 숨김 — canvas가 대체 */
    lv_obj_add_flag(bk_ui->automodestart_AutoFreezeIm,        LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(bk_ui->automodestart_AutoDefrostIm,       LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(bk_ui->automodestart_AutoFermentation1Im, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(bk_ui->automodestart_AutoFermentation2Im, LV_OBJ_FLAG_HIDDEN);


    /* Freeze / Defrost / Fermentation — settings는 C, F 모드이면 C→F 변환 표시 */
    {
        int _is_f = (strcmp(settings_get_str("Degree"), "\xc2\xb0""F") == 0);
#define _LT(obj, key) do { \
    const char *_sv = settings_get_str(key); \
    if (_sv && _sv[0]) { \
        char _fb[16]; \
        int _tv = _is_f ? (atoi(_sv) * 9 / 5 + 32) : atoi(_sv); \
        snprintf(_fb, sizeof(_fb), "%02d", _tv); \
        lv_label_set_text(obj, _fb); \
    } else lv_label_set_text(obj, ""); \
} while(0)
        /* Freeze */
        _LT(bk_ui->automodestart_AutoFreezeTempTxt,            "CurrentSaveFreezeTemp");
        lv_label_set_text(bk_ui->automodestart_AutoFreezeTimeHourTxt, settings_get_str("CurrentSaveFreezeTimeHour"));
        lv_label_set_text(bk_ui->automodestart_AutoFreezeTimeMinTxt,  settings_get_str("CurrentSaveFreezeTimeMin"));
        /* Defrost */
        _LT(bk_ui->automodestart_AutoDefrostTempTxt,           "CurrentSaveDefreezeTemp");
        lv_label_set_text(bk_ui->automodestart_AutoDefrostTimeHourTxt,    settings_get_str("CurrentSaveDefreezeTimeHour"));
        lv_label_set_text(bk_ui->automodestart_AutoDefrostTimeMinTxt,     settings_get_str("CurrentSaveDefreezeTimeMin"));
        /* Fermentation 1 */
        _LT(bk_ui->automodestart_AutoFermentation1TempTxt,     "CurrentSaveFermentation1Temp");
        lv_label_set_text(bk_ui->automodestart_AutoFermentation1HumidityTxt, settings_get_str("CurrentSaveFermentation1Humidity"));
        lv_label_set_text(bk_ui->automodestart_AutoFermentation1TimeHourTxt, settings_get_str("CurrentSaveFermentation1TimeHour"));
        lv_label_set_text(bk_ui->automodestart_AutoFermentation1TimeMinTxt,  settings_get_str("CurrentSaveFermentation1TimeMin"));
        /* Fermentation 2 */
        _LT(bk_ui->automodestart_AutoFermentation2TempTxt,     "CurrentSaveFermentation2Temp");
        lv_label_set_text(bk_ui->automodestart_AutoFermentation2HumidityTxt, settings_get_str("CurrentSaveFermentation2Humidity"));
        lv_label_set_text(bk_ui->automodestart_AutoFermentation2TimeHourTxt, settings_get_str("CurrentSaveFermentation2TimeHour"));
        lv_label_set_text(bk_ui->automodestart_AutoFermentation2TimeMinTxt,  settings_get_str("CurrentSaveFermentation2TimeMin"));
#undef _LT
    }
    /* Completion date — 레이블 표시 + TX state 동기 */
    {
        const char *_cy  = settings_get_str("CurrentCompleteYear");
        const char *_cm  = settings_get_str("CurrentCompleteMonth");
        const char *_cd  = settings_get_str("CurrentCompleteDay");
        const char *_ch  = settings_get_str("CurrentCompleteHour");
        const char *_cmn = settings_get_str("CurrentCompleteMin");
        lv_label_set_text(bk_ui->automodestart_AutoModeCompleteYear,  _cy);
        lv_label_set_text(bk_ui->automodestart_AutoModeCompleteMonth, _cm);
        lv_label_set_text(bk_ui->automodestart_AutoModeCompleteDay,   _cd);
        lv_label_set_text(bk_ui->automodestart_AutoModeCompleteHour,  _ch);
        lv_label_set_text(bk_ui->automodestart_AutoModeCompleteMin,   _cmn);
        /* TX send_complete_* 동기: 정전복구/재시작 후 device_state가 0인 경우 보정 */
        g_device_state.send_complete_year  = atoi(_cy);
        g_device_state.send_complete_month = atoi(_cm);
        g_device_state.send_complete_day   = atoi(_cd);
        g_device_state.send_complete_hour  = atoi(_ch);
        g_device_state.send_complete_min   = atoi(_cmn);
        /* 완료시각 00:00 → MCU가 0x42(저온발효 자율) 진입 원인.
         * 설정이 기본값(미설정 00:00)이면 08:00으로 보정 */
        if (g_device_state.send_complete_hour == 0 && g_device_state.send_complete_min == 0) {
            g_device_state.send_complete_hour = 8;
            lv_label_set_text(bk_ui->automodestart_AutoModeCompleteHour, "08");
        }
    }

    /* 정전 복구: 이미 완료된 phase는 00:00으로 강제 표시.
     * SCREEN_LOAD_START는 CurrentSave*TimeHour 를 그대로 읽으므로
     * 완료된 phase가 원래 설정 시간(예: 05h)을 표시하는 버그 방지. */
    if (g_device_state.black_out_checking) {
        int _cur = (int)g_device_state.current_op_mode;
        if (_cur > OP_MODE_FREEZE) {
            lv_label_set_text(bk_ui->automodestart_AutoFreezeTimeHourTxt, "00");
            lv_label_set_text(bk_ui->automodestart_AutoFreezeTimeMinTxt,  "00");
        }
        if (_cur > OP_MODE_DEFROST) {
            lv_label_set_text(bk_ui->automodestart_AutoDefrostTimeHourTxt, "00");
            lv_label_set_text(bk_ui->automodestart_AutoDefrostTimeMinTxt,  "00");
        }
        if (_cur > OP_MODE_FERM1) {
            lv_label_set_text(bk_ui->automodestart_AutoFermentation1TimeHourTxt, "00");
            lv_label_set_text(bk_ui->automodestart_AutoFermentation1TimeMinTxt,  "00");
        }
    }

    /* 진도 아크 canvas — buf 영구, canvas는 화면 수명과 동일 */
    if (!s_arc_canvas) {
        if (!s_arc_buf)
            s_arc_buf = lv_malloc(CIRCLE_CANVAS_W * CIRCLE_CANVAS_H * 4u);
        if (s_arc_buf) {
            s_arc_canvas = lv_canvas_create(bk_ui->automodestart);
            lv_canvas_set_buffer(s_arc_canvas, s_arc_buf,
                                 CIRCLE_CANVAS_W, CIRCLE_CANVAS_H,
                                 LV_COLOR_FORMAT_ARGB8888);
            lv_obj_set_pos(s_arc_canvas, 362, 117);
            lv_canvas_fill_bg(s_arc_canvas, lv_color_hex(0x000000), LV_OPA_TRANSP);
            lv_obj_remove_flag(s_arc_canvas, LV_OBJ_FLAG_CLICKABLE);
        }
    }

    /* GIF 애니메이션 클립 컨테이너 lazy-create (첫 로드 시 1회만)
     * fermentation 이미지는 _img_set_src_deferred — PNG 디코딩 없음.
     * 실제 디코딩은 SCREEN_LOADED → _gif_anim_start_ams → _img_ensure_src 에서. */

    if (!s_ams_defrost_clip) {
        lv_obj_t *par = bk_ui->automodestart;
        
              /* 해동 낙하 클립 (522,188) 55×45 (20% 축소) */
        s_ams_defrost_clip = _make_clip_ams(par, 522, 188-3, 63, 45, true);
        if (!s_ams_defrost_clip) goto clip_alloc_failed;
        s_ams_defrost_img  = lv_image_create(s_ams_defrost_clip);
        _img_set_src_timed(s_ams_defrost_img, "/images/defrost_gif.png");
        lv_obj_set_pos(s_ams_defrost_img, 0, 0);
        lv_obj_remove_flag(s_ams_defrost_img, LV_OBJ_FLAG_CLICKABLE);

        /* 영어인 경우 발효 애니메이션 32px 아래로 이동 */
        int _ferm_dy = (settings_get_int("LANGUAGE") == 2) ? 32 : 0;

        /* 발효1 상단 중첩 클립 (522,272) 63×20 */
        s_ams_ferm1_outer = _make_clip_ams(par, 522, 272 + _ferm_dy, 63, 20, true);
        if (!s_ams_ferm1_outer) goto clip_alloc_failed;
        s_ams_ferm1_inner = _make_clip_ams(s_ams_ferm1_outer, 0, 0, 55, 20, false);
        if (!s_ams_ferm1_inner) goto clip_alloc_failed;
        s_ams_ferm1_top   = lv_image_create(s_ams_ferm1_inner);
        _img_set_src_deferred(s_ams_ferm1_top, "/images/fermentation1_gif.png");
        lv_obj_set_pos(s_ams_ferm1_top, 0, 0);
        lv_obj_remove_flag(s_ams_ferm1_top, LV_OBJ_FLAG_CLICKABLE);
        /* 발효1 하단 고정 클립 (522,292) 63×25 — outer(272~292)와 이음매 없이 이어지도록 */
        s_ams_ferm1_btm     = _make_clip_ams(par, 522, 292 + _ferm_dy, 63, 25, true);
        if (!s_ams_ferm1_btm) goto clip_alloc_failed;
        s_ams_ferm1_btm_img = lv_image_create(s_ams_ferm1_btm);
        _img_set_src_deferred(s_ams_ferm1_btm_img, "/images/fermentation1_gif.png");
        lv_obj_set_pos(s_ams_ferm1_btm_img, 0, -20);
        lv_obj_remove_flag(s_ams_ferm1_btm_img, LV_OBJ_FLAG_CLICKABLE);

        /* 발효2 상단 중첩 클립 (440,272) 63×20 */
        s_ams_ferm2_outer = _make_clip_ams(par, 440, 275-3 + _ferm_dy, 63, 20, true);
        if (!s_ams_ferm2_outer) goto clip_alloc_failed;
        s_ams_ferm2_inner = _make_clip_ams(s_ams_ferm2_outer, 0, 0, 55, 20, false);
        if (!s_ams_ferm2_inner) goto clip_alloc_failed;
        s_ams_ferm2_top   = lv_image_create(s_ams_ferm2_inner);
        _img_set_src_deferred(s_ams_ferm2_top, "/images/fermentation2_gif.png");
        lv_obj_set_pos(s_ams_ferm2_top, 0, 0);
        lv_obj_remove_flag(s_ams_ferm2_top, LV_OBJ_FLAG_CLICKABLE);
        /* 발효2 하단 고정 클립 (440,292) 63×25 — outer(272~292)와 이음매 없이 이어지도록 */
        s_ams_ferm2_btm     = _make_clip_ams(par, 440, 292 + _ferm_dy, 63, 25, true);
        if (!s_ams_ferm2_btm) goto clip_alloc_failed;
        s_ams_ferm2_btm_img = lv_image_create(s_ams_ferm2_btm);
        _img_set_src_deferred(s_ams_ferm2_btm_img, "/images/fermentation2_gif.png");
        lv_obj_set_pos(s_ams_ferm2_btm_img, 0, -20);
        lv_obj_remove_flag(s_ams_ferm2_btm_img, LV_OBJ_FLAG_CLICKABLE);
        goto clip_alloc_done;

clip_alloc_failed:
        bk_printf(TAG "[AMS] clip alloc failed — GIF clips disabled\n");
        s_ams_defrost_clip  = s_ams_defrost_img  = NULL;
        s_ams_ferm1_outer   = s_ams_ferm1_inner  = NULL;
        s_ams_ferm1_top     = s_ams_ferm1_btm    = s_ams_ferm1_btm_img = NULL;
        s_ams_ferm2_outer   = s_ams_ferm2_inner  = NULL;
        s_ams_ferm2_top     = s_ams_ferm2_btm    = s_ams_ferm2_btm_img = NULL;
clip_alloc_done:;
    }

    /* 애니메이션 ON/OFF 토글 버튼 (속도 비교용, lazy-create) */
    if (!s_anim_toggle_btn) {
        s_anim_toggle_btn = lv_button_create(bk_ui->automodestart);
        lv_obj_set_pos(s_anim_toggle_btn, 5, 555);
        lv_obj_set_size(s_anim_toggle_btn, 130, 38);
        lv_obj_set_style_bg_color(s_anim_toggle_btn, lv_color_hex(0x222222), 0);
        lv_obj_set_style_bg_opa(s_anim_toggle_btn, LV_OPA_70, 0);
        lv_obj_set_style_border_width(s_anim_toggle_btn, 0, 0);
        lv_obj_set_style_radius(s_anim_toggle_btn, 6, 0);
        lv_obj_add_event_cb(s_anim_toggle_btn, _anim_toggle_event_cb, LV_EVENT_ALL, NULL);
        s_anim_toggle_label = lv_label_create(s_anim_toggle_btn);
        lv_obj_set_style_text_color(s_anim_toggle_label, lv_color_hex(0xFFFFFF), 0);
        lv_obj_center(s_anim_toggle_label);
    }
    lv_label_set_text(s_anim_toggle_label, s_anim_enabled ? "ANIM:ON" : "ANIM:OFF");

    /* 정전 복구 아이콘 */
    if (g_device_state.black_out_checking) {
        _img_ensure_src(bk_ui->automodestart_blackout);
        lv_obj_clear_flag(bk_ui->automodestart_blackout, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(bk_ui->automodestart_blackout, LV_OBJ_FLAG_HIDDEN);
    }

    ui_lang_apply_automodestart(bk_ui);
    _ams_bg_load(bk_ui);  /* canvas dsc로 JPEG 경로 덮어쓰기 → 이후 렌더 시 decode 없음 */
}
