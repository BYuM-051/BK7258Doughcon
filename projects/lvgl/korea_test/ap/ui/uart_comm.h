/* uart_comm.h — UART communication task (SerialComm.java → C)
 * Implements the same state machine: StartRun1 → 0x10, StartRun2 → 0x11,
 * then periodic 0x33 / 0x31 / 0x30 / 0x50.  RX dispatches to device_state.
 */
#ifndef __UART_COMM_H__
#define __UART_COMM_H__

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Call once at startup; opens UART and starts the comm task */
void uart_comm_init(void);

/* Periodic tick — call every ~200 ms from a timer/task.
 * Runs one TX+RX cycle identical to SerialComm.run(). */
void uart_comm_tick(void);

/* Force re-send of the first-start sequence (0x31) next tick */
void uart_comm_trigger_first_start(void);

void uart_comm_trigger_start_run(void);


/* Force change-setting sequence (0x30) next tick */
void uart_comm_trigger_change_setting(void);

/* Force hardware test sequence (0x50) next tick */
void uart_comm_trigger_hw_test(void);

/* 다음 0x33 STATUS TX를 1000ms 주기 대기 없이 즉시 앞당긴다 — 고내등(Lamp)처럼
 * 즉각 반영되어야 하는 필드를 바꾼 직후 호출한다. 직전 TX의 RX 응답을 기다리는
 * 중이면 무시하고 그 사이클이 끝난 뒤(최대 200ms) 자연히 다음 TX로 넘어간다. */
void uart_comm_trigger_immediate_tx(void);

/* RX sequence counter — incremented on each valid STATUS/CONDATA packet.
 * UI timers compare against their own last-seen value to skip processing
 * when no new UART data has arrived since the previous timer fire. */
extern volatile uint32_t g_uart_rx_seq;

#ifdef __cplusplus
}
#endif
#endif /* __UART_COMM_H__ */
