/* hardware_hal.h — Beken7258 HAL
 *
 * Build mode selection (define before including or in CMakeLists/Makefile):
 *   -DHAL_USE_EMULATOR   PC simulator loopback, no SDK required
 *   (not defined)        BK7258 QFN88 real hardware
 */
#ifndef __HARDWARE_HAL_H__
#define __HARDWARE_HAL_H__

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* GPIO driver must be initialized once at startup (real hardware only).
 * Call hal_gpio_init() from your app_main / device_state_init() before
 * the first hal_led_*_set() call. In emulator mode this is a no-op. */
void hal_gpio_init(void);

/* UART */
void hal_uart_open(void);
void hal_uart_close(void);
void hal_uart_write(const uint8_t *buf, int len);
int  hal_uart_read(uint8_t *buf, int max_len);   /* returns bytes read, 0 on timeout */
void hal_notify_comm_error(void);                /* called when comm error threshold hit */

/* Buzzer */
void hal_buzzer_start(int freq_hz, int duration_ms);
void hal_buzzer_stop(void);
void hal_buzzer_beep(void);       /* standard 80ms key-click beep */
void hal_buzzer_complete(void);   /* 완료음: 250ms ON × 750ms OFF × 10회 */

/* Backlight */
void hal_backlight_set(int level);   /* 0=off, 100=full */

/* System restart — Power 키 3초 이상 길게 눌렀을 때 사용. 실제 하드웨어는
 * bk_reboot()로 즉시 재부팅; 에뮬레이터는 로그만 남기고 종료하지 않는다. */
void hal_system_restart(void);

/* 터치 드라이버 자체를 완전히 열고/닫는다 (drv_tp_open/drv_tp_close 래퍼).
 * false로 닫으면 터치 컨트롤러의 HW 인터럽트/스캔 스레드까지 정지되어, 잠금
 * 중에는 터치 좌표가 아예 큐에 쌓일 수 없다. true로 다시 열면 큐가 새로
 * 생성되어 항상 빈 상태로 시작하므로, 잠금 중 눌렸던 stale 터치가 해제 즉시
 * 처리되는 문제가 원천적으로 발생하지 않는다. */
void hal_touch_set_enabled(bool enabled);

/* LEDs / outputs */
void hal_led_lamp_set(bool on);      /* interior lamp */
void hal_led_lock_set(bool on);      /* lock indicator */
void hal_led_power_set(bool on);     /* power indicator */

/* Lock/Power/Lamp 물리 키(CN2)는 GPIO 27-39 DVP 핀 그룹에서 bk_gpio_get_input()/
 * 인터럽트가 동작하지 않아, 공식 key 컴포넌트(CONFIG_BUTTON, ap/components/key)로
 * 대체됨 — main_activity.c의 _key_driver_start() 참고. */

/* Real-time clock read / write.
 * hal_rtc_get returns true if time came from DS1338Z, false if from AON RTC fallback. */
bool hal_rtc_get(int *year, int *month, int *day,
                 int *hour, int *min,   int *sec);
void hal_rtc_set(int  year, int  month, int  day,
                 int  hour, int  min,   int  sec);

#ifdef __cplusplus
}
#endif
#endif /* __HARDWARE_HAL_H__ */
