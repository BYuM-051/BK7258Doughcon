/*
 * main_activity.c
 * MainActivity.java → C 변환
 *
 * Java 원본: com.example.tubroair.MainActivity (1644줄)
 *
 * 매핑 규칙:
 *   mainApplication.settingData.findSetting("X").getValue()  → settings_get_str("X")
 *   mainApplication.settingData.findSetting("X").setValue(Y) → settings_set_str("X", Y)
 *   mainApplication.settingData.saveAll()                    → settings_save_all()
 *   mainApplication.settingData.saveAllReset()               → settings_save_all_reset()
 *   new Thread(new BuzzerRunnable(...)).start()              → hw_buzzer_beep()
 *   mainApplication.ledLamp_control(x)                       → hw_led_lamp_control(x)
 *   mainApplication.ledLock_control(x)                       → hw_led_lock_control(x)
 *   mainApplication.ledPower_control(x)                      → hw_led_power_control(x)
 *   mainApplication.backlight(x)                             → hw_backlight_set(x)
 *   System.exit(0) + AlarmManager restart                    → hw_system_restart()
 *   GregorianCalendar                                        → struct tm / time_t (<time.h>)
 *   System.currentTimeMillis()                               → lv_tick_get() [ms]
 *   replaceFragment(XxxFragment.newInstance())               → _load_screen(SCR_XXX)
 */

#include "main_activity.h"
#include "settings.h"
#include "beken_ui.h"
#include "custom_func.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include "hardware_hal.h"
#include "device_state.h"
#include "uart_comm.h"
#include "lv_vendor.h"
#include <os/os.h>

/* CN2 확장보드 Lock/Power/Lamp 물리 키 — 공식 key 컴포넌트(CONFIG_BUTTON,
 * multi_button 기반)로 GPIO 27/28/29(회로도 원본 배정, active-low)를 구동.
 * 자체 폴링/인터럽트 방식은 이 핀 그룹에서 동작하지 않는 것으로 확인되어
 * 공식 컴포넌트로 전환. Long-press 판정은 multi_button의 LONG_TICKS
 * (=3000ms, multi_button.h)를 그대로 사용 — 기존 3초 기준과 정확히 일치. */
#include "key_adapter.h"

static void _toggle_lock(void);
static void _toggle_lamp(void);
static void _screen_toggle(void);
static void _power_long_reset(void);

#define EVT_LOCK_SHORT   (USER_EVENT_START + 0)
#define EVT_POWER_SHORT  (USER_EVENT_START + 1)
#define EVT_LAMP_SHORT   (USER_EVENT_START + 2)
#define EVT_POWER_LONG   (USER_EVENT_START + 3)

static KeyConfig_t s_key_configs[3] = {
    { 27, 0, EVT_LOCK_SHORT,  EVENT_NONE, EVENT_NONE,     EVENT_NONE },
    { 28, 0, EVT_POWER_SHORT, EVENT_NONE, EVT_POWER_LONG, EVENT_NONE },
    { 29, 0, EVT_LAMP_SHORT,  EVENT_NONE, EVENT_NONE,     EVENT_NONE },
};

/* 공식 key 컴포넌트는 자체 스레드(key_thread, key_adapter.c의 rtos_create_thread)에서
 * 이 핸들러를 호출한다. LVGL은 스레드 세이프하지 않아서 _screen_toggle()의
 * _load_screen()(lv_scr_load/lv_obj_* 조작)을 락 없이 호출하면 다른 스레드(LVGL
 * 렌더링 태스크)와 동시접근이 발생해 lv_obj_get_screen assert로 크래시한다
 * (실제로 재현됨). beken_ui.c 부팅 시퀀스와 동일하게 lv_vendor_disp_lock/unlock
 * 으로 감싸 LVGL 접근을 직렬화한다. */
static void _key_event_handler(uint8_t event)
{
    lv_vendor_disp_lock();
    switch (event) {
        case EVT_LOCK_SHORT:  _toggle_lock();       break;
        case EVT_POWER_SHORT: _screen_toggle();      break;
        case EVT_POWER_LONG:  _power_long_reset();   break;
        case EVT_LAMP_SHORT:  _toggle_lamp();        break;
        default: printf("[KEY] unknown key-component event: %d\n", event); break;
    }
    lv_vendor_disp_unlock();
}

static void _key_driver_start(void)
{
    bk_key_register_event_handler(_key_event_handler);
    bk_key_driver_init(s_key_configs, 3);
    printf("[KEY] bk_key_driver_init done (GPIO 27/28/29, active_level=0)\n");
}


// char* DetailPassword = "0603";
// char* NeurosysPassword = "71960";

/* ── 전역 상태 객체 ─────────────────────────────────────────────── */
main_activity_t g_main_activity;

extern bk_lv_ui_t bk_lv_tool_ui;

/* ══════════════════════════════════════════════════════════════════
 * 내부 헬퍼
 * ══════════════════════════════════════════════════════════════════ */

/* settings_get_str() 를 int로 읽는 편의 매크로 */
static inline int _sget_int(const char *key)
{
    const char *v = settings_get_str(key);
    return (v && v[0]) ? atoi(v) : 0;
}

/* int → settings_set_str */
static inline void _sset_int(const char *key, int val)
{
    char buf[16];
    snprintf(buf, sizeof(buf), "%d", val);
    settings_set_str(key, buf);
}

/* Scale 1024x600 design → 800x480 display */
static void _apply_ui_scale(lv_obj_t *scr)
{
    // lv_obj_set_style_transform_scale_x(scr, 200, 0);
    // lv_obj_set_style_transform_scale_y(scr, 204, 0);
    // lv_obj_set_style_transform_pivot_x(scr, 0, 0);
    // lv_obj_set_style_transform_pivot_y(scr, 0, 0);
}

/* 화면 전환: screen_id → init_page + lv_scr_load */
static void _load_screen(int screen_id)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    uint32_t _ls_t = lv_tick_get();
    printf("[SCREEN] _load_screen(%d) start\n", screen_id);
    switch (screen_id) {
        case SCR_MAIN:
            init_page_main(bk_ui);
            lv_scr_load(bk_ui->main);
            _apply_ui_scale(bk_ui->main);
            break;
        // case SCR_MAIN_CHINA:
        //     init_page_mainchina(bk_ui);
        //     lv_scr_load(bk_ui->mainchina);
        //     break;
        // case SCR_MAIN_ENGLISH:
        //     init_page_mainenglish(bk_ui);
        //     lv_scr_load(bk_ui->mainenglish);
            // break;
        case SCR_AUTOMODE_START:
            init_page_automodestart(bk_ui);
            lv_scr_load(bk_ui->automodestart);
            _apply_ui_scale(bk_ui->automodestart);
            break;
        case SCR_AUTOMODE_END:
            init_page_automodeend(bk_ui);
            lv_scr_load(bk_ui->automodeend);
            _apply_ui_scale(bk_ui->automodeend);
            break;
        case SCR_AUTODRYMODE:
            init_page_autodrymode(bk_ui);
            lv_scr_load(bk_ui->autodrymode);
            _apply_ui_scale(bk_ui->autodrymode);
            break;
        case SCR_MANUAL_START:
            init_page_manualmodestart(bk_ui);
            lv_scr_load(bk_ui->manualmodestart);
            _apply_ui_scale(bk_ui->manualmodestart);
            break;
        case SCR_MANUAL_MODE:
            init_page_manualmode(bk_ui);
            lv_scr_load(bk_ui->manualmode);
            _apply_ui_scale(bk_ui->manualmode);
            break;
        default:
            break;
    }
    printf("[SCREEN] _load_screen(%d) done (init+scr_load): %lu ms\n", screen_id, lv_tick_elaps(_ls_t));
}

/*
 * Lamp 토글 — LED 점등=고내 LED 부하 ON, 소등=OFF. 기본값 OFF.
 */
static void _toggle_lamp(void)
{
    main_activity_t *ma = &g_main_activity;
    hal_buzzer_beep();
    if (ma->lamp == 0x01) {
        hal_led_lamp_set(false);
        ma->lamp = 0x00;
    } else {
        hal_led_lamp_set(true);
        ma->lamp = 0x01;
    }
    /* 실제 고내등(실내등) 하드웨어는 AP의 GPIO가 아니라 MCU가 구동함 — 이 값을
     * device_state_t.lamp에도 반영해야 uart_comm.c의 0x33 STATUS 패킷
     * payload[4]에 실려 MCU로 전송된다 (Android SerialComm.java:765,1095
     * writebuffer[4]=mainActivity.Lamp와 동일한 실내등 제어 경로).
     * ma->lamp만 갱신하면 패널의 표시용 LED만 바뀌고 실제 고내등은 안 바뀐다. */
    g_device_state.lamp = ma->lamp;
    /* payload[4]는 원래 1000ms 주기 0x33 STATUS TX에 실려야 MCU로 전달되는데,
     * 그 대기 때문에 "고내 LED 반응이 느리다"는 현장 피드백이 있었다 (PCB TACT
     * SWITCH 자체 지연이 아니라 UART 주기 지연). 다음 TX를 즉시 앞당겨 체감
     * 반응속도를 개선한다. */
    uart_comm_trigger_immediate_tx();
    printf("[KEY] LAMP  toggle -> %s\n", (ma->lamp == 0x01) ? "ON" : "OFF");
}

/*
 * Lock 토글 — LED 점등=잠김(터치 불가), 소등=해제(터치 가능). 기본값 OFF(해제).
 */
/* 터치 입력장치(GT911) 전체를 켜고 끄는 전역 핸들 — ap/components/lvgl/lvgl_v9/
 * porting/lv_port_indev.c 에서 생성되는 non-static 전역 변수. lock 시 이걸
 * 직접 비활성화해야 화면별로 개별 구현된 터치 가드(main_cb.c 등 일부 화면만
 * state->lock을 체크함 — 전 화면에 적용되지 않음)와 무관하게 모든 화면에서
 * 확실히 터치가 막힌다. */
extern lv_indev_t *indev_touchpad;

static void _toggle_lock(void)
{
    main_activity_t *ma = &g_main_activity;
    hal_buzzer_beep();
    if (ma->lock) {
        hal_led_lock_set(false);
        ma->lock      = false;
        ma->hard_lock = false;
        g_device_state.lock      = false;   /* main_cb.c 등 기존 화면별 가드와 동기화 */
        g_device_state.hard_lock = false;
        /* 큐를 비우는(flush) 방식은 flush 직후에도 HW 인터럽트가 계속 큐를 채울 수
         * 있어 근본적인 경쟁 상태가 남아있었다. 대신 잠금 해제 시 터치 드라이버를
         * 다시 열어(drv_tp_open) 큐 자체를 새로 만든다 — 항상 빈 상태로 시작하므로
         * 잠금 중 눌렸던 stale 터치가 남아있을 수 없다. */
        hal_touch_set_enabled(true);
        lv_indev_enable(indev_touchpad, true);
        /* 그래도 남아있을 수 있는 진행 중 press 상태에 대한 방어선 — 다음 "진짜"
         * release를 볼 때까지는 아무 것도 처리하지 않는다. */
        lv_indev_wait_release(indev_touchpad);
    } else {
        hal_led_lock_set(true);
        ma->lock      = true;
        ma->hard_lock = true;
        g_device_state.lock      = true;
        g_device_state.hard_lock = true;
        /* 잠그는 순간 이미 눌려 있던/처리 중이던 터치를 즉시 무효화 — disable만으로는
         * 진행 중이던 press/click 처리가 끝까지 이어질 수 있어 reset으로 강제 중단 */
        lv_indev_reset(indev_touchpad, NULL);
        lv_indev_enable(indev_touchpad, false);
        /* 터치 드라이버 자체(HW 인터럽트 + 스캔 스레드)를 완전히 닫는다 — 잠금
         * 중에는 터치 좌표가 물리적으로 큐에 쌓일 수 없어, 해제 시 stale 터치가
         * 한꺼번에 처리되는 문제의 근본 원인을 제거한다. */
        hal_touch_set_enabled(false);
    }
    printf("[KEY] LOCK  toggle -> %s\n", ma->lock ? "LOCKED(터치불가)" : "UNLOCKED(터치가능)");
}

/*
 * Power 짧게 누름 — 화면 ON/OFF. Power/정전복구 플래그와 완전히 분리된 단순 토글.
 * LED 점등=화면 ON, 소등=화면 OFF. 기본값 ON. OFF→ON 시 선택화면 바로 표시.
 */
static void _screen_toggle(void)
{
    main_activity_t *ma = &g_main_activity;
    device_state_t *state = &g_device_state;
    hal_buzzer_beep();
    if (ma->screen_on) {
        /* OFF: 실제 운전(자동/수동/자동건조)도 정지시켜 MCU를 op=00(정지/메인메뉴)
         * 상태로 보낸다 — automodestart_startbt_event_cb()(실제 "정지" 버튼)와
         * 동일한 플래그 조합. uart_comm.c의 0x33 STATUS 송신 로직상
         * operation=false면 drive_mode가 자동으로 0(=op 00)이 되므로 별도
         * 커맨드 없이 다음 TX 사이클에 바로 반영된다. */
        state->operation           = false;
        state->start_run           = true;
        state->first_start         = false;
        state->auto_mode_start     = false;
        state->auto_dry_mode_start = false;
        state->manual_start        = false;
        state->manual_current_mode = 0;
        state->black_out_checking  = false;
        settings_set_str("saveChecking", "0");
        settings_save_dirty();

        /* 백라이트부터 끄고(화면 전환이 보이지 않게) 선택화면으로 미리
         * 전환해둔다 — ON 시에는 단순히 백라이트만 켜면 되므로, 정전복구 등
         * 다른 자동 화면전환 로직과 겹칠 타이밍 창이 줄어든다. */
        hal_led_power_set(false);
        hal_backlight_set(0);
        _load_screen(SCR_MAIN);
        ma->screen_on = false;
    } else {
        /* ON: 백라이트만 켠다 — 화면 전환 없음 (OFF 때 이미 선택화면으로 가 있음) */
        hal_led_power_set(true);
        hal_backlight_set(100);
        ma->screen_on = true;
    }
    printf("[KEY] POWER short-press -> screen %s\n", ma->screen_on ? "ON" : "OFF");
}

/*
 * Power 3초 이상 길게 누름 — 어떤 화면/모드에 있든 메인 화면으로 전환한 뒤
 * 재부팅한다. hal_system_restart()가 즉시 재부팅하며 반환하지 않으므로,
 * 화면 전환은 그 직전에 수행해 다음 부팅 때 메인 화면이 먼저 보이게 한다.
 */
static void _power_long_reset(void)
{
    printf("[KEY] POWER long-press -> main screen + reboot\n");

    /* 먼저 메인 화면으로 전환 — 정전 재가동(_blackout_recovery) 로직과 겹치지
     * 않게 한다. */
    hal_backlight_set(100);
    _load_screen(SCR_MAIN);

    /* saveChecking=0으로 정전복구 플래그를 끈다. settings_set_str()+
     * settings_save_dirty()는 실제 flash 쓰기를 uart_comm 백그라운드 태스크로
     * 미루므로(200ms 주기), reboot이 그 전에 실행되면 값이 flash에 반영되지
     * 않아 재부팅 후 saveChecking==1을 그대로 읽어 정전 재가동(운전 화면)으로
     * 복귀해버리는 문제가 있었다. settings_save_all_sync()로 즉시 동기 기록해
     * 다음 부팅 시 확실히 메인 화면으로 오게 한다. */
    settings_set_str("saveChecking", "0");
    settings_set_str("Power", "0");
    settings_save_all_sync();

    hal_system_restart();
}

/*
 * F3/F4/F5 공통 처리 (onKeyUp, 팝업 키 핸들러 공유) — press_start_time 기반
 * 눌린 시간으로 짧게/길게를 직접 판별하는 경로 (콜백 방식 키 컴포넌트가 아닌
 * 팝업 등에서 여전히 사용).
 */
static void _handle_key_up(int key_code)
{
    main_activity_t *ma = &g_main_activity;

    switch (key_code) {
        case 0xF5: /* KEYCODE_F5: 램프 토글 */
            _toggle_lamp();
            break;

        case 0xF4: /* KEYCODE_F4: 잠금 토글 */
            _toggle_lock();
            break;

        case 0xF3: { /* KEYCODE_F3: 화면 ON/OFF 토글 / 3초 리셋 */
            uint32_t held_ms = lv_tick_elaps(ma->press_start_time);
            ma->press_start_time = 0;

            if (held_ms >= 3000) {
                _power_long_reset();
            } else {
                _screen_toggle();
            }
            break;
        }
    }
}

/* ══════════════════════════════════════════════════════════════════
 * 정전 복구 (saveChecking == 1)
 *   Java: onCreate 내 if(saveChecking==1) 블록 → 별도 함수로 분리
 * ══════════════════════════════════════════════════════════════════ */

static void _blackout_recovery(void)
{
    main_activity_t *ma  = &g_main_activity;
    ma_send_t        *sd = &ma->send;

    int save_op = _sget_int("saveOperationTemp");

    /* FirstOperatorMode 결정: Android 방식 — STATUS(0x33)에서 X0(시작명령)을 보내고
     * payload[19/20]=cur_remain_h/m을 실어 MCU가 잔여시간부터 카운트다운하도록 함.
     * FIRST_START 없이 STATUS X0만 전송 (Android 정전복구와 동일). */
    if      (save_op == 0)  ma->first_operator_mode = 0x10;  /* FREEZE  X0 */
    else if (save_op == 1)  ma->first_operator_mode = 0x20;  /* DEFROST X0 */
    else if (save_op == 2)  ma->first_operator_mode = 0x30;  /* FERM1   X0 */
    else if (save_op == 3)  ma->first_operator_mode = 0x40;  /* FERM2   X0 */
    else if (save_op == 4)  ma->first_operator_mode = 0x50;
    else if (save_op == 7)  ma->first_operator_mode = 0x33;
    else if (save_op == 8)  ma->first_operator_mode = 0x43;
    else if (save_op == 9)  ma->first_operator_mode = 0x34;
    else if (save_op == 10) ma->first_operator_mode = 0x44;

    if (save_op != 4) {
        /* ── 냉동/해동/발효 복구 ─────────────────────────────── */
        sd->freeze_temp    = _sget_int("CurrentSaveFreezeTemp");
        sd->freeze_hour    = _sget_int("CurrentSaveFreezeTimeHour");
        sd->freeze_min     = _sget_int("CurrentSaveFreezeTimeMin");
        sd->unfreeze_temp  = _sget_int("CurrentSaveDefreezeTemp");
        sd->unfreeze_hour  = _sget_int("CurrentSaveDefreezeTimeHour");
        sd->unfreeze_min   = _sget_int("CurrentSaveDefreezeTimeMin");
        sd->ferm1_temp     = _sget_int("CurrentSaveFermentation1Temp");
        sd->ferm1_hum      = _sget_int("CurrentSaveFermentation1Humidity");
        sd->ferm1_hour     = _sget_int("CurrentSaveFermentation1TimeHour");
        sd->ferm1_min      = _sget_int("CurrentSaveFermentation1TimeMin");
        sd->ferm2_temp     = _sget_int("CurrentSaveFermentation2Temp");
        sd->ferm2_hum      = _sget_int("CurrentSaveFermentation2Humidity");
        sd->ferm2_hour     = _sget_int("CurrentSaveFermentation2TimeHour");
        sd->ferm2_min      = _sget_int("CurrentSaveFermentation2TimeMin");

        int total_freeze = sd->freeze_hour * 60 + sd->freeze_min;
        int total_unfreeze = sd->unfreeze_hour * 60 + sd->unfreeze_min;
        int total_ferm1    = sd->ferm1_hour  * 60 + sd->ferm1_min;
        int total_ferm2    = sd->ferm2_hour  * 60 + sd->ferm2_min;

        int cur_remain_h = _sget_int("saveCurrentRemainHour");
        int cur_remain_m = _sget_int("saveCurrentRemainMin");
        int cur_remain   = cur_remain_h * 60 + cur_remain_m; /* 현재 행정 잔여분 */

        /* 재시작 시각 저장 */
        main_activity_save_current_calendar();
        sd->remain_hour = _sget_int("saveRemainHour");
        sd->remain_min  = _sget_int("saveRemainMin");
        int total_remain = sd->remain_hour * 60 + sd->remain_min; /* 전체 잔여분 */

        /* 재시작 시각 (struct tm) */
        struct tm t_now = {0};
        t_now.tm_year  = ma->save_current_year  - 1900;
        t_now.tm_mon   = ma->save_current_month - 1;
        t_now.tm_mday  = ma->save_current_day;
        t_now.tm_hour  = ma->save_current_hour;
        t_now.tm_min   = ma->save_current_min;
        t_now.tm_sec   = 0;
        time_t t_restart = mktime(&t_now);

        /* 원래 완료 시각 */
        struct tm t_origin = {0};
        t_origin.tm_year = _sget_int("originCompleteYear")  - 1900;
        t_origin.tm_mon  = _sget_int("originCompleteMonth") - 1;
        t_origin.tm_mday = _sget_int("originCompleteDay");
        t_origin.tm_hour = _sget_int("originCompleteHour");
        t_origin.tm_min  = _sget_int("originCompleteMin");
        t_origin.tm_sec  = 0;
        time_t t_orig = mktime(&t_origin);

        /* 재시작 + 발효1 + 발효2 후 완료 시각 (초) */
        time_t t_ferm_end = t_restart + (time_t)(total_ferm1 + total_ferm2) * 60;

        /* 원래 완료 - (재시작+발효) = 해동 잔여 가능 시간 */
        double diff_sec  = difftime(t_orig, t_ferm_end);
        int    check_min = (int)(diff_sec / 60.0);

        ma_blackout_t *bo = &ma->blackout;

        if (save_op == 0) { /* 냉동 중 정전 */
            total_remain += cur_remain - total_freeze;
            bo->freeze_hour   = cur_remain_h;
            bo->freeze_min    = cur_remain_m;
            bo->unfreeze_hour = sd->unfreeze_hour;
            bo->unfreeze_min  = sd->unfreeze_min;
            bo->ferm1_hour    = sd->ferm1_hour;
            bo->ferm1_min     = sd->ferm1_min;
            bo->ferm2_hour    = sd->ferm2_hour;
            bo->ferm2_min     = sd->ferm2_min;

        } else if (save_op == 1) { /* 해동 중 정전 */
            if (diff_sec < 0) {
                /* 해동 시간조차 없음 → 냉동·해동 건너뜀 */
                total_remain -= total_freeze + total_unfreeze;
                ma->first_operator_mode = 0x30;  /* FERM1 X0: skip freeze/defrost */
                bo->freeze_hour   = 0; bo->freeze_min   = 0;
                bo->unfreeze_hour = 0; bo->unfreeze_min = 0;
                bo->ferm1_hour    = sd->ferm1_hour; bo->ferm1_min = sd->ferm1_min;
                bo->ferm2_hour    = sd->ferm2_hour; bo->ferm2_min = sd->ferm2_min;
            } else {
                total_remain += check_min - total_freeze - total_unfreeze;
                bo->freeze_hour   = 0; bo->freeze_min   = 0;
                bo->unfreeze_hour = (check_min / 60 < 0) ? 0 : check_min / 60;
                bo->unfreeze_min  = (check_min % 60 < 0) ? 0 : check_min % 60;
                bo->ferm1_hour    = sd->ferm1_hour; bo->ferm1_min = sd->ferm1_min;
                bo->ferm2_hour    = sd->ferm2_hour; bo->ferm2_min = sd->ferm2_min;
            }

        } else if (save_op == 2) { /* 발효1 중 정전 */
            total_remain += cur_remain - total_freeze - total_unfreeze - total_ferm1;
            bo->freeze_hour = 0; bo->freeze_min = 0;
            bo->unfreeze_hour = 0; bo->unfreeze_min = 0;
            // bo->ferm1_hour = cur_remain_h; bo->ferm1_min = cur_remain_m;
            bo->ferm1_hour = sd->ferm1_hour; bo->ferm1_min = sd->ferm1_min;

            bo->ferm2_hour = sd->ferm2_hour; bo->ferm2_min = sd->ferm2_min;

        } else if (save_op == 3) { /* 발효2 중 정전 */
            total_remain += cur_remain - total_freeze - total_unfreeze - total_ferm1 - total_ferm2;
            bo->freeze_hour = 0; bo->freeze_min = 0;
            bo->unfreeze_hour = 0; bo->unfreeze_min = 0;
            bo->ferm1_hour = 0; bo->ferm1_min = 0;
            // bo->ferm2_hour = cur_remain_h; bo->ferm2_min = cur_remain_m;
            bo->ferm2_hour = sd->ferm2_hour; bo->ferm2_min = sd->ferm2_min;

        } else if (save_op == 7 || save_op == 8 ||
                   save_op == 9 || save_op == 10) { /* 과발효방지 포함 완료 */
            total_remain += cur_remain - total_freeze - total_unfreeze - total_ferm1 - total_ferm2;
            bo->freeze_hour = 0; bo->freeze_min = 0;
            bo->unfreeze_hour = 0; bo->unfreeze_min = 0;
            bo->ferm1_hour = 0; bo->ferm1_min = 0;
            bo->ferm2_hour = 0; bo->ferm2_min = 0;
            bo->complete = true;
        }

        /* 새 완료 시각 계산 */
        int tot_h, tot_m;
        if (ma->day_period != 0) {
            tot_h = total_remain / 60; if (tot_h < 0) tot_h = 0;
            tot_m = total_remain % 60; if (tot_m < 0) tot_m = 0;
        } else {
            tot_h = cur_remain_h;
            tot_m = cur_remain_m;
        }
        struct tm t_complete = t_now;
        t_complete.tm_hour += tot_h;
        t_complete.tm_min  += tot_m;
        mktime(&t_complete); /* normalize */

        sd->complete_year  = t_complete.tm_year + 1900;
        sd->complete_month = t_complete.tm_mon  + 1;
        sd->complete_day   = t_complete.tm_mday;
        sd->complete_hour  = t_complete.tm_hour;
        sd->complete_min   = t_complete.tm_min;

        bo->checking     = true;
        bo->checking_cmd = false;

        /* ── device_state에 복구 파라미터 복사 ── */
        g_device_state.send_freeze_temp    = sd->freeze_temp;
        g_device_state.send_freeze_hour    = bo->freeze_hour;
        g_device_state.send_freeze_min     = bo->freeze_min;
        g_device_state.send_defreeze_temp  = sd->unfreeze_temp;
        g_device_state.send_defreeze_hour  = bo->unfreeze_hour;
        g_device_state.send_defreeze_min   = bo->unfreeze_min;
        g_device_state.send_ferm1_temp     = sd->ferm1_temp;
        g_device_state.send_ferm1_humidity = sd->ferm1_hum;
        g_device_state.send_ferm1_hour     = bo->ferm1_hour;
        g_device_state.send_ferm1_min      = bo->ferm1_min;
        g_device_state.send_ferm2_temp     = sd->ferm2_temp;
        g_device_state.send_ferm2_humidity = sd->ferm2_hum;
        g_device_state.send_ferm2_hour     = bo->ferm2_hour;
        g_device_state.send_ferm2_min      = bo->ferm2_min;
        /* TX payload[17/18] elapsed = bo_total - remain.
         * send_freeze_hour = cur_remain(10h26m) ≠ orig_total(14h1m) → elapsed=0 이 되어
         * MCU가 NVRAM total(14h1m)을 기준으로 0부터 카운트다운 → NVRAM 복귀 버그.
         * 원래 설정시간을 별도 보관하여 TX에서 정확한 elapsed를 계산한다. */
        g_device_state.bo_freeze_total_min  = total_freeze;
        g_device_state.bo_defrost_total_min = total_unfreeze;
        g_device_state.bo_ferm1_total_min   = total_ferm1;
        g_device_state.bo_ferm2_total_min   = total_ferm2;
        g_device_state.send_complete_year  = sd->complete_year;
        g_device_state.send_complete_month = sd->complete_month;
        g_device_state.send_complete_day   = sd->complete_day;
        g_device_state.send_complete_hour  = sd->complete_hour;
        g_device_state.send_complete_min   = sd->complete_min;
        /* automodestart/end 화면은 CurrentComplete* settings 키를 직접 읽음.
         * send_complete_* 만 갱신하면 화면 로드 시 정전 전 구 완료시각이 표시되는 버그.
         * _load_screen() 호출 전에 갱신해야 SCREEN_LOAD_START 핸들러가 올바른 값을 얻음. */
        {
            char _buf[8];
            snprintf(_buf, sizeof(_buf), "%04d", sd->complete_year);  settings_set_str("CurrentCompleteYear",  _buf);
            snprintf(_buf, sizeof(_buf), "%02d", sd->complete_month); settings_set_str("CurrentCompleteMonth", _buf);
            snprintf(_buf, sizeof(_buf), "%02d", sd->complete_day);   settings_set_str("CurrentCompleteDay",   _buf);
            snprintf(_buf, sizeof(_buf), "%02d", sd->complete_hour);  settings_set_str("CurrentCompleteHour",  _buf);
            snprintf(_buf, sizeof(_buf), "%02d", sd->complete_min);   settings_set_str("CurrentCompleteMin",   _buf);
        }
        g_device_state.first_operator_mode = ma->first_operator_mode;
        g_device_state.black_out_checking  = true;
        /* payload[19/20] 기준값: saveCurrentRemainHour/Min (현재 행정 잔여시간).
         * 정전 후 완전 재부팅 시 RAM이 0으로 초기화되므로 명시적으로 설정해야 한다.
         * (소프트 리셋은 이전 remain_hour 값이 RAM에 잔류해 우연히 동작하나, 비신뢰성) */
        g_device_state.remain_hour = cur_remain_h;
        g_device_state.remain_min  = cur_remain_m;
        /* payload[1]=day_period: zero-init이면 0 → TX에서 수동모드로 분류(day=0).
         * saveDayPeriod 값을 명시적으로 복사해 오토모드 분기가 유지되도록 함 */
        g_device_state.day_period  = ma->day_period;

        if (ma->day_period == 0) {
            /* 수동 복구: 자동복구와 동일하게 운전 상태로 즉시 재시작 */
            int _mmode = (save_op == 0) ? MANUAL_MODE_FREEZE :
                         (save_op == 1) ? MANUAL_MODE_DEFROST : MANUAL_MODE_FERM;
            ma->manual_current_mode            = _mmode;
            g_device_state.manual_current_mode = _mmode;
            g_device_state.manual_start        = true;
            g_device_state.operation           = true;
            uart_comm_trigger_start_run();
            _load_screen(SCR_MANUAL_START);

        } else {
            /* 자동 복구: AutoStart 표시 문자열 채우기 */
            ma_autostart_str_t *as = &ma->autostart_str;
            zero_add(NULL, as->freeze_temp,    sizeof(as->freeze_temp));    /* placeholder */
            snprintf(as->freeze_temp,    sizeof(as->freeze_temp),    "%02d", sd->freeze_temp);
            snprintf(as->defrost_temp,   sizeof(as->defrost_temp),   "%02d", sd->unfreeze_temp);
            snprintf(as->defrost_hour,   sizeof(as->defrost_hour),   "%02d", bo->unfreeze_hour);
            snprintf(as->defrost_min,    sizeof(as->defrost_min),    "%02d", bo->unfreeze_min);
            snprintf(as->ferm1_temp,     sizeof(as->ferm1_temp),     "%02d", sd->ferm1_temp);
            snprintf(as->ferm1_hum,      sizeof(as->ferm1_hum),      "%02d", sd->ferm1_hum);
            snprintf(as->ferm1_hour,     sizeof(as->ferm1_hour),     "%02d", bo->ferm1_hour);
            snprintf(as->ferm1_min,      sizeof(as->ferm1_min),      "%02d", bo->ferm1_min);
            snprintf(as->ferm2_temp,     sizeof(as->ferm2_temp),     "%02d", sd->ferm2_temp);
            snprintf(as->ferm2_hum,      sizeof(as->ferm2_hum),      "%02d", sd->ferm2_hum);
            snprintf(as->ferm2_hour,     sizeof(as->ferm2_hour),     "%02d", bo->ferm2_hour);
            snprintf(as->ferm2_min,      sizeof(as->ferm2_min),      "%02d", bo->ferm2_min);
            snprintf(as->complete_year,  sizeof(as->complete_year),  "%04d", sd->complete_year);
            snprintf(as->complete_month, sizeof(as->complete_month), "%02d", sd->complete_month);
            snprintf(as->complete_day,   sizeof(as->complete_day),   "%02d", sd->complete_day);
            snprintf(as->complete_hour,  sizeof(as->complete_hour),  "%02d", sd->complete_hour);
            snprintf(as->complete_min,   sizeof(as->complete_min),   "%02d", sd->complete_min);

            if (bo->complete) {
                if (save_op == 9 || save_op == 10) {
                    /* 정전복구: 저온발효(0x34/0x44) 진행 중 → Android AutoModeOver=true 동일
                     * manualmodestart(FERM 모드, StartBt INVISIBLE) 화면으로 복귀 */
                    ma->auto_mode_end                  = false;
                    ma->auto_mode_over                 = true;
                    g_device_state.over_ferm_active    = true;
                    g_device_state.auto_mode_over      = true;
                    g_device_state.manual_current_mode = MANUAL_MODE_FERM2;
                    _load_screen(SCR_MANUAL_START);
                } else if (save_op != 3) {
                    _load_screen(SCR_AUTOMODE_END);
                }
            } else {
                /* 자동 복구 재시작: 통신 즉시 기동 */
                ma->auto_mode = true;
                /* 초기 표시 모드 설정 (MCU 응답 전 UI 선행 표시) */
                switch (ma->first_operator_mode) {
                    case 0x10: g_device_state.current_op_mode = OP_MODE_FREEZE;  break;
                    case 0x20: g_device_state.current_op_mode = OP_MODE_DEFROST; break;
                    case 0x30: case 0x33: g_device_state.current_op_mode = OP_MODE_FERM1; break;
                    case 0x40: case 0x43: g_device_state.current_op_mode = OP_MODE_FERM2; break;
                    default:   g_device_state.current_op_mode = OP_MODE_FREEZE;  break;
                }
                g_device_state.auto_mode_start = true;
                g_device_state.operation       = true;
                /* 정전복구 시퀀스: SETDATA(0x10) 건너뜀 → CONDATA(0x11) → STATUS X0.
                 * uart_comm_init()이 start_run2=true 로 설정하여 CONDATA부터 시작.
                 * CONDATA_ACK 후 start_run=true → STATUS X0 + payload[19/20]=cur_remain_h/m. */
                uart_comm_trigger_start_run();  /* start_run=true; uart_comm_init()이 false로 덮어씀 */
                printf("[BLACKOUT] auto recovery: op=0x%02x CONDATA→STATUS X0\n",
                       ma->first_operator_mode);
                _load_screen(SCR_AUTOMODE_START);
            }
        }

    } else {
        /* ── 건조 복구 ───────────────────────────────────────── */
        ma_send_t     *sd = &ma->send;
        ma_blackout_t *bo = &ma->blackout;

        sd->dry_temp  = _sget_int("CurrentSaveDryTemp");
        sd->dry_hum   = _sget_int("CurrentSaveDryHumidity");
        sd->dry_hour  = _sget_int("CurrentSaveDryTimeHour");
        sd->dry_min   = _sget_int("CurrentSaveDryTimeMin");
        bo->dry_hour  = _sget_int("saveCurrentRemainHour");
        bo->dry_min   = _sget_int("saveCurrentRemainMin");

        bo->checking     = true;
        bo->checking_cmd = false;

        /* CurrentSaveDryTimeHour/Min = 원래 총 설정시간 유지 (덮어쓰지 않음).
         * 복구 중 remain 표시는 _refresh_running_ui_adm이 remain_hour/min에서 직접 처리.
         * 덮어쓰면 복구 완료 후 다음 방문 시 remain 시간이 설정 시간으로 잘못 표시됨. */
        g_device_state.send_dry_temp       = sd->dry_temp;
        g_device_state.send_dry_humidity   = sd->dry_hum;
        /* send_dry_hour = 원래 총 설정시간 (sd->dry_hour: line 520에서 CurrentSaveDryTimeHour
         * 덮어쓰기 전에 읽은 값). bo->dry_hour는 saveCurrentRemainHour(잔여시간)이므로
         * 잔여≠총시간일 때 elapsed = total-remain 계산이 틀린다. */
        g_device_state.send_dry_hour       = sd->dry_hour;
        g_device_state.send_dry_min        = sd->dry_min;
        /* remain을 복원값으로 초기화: blackout 가드가 "MCU값 > cur값"이면 갱신 거부하므로
         * remain=0인 상태로 MCU의 3h5m이 거부되면 UI 타이머가 완료로 오판한다. */
        g_device_state.remain_hour         = (uint8_t)bo->dry_hour;
        g_device_state.remain_min          = (uint8_t)bo->dry_min;
        g_device_state.first_operator_mode = 0x50;
        g_device_state.black_out_checking  = true;
        g_device_state.auto_dry_mode_start = true;
        g_device_state.operation           = true;
        uart_comm_trigger_start_run();
        printf("[BLACKOUT] dry recovery: remain=%dh%dm\n", bo->dry_hour, bo->dry_min);
        _load_screen(SCR_AUTODRYMODE);
    }
}

/* ══════════════════════════════════════════════════════════════════
 * onCreate → main_activity_on_create()
 * ══════════════════════════════════════════════════════════════════ */
void main_activity_on_create(void)
{
    main_activity_t *ma = &g_main_activity;
    memset(ma, 0, sizeof(*ma));

#if !UI_BG_SOLID_COLOR && UI_BG_CANVAS_PRELOAD_ENABLE
    /* ── 0. bg.jpg 버퍼 선점 (malloc만) ─────────────────────────
     * LVGL task 시작 전에 1.06 MB 연속 블록 확보.
     * canvas 생성 + JPEG decode는 _bg_set 첫 호출 시 지연 실행됨 */
    bg_canvas_buf_alloc();
#endif

#if UI_CANVAS_BUF_PERMANENT_ENABLE
    /* reset_popup.png(알파 있는 RGBA) ARGB8888 버퍼(728KB) 선점 — psram_malloc_cm이
     * 실패 시 NULL 반환이 아니라 즉시 assert 크래시하므로, 이 위험한 할당을
     * 단편화가 쌓이기 전인 지금(부팅 극초반)에 미리 끝내둔다. 이후로는 이
     * 버퍼를 재사용만 하고 다시 malloc하지 않는다. */
    popupreset_canvas_buf_alloc();

    /* auto_mode_start_bgi*.jpg(1024x540, RGB565 ~1.06MB) canvas 버퍼도 동일 이유로
     * 부팅 극초반 선점 — 기존엔 automodestart 화면 진입/이탈마다 free+malloc을
     * 반복해 단편화에 가장 취약한 경로였다(teraterm719final.log 자동운전
     * 시작 버튼 클릭 직후 want=1105924 B 크래시로 실측 확인). */
    automodestart_canvas_buf_alloc();

    /* testmode_box.jpg(984x433, RGB565 ~832KB) canvas 버퍼도 동일 이유로 부팅
     * 극초반 선점 — settingmodetest 화면 진입/이탈마다 free+malloc을 반복해
     * 단편화에 취약했다(teraterm719final.log에서 want=852148 B 크래시로 실측 확인,
     * password_popup preload 직후 settingmodetest_bg_preload()에서 발생). */
    settingmodetest_canvas_buf_alloc();
#endif /* UI_CANVAS_BUF_PERMANENT_ENABLE */

    /* password_popup / manualmodestart ferm2 캔버스는 영구 고정을 시도했으나,
     * 실측 결과(4개 버퍼 2.77MB 고정만으로도 free 4.19MB→1.25MB, 공유 캐시 쪽
     * 평범한 아이콘 decode가 단편화로 실패) 여유 공간을 지나치게 압박함이 확인되어
     * 원래 방식(진입 시 alloc, 이탈 시 free)으로 되돌림 — 실제 크래시가 로그로
     * 확인된 reset_popup/automodestart/testmode_box 세 개만 UI_CANVAS_BUF_PERMANENT_ENABLE로
     * 토글 가능하게 남겨둠. */

    /* ── 1. 설정 로드 ─────────────────────────────────────────── */
    ma->language = _sget_int("LANGUAGE");
    ma->mute     = (_sget_int("Mute") == 1);
    ma->degree =0;

    /* ── 2. 초기 상태 플래그 ──────────────────────────────────── */
    ma->exchange_check_true  = false;
    ma->exchange_check_false = false;
    ma->auto_dry_mode_start  = false;
    ma->auto_mode            = false;
    ma->auto_mode_start      = false;
    ma->auto_dry_mode        = false;
    ma->memory_mode_check    = 0;
    ma->manual_current_mode  = 0;
    ma->manual_start         = false;
    ma->test_mode            = false;
    ma->change_setting       = false;
    ma->operation            = false;
    ma->auto_mode_end        = false;
    ma->hard_lock            = false;
    ma->divible_point        = false;
    ma->lamp                 = 0x00;
    ma->lock                 = false;

    /* Lock/Power/Lamp 키+LED 확장보드(CN2) GPIO 설정 — 최초 hal_led_*_set() 호출 전 1회 */
    hal_gpio_init();
    hal_led_lamp_set(false);
    hal_led_lock_set(false);

    ma->startup_time = lv_tick_get();
    hal_buzzer_beep();
    _key_driver_start();   /* Lock/Power/Lamp 물리 키 (공식 key 컴포넌트) */

    /* ── 3. 전원 상태 (정전복구용 플래그 — 화면 ON/OFF LED와는 무관, 그대로 유지) ── */
    if (_sget_int("Power") == 1) {
        ma->power     = true;
        ma->hard_lock = true;
        memset(ma->save_test_test, 0, sizeof(ma->save_test_test));
        ma->start_run = true;
        ma->blackout.checking              = false;
        g_device_state.black_out_checking  = false;
        settings_set_str("saveChecking", "0");
        settings_save_dirty();
    } else {
        ma->power = false;
    }

    /* ── 3b. 화면 ON/OFF (Power/정전복구 플래그와 완전히 분리된 단순 상태) ──
     * 기본값: 화면 ON, LED ON */
    ma->screen_on = true;
    hal_led_power_set(true);
    hal_backlight_set(100);

    /* ── 4. saveChecking 읽기 ─────────────────────────────────── */
    ma->save_checking = _sget_int("saveChecking");

    /* ── 5. 에러 팝업 초기화 (LVGL 객체 생성은 별도 호출) ──────── */
    main_activity_show_connect_error();
    main_activity_show_connect_lost();

    // /* ── 6. 언어별 초기 화면 로드 ────────────────────────────── */
    // if (ma->language == 1)
    //     _load_screen(SCR_MAIN_CHINA);
    // else if (ma->language == 2)
    //     _load_screen(SCR_MAIN_ENGLISH);
    // else
    //     _load_screen(SCR_MAIN);

    /* ── 7. 정전 복구 ─────────────────────────────────────────── */
    if (ma->save_checking == 1) {
        ma->day_period = _sget_int("saveDayPeriod");
        _blackout_recovery();
        settings_save_dirty();  /* CurrentComplete* 갱신 분 즉시 flash 반영 */
    } else {
        ma->start_run1 = true;
        ma->start_run2 = false;
        ma->start_run  = false;
    }
}

/* ══════════════════════════════════════════════════════════════════
 * replaceFragment → main_activity_replace_screen()
 * ══════════════════════════════════════════════════════════════════ */
void main_activity_replace_screen(int screen_id)
{
    main_activity_t *ma = &g_main_activity;
    if (ma->mute) hal_buzzer_beep();
    _load_screen(screen_id);
}

/* ══════════════════════════════════════════════════════════════════
 * onKeyDown → main_activity_on_key_down()
 * ══════════════════════════════════════════════════════════════════ */
void main_activity_on_key_down(int key_code)
{
    main_activity_t *ma = &g_main_activity;
    /* IdleTracker.updateInteractionTime() → TODO: idle_tracker_update() */
    if (key_code == 0xF3) {
        if (ma->press_start_time == 0)
            ma->press_start_time = lv_tick_get();
    }
}

/* ══════════════════════════════════════════════════════════════════
 * onKeyUp → main_activity_on_key_up()
 * ══════════════════════════════════════════════════════════════════ */
void main_activity_on_key_up(int key_code)
{
    _handle_key_up(key_code);
}

/* ══════════════════════════════════════════════════════════════════
 * connecterror() → 통신 NACK/체크섬 오류 팝업
 *
 * Java AlertDialog → LVGL lv_msgbox (투명 배경, y=19)
 * 팝업이 뜬 상태에서도 F3/F4/F5는 _handle_key_up 으로 처리됨
 * divible_point 로직: 팝업 위에서 첫 키는 무시, 두 번째부터 처리
 * ══════════════════════════════════════════════════════════════════ */
// LV_KEY_F3
static void _connect_error_key_cb(lv_event_t *e)
{
    main_activity_t *ma = &g_main_activity;
    if (lv_event_get_code(e) != LV_EVENT_KEY) return;

    uint32_t key = lv_event_get_key(e);
    bool is_up = (lv_indev_get_state(lv_indev_active()) != LV_INDEV_STATE_PRESSED);

    if (!is_up) {
        /* KEY_DOWN: F3 시작 시간 기록 */
        if (key == 0xF3 && ma->press_start_time == 0)
            ma->press_start_time = lv_tick_get();
        return;
    }
    /* KEY_UP: divible_point 로직 */
    if (ma->divible_point) {
        ma->divible_point = false;
        _handle_key_up((int)key);
    } else {
        ma->divible_point = true;
    }
}

void main_activity_show_connect_error(void)
{
    main_activity_t *ma = &g_main_activity;
    if (ma->connect_error_popup) return; /* 이미 존재 */

    /* 언어별 메시지 */
    const char *msg;
    if (ma->language == 1)      msg = "통신 오류 (중문)";
    else if (ma->language == 2) msg = "Connection Error";
    else                        msg = "통신 오류";

    lv_obj_t *popup = lv_obj_create(lv_layer_top());
    lv_obj_set_size(popup, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_pos(popup, 0, 19);
    lv_obj_set_style_bg_opa(popup, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(popup, 0, 0);

    lv_obj_t *label = lv_label_create(popup);
    lv_label_set_text(label, msg);

    lv_obj_add_event_cb(popup, _connect_error_key_cb, LV_EVENT_KEY, NULL);
    lv_obj_add_flag(popup, LV_OBJ_FLAG_HIDDEN); /* 처음엔 숨김 */

    ma->connect_error_popup = popup;
}

void main_activity_hide_connect_error(void)
{
    main_activity_t *ma = &g_main_activity;
    if (ma->connect_error_popup)
        lv_obj_add_flag(ma->connect_error_popup, LV_OBJ_FLAG_HIDDEN);
}

/* ══════════════════════════════════════════════════════════════════
 * madealert() → 통신 단절 팝업 (cancelable=false)
 * ══════════════════════════════════════════════════════════════════ */

static void _connect_popup_key_cb(lv_event_t *e)
{
    main_activity_t *ma = &g_main_activity;
    if (lv_event_get_code(e) != LV_EVENT_KEY) return;

    uint32_t key   = lv_event_get_key(e);
    bool     is_up = (lv_indev_get_state(lv_indev_active()) != LV_INDEV_STATE_PRESSED);

    if (!is_up) {
        if (key == 0xF3 && ma->press_start_time == 0)
            ma->press_start_time = lv_tick_get();
        return;
    }
    if (ma->divible_point) {
        ma->divible_point = false;
        _handle_key_up((int)key);
    } else {
        ma->divible_point = true;
    }
}

void main_activity_show_connect_lost(void)
{
    main_activity_t *ma = &g_main_activity;
    if (ma->connect_popup) return;

    const char *msg;
    if (ma->language == 1)      msg = "통신 단절 (중문)";
    else if (ma->language == 2) msg = "Connection Lost";
    else                        msg = "통신 단절";

    lv_obj_t *popup = lv_obj_create(lv_layer_top());
    lv_obj_set_size(popup, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_pos(popup, 0, 19);
    lv_obj_set_style_bg_opa(popup, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(popup, 0, 0);

    lv_obj_t *label = lv_label_create(popup);
    lv_label_set_text(label, msg);

    lv_obj_add_event_cb(popup, _connect_popup_key_cb, LV_EVENT_KEY, NULL);
    lv_obj_add_flag(popup, LV_OBJ_FLAG_HIDDEN);

    ma->connect_popup = popup;
}

void main_activity_hide_connect_lost(void)
{
    main_activity_t *ma = &g_main_activity;
    if (ma->connect_popup)
        lv_obj_add_flag(ma->connect_popup, LV_OBJ_FLAG_HIDDEN);
}

/* ══════════════════════════════════════════════════════════════════
 * saveCurrentCalendar() → main_activity_save_current_calendar()
 * ══════════════════════════════════════════════════════════════════ */
void main_activity_save_current_calendar(void)
{
    main_activity_t *ma = &g_main_activity;
    /* BK7258: time(NULL)/localtime()은 HW RTC와 동기화되지 않아 1970 반환.
     * hal_rtc_get()으로 직접 읽어야 정확한 현재 시각을 얻을 수 있음. */
    int y, mo, d, h, mi, s;
    hal_rtc_get(&y, &mo, &d, &h, &mi, &s);
    ma->save_current_year  = y;
    ma->save_current_month = mo;
    ma->save_current_day   = d;
    ma->save_current_hour  = h;
    ma->save_current_min   = mi;
}

