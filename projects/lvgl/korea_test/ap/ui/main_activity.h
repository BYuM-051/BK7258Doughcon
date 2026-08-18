#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include "lvgl.h"
#include "beken_ui.h"

/* ── UART/Modbus 송신 데이터 ─────────────────────────────────────── */
typedef struct {
    int complete_year, complete_month, complete_day, complete_hour, complete_min;
    int remain_hour,   remain_min;
    int freeze_temp,   freeze_hour,   freeze_min;
    int unfreeze_temp, unfreeze_hour, unfreeze_min;
    int ferm1_temp,    ferm1_hum,     ferm1_hour,  ferm1_min;
    int ferm2_temp,    ferm2_hum,     ferm2_hour,  ferm2_min;
    int dry_temp,      dry_hum,       dry_hour,    dry_min;
    int start_year,    start_month,   start_day,   start_hour, start_min;
    int drive_mode;
} ma_send_t;

/* ── 정전 복구 데이터 ────────────────────────────────────────────── */
typedef struct {
    int  freeze_hour,   freeze_min;
    int  unfreeze_hour, unfreeze_min;
    int  ferm1_hour,    ferm1_min;
    int  ferm2_hour,    ferm2_min;
    int  dry_hour,      dry_min;
    bool complete;       /* 완료 후 정전 여부 */
    bool checking;       /* 정전 복구 진행 중 */
    bool checking_cmd;   /* 첫 통신 정전 확인 플래그 */
} ma_blackout_t;

/* ── AutoModeStart 화면 표시 문자열 ─────────────────────────────── */
typedef struct {
    char freeze_temp[8];
    char defrost_temp[8],   defrost_hour[8],   defrost_min[8];
    char ferm1_temp[8],     ferm1_hum[8],      ferm1_hour[8],  ferm1_min[8];
    char ferm2_temp[8],     ferm2_hum[8],      ferm2_hour[8],  ferm2_min[8];
    char complete_year[8],  complete_month[8], complete_day[8];
    char complete_hour[8],  complete_min[8];
} ma_autostart_str_t;

/* ── MainActivity 전체 상태 구조체 ──────────────────────────────── */
typedef struct {
    /* 기본 설정 */
    int  language;          /* 0=KR, 1=CN, 2=EN */
    int  degree;            /* 0=C 1=F*/
    bool mute;
    bool power;
    bool lock;
    bool hard_lock;
    int  lamp;              /* 0x00=off, 0x01=on */
    bool screen_on;         /* LCD 화면 ON/OFF 버튼 상태 — Power/정전복구 플래그와 무관한 단순 토글.
                              * true=화면 ON(LED ON), false=화면 OFF(LED OFF). 기본값 true. */

    /* 모드 플래그 */
    bool auto_mode;
    bool auto_mode_start;
    bool auto_dry_mode;
    bool auto_dry_mode_start;
    bool auto_mode_end;
    bool auto_mode_over;
    bool manual_start;
    int  manual_current_mode;  /* 1=냉동, 2=해동, 3=발효 */
    bool test_mode;
    bool change_setting;
    bool operation;
    bool first_start;
    bool first_send;
    bool first_receive;
    bool first_freeze;
    bool first_defrost;
    int  first_operator_mode;
    int  day_period;
    bool drive_check;

    /* 구동 상태 */
    bool start_run, start_run1, start_run2;
    bool testing,   test_receive;

    /* 메모리 모드 */
    int  memory_mode_check;    /* 1=저장, 2=불러오기 */
    char memory_auto_save[15][16];
    char memory_auto_load[15][16];

    /* 건조 시작 시간 */
    char dry_start_year[8], dry_start_month[8], dry_start_day[8];
    char dry_start_hour[8], dry_start_min[8];

    /* 자동 잔여 시간 */
    int  auto_start_remain_hour, auto_start_remain_min;
    int  auto_start_operator_hour, auto_start_operator_min;

    /* 에러 / 팝업 상태 */
    bool error_popup;
    bool error_check;
    bool exchange_check_true, exchange_check_false;
    bool divible_point;        /* 팝업 키 이벤트 skip 플래그 */

    /* 통신 저장 배열 */
    int  save_operation[15];
    int  save_test[15];
    int  save_test_test[9];
    int  send_save_value1[26];

    /* 정전 복구 재시작 시각 */
    int  save_current_year, save_current_month, save_current_day;
    int  save_current_hour, save_current_min;

    /* 저장 체크 */
    int  save_checking;

    /* 통신 연결 상태 */
    int  connection_success;
    int  error_wait;

    /* 키 입력 타이밍 */
    uint32_t press_start_time; /* F3 long-press 기록 (lv_tick ms) */
    uint32_t startup_time;

    /* 하위 구조체 */
    ma_send_t         send;
    ma_blackout_t     blackout;
    ma_autostart_str_t autostart_str;

    /* LVGL 팝업 객체 */
    lv_obj_t *connect_error_popup; /* NACK/체크섬 오류 팝업 */
    lv_obj_t *connect_popup;       /* 통신 단절 팝업 */


    // char* DetailPassword = "0603";
    // // char* COUNTRY;

    // char* NeurosysPassword = "71960";

} main_activity_t;

extern main_activity_t g_main_activity;

/* ── 함수 선언 ─────────────────────────────────────────────────── */
void main_activity_on_create(void);
void main_activity_on_key_down(int key_code);
void main_activity_on_key_up(int key_code);
void main_activity_replace_screen(int screen_id);
void main_activity_save_current_calendar(void);
void main_activity_show_connect_error(void);
void main_activity_show_connect_lost(void);
void main_activity_hide_connect_error(void);
void main_activity_hide_connect_lost(void);

/* ── 유틸리티 함수 ─────────────────────────────────────────────── */
void        zero_add(const char *in, char *out, size_t out_size);
double      changer_off(double off);
double      reverse_changer_off(double off);
double      changer_od(double od);
double      reverse_changer_od(double od);
double      degree_change(double ch);
double      degree_basic_change(double ch);
double      reverse_degree_change(double ch);
double      reverse_degree_basic_change(double ch);
const char *true_false_str(const char *val);

/* ── 화면 ID 열거형 ─────────────────────────────────────────────── */
typedef enum {
    SCR_MAIN = 0,
    SCR_MAIN_CHINA,
    SCR_MAIN_ENGLISH,
    SCR_AUTOMODE_START,
    SCR_AUTOMODE_END,
    SCR_AUTODRYMODE,
    SCR_MANUAL_START,
    SCR_MANUAL_MODE,
} screen_id_t;

// /* ── 하드웨어 HAL 선언 (플랫폼에서 구현) ───────────────────────── */
// void hw_led_lamp_control(bool on);
// void hw_led_lock_control(bool on);
// void hw_led_power_control(bool on);   /* true=전원 LED 켜기(off표시) */
// void hw_backlight_set(int percent);   /* 0=끄기, 100=최대 */
// void hw_buzzer_beep(int ms);
// void hw_system_restart(void);
