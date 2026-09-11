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

/* Recovery marker: send STATUS[21]=0x11 until STATUS response[14]=0x21. */
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
        /* SerialComm.SaveValue2: temp/h/m for freeze and defrost;
         * temp/h/m/humidity for fermentation and optional dry stage. */
        if (pkt->data_len >= 2)
        {
            st->current_temp = (int8_t)pkt->data[0];
            st->current_humidity = pkt->data[1];
        }
        if (!st->black_out_checking && pkt->data_len >= 16)
        {
            st->send_freeze_temp    = (int8_t)pkt->data[2];
            st->send_freeze_hour    = pkt->data[3];
            st->send_freeze_min     = pkt->data[4];
            st->send_defreeze_temp  = (int8_t)pkt->data[5];
            st->send_defreeze_hour  = pkt->data[6];
            st->send_defreeze_min   = pkt->data[7];
            st->send_ferm1_temp     = (int8_t)pkt->data[8];
            st->send_ferm1_hour     = pkt->data[9];
            st->send_ferm1_min      = pkt->data[10];
            st->send_ferm1_humidity = pkt->data[11];
            st->send_ferm2_temp     = (int8_t)pkt->data[12];
            st->send_ferm2_hour     = pkt->data[13];
            st->send_ferm2_min      = pkt->data[14];
            st->send_ferm2_humidity = pkt->data[15];
            if (pkt->data_len >= 20)
            {
                st->send_dry_temp     = (int8_t)pkt->data[16];
                st->send_dry_hour     = pkt->data[17];
                st->send_dry_min      = pkt->data[18];
                st->send_dry_humidity = pkt->data[19];
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
        /* Legacy STATUS has 13 data bytes, Turbo has the extra confirmation byte.
         * Do not combine a short packet with stale fields from the last response. */
        if (pkt->data_len < 13 || pkt->data[10] >= 60 || pkt->data[12] >= 60)
        {
            return;
        }
        int previousStatus[5] = {st->saveoperation[5], st->saveoperation[10],
                                 st->saveoperation[11], st->saveoperation[12],
                                 st->saveoperation[13]};
        int len = pkt->data_len < 14 ? pkt->data_len : 14;
        st->saveoperation[0] = pkt->cmd;
        st->saveoperation[14] = 0;
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
        bk_printf(TAG "[UART] RX 0x43 STATUS op=0x%02X err=%02X%02X%02X%02X remain=%dh%dm"
                    " [14]=0x%02X bo=%d\n",
                st->saveoperation[5], err6, err7, err8, err9,
                st->saveoperation[10], st->saveoperation[11],
                (uint8_t)st->saveoperation[14],
                (int)st->black_out_checking);

        /* 0x21 confirms the recovery request; it is not a communication error. */
        if (st->black_out_checking && st->saveoperation[14] == 0x21) {
            /* 정전복구 MCU 에코: 이전 단계 X1 확인 코드 — 정상 */
            s_blackout_checking_cmd = false;
            s_error_counting = 0;
            st->error_popup  = false;
            st->error_cut    = false;
            UART_LOG("RX 0x43 blackout echo [14]=0x21 → normal confirm, cmd_flag cleared");
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

        /* The controller owns stage transitions. A local UI timer must not advance
         * a phase or persist a phase paired with another phase's remaining time. */
        uint8_t op = (uint8_t)st->saveoperation[5];
        int previousMode = st->current_op_mode;
        int reportedMode = _mode_from_op_byte(op);
        bool accepted = (op >= 0x10 && op <= 0x12) ||
                        (op >= 0x20 && op <= 0x22) ||
                        (op >= 0x30 && op <= 0x34) ||
                        (op >= 0x40 && op <= 0x44) ||
                        (op >= 0x50 && op <= 0x52);
        if (st->first_receive)
        {
            accepted = accepted && reportedMode == previousMode &&
                       ((op & 0x0F) == 0 || (op & 0x0F) == 1);
            if (accepted)
            {
                st->first_receive = false;
            }
        }
        else if (st->auto_mode_start && reportedMode < previousMode)
        {
            accepted = false;
        }

        if (accepted)
        {
            st->current_op_mode = reportedMode;
            bool initialEcho = st->black_out_checking && reportedMode == previousMode &&
                               (op & 0x0F) == 0;
            if (!initialEcho)
            {
                /* A new phase may have MORE remaining time than the old phase.
                 * Preserve the controller's value instead of a global decrease-only guard. */
                st->remain_hour = st->saveoperation[10];
                st->remain_min  = st->saveoperation[11];
            }
            st->elapsed_hour = st->saveoperation[12];
            st->elapsed_min  = st->saveoperation[13];

            if (st->operation)
            {
                settings_set_int("saveOperationTemp", st->current_op_mode);
                settings_set_int("saveCurrentRemainHour", st->remain_hour);
                settings_set_int("saveCurrentRemainMin", st->remain_min);
                static uint32_t statusSaveTick = 0;
                if (previousMode != reportedMode || lv_tick_elaps(statusSaveTick) >= 30000)
                {
                    statusSaveTick = lv_tick_get();
                    settings_save_dirty();
                }
            }
        }
        else
        {
            /* TX echoes these fields. Rejecting only the UI mode would still
             * send a stale earlier phase back to the controller next cycle. */
            st->saveoperation[5] = previousStatus[0];
            for (int i = 0; i < 4; i++) st->saveoperation[10 + i] = previousStatus[1 + i];
        }

        if (accepted && st->over_ferm_active && (op == 0x34 || op == 0x44))
        {
            st->over_ferm_active = false;
            st->over_ferm_jeon_started = true;
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
    uint8_t payload[UART_MAX_DATA] = {0};
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
                    st->current_op_mode = _mode_from_op_byte(st->first_operator_mode);
                } else if (st->first_receive) {
                    /* Retry the requested X0 until a matching stage is received.
                     * A stale previous-run STATUS must not replace the recovery command. */
                    drive_mode = (uint8_t)st->first_operator_mode;
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

        /* Midnight is valid: the full date distinguishes 00:00 from no deadline.
         * Android uses the current seconds in both date/time fields. */
        if (st->operation && !st->manual_start)
        {
            payload[11] = (uint8_t)(st->send_complete_year % 100);
            payload[12] = (uint8_t)st->send_complete_month;
            payload[13] = (uint8_t)st->send_complete_day;
            payload[14] = (uint8_t)st->send_complete_hour;
            payload[15] = (uint8_t)st->send_complete_min;
            payload[16] = (uint8_t)sc;
        }

        /* SerialComm.writeProcess: [17/18] elapsed, [19/20] remaining.
         * Sending the original total as remaining extends the controller's run. */
        bool phaseDone = drive_mode == 0x12 || drive_mode == 0x22 ||
                         drive_mode == 0x32 || drive_mode == 0x42 ||
                         drive_mode == 0x52;
        if (st->operation && !phaseDone)
        {
            int elapsed = st->saveoperation[12] * 60 + st->saveoperation[13];
            int remaining = st->remain_hour * 60 + st->remain_min;
            if (st->black_out_checking && !st->manual_start)
            {
                int total = 0;
                switch (_mode_from_op_byte(drive_mode))
                {
                    case OP_MODE_FREEZE: total = st->bo_freeze_total_min; break;
                    case OP_MODE_DEFROST: total = st->bo_defrost_total_min; break;
                    case OP_MODE_FERM1: total = st->bo_ferm1_total_min; break;
                    case OP_MODE_FERM2: total = st->bo_ferm2_total_min; break;
                    case OP_MODE_DRY: total = st->send_dry_hour * 60 + st->send_dry_min; break;
                    default: break;
                }
                elapsed = total - remaining;
            }
            else if (drive_mode == 0x50 || drive_mode == 0x51)
            {
                elapsed = st->send_dry_hour * 60 + st->send_dry_min - remaining;
            }
            else if (!st->manual_start && (drive_mode & 0x0F) == 0)
            {
                elapsed = 0;
            }
            if (st->manual_start)
            {
                remaining = 0;
            }
            if (elapsed < 0) elapsed = 0;
            payload[17] = (uint8_t)(elapsed / 60);
            payload[18] = (uint8_t)(elapsed % 60);
            payload[19] = (uint8_t)(remaining / 60);
            payload[20] = (uint8_t)(remaining % 60);
        }
        payload[21] = ((st->black_out_checking && s_blackout_checking_cmd) ||
                       st->error_popup) ? 0x11 : 0x00;
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
        /* Resume directly with STATUS X0. Do not reset/reload saved parameters. */
        g_device_state.start_run1 = false;
        g_device_state.start_run2 = false;
        g_device_state.start_run = true;
        s_blackout_checking_cmd = true;
    } else {
        s_blackout_checking_cmd = false;
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
