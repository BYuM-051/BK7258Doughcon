/* uart_comm.c — SerialComm.java translated to C.
 *
 * State machine mirrors writeProcess() priority order:
 *   StartRun1 → CMD_TX_SETDATA (0x10)
 *   StartRun2 → CMD_TX_CONDATA (0x11)
 *   StartRun  → CMD_TX_STATUS  (0x33) [or 0x31 FirstStart / 0x30 ChangeSetting / 0x50 TestMode]
 *
 * RX dispatch mirrors the run() if/else chain.
 */
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

#include "lvgl.h"
#include <os/os.h>
#include "uart_comm.h"
#include "uart_protocol.h"
#include "device_state.h"
#include "settings.h"
#include "hardware_hal.h"
#include "rtc_sync.h"
#include <driver/aon_rtc.h>
#include "ui_config.h"
#include "custom_func.h"

#define TAG "[uart_comm.c] "
// #define bk_printf(fmt, ...) do {if(0) bk_printf(fmt, ##__VA_ARGS__); } while(0) // disable printf

#define UART_LOG(fmt, ...) do {} while(0)
//#define UART_LOG(fmt, ...) printf("[UART] " fmt "\n", ##__VA_ARGS__)
/* ---------------------------------------------------------------------------
 * Internal state (mirrors SerialComm fields)
 * ---------------------------------------------------------------------------*/
static int s_error_counting     = 0;
static int s_all_error_counting = 0;
static int s_comm_error_count   = 0;
static const int k_comm_recover_threshold = 30; /* 무응답 30회 → 조용히 UART 재초기화 시도 */

/* 짧은 끊김(30초)은 재초기화로 조용히 복구하고, 재초기화를 반복해도
 * 계속 실패할 때만(진짜 장애) 오버레이(error_cut)를 띄운다. */
static int s_reinit_count = 0;
static const int k_reinit_cut_threshold = 3;  /* 재초기화 3회(약 90초) 연속 실패 시 오버레이 표시 */

/* 단계 완료 감지: 벽시계(lv_tick) 기반
 * MCU가 X1(운전중) 코드를 처음 보낸 시점부터 설정시간 경과 시 current_op_mode 선행 갱신
 * saveoperation[10/11](remain)은 항상 0이므로 사용 불가 → lv_tick 폴백 사용 */
static uint8_t  s_prev_op_byte     = 0;
static uint32_t s_phase_start_tick = 0;
static bool     s_phase_tick_valid = false;

/* [미구현-B] 정전복구 X0 마커 플래그 (Android blackOutCheckingCmd 대응)
 * uart_comm_init()에서 true 설정 → 첫 STATUS TX에서 payload[21]=0x11 전송
 * saveoperation[14]==0x21 수신 시 false 해제 (미구현-C와 연동) */
static bool s_blackout_checking_cmd = false;

static void _rebuild_send_save_value1(void);  /* forward decl — defined near uart_comm_trigger_change_setting */

/* UI 타이머가 마지막으로 처리한 시점 이후 신규 UART 데이터 여부 감지용.
 * STATUS(0x43) 또는 CONDATA_ACK(0x21) 수신 시 증가. UI 타이머는 자신의
 * last-seq와 비교해 변화 없으면 label 갱신 등 처리를 건너뜀. */
volatile uint32_t g_uart_rx_seq = 0;

/* ---------------------------------------------------------------------------
 * HAL wrappers — replace with actual Beken7258 UART send/receive
 * ---------------------------------------------------------------------------*/
static void _uart_write(const uint8_t *buf, int len)
{
    hal_uart_write(buf, len);
}

static int _uart_read(uint8_t *buf, int max_len)
{
    return hal_uart_read(buf, max_len);
}

/* ---------------------------------------------------------------------------
 * Helpers
 * ---------------------------------------------------------------------------*/
static void _get_rtc(int *year2, int *month, int *day, int *hour, int *min, int *sec)
{
    struct timeval tv;
    bk_rtc_gettimeofday(&tv, NULL);
    time_t now = tv.tv_sec;
    struct tm *t = localtime(&now);
    *year2 = t->tm_year % 100;
    *month = t->tm_mon + 1;
    *day   = t->tm_mday;
    *hour  = t->tm_hour;
    *min   = t->tm_min;
    *sec   = t->tm_sec;
}

static void _log_hex(const char *tag, const uint8_t *buf, int len)
{
    char hex[256];
    int  pos = 0;
    for (int i = 0; i < len && pos < (int)sizeof(hex) - 4; i++)
        pos += snprintf(hex + pos, sizeof(hex) - pos, "%02X ", buf[i]);
    UART_LOG("%s [%d] %s", tag, len, hex);
}

static void _send(const uint8_t *payload, uint8_t plen)
{
    uint8_t frame[UART_MAX_PACKET];
    int flen = uart_build_frame(payload, plen, frame, sizeof(frame));
    if (flen > 0) {
        _uart_write(frame, flen);
        UART_LOG("TX cmd=0x%02X len=%d frame_len=%d", payload[0], plen, flen);
        _log_hex("TX HEX", frame, flen);
    } else {
        UART_LOG("TX build_frame failed cmd=0x%02X", payload[0]);
    }
}

/* ---------------------------------------------------------------------------
 * SaveSetting — mirrors SerialComm.SaveSetting(), called on 0x20 response
 *
 * [버그 수정] MCU cold-start 시 sv1[1..23] 전체 0 → 기존 코드가 flash에 저장된
 * AP 설정(예: "ON")을 "OFF"로 덮어쓰고 settings_save_all_sync()로 즉시 commit.
 * 수정: sv1[N]>0일 때만 AP 설정 갱신. 0=MCU미설정 → flash 보존.
 * send_save_value1[]: MCU 0인 boolean 필드는 AP settings로 보정.
 * ---------------------------------------------------------------------------*/
static void _save_setting(const int *sv1, int len)
{
    if (!sv1 || len < 24) return;

    bool has_valid = false;

    if (sv1[1]  > 0) { settings_set_int("DetailHumidificationTime0",      sv1[1]);  has_valid = true; }
    if (sv1[2]  > 0) { settings_set_int("DetailHumidificationTime1",      sv1[2]);  has_valid = true; }
    if (sv1[3]  > 0) { settings_set_int("DetailWaterInterval0",           sv1[3]);  has_valid = true; }
    if (sv1[4]  > 0) { settings_set_int("DetailWaterInterval1",           sv1[4]);  has_valid = true; }
    if (sv1[5]  > 0) { settings_set_int("DetailHumidificationHeaterTime", sv1[5]);  has_valid = true; }

    /* boolean: non-zero=ON만 기록, 0=cold-start → flash 보존 (OFF 덮어쓰기 금지) */
    if (sv1[6] > 0) { settings_set_str("DetailDefrostOnOff", "ON"); has_valid = true; }

    if (sv1[7]  > 0) { settings_set_int("DetailDefrostTime",       sv1[7]);  has_valid = true; }
    if (sv1[8]  > 0) { settings_set_int("DetailDefrostReturnTemp", sv1[8]);  has_valid = true; }

    {
        char buf[16];
        if (sv1[9]  > 0) { snprintf(buf, sizeof(buf), "%.1f", sv1[9]  * 0.5f); settings_set_str("DetailTempOff",            buf); has_valid = true; }
        if (sv1[10] > 0) { snprintf(buf, sizeof(buf), "%.1f", sv1[10] * 0.5f); settings_set_str("DetailTempOd",             buf); has_valid = true; }
        if (sv1[11] > 0) { snprintf(buf, sizeof(buf), "%.1f", sv1[11] * 0.5f); settings_set_str("DetailFermentationTempOff", buf); has_valid = true; }
        if (sv1[12] > 0) { snprintf(buf, sizeof(buf), "%.1f", sv1[12] * 0.5f); settings_set_str("DetailFermentationTempOd",  buf); has_valid = true; }
    }

    if (sv1[13] > 0) { settings_set_int("DetailHumidityOff", sv1[13]); has_valid = true; }
    if (sv1[14] > 0) { settings_set_int("DetailHumidityOd",  sv1[14]); has_valid = true; }

    /* [핵심] 기존 else{"OFF"} 제거: cold-start sv1[15]=0 → "OFF" flash 저장 버그 수정 */
    if (sv1[15] > 0) {
        settings_set_str("DetailOverFermentationOnOff", "ON");
        settings_set_int("DetailOverFermentation", sv1[15]);
        has_valid = true;
    }

    if (sv1[16] > 0) { settings_set_int("DetailHumidityRevision",         sv1[16]); has_valid = true; }
    if (sv1[17] > 0) { settings_set_int("DetailTempRevision",             sv1[17]); has_valid = true; }
    if (sv1[18] > 0) { settings_set_int("DetailFermentationTempRevision", sv1[18]); has_valid = true; }
    if (sv1[19] > 0) { settings_set_int("DetailDamperOnSol",              sv1[19]); has_valid = true; }
    if (sv1[20] > 0) { settings_set_int("DetailDamperOffSol",             sv1[20]); has_valid = true; }
    if (sv1[21] > 0) { settings_set_int("DetailDamperFanOn",              sv1[21]); has_valid = true; }
    if (sv1[22] > 0) { settings_set_int("DetailDamperFanOff",             sv1[22]); has_valid = true; }
    /* sv1[23]: MCU 펌웨어 버전 겸 DetailFan. >0="fan OFF(오버라이드)", 0=미설정 → flash 보존 */
    if (sv1[23] > 0) { settings_set_str("DetailFan", "OFF"); has_valid = true; }

    /* cold-start(sv1 전체 0) 시 save 금지: flash의 사용자 설정 보호 */
    if (has_valid) {
        settings_save_all_sync();
    }

    /* ── send_save_value1[1..23]: 0x30 CHANGE 재전송용 ────────────────────
     * MCU cold-start(NVRAM 없음) 시 sv1 전체 0 → flash에는 사용자 설정이 남아있음.
     * MCU 버전([23])만 MCU 수신값으로 보존하고, 나머지는 flash 현재값으로 재빌드.
     * → 정전복구 후 change_setting 트리거 시에도 올바른 값이 MCU로 전달됨. */
    device_state_t *st = &g_device_state;
    st->mcu_version = (uint8_t)(sv1[23] & 0xFF);
    /* flash 기반 full rebuild — _rebuild_send_save_value1()과 동일 로직,
     * settings_save 완료 후 호출되므로 flash에 최신값이 반영된 상태 */
    _rebuild_send_save_value1();
}

/* Map saveoperation[5] drive mode byte → OP_MODE_* */
static int _mode_from_op_byte(int op_byte)
{
    if (op_byte >= 0x50) return OP_MODE_DRY;
    if (op_byte >= 0x40) return (op_byte == 0x43 ? 8 : (op_byte == 0x44 ? 10 : (op_byte == 0x42 ? 8 : OP_MODE_FERM2)));
    if (op_byte >= 0x30) return (op_byte == 0x33 ? 7 : (op_byte == 0x34 ? 9 : OP_MODE_FERM1));
    if (op_byte >= 0x20) return OP_MODE_DEFROST;
    return OP_MODE_FREEZE;
}

/* ---------------------------------------------------------------------------
 * RX dispatch
 * ---------------------------------------------------------------------------*/
static void _handle_rx(const uart_packet_t *pkt)
{
    device_state_t *st = &g_device_state;

    if (!pkt->valid) {
        s_all_error_counting++;
        UART_LOG("RX invalid packet (all_err=%d)", s_all_error_counting);
        if (s_all_error_counting > 15) st->error_popup = true;
        if (s_all_error_counting > 34) st->error_cut    = true;
        return;
    }

    s_all_error_counting = 0;
    s_comm_error_count   = 0;

    UART_LOG("RX cmd=0x%02X len=%d", pkt->cmd, pkt->data_len);

    switch (pkt->cmd) {

    case CMD_RX_SETDATA_ACK: {   /* 0x20 */
        /* pkt->data[0..22] = data bytes 1-23 from device (SaveValue1) */
        int sv1[24];
        sv1[0] = pkt->cmd;
        for (int i = 0; i < pkt->data_len && i < 23; i++) sv1[i + 1] = pkt->data[i];
        _save_setting(sv1, 24);
        st->start_run1 = false;
        st->start_run2 = true;
        UART_LOG("RX 0x20 SETDATA_ACK → save_setting done, start_run2=true");
        break;
    }

    case CMD_RX_CONDATA_ACK: {  /* 0x21: current state + all op params (up to 20 bytes) */
        /* byte[0]=curr_temp, [1]=curr_hum,
         * [2..4]=freeze(temp/h/m), [5..7]=defrost(temp/h/m),
         * [8..11]=ferm1(temp/hum/h/m), [12..15]=ferm2(temp/hum/h/m),
         * [16..19]=dry(temp/hum/h/m) */
        if (pkt->data_len >= 2) {
            st->current_temp     = (int8_t)pkt->data[0];
            st->current_humidity = pkt->data[1];
        }
        if (pkt->data_len >= 20) {
            /* 설정 온도는 항상 갱신 (현재 측정 온도/습도는 위에서 이미 갱신) */
            st->send_freeze_temp     = pkt->data[2];
            st->send_defreeze_temp   = pkt->data[5];
            st->send_ferm1_temp      = pkt->data[8];
            st->send_ferm2_temp      = pkt->data[12];
            st->send_dry_temp        = pkt->data[16];
            /* 습도·시간: 정전 복구 중에는 덮어쓰지 않는다.
             * MCU NVRAM 습도(ferm1=3%, ferm2=2% 등)가 설정값(70%,80%)과 다를 수 있으며,
             * 이를 그대로 FIRST_START에 실어 보내면 MCU 검증 실패로 remain=0h0m이 된다.
             * _blackout_recovery()가 flash에서 올바른 값(send_ferm1_humidity=70% 등)을
             * 이미 복원해 두었으므로 보호한다. */
            if (!st->black_out_checking) {
                st->send_ferm1_humidity  = pkt->data[9];
                st->send_ferm2_humidity  = pkt->data[13];
                st->send_dry_humidity    = pkt->data[17];
                st->send_freeze_hour     = pkt->data[3];
                st->send_freeze_min      = pkt->data[4];
                st->send_defreeze_hour   = pkt->data[6];
                st->send_defreeze_min    = pkt->data[7];
                st->send_ferm1_hour      = pkt->data[10];
                st->send_ferm1_min       = pkt->data[11];
                st->send_ferm2_hour      = pkt->data[14];
                st->send_ferm2_min       = pkt->data[15];
                st->send_dry_hour        = pkt->data[18];
                st->send_dry_min         = pkt->data[19];
            }
        }
        st->start_run1 = false;
        st->start_run2 = false;
        st->start_run  = true;
        g_uart_rx_seq++;
        UART_LOG("RX 0x21 CONDATA_ACK temp=%d hum=%d freeze=%d° defrost=%d° → start_run=true",
                 st->current_temp, st->current_humidity,
                 st->send_freeze_temp, st->send_defreeze_temp);

        /* RTC sync: MCU appends yr2,mo,day,hr,mn,sc as bytes 20..25
         * (MCU firmware must extend 0x21 response from 20 to 26 bytes) */
        if (pkt->data_len >= 26) {
            rtc_sync_from_mcu(pkt->data[20], pkt->data[21], pkt->data[22],
                               pkt->data[23], pkt->data[24], pkt->data[25]);
        }
        break;
    }

    case CMD_RX_CHANGE_ACK: {   /* 0x40 */
        uint8_t ack = pkt->data_len > 0 ? pkt->data[0] : 0;
        UART_LOG("RX 0x40 CHANGE_ACK ack=0x%02X err_cnt=%d", ack, s_error_counting);
        if (ack == 0x4E) {
            s_error_counting++;
            UART_LOG("RX 0x40 NACK (0x4E) err_cnt=%d", s_error_counting);
        } else if (ack == 0x41) {
            s_error_counting      = 0;
            st->change_setting    = false;
            st->error_popup       = false;
            st->error_cut         = false;
            st->start_run         = true;
            UART_LOG("RX 0x40 ACK (0x41) → change_setting done");
        }
        if (s_error_counting > 30) {
            st->change_setting = false;
            st->start_run      = true;
            st->error_popup    = true;
            UART_LOG("RX 0x40 error_popup triggered (err_cnt=%d)", s_error_counting);
        }
        if (s_error_counting > 34) {
            st->error_cut = true;
            UART_LOG("RX 0x40 error_cut triggered");
        }
        break;
    }

    case CMD_RX_FIRST_ACK: {    /* 0x41 */
        uint8_t ack = pkt->data_len > 0 ? pkt->data[0] : 0;
        UART_LOG("RX 0x41 FIRST_ACK ack=0x%02X err_cnt=%d", ack, s_error_counting);
        if (ack == 0x4E) {
            s_error_counting++;
            UART_LOG("RX 0x41 NACK (0x4E) err_cnt=%d", s_error_counting);
        } else {
            s_error_counting  = 0;
            st->error_cut     = false;
            st->error_popup   = false;
            st->test_mode     = false;
            st->first_start   = false;
            st->start_run     = true;
            UART_LOG("RX 0x41 ACK → first_start done, start_run=true");
            if (st->auto_mode_start) {
                st->operation = true;
                if (st->first_freeze) {
                    st->first_send         = true;
                    st->first_operator_mode = 0x10;
                    UART_LOG("RX 0x41 auto_mode: first_freeze → op_mode=0x10");
                } else if (st->first_defrost) {
                    st->first_send         = true;
                    st->first_operator_mode = 0x20;
                    UART_LOG("RX 0x41 auto_mode: first_defrost → op_mode=0x20");
                }
            } else if (st->auto_dry_mode_start) {
                st->operation          = true;
                st->first_send         = true;
                st->first_operator_mode = 0x50;
                UART_LOG("RX 0x41 auto_dry_mode → op_mode=0x50");
            }
        }
        if (s_error_counting > 15) {
            st->error_popup = true;
            UART_LOG("RX 0x41 error_popup triggered (err_cnt=%d)", s_error_counting);
            if (s_error_counting > 34) {
                st->error_cut = true;
                UART_LOG("RX 0x41 error_cut triggered");
            }
        }
        break;
    }

    case CMD_RX_STATUS: {        /* 0x43 */
        /* data[0..13] → saveoperation[1..14]; protocol spec has 14 bytes */
        int len = pkt->data_len < 14 ? pkt->data_len : 14;
        for (int i = 0; i < len; i++) st->saveoperation[i + 1] = pkt->data[i];

        /* Extract named fields from saveoperation indices */
        st->current_temp     = (int8_t)st->saveoperation[1];
        st->current_humidity = (uint8_t)st->saveoperation[2];
        st->lamp_from_mcu    = st->saveoperation[3];

        /* Error bit decoding from saveoperation bytes 6-9 (indices 6..9) */
        uint8_t err6 = (uint8_t)st->saveoperation[6];
        uint8_t err7 = (uint8_t)st->saveoperation[7];
        uint8_t err8 = (uint8_t)st->saveoperation[8];
        uint8_t err9 = (uint8_t)st->saveoperation[9];

        st->error_flags[0] = err6;
        st->error_flags[1] = err7;
        st->error_flags[2] = err8;
        st->error_flags[3] = err9;

        /* 아이콘 플래그: saveoperation[6] 비트 분해 (Android SerialComm.errorTenToBit data==6) */
        st->op_comp          = (err6 >> 0) & 1;
        st->op_fire_heater   = (err6 >> 1) & 1;
        st->op_humid_heater  = (err6 >> 2) & 1;
        st->op_water_pump    = (err6 >> 3) & 1;
        st->op_fan           = (err6 >> 4) & 1;
        st->op_frozen_heater = (err6 >> 6) & 1;
        /* saveoperation[7] 비트 분해 (Android data==7) */
        st->op_damper        = (err7 >> 0) & 1;
        /* 에러 아이콘: error_flags[2],[3] 비트 중 하나라도 set */
        st->op_error         = (err8 || err9) ? true : false;

        bk_printf(TAG "[UART] RX 0x43 STATUS op=0x%02X err=%02X%02X%02X%02X remain=%dh%dm\n",
                 st->saveoperation[5], err6, err7, err8, err9,
                 st->saveoperation[10], st->saveoperation[11]);

#if 0   /* [미구현-C] 기존: saveoperation[14]!=0 을 무조건 에러 처리
         * 문제: 정전복구 첫 STATUS RX에서 [14]=0x21(DEFROST X1 에코)이 오탐됨
         * Android SerialComm.java: [14]==0x21 → blackOutCheckingCmd 해제(정상) */
        if (st->saveoperation[14] != 0) {
            s_error_counting++;
            UART_LOG("RX 0x43 device error flag=%d err_cnt=%d",
                     st->saveoperation[14], s_error_counting);
            if (s_error_counting > 15) {
                st->error_popup = true;
                UART_LOG("RX 0x43 error_popup triggered");
                if (s_error_counting > 34) {
                    st->error_cut = true;
                    UART_LOG("RX 0x43 error_cut triggered");
                }
            }
        } else {
            s_error_counting = 0;
            st->error_popup  = false;
            st->error_cut    = false;
        }
#endif  /* [미구현-C] 기존 끝 */

        /* [미구현-C 구현] saveoperation[14]==0x21: 정전복구 정상 에코 — 에러 아님
         * Android: if (saveoperation[14] == 0x21) { blackOutCheckingCmd=false; }
         *          else if (saveoperation[14] != 0) { errorCounting++; } */
        if (st->black_out_checking && st->saveoperation[14] == 0x21) {
            /* 정전복구 MCU 에코: 이전 단계 X1 확인 코드 — 정상 */
            #if 1
            s_blackout_checking_cmd = false;  /* [미구현-B] X0 마커 해제 */
            s_error_counting = 0;
            st->error_popup  = false;
            st->error_cut    = false;
            UART_LOG("RX 0x43 blackout echo [14]=0x21 → normal confirm, cmd_flag cleared");
            #endif
        } else if (st->saveoperation[14] != 0) {
            s_error_counting++;
            UART_LOG("RX 0x43 device error flag=%d err_cnt=%d",
                     st->saveoperation[14], s_error_counting);
            if (s_error_counting > 15) {
                st->error_popup = true;
                UART_LOG("RX 0x43 error_popup triggered");
                if (s_error_counting > 34) {
                    st->error_cut = true;
                    UART_LOG("RX 0x43 error_cut triggered");
                }
            }
        } else {
            s_error_counting = 0;
            st->error_popup  = false;
            st->error_cut    = false;
        }

        /* Update operation mode from saveoperation[5]
         * 벽시계로 이미 앞서 갱신된 current_op_mode를 MCU 지연 코드로 되돌리지 않음
         * MCU 보고 모드 >= 현재 모드일 때만 갱신.
         *
         * 단, 새 운전을 막 시작한 직후(first_receive=true, first_start 전송 시 set)에는
         * MCU가 아직 "이전" 사이클의 마지막 단계(예: 저온발효/FERM2) 코드를 잠깐 더
         * 보내고 있을 수 있다. 이때 위 규칙을 그대로 적용하면 방금 FREEZE로 리셋한
         * current_op_mode가 곧바로 이전 사이클의 단계로 되돌아가 버린다
         * (재운전 시 저온발효로 바로 진입하는 버그). MCU가 실제로 새 단계를
         * 보고할 때까지는 전진 동기화를 보류한다. */
        if (st->saveoperation[5] != 0) {
            int _mcu_mode = _mode_from_op_byte(st->saveoperation[5]);
            if (st->first_receive) {
                if (_mcu_mode == st->current_op_mode)
                    st->first_receive = false;
            } else if (_mcu_mode >= st->current_op_mode) {
                st->current_op_mode = _mcu_mode;
            }
        }
        /* 과발효방지 override 해제:
         *  0x34/0x44 : MCU 저온발효 시작 → over_ferm_active=false
         *              automodeend 화면 → automodestart 복귀 → 저온발효 표시 */
        if (st->over_ferm_active &&
            (st->saveoperation[5] == 0x34 || st->saveoperation[5] == 0x44)) {
            st->over_ferm_active       = false;
            st->over_ferm_jeon_started = true;
            bk_printf(TAG "[OVER_FERM] cleared: MCU op=0x%02X → jeon_started flag set\n", st->saveoperation[5]);
        }

        /* Remain time from saveoperation[10] and [11]
         *
         * 정전복구 가드 배경:
         *   Android MCU: 배터리백업 NVRAM → 항상 정확한 flash 저장값(2h24m) 보고.
         *   BK7258 MCU: NVRAM 없음 → 정전 후 두 가지 문제 발생:
         *     ① MCU가 0h0m 보고: flash 복원값 유지 (기존 가드)
         *     ② MCU가 NVRAM total(2h59m)로 복귀: 증가 방지 (신규 가드)
         *
         * 현상(②): 정전 전 마지막 TX payload[17/18]=0h35m, [19/20]=2h24m 수신.
         *   MCU NVRAM: total = elapsed(35m) + remain(2h24m) = 2h59m 기록.
         *   복구 후: X0 수신 시 2h24m 에코(1사이클) → 다음 사이클부터 NVRAM total(2h59m) 보고.
         *   → remain_hour가 2h24m → 2h59m 으로 증가해 표시가 뒤바뀌는 문제 발생.
         *
         * 해결: 정전복구 중 MCU가 현재 remain보다 큰 값을 보고하면 갱신 거부.
         *   MCU가 2h24m → 2h23m → ... 처럼 감소할 때만 갱신 허용.
         *   35분 후 MCU가 2h24m까지 카운트다운되면 갱신 재개 → 정상 동작. */
#if 0   /* 기존 가드: 0h0m 방지만 했으나 NVRAM total 역전(2h24m→2h59m) 미대응 */
        if (!st->black_out_checking ||
            st->saveoperation[10] != 0 || st->saveoperation[11] != 0) {
            st->remain_hour = st->saveoperation[10];
            st->remain_min  = st->saveoperation[11];
        }
#endif  /* 기존 가드 끝 */
        if (!st->black_out_checking) {
            /* 운전 시작 직후 MCU 미확인 단계(0x41 ACK 전): MCU가 0h0m 보고 시 설정값 유지.
             * 0x41 ACK 수신 후 operation=true로 전환되면 MCU 보고값 그대로 갱신. */
            bool _starting = (st->auto_dry_mode_start || st->auto_mode_start) && !st->operation;
            if (_starting && st->saveoperation[10] == 0 && st->saveoperation[11] == 0) {
                /* 시작 확인 전 MCU 초기 0h0m → 설정값 유지 (3:00→0:00→3:00 플리커 방지) */
            } else {
                st->remain_hour = st->saveoperation[10];
                st->remain_min  = st->saveoperation[11];
            }
        } else if (st->saveoperation[10] == 0 && st->saveoperation[11] == 0) {
            /* ① MCU가 0h0m: 완료 코드(X2)면 진짜 완료→remain 갱신, 아니면 X0 초기 에코→유지 */
            uint8_t _mc = (uint8_t)st->saveoperation[5];
            if (_mc == 0x12 || _mc == 0x22 || _mc == 0x32 || _mc == 0x42 || _mc == 0x52) {
                st->remain_hour = 0;
                st->remain_min  = 0;
            }
        } else {
            int _mcu_rm = (int)st->saveoperation[10] * 60 + (int)st->saveoperation[11];
            int _cur_rm = (int)st->remain_hour * 60 + (int)st->remain_min;
            if (_mcu_rm <= _cur_rm) {
                /* MCU가 현재 이하로 보고 → 정상 카운트다운, 갱신 허용 */
                st->remain_hour = st->saveoperation[10];
                st->remain_min  = st->saveoperation[11];
            }
            /* ② MCU가 현재보다 증가 보고(_mcu_rm > _cur_rm):
             *    NVRAM total 복귀(2h59m > 2h24m) → flash 복원값 유지 */
        }

        /* ── 벽시계 기반 단계 완료 감지 → current_op_mode 선행 갱신 ─────────
         * MCU가 X1(운전중) 코드를 처음 보낸 순간부터 설정시간이 경과하면 완료로 판단.
         * saveoperation[10/11](remain)은 항상 0이어서 사용 불가.
         * AUTO_MODE_TEST와 동일한 lv_tick 방식으로 처리 */
        {
            uint8_t _op  = (uint8_t)st->saveoperation[5];
            bool _is_x01 = (_op == 0x10 || _op == 0x11 ||
                            _op == 0x20 || _op == 0x21 ||
                            _op == 0x30 || _op == 0x31 ||
                            _op == 0x40 || _op == 0x41);
            bool _is_x1  = (_op == 0x11 || _op == 0x21 ||
                            _op == 0x31 || _op == 0x41);

            /* 오피 바이트가 바뀌거나 X1이 처음 나타날 때 타이머 시작 */
            if (_op != s_prev_op_byte) {
                s_prev_op_byte = _op;
                if (_is_x01) {
                    s_phase_start_tick = lv_tick_get();
                    s_phase_tick_valid = true;
                } else {
                    s_phase_tick_valid = false;
                }
            }
            if (_is_x1 && !s_phase_tick_valid) {
                s_phase_start_tick = lv_tick_get();
                s_phase_tick_valid = true;
            }

            /* 설정시간 경과 → current_op_mode 선행 갱신
             * MCU op 코드가 아닌 current_op_mode(UI 기준 현재 단계)로 설정시간 결정
             * MCU가 계속 0x11을 보내도 defrost→ferm1→ferm2 단계 타이머가 올바르게 동작 */
            if (_is_x1 && s_phase_tick_valid) {
                int cfg_min = 0;
                switch (st->current_op_mode) {
                    case OP_MODE_FREEZE:  cfg_min = st->send_freeze_hour  * 60 + st->send_freeze_min;  break;
                    case OP_MODE_DEFROST: cfg_min = st->send_defreeze_hour * 60 + st->send_defreeze_min; break;
                    case OP_MODE_FERM1:   cfg_min = st->send_ferm1_hour   * 60 + st->send_ferm1_min;   break;
                    case OP_MODE_FERM2:   cfg_min = st->send_ferm2_hour   * 60 + st->send_ferm2_min;   break;
                    default: break;
                }
                uint32_t cfg_ms  = (uint32_t)cfg_min * 60u * 1000u;
                uint32_t elapsed = lv_tick_elaps(s_phase_start_tick);
                if (cfg_ms > 0 && elapsed >= cfg_ms &&
                        st->current_op_mode < OP_MODE_FERM2) {
                    int prev = st->current_op_mode;
                    st->current_op_mode++;
                    UART_LOG("wall-clock phase done: op=0x%02X cur_op %d→%d elapsed=%ums cfg=%ums",
                             _op, prev, st->current_op_mode,
                             (unsigned)elapsed, (unsigned)cfg_ms);
                    s_phase_tick_valid = false;  /* 다음 X1 패킷에서 다음 단계 타이머 재시작 */
                }
            }
        }

        settings_set_int("saveOperationTemp",     st->current_op_mode);
        settings_set_int("saveCurrentRemainHour", st->remain_hour);
        settings_set_int("saveCurrentRemainMin",  st->remain_min);
        /* Power-off recovery: save at most once per 30 s to avoid flash wear */
        {
            static uint32_t s_status_save_tick = 0;
            if (lv_tick_elaps(s_status_save_tick) >= 30000) {
                s_status_save_tick = lv_tick_get();
                settings_save_dirty();
            }
        }
        g_uart_rx_seq++;
        break;
    }

    case CMD_RX_HW_TEST_ACK: {   /* 0x60: 14-byte sensor/actuator status */
        if (pkt->data_len >= 14) {
            st->hw_test.f_temp         = (int8_t)pkt->data[0];
            st->hw_test.defrost_temp   = (int8_t)pkt->data[1];
            st->hw_test.humidity       = pkt->data[2];
            st->hw_test.rt_temp        = (int8_t)pkt->data[3];
            st->hw_test.error_code     = pkt->data[4];
            st->hw_test.comp           = pkt->data[5];
            st->hw_test.fan            = pkt->data[6];
            st->hw_test.fire_heater    = pkt->data[7];
            st->hw_test.humid_heater   = pkt->data[8];
            st->hw_test.water_valve    = pkt->data[9];
            st->hw_test.defrost_heater = pkt->data[10];
            st->hw_test.dc_led         = pkt->data[11];
            st->hw_test.cabinet_heater = pkt->data[12];
            st->hw_test.damper         = pkt->data[13];
        }
        st->testing     = true;
        st->testreceive = true;
        UART_LOG("RX 0x60 HW_TEST_ACK f_temp=%d defrost=%d hum=%d err=0x%02X comp=%d fan=%d",
                 st->hw_test.f_temp, st->hw_test.defrost_temp,
                 st->hw_test.humidity, st->hw_test.error_code,
                 st->hw_test.comp, st->hw_test.fan);
        break;
    }

    default:
        s_all_error_counting++;
        UART_LOG("RX unknown cmd=0x%02X (all_err=%d)", pkt->cmd, s_all_error_counting);
        if (s_all_error_counting > 15) st->error_popup = true;
        if (s_all_error_counting > 34) st->error_cut   = true;
        break;
    }
}

/* ---------------------------------------------------------------------------
 * Write process (writeProcess in Java)
 * ---------------------------------------------------------------------------*/
static void _write_process(void)
{
    device_state_t *st = &g_device_state;
    uint8_t payload[UART_MAX_DATA];
    int yr2, mo, day, hr, mn, sc;
    _get_rtc(&yr2, &mo, &day, &hr, &mn, &sc);

    if (st->start_run1) {
        /* 0x10 SETDATA: cmd + "SETDATA" + RTC (16 total) */
        UART_LOG("TX 0x10 SETDATA RTC=%02d-%02d-%02d %02d:%02d:%02d", yr2, mo, day, hr, mn, sc);
        payload[0]  = CMD_TX_SETDATA;
        payload[1]  = 0x53; payload[2]  = 0x45; payload[3] = 0x54;
        payload[4]  = 0x44; payload[5]  = 0x41; payload[6] = 0x54; payload[7] = 0x41;
        payload[8]  = (uint8_t)yr2;
        payload[9]  = (uint8_t)mo;
        payload[10] = (uint8_t)day;
        payload[11] = (uint8_t)hr;
        payload[12] = (uint8_t)mn;
        payload[13] = (uint8_t)sc;
        payload[14] = 0x00; payload[15] = 0x00;
        _send(payload, 16);

    } else if (st->start_run2) {
        /* 0x11 CONDATA: cmd + "CONDATA" + RTC (16 total) */
        UART_LOG("TX 0x11 CONDATA RTC=%02d-%02d-%02d %02d:%02d:%02d", yr2, mo, day, hr, mn, sc);
        payload[0]  = CMD_TX_CONDATA;
        payload[1]  = 0x43; payload[2]  = 0x4F; payload[3] = 0x4E;
        payload[4]  = 0x44; payload[5]  = 0x41; payload[6] = 0x54; payload[7] = 0x41;
        payload[8]  = (uint8_t)yr2;
        payload[9]  = (uint8_t)mo;
        payload[10] = (uint8_t)day;
        payload[11] = (uint8_t)hr;
        payload[12] = (uint8_t)mn;
        payload[13] = (uint8_t)sc;
        payload[14] = 0x00; payload[15] = 0x00;
        _send(payload, 16);

    } else if (st->test_mode) {
        /* 0x50 HW Test: cmd + savetesttest[0..8] + 7 zeros (16 total) */
        UART_LOG("TX 0x50 HW_TEST data=%d,%d,%d,%d,%d,%d,%d,%d,%d",
                 st->savetesttest[0], st->savetesttest[1], st->savetesttest[2],
                 st->savetesttest[3], st->savetesttest[4], st->savetesttest[5],
                 st->savetesttest[6], st->savetesttest[7], st->savetesttest[8]);
        payload[0] = CMD_TX_HW_TEST;
        for (int i = 0; i < 9; i++)
            payload[1 + i] = (uint8_t)st->savetesttest[i];
        for (int i = 9; i < 15; i++)
            payload[1 + i] = 0x00;
        _send(payload, 16);

    } else if (st->change_setting) {
        /* 0x30 Change Setting: cmd + SendSaveValue1[1..23] + RTC + 2 zeros (32 total)
         * change_setting을 first_start보다 먼저 처리: autodrymode/manualmodestart에서
         * 두 플래그가 동시에 설정될 수 있으므로 0x30을 MCU에 먼저 보내 설정을
         * 확정한 뒤 0x31로 진행해야 MCU가 0x41로 응답한다. */
        UART_LOG("TX 0x30 CHANGE_SETTING RTC=%02d-%02d-%02d %02d:%02d:%02d", yr2, mo, day, hr, mn, sc);
        payload[0] = CMD_TX_CHANGE;
        for (int i = 1; i < 24; i++)
            payload[i] = (uint8_t)st->send_save_value1[i];
        payload[24] = (uint8_t)yr2;
        payload[25] = (uint8_t)mo;
        payload[26] = (uint8_t)day;
        payload[27] = (uint8_t)hr;
        payload[28] = (uint8_t)mn;
        payload[29] = (uint8_t)sc;
        payload[30] = 0x00; payload[31] = 0x00;
        _send(payload, 32);

    } else if (st->first_start) {
        /* 0x31 First Start: cmd + all params + start time (25 total) */
        UART_LOG("TX 0x31 FIRST_START freeze=%d°/%dh%dm defrost=%d°/%dh%dm ferm1=%d°%d%%/%dh%dm ferm2=%d°%d%%/%dh%dm dry=%d°%d%%/%dh%dm",
                 st->send_freeze_temp, st->send_freeze_hour, st->send_freeze_min,
                 st->send_defreeze_temp, st->send_defreeze_hour, st->send_defreeze_min,
                 st->send_ferm1_temp, st->send_ferm1_humidity, st->send_ferm1_hour, st->send_ferm1_min,
                 st->send_ferm2_temp, st->send_ferm2_humidity, st->send_ferm2_hour, st->send_ferm2_min,
                 st->send_dry_temp, st->send_dry_humidity, st->send_dry_hour, st->send_dry_min);
        payload[0]  = CMD_TX_FIRST_START;
        payload[1]  = (uint8_t)st->send_freeze_temp;
        payload[2]  = (uint8_t)st->send_freeze_hour;
        payload[3]  = (uint8_t)st->send_freeze_min;
        payload[4]  = (uint8_t)st->send_defreeze_temp;
        payload[5]  = (uint8_t)st->send_defreeze_hour;
        payload[6]  = (uint8_t)st->send_defreeze_min;
        payload[7]  = (uint8_t)st->send_ferm1_temp;
        payload[8]  = (uint8_t)st->send_ferm1_hour;
        payload[9]  = (uint8_t)st->send_ferm1_min;
        payload[10] = (uint8_t)st->send_ferm1_humidity;
        payload[11] = (uint8_t)st->send_ferm2_temp;
        payload[12] = (uint8_t)st->send_ferm2_hour;
        payload[13] = (uint8_t)st->send_ferm2_min;
        payload[14] = (uint8_t)st->send_ferm2_humidity;
        payload[15] = (uint8_t)st->send_dry_temp;
        payload[16] = (uint8_t)st->send_dry_hour;
        payload[17] = (uint8_t)st->send_dry_min;
        payload[18] = (uint8_t)st->send_dry_humidity;
        payload[19] = (uint8_t)yr2;
        payload[20] = (uint8_t)mo;
        payload[21] = (uint8_t)day;
        payload[22] = (uint8_t)hr;
        payload[23] = (uint8_t)mn;
        payload[24] = (uint8_t)sc;
        st->send_start_year  = yr2;
        st->send_start_month = mo;
        st->send_start_day   = day;
        st->send_start_hour  = hr;
        st->send_start_min   = mn;
        _send(payload, 25);

    } else if (st->start_run) {
        /* 0x33 Status: always sent during normal operation (23 total) */
        payload[0] = CMD_TX_STATUS;
        payload[1] = (uint8_t)st->day_period;

        /* Determine drive mode byte */
        uint8_t drive_mode = 0;
        if (st->operation) {
            if (st->auto_mode_start || st->auto_dry_mode_start) {
                if (st->first_send) {
                    drive_mode       = (uint8_t)st->first_operator_mode;
                    st->first_send   = false;
                    st->first_receive = true;
                    /* 새 운전 시작: current_op_mode와 벽시계 타이머 초기화 */
                    st->current_op_mode = _mode_from_op_byte(st->first_operator_mode);
                    s_prev_op_byte      = 0;
                    s_phase_tick_valid  = false;
                } else {
                    /* Android 원본: saveoperation[5] 그대로 에코
                     * MCU는 0x11(냉동중), 0x21(해동중) 등 자신의 상태 코드를 받으면
                     * "계속 운전" 명령으로 해석함 */
                    drive_mode = (uint8_t)st->saveoperation[5];
                    if (drive_mode != 0) {
                        st->first_operator_mode = 0;
                    } else {
                        drive_mode = (uint8_t)st->first_operator_mode;
                    }
                    /* 과발효방지: MCU 에코 그대로 유지 (0x42/0x43 모두)
                     * MCU가 0x34로 전환할 때까지 수신 op 그대로 echo
                     * RX에서 0x34 감지 시 over_ferm_active=false → echo 0x34 */
                }
            } else if (st->manual_start) {
                payload[1]=0;
                if (st->manual_current_mode == MANUAL_MODE_FREEZE)       drive_mode = 0x10;
                else if (st->manual_current_mode == MANUAL_MODE_DEFROST) drive_mode = 0x20;
                else if (st->manual_current_mode == MANUAL_MODE_FERM)    drive_mode = 0x30;
                else if (st->manual_current_mode == MANUAL_MODE_FERM2)   drive_mode = 0x30; /* auto_mode_over 비정상 상태 안전장치 */
            } else {
                drive_mode = (uint8_t)st->saveoperation[5];
            }
        }

        /* 과발효방지 대기 중: drive=0x43 고정
         * MCU는 0x43 echo + payload[11-16] comp_time을 보고 저온발효(0x34) 전환 시점 결정.
         * 구형 보드(0x42 done)는 0x43 echo가 없으면 0x34로 전환하지 않음. */
      //  if (st->over_ferm_active) drive_mode = 0x43;

        UART_LOG("TX 0x33 STATUS drive=0x%02X op=%d lamp=%d mcu_remain=%dh%dm day =%d",
                 drive_mode, st->operation, st->lamp,
                 st->saveoperation[10], st->saveoperation[11], payload[1]);
        payload[2] = drive_mode;
        payload[3] = st->operation ? 0x01 : 0x00;
        payload[4] = (uint8_t)st->lamp;

        payload[5]  = (uint8_t)yr2;
        payload[6]  = (uint8_t)mo;
        payload[7]  = (uint8_t)day;
        payload[8]  = (uint8_t)hr;
        payload[9]  = (uint8_t)mn;
        payload[10] = (uint8_t)sc;

        /* Complete time */
        payload[11] = (uint8_t)(st->send_complete_year  % 100);
        payload[12] = (uint8_t)st->send_complete_month;
        payload[13] = (uint8_t)st->send_complete_day;
        payload[14] = (uint8_t)st->send_complete_hour;
        payload[15] = (uint8_t)st->send_complete_min;
        payload[16] = (uint8_t)sc;   /* Android: currentTime.getSecond() — 현재 초를 완료시각 초 필드에 동일하게 */

        /* payload[17/18]: 동작시간 (Android 주석: data7 시/분 동작시간) */
        {
            int  _drive_phase = _mode_from_op_byte(drive_mode);
            bool _phase_done  = (st->current_op_mode > _drive_phase);

            if (st->black_out_checking) {
                if (st->manual_start) {
                    /* 수동운전 정전복구: elapsed/total 미사용 (수동운전은 시간 설정 없음).
                     * auto_mode_start 경로에서 로드된 send_ferm*_hour/min이 잔류하면
                     * blackout 계산 경로에서 자동운전 발효시간이 payload에 혼입되므로
                     * 명시적으로 0 전송. */
                    payload[17] = 0; payload[18] = 0;
                    payload[19] = 0; payload[20] = 0;
                } else {
                /* 완료 코드(X2): blackout 경로에서도 elapsed=0 전송 */
                bool _is_done_code = (drive_mode == 0x12 || drive_mode == 0x22 ||
                                      drive_mode == 0x32 || drive_mode == 0x42 ||
                                      drive_mode == 0x52);
                if (_is_done_code) {
                    payload[17] = 0; payload[18] = 0;
                    payload[19] = 0; payload[20] = 0;
                } else {
                /* 정전복구: elapsed = 원래 단계 설정시간 - 현재 잔여시간
                 * send_freeze_hour = cur_remain(10h26m) 이므로 그대로 쓰면 elapsed=0.
                 * bo_freeze_total_min = 원래 설정시간(14h1m)을 사용해야
                 * elapsed=3h35m → MCU가 NVRAM(14h1m)-3h35m=10h26m 로 올바르게 echo. */
                int _total_min = 0;
                switch (_drive_phase) {
                    case OP_MODE_FREEZE:
                        _total_min = (st->bo_freeze_total_min  > 0) ? st->bo_freeze_total_min
                                     : st->send_freeze_hour  * 60 + st->send_freeze_min;
                        break;
                    case OP_MODE_DEFROST:
                        _total_min = (st->bo_defrost_total_min > 0) ? st->bo_defrost_total_min
                                     : st->send_defreeze_hour * 60 + st->send_defreeze_min;
                        break;
                    case OP_MODE_FERM1:
                        _total_min = (st->bo_ferm1_total_min   > 0) ? st->bo_ferm1_total_min
                                     : st->send_ferm1_hour   * 60 + st->send_ferm1_min;
                        break;
                    case OP_MODE_FERM2:
                        _total_min = (st->bo_ferm2_total_min   > 0) ? st->bo_ferm2_total_min
                                     : st->send_ferm2_hour   * 60 + st->send_ferm2_min;
                        break;
                    case OP_MODE_DRY:
                        _total_min = st->send_dry_hour * 60 + st->send_dry_min;
                        break;
                    default: break;
                }
                int _remain_min = (int)st->remain_hour * 60 + (int)st->remain_min;
                int _elap_min   = _total_min - _remain_min;
                if (_elap_min < 0) _elap_min = 0;
                payload[17] = (uint8_t)(_elap_min / 60);
                payload[18] = (uint8_t)(_elap_min % 60);
                /* Android: writebuffer[19/20] = blackOutSendFreezeHour/Min (원래 총 설정시간)
                 * MCU가 NVRAM 재설정 시 total 기준: NVRAM_total - elapsed = remain */
                payload[19] = (uint8_t)(_total_min / 60);
                payload[20] = (uint8_t)(_total_min % 60);
                } /* end else: _is_done_code */
                } /* end else: auto blackout elapsed/total */
            } else {
#if 0   /* [미구현-A] 기존: X0=0, X1=saveoperation[12/13] 에코 — blackout 아닐 때만 사용하던 로직
         * X0 시작 코드: 0 전송 → MCU echo loop 초기화
         * X1 운전 중:  saveoperation[12/13] 에코 (Android SerialComm 동일)
         * X2 완료 코드: 0 전송 */
                (void)_drive_phase; (void)_phase_done;
#endif  /* [미구현-A] 기존 끝 */
                /* 정전복구 아닐 때 기존 로직 유지 */
                if (drive_mode == 0x12 || drive_mode == 0x22 ||
                        drive_mode == 0x32 || drive_mode == 0x42 ||
                        drive_mode == 0x52) {
                    /* 단계 완료 코드 (DRY 0x52 포함): 0 전송 → MCU 재시작 방지
                     * 0x52에 non-zero payload[17/18]을 보내면 MCU가 DRY 재시작하는 버그 */
                    payload[17] = 0;
                    payload[18] = 0;
                } else if (drive_mode == 0x10 || drive_mode == 0x20 ||
                           drive_mode == 0x30 || drive_mode == 0x40) {
                    /* X0 시작 코드: 0으로 echo loop 초기화 */
                    payload[17] = 0;
                    payload[18] = 0;
                } else if (drive_mode == 0x11 || drive_mode == 0x21 ||
                           drive_mode == 0x31 || drive_mode == 0x41) {
                    /* X1 운전 중: 완료 감지되면 0, 아니면 [12/13] 에코 */
                    payload[17] = _phase_done ? 0 : (uint8_t)st->saveoperation[12];
                    payload[18] = _phase_done ? 0 : (uint8_t)st->saveoperation[13];
                } else if (drive_mode >= 0x50) {
                    /* 0x50(X0), 0x51(X1): elapsed = total - remain 계산 전송
                     * DRY MCU는 saveoperation[12/13](elapsed)를 0으로 유지하고
                     * saveoperation[10/11](remain)만 카운트다운함.
                     * payload[17/18]는 운전 경과시간(동작시간)이어야 하므로
                     * total - remain 으로 역산하여 전송. */
                    {
                        int _dry_total  = st->send_dry_hour * 60 + st->send_dry_min;
                        int _dry_remain = (int)st->remain_hour * 60 + (int)st->remain_min;
                        int _dry_elap   = _dry_total - _dry_remain;
                        if (_dry_elap < 0) _dry_elap = 0;
                        payload[17] = (uint8_t)(_dry_elap / 60);
                        payload[18] = (uint8_t)(_dry_elap % 60);
                    }
                } else {
                    payload[17] = (uint8_t)st->saveoperation[12];
                    payload[18] = (uint8_t)st->saveoperation[13];
                }
            }
        }
        if (!st->black_out_checking) {
            /* 정상 운전: MCU remain 에코 (Android: writebuffer[19/20] = saveoperation[10/11]) */
            payload[19] = (uint8_t)st->remain_hour;
            payload[20] = (uint8_t)st->remain_min;
        }
        /* 정전복구 시: payload[19/20] = 원래 총 설정시간 → 위 black_out_checking 블록에서 설정 */

#if 0   /* [미구현-B] 기존: error_popup 여부만으로 payload[21] 결정
         * 문제: 정전복구 첫 X0 전송 시 payload[21]=0x11(이전 단계 X1 마커)이 전송되지 않음
         * Android: blackOutCheckingCmd=true → payload[21]=0x11, false → 0x00 */
        payload[21] = st->error_popup ? 0x11 : 0x00;
#endif  /* [미구현-B] 기존 끝 */
        /* [미구현-B 구현] 정전복구 첫 STATUS TX: payload[21]=0x11 (이전 단계 X1 마커)
         * Android SerialComm.java: if (blackOutCheckingCmd) writebuffer[21]=0x11
         * saveoperation[14]==0x21 수신 시 s_blackout_checking_cmd=false → 0x00 복귀 */
        if (st->black_out_checking && s_blackout_checking_cmd) {
            payload[21] = 0x11;
        } else {
            payload[21] = st->error_popup ? 0x11 : 0x00;
        }
        payload[22] = st->error_cut   ? 0x88 : 0x00;

        bk_printf(TAG "[UART] TX 0x33 payload[17/18]=%dh%dm [19/20]=%dh%dm (drive=0x%02X)\n",
                 payload[17], payload[18], payload[19], payload[20], drive_mode);
        {
            int _dy, _dm, _dd, _dh, _dmn, _ds;
            bool _rtc_ok = hal_rtc_get(&_dy, &_dm, &_dd, &_dh, &_dmn, &_ds);
            bk_printf(TAG "[UART] TX ctx: rtc_ok=%d now=%04d-%02d-%02d %02d:%02d comp=%04d-%02d-%02d %02d:%02d "
                   "remain=%dh%dm send_freeze=%dh%dm send_defreeze=%dh%dm send_ferm1=%dh%dm send_ferm2=%dh%dm\n",
                   (int)_rtc_ok, _dy, _dm, _dd, _dh, _dmn,
                   st->send_complete_year, st->send_complete_month, st->send_complete_day,
                   st->send_complete_hour, st->send_complete_min,
                   (int)st->remain_hour, (int)st->remain_min,
                   st->send_freeze_hour, st->send_freeze_min,
                   st->send_defreeze_hour, st->send_defreeze_min,
                   st->send_ferm1_hour, st->send_ferm1_min,
                   st->send_ferm2_hour, st->send_ferm2_min);
        }
        _send(payload, 23);
    }
}

/* ---------------------------------------------------------------------------
 * Timing constants (mirrors Android SerialComm timing)
 *   Android: Thread.sleep(300) initial, select(1000ms) per cycle
 *   BK7258:  1000ms TX interval, 200ms TX→RX delay (9600bps 27B≈28ms + MCU≈100ms + RX≈15ms)
 * ---------------------------------------------------------------------------*/
#define UART_CYCLE_MS   1000u   /* full TX→RX period */
#define UART_RX_WAIT_MS  200u   /* wait after TX before reading */

static enum { S_INIT_WAIT, S_TX, S_WAIT_RX } s_cycle_state = S_INIT_WAIT;
static uint32_t s_cycle_tick = 0;
static uint32_t s_tx_tick    = 0;

/* comm_error_count가 k_comm_recover_threshold에 도달했을 때 호출.
 * UART를 재오픈해 조용히 복구를 시도하고, 재초기화가 누적 반복되면
 * (즉 재초기화로도 통신이 살아나지 않으면) 그때 비로소 error_cut을 세운다. */
static void _uart_comm_recover(const char *reason)
{
    UART_LOG("%s → UART re-init (count=%d, reinit=%d/%d)",
             reason, s_comm_error_count, s_reinit_count + 1, k_reinit_cut_threshold);
    hal_uart_close();
    hal_uart_open();
    s_comm_error_count        = 0;
    g_device_state.start_run1 = false;
    g_device_state.start_run2 = false;
    g_device_state.start_run  = true;
    s_cycle_state             = S_INIT_WAIT;
    s_cycle_tick              = lv_tick_get();

    s_reinit_count++;
    if (s_reinit_count >= k_reinit_cut_threshold) {
        UART_LOG("comm_error persistent after %d reinit attempts → error_cut", s_reinit_count);
        hal_notify_comm_error();
        g_device_state.error_cut = true;
    }
}

/* ---------------------------------------------------------------------------
 * Public API
 * ---------------------------------------------------------------------------*/
void uart_comm_init(void)
{
    UART_LOG("init: opening UART");
    hal_uart_open();

    if (g_device_state.black_out_checking) {
        /* 정전복구: SETDATA(0x10)는 MCU 상태를 IDLE로 초기화하므로 건너뜀.
         * CONDATA(0x11)는 MCU 현재 상태를 조회하며 리셋하지 않으므로 유지.
         * CONDATA_ACK(0x21) 수신 후 MCU가 remain을 정상 보고하기 시작한다. */
        g_device_state.start_run1 = false;   /* SETDATA 건너뜀 */
        g_device_state.start_run2 = false;    /* CONDATA → CONDATA_ACK → STATUS X0 */
        g_device_state.start_run  = true;
        s_blackout_checking_cmd   = true;    /* [미구현-B] 첫 X0에 payload[21]=0x11 마커 */
        UART_LOG("init: blackout recovery — skip SETDATA, CONDATA direct → STATUS X0");
    } else {
        g_device_state.start_run1 = true;
        g_device_state.start_run2 = false;
        g_device_state.start_run  = false;
    }

    s_cycle_state = S_INIT_WAIT;
    s_cycle_tick  = lv_tick_get();
    UART_LOG("init: done, waiting 300ms before first TX");
}

void uart_comm_tick(void)
{
    rtc_sync_periodic_save();

//     /* PSRAM free 로그 — 화면 전환 시마다 출력 (crash 직전 추세 추적용) */
// #if !UI_PRENDERING_ENABLE
//     static lv_obj_t *s_last_scr = NULL;
//     lv_obj_t *_cur_scr = lv_scr_act();
// #else
//     static lv_obj_t *s_last_scr = NULL;
//     extern lv_obj_t *currentPage;
//     lv_obj_t *_cur_scr = currentPage;
// #endif
//     if (_cur_scr != s_last_scr) {
//         s_last_scr = _cur_scr;
//         uint32_t _free_now = (uint32_t)rtos_get_psram_free_heap_size();
//         bk_printf(TAG "[PSRAM] screen_change  free=%u B  min=%u B  t=%lu ms\n",
//                (unsigned)_free_now,
//                (unsigned)rtos_get_psram_minimum_free_heap_size(),
//                (unsigned long)lv_tick_get());

// #if UI_CACHE_DROP_LOW_MEM_ENABLE
//         /* 화면이 바뀔 때마다(=이 블록 진입 시점마다) 체크 — 실제 위험한 decode
//          * 호출 직전(예: ui_lang_apply_picker())에서도 동일 헬퍼를 호출해
//          * 체크 지점을 늘림(custom_func.c, 쿨다운 공유). */
//         ui_cache_drop_if_low_mem();
// #endif
//     }

    switch (s_cycle_state) {

    case S_INIT_WAIT:
        /* 300ms initial delay — mirrors Android Thread.sleep(300) */
        if (lv_tick_elaps(s_cycle_tick) < 300u) return;
        /* first TX — skip the 1000ms wait, go directly */
        s_cycle_tick  = lv_tick_get();
        _write_process();
        s_tx_tick     = lv_tick_get();
        s_cycle_state = S_WAIT_RX;
        return;

    case S_TX:
        if (lv_tick_elaps(s_cycle_tick) < UART_CYCLE_MS) return;
        s_cycle_tick  = lv_tick_get();
        _write_process();
        s_tx_tick     = lv_tick_get();
        s_cycle_state = S_WAIT_RX;
        return;

    case S_WAIT_RX:
        /* Wait UART_RX_WAIT_MS after TX before reading */
        if (lv_tick_elaps(s_tx_tick) < UART_RX_WAIT_MS) return;
        s_cycle_state = S_TX;
        break;
    }

    /* Read and dispatch */
    uint8_t rxbuf[UART_MAX_PACKET];
    int rlen = _uart_read(rxbuf, sizeof(rxbuf));
    if (rlen > 0) {
        _log_hex("RX HEX", rxbuf, rlen);
        uart_packet_t pkt;
        if (uart_parse_frame(rxbuf, (uint8_t)rlen, &pkt)) {
            s_comm_error_count = 0;
            s_reinit_count     = 0;  /* 통신 정상화 → 누적 재초기화 카운트도 리셋 */
            _handle_rx(&pkt);
        } else {
            s_comm_error_count++;
            if (s_comm_error_count <= k_comm_recover_threshold)
                UART_LOG("RX parse error len=%d comm_err=%d/%d", rlen, s_comm_error_count, k_comm_recover_threshold);
            if (s_comm_error_count == k_comm_recover_threshold) {
                _uart_comm_recover("RX parse error");
            }
        }
    } else {
        s_comm_error_count++;
        if (s_comm_error_count <= k_comm_recover_threshold)
            UART_LOG("RX no data (comm_err=%d/%d)", s_comm_error_count, k_comm_recover_threshold);
        if (s_comm_error_count == k_comm_recover_threshold) {
            _uart_comm_recover("RX no data");
        }
    }
}

void uart_comm_trigger_first_start(void)
{
    g_device_state.first_start = true;
    g_device_state.start_run   = false;
}
void uart_comm_trigger_start_run(void)
{
    g_device_state.first_start = false;
    g_device_state.start_run   = true;
}

/* settings flash → send_save_value1[1..22] 재빌드
 * [9]-[12]: AP는 "0.5" 단위 문자열 저장 → MCU는 0.5°C×2=1 단위 정수
 * [23]: MCU 버전 겸 DetailFan — MCU에서 받은 값 그대로 유지 */
static void _rebuild_send_save_value1(void)
{
    device_state_t *st = &g_device_state;
    st->send_save_value1[1]  = settings_get_int("DetailHumidificationTime0");
    st->send_save_value1[2]  = settings_get_int("DetailHumidificationTime1");
    st->send_save_value1[3]  = settings_get_int("DetailWaterInterval0");
    st->send_save_value1[4]  = settings_get_int("DetailWaterInterval1");
    st->send_save_value1[5]  = settings_get_int("DetailHumidificationHeaterTime");
    st->send_save_value1[6]  = (strcmp(settings_get_str("DetailDefrostOnOff"), "ON") == 0) ? 1 : 0;
    st->send_save_value1[7]  = settings_get_int("DetailDefrostTime");
    st->send_save_value1[8]  = settings_get_int("DetailDefrostReturnTemp");
    /* 0.5°C 단위: "0.5" → ×2 → 1, "1.0" → 2 */
#define _F2I(key) ((int)(atof(settings_get_str(key)) * 2.0 + 0.5))
    st->send_save_value1[9]  = _F2I("DetailTempOff");
    st->send_save_value1[10] = _F2I("DetailTempOd");
    st->send_save_value1[11] = _F2I("DetailFermentationTempOff");
    st->send_save_value1[12] = _F2I("DetailFermentationTempOd");
#undef _F2I
    st->send_save_value1[13] = settings_get_int("DetailHumidityOff");
    st->send_save_value1[14] = settings_get_int("DetailHumidityOd");
    st->send_save_value1[15] = (strcmp(settings_get_str("DetailOverFermentationOnOff"), "ON") == 0)
                                ? settings_get_int("DetailOverFermentation") : 0;
    st->send_save_value1[16] = settings_get_int("DetailHumidityRevision");
    st->send_save_value1[17] = settings_get_int("DetailTempRevision");
    st->send_save_value1[18] = settings_get_int("DetailFermentationTempRevision");
    st->send_save_value1[19] = settings_get_int("DetailDamperOnSol");
    st->send_save_value1[20] = settings_get_int("DetailDamperOffSol");
    st->send_save_value1[21] = settings_get_int("DetailDamperFanOn");
    st->send_save_value1[22] = settings_get_int("DetailDamperFanOff");
    /* [23]: MCU 버전/DetailFan — MCU 수신값 보존 (st->mcu_version) */
    st->send_save_value1[23] = st->mcu_version;
}

void uart_comm_trigger_change_setting(void)
{
    _rebuild_send_save_value1();
    g_device_state.change_setting = true;
    g_device_state.start_run      = false;
}

void uart_comm_trigger_hw_test(void)
{
    g_device_state.test_mode  = true;
    g_device_state.start_run  = false;
}

/* 다음 0x33 STATUS TX를 UART_CYCLE_MS(1000ms) 대기 없이 즉시 앞당긴다.
 * 고내등(Lamp) 등 payload[4] 같은 필드는 값만 바꿔두면 원래는 최대 1000ms 뒤
 * 다음 정기 TX 사이클에야 MCU로 전달되어 "반응이 느리다"는 현장 피드백의
 * 원인이 되었다 — 버튼을 누른 즉시 이 함수를 호출해 그 대기를 건너뛴다.
 * S_WAIT_RX(직전 TX의 응답 대기 중)일 때는 건드리지 않는다 — MCU가 아직
 * 응답 중일 수 있어 끼어들면 프레임 충돌/파싱 오류 위험이 있다. 이 경우는
 * 최대 UART_RX_WAIT_MS(200ms) 뒤 자연히 S_TX로 넘어가 다음 tick에 바로 나간다. */
void uart_comm_trigger_immediate_tx(void)
{
    if (s_cycle_state == S_TX) {
        s_cycle_tick = lv_tick_get() - UART_CYCLE_MS;
    }
}
