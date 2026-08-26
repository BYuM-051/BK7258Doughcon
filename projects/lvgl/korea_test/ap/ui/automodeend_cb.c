#include "lvgl.h"
#include "beken_ui.h"
#include "custom_func.h"
#include "event_runtime.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "beken_ui.h"
#include "settings.h"
#include "device_state.h"
#include "ui_animations.h"
#include "ui_lang.h"
#include "hardware_hal.h"

#define TAG "[automodeend_cb.c] "
#include "preRenderer.h"
#define bk_printf(fmt, ...) do {if(0) printf(fmt, ##__VA_ARGS__); } while(0) // disable printf
extern bk_lv_ui_t bk_lv_tool_ui;
extern lv_obj_t *preRenderRoot;

static uint32_t   last_click_time      = 0;
static lv_timer_t *s_over_ferm_timer  = NULL;
static uint32_t   s_over_ferm_start_tick = 0;  /* 과발효방지 진입 시각 (lv_tick) */
static uint32_t   s_over_ferm_total_ms   = 0;  /* over_min × 60000 ms */

/* 과발효방지 대기 타이머 (500ms):
 *  1) Android completeOverCalendar 와 동일한 카운트다운 OverTime 갱신
 *  2) MCU 0x34 수신(over_ferm_jeon_started) 감지 → automodestart 복귀 */
static void _over_ferm_wait_timer_cb(lv_timer_t *t)
{
    (void)t;
    device_state_t *state = &g_device_state;
    bk_lv_ui_t     *bk_ui = &bk_lv_tool_ui;

    /* ① 카운트다운 갱신 (Android: completeOverCalendar - 현재 분) */
    if (s_over_ferm_total_ms > 0 &&
        bk_ui->automodeend_auto_mode_end_overtime &&
        lv_obj_is_valid(bk_ui->automodeend_auto_mode_end_overtime)) {
        uint32_t elapsed = lv_tick_elaps(s_over_ferm_start_tick);
        int remain_min   = (s_over_ferm_total_ms > elapsed)
                           ? (int)((s_over_ferm_total_ms - elapsed) / 60000)
                           : 0;
        char buf[16];
        snprintf(buf, sizeof(buf), "%d", remain_min);
        /* 위치는 화면 진입 분기에서 이미 언어별로 한 번 설정됨 — 카운트다운 중
         * 언어가 바뀌지 않으므로 매 tick 재설정할 필요 없이 텍스트만 갱신 */
        lv_label_set_text(bk_ui->automodeend_auto_mode_end_overtime, buf);
    }

    /* ② MCU 0x34 수신 OR 과발효 시간 경과 (구형 보드 0x42 대응) → FERM2 화면 전환
     * 신형 보드(0x43): MCU가 comp_time 도달 시 0x34 전송 → 즉시 전환
     * 구형 보드(0x42): drive=0x43 echo를 이해 못해 0x34 미전환 → 타임아웃 후 전환
     *   타임아웃 시 over_ferm_active=false → TX가 0x42 에코 → MCU 0x34 전환 */
    {
        uint32_t _elapsed = lv_tick_elaps(s_over_ferm_start_tick);
        bool _timed_out = (s_over_ferm_total_ms > 0 && _elapsed >= s_over_ferm_total_ms);
        if (!state->over_ferm_jeon_started && !_timed_out) return;
        if (_timed_out && !state->over_ferm_jeon_started) {
            /* over_ferm_active=true 유지: drive=0x43 계속 전송
             * MCU가 0x43 echo를 받아야 0x34로 전환함 (drive=0x42로 바꾸면 전환 안됨)
             * uart_comm.c가 0x34 수신 시 over_ferm_active=false로 해제함 */
            bk_printf(TAG "[OVER_FERM] 타임아웃 → FERM2 화면 강제 전환 (drive=0x43 유지, MCU 0x34 대기 중)\n");
        }
    }

    state->over_ferm_jeon_started = false;
    s_over_ferm_total_ms          = 0;
    if (s_over_ferm_timer) {
        lv_timer_delete(s_over_ferm_timer);
        s_over_ferm_timer = NULL;
    }
    /* Android: AutoModeOver=true → ManualModeStartFragment (fermentation_bg, StartBt INVISIBLE)
     * manual_current_mode=FERM, auto_mode_over=true 로 manualmodestart 로드 */
    state->manual_current_mode = MANUAL_MODE_FERM2;
    state->auto_mode_over      = true;
    /* FERM/FERM2 혼용 방지: 이전 수동운전(FERM 등) 화면 재사용하지 않고 항상 새로 생성 */
    if (bk_ui->manualmodestart && lv_obj_is_valid(bk_ui->manualmodestart))
        destroy_page_manualmodestart(bk_ui);
    init_page_manualmodestart(bk_ui);
    lv_scr_load(bk_ui->manualmodestart);
    bk_printf(TAG "[OVER_FERM] jeon_started → manualmodestart 발효 화면 (AutoModeOver=true)\n");
}

void automodeend_stopbt_event_cb(lv_event_t *e);
void automodeend_load_start_event_cb(lv_event_t *e);
void automodeend_loaded_event_cb(lv_event_t *e);
void automodeend_unload_start_event_cb(lv_event_t *e);
void automodeend_unloaded_event_cb(lv_event_t *e);

/* ── 운전 기록 슬롯 관리 ──────────────────────────────────────────────
 * 슬롯 0이 가장 최신, 슬롯 4가 가장 오래된 기록 (최대 5슬롯)
 * RecordMoveSave(n): 슬롯 n-1→n, ..., 0→1 순서로 시프트
 * RecordSave(0)    : 현재 운전 데이터를 슬롯 0에 기록
 * ─────────────────────────────────────────────────────────────────── */
static const char * const k_rec_keys[] = {
    "RecordFreezeTemp",
    "RecordFreezeTimeHour",
    "RecordFreezeTimeMin",
    "RecordDeFreezeTemp",
    "RecordDeFreezeTimeHour",
    "RecordDeFreezeTimeMin",
    "RecordFermentation1Temp",
    "RecordFermentation1Humidity",
    "RecordFermentation1TimeHour",
    "RecordFermentation1TimeMin",
    "RecordFermentation2Temp",
    "RecordFermentation2Humidity",
    "RecordFermentation2TimeHour",
    "RecordFermentation2TimeMin",
    "RecordStartMonth",
    "RecordStartDay",
    "RecordStartHour",
    "RecordStartMin",
    "RecordEndMonth",
    "RecordEndDay",
    "RecordEndHour",
    "RecordEndMin",
};
#define REC_KEY_N  ((int)(sizeof(k_rec_keys)/sizeof(k_rec_keys[0])))

static void _record_move_save(int max_slot)
{
    char fk[48], tk[48];
    for (int i = max_slot; i > 0; i--) {
        for (int k = 0; k < REC_KEY_N; k++) {
            snprintf(fk, sizeof(fk), "%s%d", k_rec_keys[k], i - 1);
            snprintf(tk, sizeof(tk), "%s%d", k_rec_keys[k], i);
            settings_set_str(tk, settings_get_str(fk));
        }
    }
}

static void _record_save_slot0(device_state_t *state)
{
    char buf[16];
#define RS(key, val) do { snprintf(buf, sizeof(buf), "%02d", (val)); \
                          settings_set_str(key, buf); } while(0)

    /* 정전복구 운전: send_*_hour/min은 _blackout_recovery()가 잔여분으로 덮어씀.
     * bo_*_total_min이 원래 설정 전체 시간 — 기록에는 원래 시간을 저장. */
    bool bo = state->black_out_checking;
    int frz_h = (bo && state->bo_freeze_total_min  > 0) ? state->bo_freeze_total_min  / 60 : state->send_freeze_hour;
    int frz_m = (bo && state->bo_freeze_total_min  > 0) ? state->bo_freeze_total_min  % 60 : state->send_freeze_min;
    int dfr_h = (bo && state->bo_defrost_total_min > 0) ? state->bo_defrost_total_min / 60 : state->send_defreeze_hour;
    int dfr_m = (bo && state->bo_defrost_total_min > 0) ? state->bo_defrost_total_min % 60 : state->send_defreeze_min;
    int fm1_h = (bo && state->bo_ferm1_total_min   > 0) ? state->bo_ferm1_total_min   / 60 : state->send_ferm1_hour;
    int fm1_m = (bo && state->bo_ferm1_total_min   > 0) ? state->bo_ferm1_total_min   % 60 : state->send_ferm1_min;
    int fm2_h = (bo && state->bo_ferm2_total_min   > 0) ? state->bo_ferm2_total_min   / 60 : state->send_ferm2_hour;
    int fm2_m = (bo && state->bo_ferm2_total_min   > 0) ? state->bo_ferm2_total_min   % 60 : state->send_ferm2_min;

    RS("RecordFreezeTemp0",            state->send_freeze_temp);
    RS("RecordFreezeTimeHour0",        frz_h);
    RS("RecordFreezeTimeMin0",         frz_m);
    RS("RecordDeFreezeTemp0",          state->send_defreeze_temp);
    RS("RecordDeFreezeTimeHour0",      dfr_h);
    RS("RecordDeFreezeTimeMin0",       dfr_m);
    RS("RecordFermentation1Temp0",     state->send_ferm1_temp);
    RS("RecordFermentation1Humidity0", state->send_ferm1_humidity);
    RS("RecordFermentation1TimeHour0", fm1_h);
    RS("RecordFermentation1TimeMin0",  fm1_m);
    RS("RecordFermentation2Temp0",     state->send_ferm2_temp);
    RS("RecordFermentation2Humidity0", state->send_ferm2_humidity);
    RS("RecordFermentation2TimeHour0", fm2_h);
    RS("RecordFermentation2TimeMin0",  fm2_m);

    /* 시작시간: saveChecking==1이면 origin 값, 아니면 send_start_* */
    if (atoi(settings_get_str("saveChecking")) == 1) {
        settings_set_str("RecordStartMonth0", settings_get_str("originMonth"));
        settings_set_str("RecordStartDay0",   settings_get_str("originDay"));
        settings_set_str("RecordStartHour0",  settings_get_str("originHour"));
        settings_set_str("RecordStartMin0",   settings_get_str("originMin"));
    } else {
        RS("RecordStartMonth0", state->send_start_month);
        RS("RecordStartDay0",   state->send_start_day);
        RS("RecordStartHour0",  state->send_start_hour);
        RS("RecordStartMin0",   state->send_start_min);
    }

    /* 완료시간 */
    settings_set_str("RecordEndMonth0", settings_get_str("CurrentCompleteMonth"));
    settings_set_str("RecordEndDay0",   settings_get_str("CurrentCompleteDay"));
    settings_set_str("RecordEndHour0",  settings_get_str("CurrentCompleteHour"));
    settings_set_str("RecordEndMin0",   settings_get_str("CurrentCompleteMin"));

#undef RS

    settings_set_str("SaveWriting", "1");  /* 중복 저장 방지 */
}

/* ── 과발효방지 사전 처리 (lv_scr_load 전 automodestart_cb.c에서 호출)
 * 부저+기록저장을 SCREEN_LOAD_START 콜백 밖에서 실행 → 이벤트 콜백 중
 * flash 기록으로 인한 크래시/무응답 방지 */
void automodeend_save_record_and_buzzer(device_state_t *state)
{
    hal_buzzer_complete();
    if (atoi(settings_get_str("SaveWriting")) == 0) {
        char s0[32]={0}, s1[32]={0}, s2[32]={0}, s3[32]={0};
        const char *_p;
        _p = settings_get_str("RecordFreezeTemp0"); if (_p) strncpy(s0,_p,sizeof(s0)-1);
        _p = settings_get_str("RecordFreezeTemp1"); if (_p) strncpy(s1,_p,sizeof(s1)-1);
        _p = settings_get_str("RecordFreezeTemp2"); if (_p) strncpy(s2,_p,sizeof(s2)-1);
        _p = settings_get_str("RecordFreezeTemp3"); if (_p) strncpy(s3,_p,sizeof(s3)-1);
        bk_printf(TAG "[OVER_FERM][REC] s0='%s' s1='%s' s2='%s' s3='%s'\n", s0,s1,s2,s3);
        if      (s0[0]=='\0') { }
        else if (s1[0]=='\0') { _record_move_save(1); }
        else if (s2[0]=='\0') { _record_move_save(2); }
        else if (s3[0]=='\0') { _record_move_save(3); }
        else                  { _record_move_save(4); }
        _record_save_slot0(state);
        settings_save_dirty();
        bk_printf(TAG "[OVER_FERM] 기록 저장 완료\n");
    }
}

/* ── Stop 버튼 ────────────────────────────────────────────────────── */
void automodeend_stopbt_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    device_state_t *state = &g_device_state;
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    if (lv_tick_elaps(last_click_time) < 250) return;
    last_click_time = lv_tick_get();

    if (bk_ui->automode == NULL || !lv_obj_is_valid(bk_ui->automode))
        init_page_automode(bk_ui);
    lv_scr_load(bk_ui->automode);

    /* 과발효방지 타이머 정리 */
    if (s_over_ferm_timer) {
        lv_timer_delete(s_over_ferm_timer);
        s_over_ferm_timer = NULL;
    }

    state->operation              = false;
    state->start_run              = true;
    state->first_start            = false;
    state->auto_mode_start        = false;
    state->auto_mode_over         = false;
    state->over_ferm_active       = false;
    state->over_ferm_jeon_started = false;
    state->manual_current_mode    = 0;
    state->black_out_checking     = false;
    state->bo_freeze_total_min    = 0;
    state->bo_defrost_total_min   = 0;
    state->bo_ferm1_total_min     = 0;
    state->bo_ferm2_total_min     = 0;

    settings_set_str("saveChecking", "0");
    settings_set_str("SaveWriting",  "0");
    settings_save_dirty();
}

/* ── 화면 로드 ────────────────────────────────────────────────────── */
void automodeend_loaded_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    ui_title_anim(bk_ui->automodeend_title);
}

void automodeend_unload_start_event_cb(lv_event_t *e)
{
    (void)e;
}

void automodeend_unloaded_event_cb(lv_event_t *e)
{
    (void)e;
}

void automodeend_load_start_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    device_state_t *state = &g_device_state;

    /* 영어인 경우 완료시간 레이블 15px 우측 이동 */
    {
        lv_coord_t _dx = (settings_get_int("LANGUAGE") == 2) ? 15 : 0;
        lv_obj_set_pos(bk_ui->automodeend_AutoModeCompleteYear,  419 + _dx, 360);
        lv_obj_set_pos(bk_ui->automodeend_AutoModeCompleteMonth, 535 + _dx, 360);
        lv_obj_set_pos(bk_ui->automodeend_AutoModeCompleteDay,   603 + _dx, 360);
        lv_obj_set_pos(bk_ui->automodeend_AutoModeCompleteHour,  687 + _dx, 360);
        lv_obj_set_pos(bk_ui->automodeend_AutoModeCompleteMin,   751 + _dx, 360);
    }

    /* Android AutoModeEndFragment 와 동일:
     * over_ferm_active=true → 부저 + 기록저장(무조건) + bgi1 배경 + 카운트다운 타이머
     * MCU 0x34 수신 시 타이머에서 automodestart 복귀 */
    /* 과발효방지: 부저+기록저장은 automodestart_cb.c에서 lv_scr_load 전에 처리됨
     * (automodeend_save_record_and_buzzer 호출) — 여기서는 UI만 설정 */
    if (state->over_ferm_active) {
        ui_lang_apply_automodeend(bk_ui);
        {
            int _l = settings_get_int("LANGUAGE");
            const char *_ls = (_l == 1) ? "_china" : (_l == 2) ? "_english" : "";
            char _bg1[128];
            snprintf(_bg1, sizeof(_bg1), "/images/auto_mode_end_bgi1%s.jpg", _ls);
            _img_set_src_timed(bk_ui->automodeend_auto_mode_end_bg, _bg1);
        }
        int  over_min_w = atoi(settings_get_str("DetailOverFermentation"));
        s_over_ferm_start_tick = lv_tick_get();
        s_over_ferm_total_ms   = (uint32_t)over_min_w * 60000u;
        char buf_w[16];
        snprintf(buf_w, sizeof(buf_w), "%d", over_min_w);
        lv_label_set_text(bk_ui->automodeend_auto_mode_end_overtime, buf_w);
        {
            /* 언어별 위치를 화면 진입 시 한 번만 설정 — 이후 카운트다운 중에는
             * _over_ferm_wait_timer_cb가 텍스트만 갱신하고 위치는 건드리지 않음 */
            int _l = settings_get_int("LANGUAGE");
            int _ox = 277, _oy = 470+5;
            if (_l == 1) _ox = 288, _oy = 475; /* 한자: 3px 아래 */
            else if (_l == 2) _ox = 630, _oy = 473; /* 영어: x +200 */
            lv_obj_set_pos(bk_ui->automodeend_auto_mode_end_overtime, _ox, _oy);
        }
        lv_label_set_text(bk_ui->automodeend_AutoModeCompleteYear,  settings_get_str("CurrentCompleteYear"));
        lv_label_set_text(bk_ui->automodeend_AutoModeCompleteMonth, settings_get_str("CurrentCompleteMonth"));
        lv_label_set_text(bk_ui->automodeend_AutoModeCompleteDay,   settings_get_str("CurrentCompleteDay"));
        lv_label_set_text(bk_ui->automodeend_AutoModeCompleteHour,  settings_get_str("CurrentCompleteHour"));
        lv_label_set_text(bk_ui->automodeend_AutoModeCompleteMin,   settings_get_str("CurrentCompleteMin"));
        if (s_over_ferm_timer) lv_timer_delete(s_over_ferm_timer);
        s_over_ferm_timer = lv_timer_create(_over_ferm_wait_timer_cb, 500, NULL);
        bk_printf(TAG "[OVER_FERM] automodeend 대기 중 (over_min=%d분, MCU 0x34 대기)\n", over_min_w);
        return;
    }

    /* 언어별 이미지/텍스트 적용 (배경은 여기서 기본값 설정됨) */
    ui_lang_apply_automodeend(bk_ui);

    /* ① 완료 부저 — 10회 (Android BuzzerCompleteRunnable 동일) */
    hal_buzzer_complete();

    /* ② 과발효(DetailOverFermentation) 체크 → 배경 + OverTime 텍스트
     *   조건: DetailOverFermentationOnOff==ON AND dayPeriod>1 AND 값>0 */
    bool over_on  = (strcmp(settings_get_str("DetailOverFermentationOnOff"), "ON") == 0);
    int  over_min = atoi(settings_get_str("DetailOverFermentation"));
    bool show_over = (over_on && state->day_period > 1 && over_min > 0);

    if (show_over) {
        /* 과발효 경고 배경 (bgi1) */
        {
            int _l = settings_get_int("LANGUAGE");
            const char *_ls = (_l == 1) ? "_china" : (_l == 2) ? "_english" : "";
            char _bg1[128];
            snprintf(_bg1, sizeof(_bg1), "/images/auto_mode_end_bgi1%s.jpg", _ls);
            _img_set_src_timed(bk_ui->automodeend_auto_mode_end_bg, _bg1);
        }
        char buf[16];
        snprintf(buf, sizeof(buf), "%d", over_min);
        lv_label_set_text(bk_ui->automodeend_auto_mode_end_overtime, buf);
        {
            int _l = settings_get_int("LANGUAGE");
            int _ox = 277, _oy = 470;
            if (_l == 1) _ox = 288, _oy = 471; /* 한자: 3px 아래 */
            else if (_l == 2) _ox = 630, _oy = 473; /* 영어: x +200 */
            lv_obj_set_pos(bk_ui->automodeend_auto_mode_end_overtime, _ox, _oy);
        }
    } else {
        /* 정상 완료 배경 (bgi) — ui_lang이 이미 설정했으므로 overtime만 클리어 */
        lv_label_set_text(bk_ui->automodeend_auto_mode_end_overtime, "");
          {
            int _l = settings_get_int("LANGUAGE");
            int _ox = 277, _oy = 470;
            if (_l == 1) _ox = 288, _oy = 471; /* 한자: 3px 아래 */
            else if (_l == 2) _ox = 630, _oy = 473; /* 영어: x +200 */
            lv_obj_set_pos(bk_ui->automodeend_auto_mode_end_overtime, _ox, _oy);
        }
    }

    /* ③ 정전 복구(blackout) 표시 */
    if (state->black_out_checking) {
        state->start_run = true;
        state->operation = true;
        _img_ensure_src(bk_ui->automodeend_blackout);
        lv_obj_clear_flag(bk_ui->automodeend_blackout, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(bk_ui->automodeend_blackout, LV_OBJ_FLAG_HIDDEN);
    }

    /* ④ 완료 날짜/시간 레이블 */
    lv_label_set_text(bk_ui->automodeend_AutoModeCompleteYear,  settings_get_str("CurrentCompleteYear"));
    lv_label_set_text(bk_ui->automodeend_AutoModeCompleteMonth, settings_get_str("CurrentCompleteMonth"));
    lv_label_set_text(bk_ui->automodeend_AutoModeCompleteDay,   settings_get_str("CurrentCompleteDay"));
    lv_label_set_text(bk_ui->automodeend_AutoModeCompleteHour,  settings_get_str("CurrentCompleteHour"));
    lv_label_set_text(bk_ui->automodeend_AutoModeCompleteMin,   settings_get_str("CurrentCompleteMin"));

    /* ⑤ 원본 완료시간 영구 저장 */
    settings_set_str("originCompleteYear",  settings_get_str("CurrentCompleteYear"));
    settings_set_str("originCompleteMonth", settings_get_str("CurrentCompleteMonth"));
    settings_set_str("originCompleteDay",   settings_get_str("CurrentCompleteDay"));
    settings_set_str("originCompleteHour",  settings_get_str("CurrentCompleteHour"));
    settings_set_str("originCompleteMin",   settings_get_str("CurrentCompleteMin"));

    /* ⑥ 운전 기록 저장 — SaveWriting==0 인 경우만 (중복 방지)
     *   빈 슬롯을 찾아 기존 기록을 시프트한 뒤 슬롯 0에 저장
     *   settings_get_str이 내부 정적 버퍼를 반환할 수 있으므로 로컬 배열에 복사 */
    if (atoi(settings_get_str("SaveWriting")) == 0) {
        char s0[32] = {0}, s1[32] = {0}, s2[32] = {0}, s3[32] = {0};
        const char *_p;
        _p = settings_get_str("RecordFreezeTemp0"); if (_p) strncpy(s0, _p, sizeof(s0)-1);
        _p = settings_get_str("RecordFreezeTemp1"); if (_p) strncpy(s1, _p, sizeof(s1)-1);
        _p = settings_get_str("RecordFreezeTemp2"); if (_p) strncpy(s2, _p, sizeof(s2)-1);
        _p = settings_get_str("RecordFreezeTemp3"); if (_p) strncpy(s3, _p, sizeof(s3)-1);

        bk_printf(TAG "[REC] SaveWriting=0 s0='%s' s1='%s' s2='%s' s3='%s'\n", s0, s1, s2, s3);

        if      (s0[0] == '\0') { /* 슬롯 0 비어있음 — 시프트 불필요 */ }
        else if (s1[0] == '\0') { _record_move_save(1); }
        else if (s2[0] == '\0') { _record_move_save(2); }
        else if (s3[0] == '\0') { _record_move_save(3); }
        else                    { _record_move_save(4); } /* 모두 참 → 슬롯4 덮어씀 */

        _record_save_slot0(state);
        settings_save_dirty();

        /* 운전기록 저장 완료 후 정전 복구 플래그 초기화.
         * _go_to_automodeend()에서 미리 클리어하면 _record_save_slot0이
         * bo_*_total_min(원래 설정 시간)을 참조하지 못하므로 여기서 클리어. */
        state->black_out_checking   = false;
        state->bo_freeze_total_min  = 0;
        state->bo_defrost_total_min = 0;
        state->bo_ferm1_total_min   = 0;
        state->bo_ferm2_total_min   = 0;
    }
}
