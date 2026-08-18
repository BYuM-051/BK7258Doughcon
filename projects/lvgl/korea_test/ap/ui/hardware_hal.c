/* hardware_hal.c — Beken7258 HAL
 *
 * Compile-time selection:
 *   -DHAL_USE_EMULATOR   →  PC simulator in-process loopback (no SDK required)
 *   (not defined)        →  BK7258 QFN88 reference board, real SDK calls
 *
 * BK7258 QFN88 pin assignment (Neurosys new board):
 *   RS-485 UART  : UART_ID_2   GPIO_30(RX) / GPIO_31(TX)  (JPEG not used, pins are free)
 *   Buzzer PWM   : PWM chan 6  GPIO_32  (26 MHz clock, 50% duty, freq programmable)
 *   Buzzer timer : TIMER chan 1 (one-shot auto-stop; chan 0 reserved for OS us-timer)
 *   Backlight    : GPIO_9   (direct GPIO, not PWM)
 *   Lamp LED     : GPIO_12
 *   Lock LED     : GPIO_13
 */

#include <time.h>
#include <string.h>
#include <stdio.h>
#include "hardware_hal.h"

/* ================================================================
 * EMULATOR — PC simulator / loopback (HAL_USE_EMULATOR)
//  * ================================================================*/
// #define HAL_USE_EMULATOR
#ifdef HAL_USE_EMULATOR

#include "uart_protocol.h"

/* ----------------------------------------------------------------
 * PC Real UART support (USB-to-UART adapter)
 * Set env var  HAL_UART_PORT=COM3  (Windows) or  HAL_UART_PORT=/dev/ttyUSB0  (Linux)
 * before launching the simulator.  If not set, falls back to loopback.
 * ----------------------------------------------------------------*/
#ifdef _WIN32
#  include <windows.h>
static HANDLE   s_com     = INVALID_HANDLE_VALUE;
#else
#  include <fcntl.h>
#  include <termios.h>
#  include <unistd.h>
#  include <errno.h>
static int      s_com     = -1;
#endif
static int      s_pc_uart = 4;   /* 1 = real COM port open */

/* --- helpers --- */
#ifdef _WIN32
static int _com_open(const char *port)
{
    char path[64];
    snprintf(path, sizeof(path), "\\\\.\\%s", port);
    s_com = CreateFileA(path, GENERIC_READ | GENERIC_WRITE,
                        0, NULL, OPEN_EXISTING, 0, NULL);
    if (s_com == INVALID_HANDLE_VALUE) {
        printf("[HAL PC-UART] CreateFile(%s) failed err=%lu\n", port, GetLastError());
        return -1;
    }
    DCB dcb = {0};
    dcb.DCBlength  = sizeof(dcb);
    dcb.BaudRate   = CBR_9600;
    dcb.ByteSize   = 8;
    dcb.Parity     = NOPARITY;
    dcb.StopBits   = ONESTOPBIT;
    if (!SetCommState(s_com, &dcb)) {
        printf("[HAL PC-UART] SetCommState failed err=%lu\n", GetLastError());
        CloseHandle(s_com); s_com = INVALID_HANDLE_VALUE; return -1;
    }
    /* Non-blocking read: return immediately with bytes available */
    COMMTIMEOUTS ct = { MAXDWORD, 0, 0, 0, 0 };
    SetCommTimeouts(s_com, &ct);
    PurgeComm(s_com, PURGE_RXCLEAR | PURGE_TXCLEAR);
    printf("[HAL PC-UART] opened %s (9600 8N1)\n", port);
    return 0;
}
static void _com_close(void)
{
    if (s_com != INVALID_HANDLE_VALUE) { CloseHandle(s_com); s_com = INVALID_HANDLE_VALUE; }
}
static int _com_write(const uint8_t *buf, int len)
{
    DWORD written = 0;
    WriteFile(s_com, buf, (DWORD)len, &written, NULL);
    return (int)written;
}
static int _com_read(uint8_t *buf, int maxlen)
{
    DWORD got = 0;
    ReadFile(s_com, buf, (DWORD)maxlen, &got, NULL);
    return (int)got;
}
#else  /* Linux / macOS */
static int _com_open(const char *port)
{
    s_com = open(port, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (s_com < 0) { perror("[HAL PC-UART] open"); return -1; }
    struct termios tio = {0};
    cfsetispeed(&tio, B9600); cfsetospeed(&tio, B9600);
    tio.c_cflag = CS8 | CLOCAL | CREAD;
    tio.c_iflag = IGNPAR;
    tio.c_oflag = 0; tio.c_lflag = 0;
    tio.c_cc[VMIN] = 0; tio.c_cc[VTIME] = 0;
    tcflush(s_com, TCIOFLUSH);
    tcsetattr(s_com, TCSANOW, &tio);
    printf("[HAL PC-UART] opened %s (9600 8N1)\n", port);
    return 0;
}
static void _com_close(void)  { if (s_com >= 0) { close(s_com); s_com = -1; } }
static int _com_write(const uint8_t *buf, int len) { return (int)write(s_com, buf, len); }
static int _com_read(uint8_t *buf, int maxlen)
{
    int n = (int)read(s_com, buf, maxlen);
    return (n < 0 && errno == EAGAIN) ? 0 : (n < 0 ? 0 : n);
}
#endif

/* Single-frame RX response buffer (loopback mode only) */
static uint8_t s_rx_frame[UART_MAX_PACKET];
static int     s_rx_len = 0;

/*
 * Default settings sent by MCU in 0x20 SETDATA_ACK (23 data bytes).
 */
static const uint8_t k_setdata_defaults[23] = {
     5,   /* DetailHumidificationTime0              */
     0,   /* DetailHumidificationTime1 (decimal)    */
     2,   /* DetailWaterInterval0                   */
     0,   /* DetailWaterInterval1                   */
    30,   /* DetailHumidificationHeaterTime         */
     1,   /* DetailDefrostOnOff  (1=ON)             */
     6,   /* DetailDefrostTime                      */
    10,   /* DetailDefrostReturnTemp                */
     0,   /* DetailTempOff    (x0.5 -> 0.0)         */
     3,   /* DetailTempOd     (x0.5 -> 1.5)         */
     0,   /* DetailFermentationTempOff              */
     6,   /* DetailFermentationTempOd (x0.5 -> 3.0) */
     0,   /* DetailHumidityOff                      */
     3,   /* DetailHumidityOd                       */
     0,   /* DetailOverFermentation (0=OFF)         */
     0,   /* DetailHumidityRevision                 */
     0,   /* DetailTempRevision                     */
     0,   /* DetailFermentationTempRevision         */
   120,   /* DetailDamperOnSol                      */
   120,   /* DetailDamperOffSol                     */
     4,   /* DetailDamperFanOn                      */
     2,   /* DetailDamperFanOff                     */
     1,   /* DetailFan  (0=ON, 1=OFF -> default OFF) */
};

static void _emu_queue(uint8_t cmd, const uint8_t *data, int data_len)
{
    uint8_t payload[UART_MAX_DATA];
    if (1 + data_len > (int)sizeof(payload)) return;

    payload[0] = cmd;
    if (data && data_len > 0)
        memcpy(&payload[1], data, data_len);

    int flen = uart_build_frame(payload, (uint8_t)(1 + data_len),
                                s_rx_frame, sizeof(s_rx_frame));
    s_rx_len = (flen > 0) ? flen : 0;
    // printf("[HAL EMU] queued RX 0x%02X data_len=%d frame_len=%d\n",
        //    cmd, data_len, flen);
}

/* Read port name from file or env var.
 * Priority: 1) env HAL_UART_PORT  2) file uart_port.cfg  3) loopback */
static const char *_get_port(char *buf, int bufsz)
{
    const char *env = getenv("HAL_UART_PORT");
    if (env && env[0] != '\0') return env;

    FILE *f = fopen("uart_port.cfg", "r");
    if (f) {
        if (fgets(buf, bufsz, f)) {
            /* strip CR/LF */
            char *p = buf + strlen(buf) - 1;
            while (p >= buf && (*p == '\r' || *p == '\n' || *p == ' ')) *p-- = '\0';
            if (buf[0] != '\0') { fclose(f); return buf; }
        }
        fclose(f);
    }
    return NULL;
}

void hal_uart_open(void)
{
    s_rx_len = 0;
    char cfg_buf[64] = {0};
    const char *port = _get_port(cfg_buf, sizeof(cfg_buf));
    if (port) {
        s_pc_uart = (_com_open(port) == 0) ? 1 : 0;
        if (!s_pc_uart)
            printf("[HAL PC-UART] open failed — falling back to loopback\n");
    } else {
        s_pc_uart = 0;
        printf("[HAL EMU] UART open (loopback)\n");
        printf("[HAL EMU]   → real board: set HAL_UART_PORT=COM5\n");
        printf("[HAL EMU]   → or create build/uart_port.cfg containing: COM5\n");
    }
}

void hal_uart_close(void)
{
    if (s_pc_uart) { _com_close(); s_pc_uart = 0; printf("[HAL PC-UART] closed\n"); }
    else           { printf("[HAL EMU] UART close\n"); }
    s_rx_len = 0;
}

void hal_uart_write(const uint8_t *buf, int len)
{
    if (!buf || len < 4 || buf[0] != UART_STX) return;

    /* Real board: send directly to USB-UART */
    if (s_pc_uart) {
        int sent = _com_write(buf, len);
        printf("[HAL PC-UART] TX cmd=0x%02X %d/%d bytes\n", buf[2], sent, len);
        return;
    }

    /* Loopback emulator */
    uint8_t tx_cmd = buf[2];
    // printf("[HAL EMU] TX 0x%02X received (frame %d bytes)\n", tx_cmd, len);

    switch (tx_cmd) {

    case CMD_TX_SETDATA:     /* 0x10 -> 0x20 + 23 bytes default settings */
        _emu_queue(CMD_RX_SETDATA_ACK, k_setdata_defaults, 23);
        break;

    case CMD_TX_CONDATA: {   /* 0x11 -> 0x21 + 20 bytes (curr state + op params) */
        uint8_t d[20] = {
            20,             /* curr_temp: 20 C       */
            65,             /* curr_humidity: 65%    */
            (uint8_t)(-18), 24, 30,  /* freeze: -18C, 24h, 30min */
             2,  0, 30,     /* defrost:  2C,  0h, 30min */
            25, 80,  4,  0, /* ferm1:   25C, 80%, 4h, 0min */
            10, 85,  2,  0, /* ferm2:   10C, 85%, 2h, 0min */
            30, 75,  1, 30, /* dry:     30C, 75%, 1h, 30min */
        };
        _emu_queue(CMD_RX_CONDATA_ACK, d, 20);
        break;
    }

    case CMD_TX_CHANGE: {    /* 0x30 -> 0x40 ACK */
        uint8_t ack = 0x41;
        _emu_queue(CMD_RX_CHANGE_ACK, &ack, 1);
        break;
    }

    case CMD_TX_FIRST_START: { /* 0x31 -> 0x41 ACK */
        uint8_t ack = 0x41;
        _emu_queue(CMD_RX_FIRST_ACK, &ack, 1);
        break;
    }

    case CMD_TX_STATUS: {    /* 0x33 -> 0x43 + 14 bytes idle status */
        uint8_t status[14] = {0};
        _emu_queue(CMD_RX_STATUS, status, 14);
        break;
    }

    case CMD_TX_HW_TEST: {   /* 0x50 -> 0x60 + 14 bytes test result */
        uint8_t result[14] = {0};
        _emu_queue(CMD_RX_HW_TEST_ACK, result, 14);
        break;
    }

    default:
        printf("[HAL EMU] unknown TX cmd=0x%02X -- no response\n", tx_cmd);
        break;
    }
}

int hal_uart_read(uint8_t *buf, int max_len)
{
    /* Real board: read directly from USB-UART (non-blocking) */
    if (s_pc_uart) {
        int n = _com_read(buf, max_len);
        if (n > 0) printf("[HAL PC-UART] RX %d bytes (first=0x%02X)\n", n, buf[0]);
        return n;
    }
    /* Loopback emulator */
    if (s_rx_len > 0 && s_rx_len <= max_len) {
        memcpy(buf, s_rx_frame, s_rx_len);
        int ret = s_rx_len;
        s_rx_len = 0;
        return ret;
    }
    return 0;
}

void hal_notify_comm_error(void)
{
    printf("[HAL EMU] !!! comm_error threshold reached\n");
}

/* --- Buzzer (emulator: no-op with log) --- */
void hal_buzzer_start(int freq_hz, int duration_ms)
{
    printf("[HAL EMU] buzzer start freq=%d ms=%d\n", freq_hz, duration_ms);
}
void hal_buzzer_stop(void)
{
    printf("[HAL EMU] buzzer stop\n");
}
void hal_buzzer_beep(void)
{
    hal_buzzer_start(4000, 40);
}
void hal_buzzer_complete(void)
{
    printf("[HAL EMU] buzzer complete (10x 250ms ON / 750ms OFF)\n");
}

/* --- Backlight (emulator: log only) --- */
void hal_backlight_set(int level)
{
    printf("[HAL EMU] backlight level=%d\n", level);
}

/* --- System restart (emulator: log only, no real reboot) --- */
void hal_system_restart(void)
{
    printf("[HAL EMU] system restart requested (no-op in emulator)\n");
}

/* --- Touch enable/disable (emulator: no-op, no drv_tp) --- */
void hal_touch_set_enabled(bool enabled)
{
    printf("[HAL EMU] touch %s\n", enabled ? "enabled" : "disabled");
}

/* --- GPIO init (emulator: no-op) --- */
void hal_gpio_init(void) { /* nothing needed in emulator */ }

/* --- LEDs (emulator: log only) --- */
void hal_led_lamp_set(bool on)  { printf("[HAL EMU] lamp  %s\n", on ? "ON" : "OFF"); }
void hal_led_lock_set(bool on)  { printf("[HAL EMU] lock  %s\n", on ? "ON" : "OFF"); }
void hal_led_power_set(bool on) { printf("[HAL EMU] power %s\n", on ? "ON" : "OFF"); }

/* --- RTC (emulator: virtual clock with offset so get/set stay consistent) --- */
static time_t s_emu_rtc_base   = 0; /* absolute time at last hal_rtc_set; 0 = use time(NULL) */
static time_t s_emu_rtc_set_at = 0; /* time(NULL) value when hal_rtc_set was called */

static time_t _emu_now(void)
{
    if (s_emu_rtc_base == 0) return time(NULL);
    return s_emu_rtc_base + (time(NULL) - s_emu_rtc_set_at);
}

bool hal_rtc_get(int *year, int *month, int *day,
                 int *hour, int *min, int *sec)
{
    time_t now = _emu_now();
    struct tm *t = localtime(&now);
    *year  = t->tm_year + 1900;
    *month = t->tm_mon  + 1;
    *day   = t->tm_mday;
    *hour  = t->tm_hour;
    *min   = t->tm_min;
    *sec   = t->tm_sec;
    return true;
}
void hal_rtc_set(int year, int month, int day,
                 int hour, int min,   int sec)
{
    struct tm t = {0};
    t.tm_year = year  - 1900;
    t.tm_mon  = month - 1;
    t.tm_mday = day;
    t.tm_hour = hour;
    t.tm_min  = min;
    t.tm_sec  = sec;
    t.tm_isdst = -1;
    s_emu_rtc_base   = mktime(&t);
    s_emu_rtc_set_at = time(NULL);
    printf("[HAL EMU] rtc set %04d-%02d-%02d %02d:%02d:%02d\n",
           year, month, day, hour, min, sec);
}

/* ================================================================
 * REAL HARDWARE — BK7258 QFN88 reference board
 * ================================================================*/
#else /* HAL_USE_EMULATOR */

#include <sys/time.h>
#include <driver/uart.h>
#include <driver/gpio.h>
#include "gpio_driver.h"
#include <driver/pwm.h>
#include <driver/aon_rtc.h>
#include <driver/i2c.h>
#include <os/os.h>
#include <components/system.h>   /* bk_reboot() */
#include <driver/drv_tp.h>       /* drv_tp_read(), tp_point_infor_t, TP_DATA_QUEUE_MAX_SIZE */
#include "lvgl.h"
#include "device_state.h"

/* ----------------------------------------------------------------
 * Pin / channel constants
 * ----------------------------------------------------------------*/
#define HAL_COMM_UART_ID        UART_ID_2   /* GPIO_30=RX, GPIO_31=TX (JPEG unused) */

#define HAL_BUZZER_PWM_CHAN     6           /* GPIO_32, PWM6           */
#define HAL_BUZZER_CLK_HZ       26000000u   /* PWM clock source: 26 MHz XTAL */

/* Lock/Power/Lamp 키+LED 확장보드 (CN2 커넥터) — 회로도 원본 배정.
 * 키는 active-low 모멘터리 스위치(눌림=LOW, 내부 pull-up) 가정. */
#define HAL_GPIO_LOCK           GPIO_33     /* Lock LED */
#define HAL_GPIO_POWER          GPIO_34     /* Power LED */
#define HAL_GPIO_LAMP           GPIO_35     /* Lamp LED */
/* 키(Lock=GPIO_27/Power=GPIO_28/Lamp=GPIO_29) 자체는 이 파일이 아니라 공식
 * key 컴포넌트(CONFIG_BUTTON)가 main_activity.c의 s_key_configs[]로 직접
 * 설정/구동한다 — 문서화용으로만 남겨둠. */

/* ----------------------------------------------------------------
 * UART — interrupt-driven RX ring buffer
 * ----------------------------------------------------------------*/
#define HAL_RX_BUF_SIZE  256

static uint8_t          s_rx_buf[HAL_RX_BUF_SIZE];
static volatile int     s_rx_head = 0;
static volatile int     s_rx_tail = 0;

static void _hal_uart_rx_isr(uart_id_t id, void *param)
{
    (void)param;
    uint8_t ch;
    while (bk_uart_read_bytes(id, &ch, 1, 0) == 1) {
        int next = (s_rx_head + 1) % HAL_RX_BUF_SIZE;
        if (next != s_rx_tail) {   /* drop on overflow */
            s_rx_buf[s_rx_head] = ch;
            s_rx_head = next;
        }
    }
}

void hal_uart_open(void)
{
    uart_config_t cfg = {
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_NONE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_FLOWCTRL_DISABLE,
        .src_clk   = UART_SCLK_XTAL_26M,
        .rx_dma_en = 0,
        .tx_dma_en = 0,
    };
    bk_uart_init(HAL_COMM_UART_ID, &cfg);
    bk_uart_register_rx_isr(HAL_COMM_UART_ID, _hal_uart_rx_isr, NULL);
    bk_uart_set_rx_timeout(HAL_COMM_UART_ID, 3);  /* 3-byte idle -> IRQ */
    bk_uart_enable_rx_interrupt(HAL_COMM_UART_ID);
    s_rx_head = s_rx_tail = 0;
}

void hal_uart_close(void)
{
    bk_uart_disable_rx_interrupt(HAL_COMM_UART_ID);
    bk_uart_deinit(HAL_COMM_UART_ID);
    s_rx_head = s_rx_tail = 0;
}

void hal_uart_write(const uint8_t *buf, int len)
{
    if (!buf || len <= 0) return;
    bk_uart_write_bytes(HAL_COMM_UART_ID, (char *)buf, (uint32_t)len);
}

int hal_uart_read(uint8_t *buf, int max_len)
{
    int n = 0;
    while (n < max_len && s_rx_tail != s_rx_head) {
        buf[n++] = s_rx_buf[s_rx_tail];
        s_rx_tail = (s_rx_tail + 1) % HAL_RX_BUF_SIZE;
    }
    return n;
}

void hal_notify_comm_error(void) { /* UI error popup은 uart_comm.c에서 처리 */ }

/* ----------------------------------------------------------------
 * Buzzer — PWM6(GPIO_32) + FreeRTOS software timer for auto-stop
 * Hardware timer ISR cannot safely call bk_pwm_stop() (PWM driver
 * uses mutex internally).  LVGL timers were avoided because they
 * only fire when lv_timer_handler() runs — which can lag by 1+ sec
 * when the LVGL task is blocked rendering flash-backed images.
 * FreeRTOS Timer Daemon Task runs independently: reliable 80ms stop.
 * ----------------------------------------------------------------*/
static bool          s_buzzer_pwm_inited = false;
static beken_timer_t s_bz_timer;
static bool          s_bz_timer_init    = false;

static void _bz_timer_cb(void *arg)
{
    (void)arg;
    bk_pwm_stop(HAL_BUZZER_PWM_CHAN);
    rtos_stop_timer(&s_bz_timer);   /* prevent periodic re-fire */
}

void hal_buzzer_start(int freq_hz, int duration_ms)
{
    if (freq_hz <= 0) return;

    if (!s_buzzer_pwm_inited) {
        bk_pwm_driver_init();
        s_buzzer_pwm_inited = true;
    }

    uint32_t period = HAL_BUZZER_CLK_HZ / (uint32_t)freq_hz;
    pwm_init_config_t cfg = {
        .period_cycle = period,
        .duty_cycle   = period / 2,
        .duty2_cycle  = 0,
        .duty3_cycle  = 0,
        .psc          = 0,
    };
    bk_pwm_stop(HAL_BUZZER_PWM_CHAN);
    bk_pwm_init(HAL_BUZZER_PWM_CHAN, &cfg);
    bk_pwm_start(HAL_BUZZER_PWM_CHAN);

    if (duration_ms > 0) {
        if (!s_bz_timer_init) {
            rtos_init_timer(&s_bz_timer, (uint32_t)duration_ms,
                            _bz_timer_cb, NULL);
            s_bz_timer_init = true;
        }
        rtos_reload_timer(&s_bz_timer);  /* start or reset 80ms countdown */
    }
}

void hal_buzzer_stop(void)
{
    if (s_bz_timer_init) rtos_stop_timer(&s_bz_timer);
    bk_pwm_stop(HAL_BUZZER_PWM_CHAN);
}

void hal_buzzer_beep(void)
{
    if (g_device_state.mute) return;
    hal_buzzer_start(3000,60);
}

/* 완료 부저: 250ms ON + 750ms OFF × 10회 (Android BuzzerCompleteRunnable 동일)
 * LVGL 타이머로 비동기 시퀀싱 — LVGL 태스크 블로킹 없음. */
static lv_timer_t *s_bz_cpl_timer = NULL;
static int         s_bz_cpl_count = 0;
static bool        s_bz_cpl_on    = false;

static void _bz_complete_cb(lv_timer_t *t)
{
    if (s_bz_cpl_on) {
        /* 250ms ON 종료 → 부저 정지 */
        hal_buzzer_stop();
        s_bz_cpl_count++;
        if (s_bz_cpl_count >= 10) {
            lv_timer_delete(t);
            s_bz_cpl_timer = NULL;
            s_bz_cpl_count = 0;
            return;
        }
        s_bz_cpl_on = false;
        lv_timer_set_period(t, 750);   /* 다음 발화: 750ms OFF 후 */
    } else {
        /* 750ms OFF 종료 → 다음 비프 시작 */
        hal_buzzer_start(3000, 0);     /* duration=0: RTOS 자동정지 없음 */
        s_bz_cpl_on = true;
        lv_timer_set_period(t, 250);   /* 다음 발화: 250ms ON 후 */
    }
}

void hal_buzzer_complete(void)
{
    if (g_device_state.mute) return;
    if (s_bz_cpl_timer) {
        lv_timer_delete(s_bz_cpl_timer);
        s_bz_cpl_timer = NULL;
        hal_buzzer_stop();
    }
    s_bz_cpl_count = 0;
    s_bz_cpl_on    = true;
    hal_buzzer_start(3000, 0);         /* 첫 번째 비프 (4kHz, auto-stop 없음) */
    s_bz_cpl_timer = lv_timer_create(_bz_complete_cb, 250, NULL);
}

/* ----------------------------------------------------------------
 * Backlight — direct GPIO_9 (ap_main.c의 lcd_backlight_open/close와 동일 시퀀스).
 * ap_main.c의 두 함수는 부팅/deinit 시점에만 호출되고 static이라 런타임에서
 * 재사용할 수 없어, 여기서 동일 로직을 그대로 구현한다.
 * ----------------------------------------------------------------*/
#define HAL_BACKLIGHT_PIN   GPIO_9

void hal_backlight_set(int level)
{
    if (level > 0) {
        gpio_dev_unmap(HAL_BACKLIGHT_PIN);
        bk_gpio_enable_output(HAL_BACKLIGHT_PIN);
        bk_gpio_pull_up(HAL_BACKLIGHT_PIN);
        bk_gpio_set_output_high(HAL_BACKLIGHT_PIN);
    } else {
        bk_gpio_pull_down(HAL_BACKLIGHT_PIN);
        bk_gpio_set_output_low(HAL_BACKLIGHT_PIN);
    }
}

/* ----------------------------------------------------------------
 * System restart — Power 키 3초 이상 길게 누름 처리에서 호출.
 * bk_reboot()는 반환하지 않음(즉시 재부팅).
 * ----------------------------------------------------------------*/
void hal_system_restart(void)
{
    printf("[HAL] system restart — bk_reboot()\n");
    bk_reboot();
}

/* ----------------------------------------------------------------
 * Touch enable/disable — drv_tp.c의 내부 큐(최대 30개)는 LVGL indev
 * enable/disable과 무관하게 하드웨어 인터럽트로 계속 채워지므로, 큐를
 * 비우는(flush) 방식은 flush 직후에도 계속 들어오는 새 터치와 경쟁하는
 * 구조적 한계가 있었다. 대신 drv_tp_close()/drv_tp_open()으로 터치
 * 드라이버 자체(HW 인터럽트 + 스캔 스레드)를 완전히 정지/재시작한다 —
 * 잠금 중에는 터치 좌표가 물리적으로 쌓일 수 없고, 재오픈 시 큐가 새로
 * 생성되어 항상 빈 상태로 시작한다.
 * ----------------------------------------------------------------*/
void hal_touch_set_enabled(bool enabled)
{
    if (enabled) {
        drv_tp_open(LV_HOR_RES, LV_VER_RES, TP_MIRROR_NONE);
        printf("[HAL] touch driver re-opened (enabled)\n");
    } else {
        drv_tp_close();
        printf("[HAL] touch driver closed (disabled) — HW interrupt + scan thread stopped\n");
    }
}

/* ----------------------------------------------------------------
 * GPIO driver init — SDK already calls bk_gpio_driver_init() at boot
 * (calling it again here would reset GPIO_0/1 to HW I2C1, breaking
 *  Sim-I2C0 that drv_tp_open() configures for GT911).
 *
 * Lock/Power/Lamp 키+LED 확장보드(CN2): GPIO_27/28/29 = 키 입력(active-low,
 * pull-up), GPIO_33/34/35 = LED 출력. usr_gpio_cfg.h 기본 테이블에서 이 핀들이
 * JPEG/I2S1 2nd function으로 매핑되어 있으나(카메라·I2S 미사용이라 실제로는 dead),
 * gpio_dev_unmap()으로 매핑을 해제한 뒤 순수 GPIO로 전환한다 (ap_main.c의
 * lcd_backlight_open()과 동일 패턴). ----------------------------------------------------------------*/
static void _gpio_out(gpio_id_t id, bool on);

/* Lock/Power/Lamp 키(GPIO_27/28/29)는 공식 key 컴포넌트(CONFIG_BUTTON,
 * ap/components/key, main_activity.c의 _key_driver_start())가 자체적으로
 * gpio_dev_unmap/enable_input/enable_pull/pull_up까지 전부 설정하므로 여기서는
 * LED 출력 핀만 초기화한다. */
void hal_gpio_init(void)
{
    //  bk_gpio_driver_init();
    printf("[HAL][GPIO] hal_gpio_init() enter\n");

    static const gpio_id_t _led_pins[3] = { HAL_GPIO_LOCK, HAL_GPIO_POWER, HAL_GPIO_LAMP };
    static const char * const _led_names[3] = { "LOCK_LED(33)", "POWER_LED(34)", "LAMP_LED(35)" };
    for (int i = 0; i < 3; i++) {
        bk_err_t r1 = gpio_dev_unmap(_led_pins[i]);
        bk_err_t r2 = bk_gpio_enable_output(_led_pins[i]);
        printf("[HAL][GPIO] %s unmap=%d enable_output=%d\n", _led_names[i], (int)r1, (int)r2);
        _gpio_out(_led_pins[i], false);   /* LED 기본 OFF */
    }

    printf("[HAL][GPIO] hal_gpio_init() done\n");
}

/* ----------------------------------------------------------------
 * LEDs — GPIO_33(Lock) / GPIO_34(Power) / GPIO_35(Lamp) output.
 * 실측 결과 LED가 active-low로 배선되어 있음 (GPIO LOW = LED ON) — 극성 반전.
 * ----------------------------------------------------------------*/
static void _gpio_out(gpio_id_t id, bool on)
{
    if (on) bk_gpio_set_output_low(id);
    else    bk_gpio_set_output_high(id);
}

void hal_led_lamp_set(bool on)  { _gpio_out(HAL_GPIO_LAMP, on); }
void hal_led_lock_set(bool on)  { _gpio_out(HAL_GPIO_LOCK, on); }
void hal_led_power_set(bool on) { _gpio_out(HAL_GPIO_POWER, on); }

/* ----------------------------------------------------------------
 * DS1338Z RTC — sim I2C ID 2 (GPIO_38=SCL, GPIO_39=SDA), same bus as GT911.
 * Bus is protected by mutex inside sim_i2c_driver; DS1338Z addr 0x68.
 * DS1307-compatible register map; CH bit (bit7 of seconds reg) must be 0.
 * ----------------------------------------------------------------*/
#define DS1338_I2C_ID   2       /* SIM_I2C_START_ID = SOC_I2C_UNIT_NUM = 2 */
#define DS1338_ADDR     0x68    /* 7-bit device address */

static inline uint8_t _bcd2bin(uint8_t b) { return (b >> 4) * 10 + (b & 0x0F); }
static inline uint8_t _bin2bcd(uint8_t d) { return ((d / 10) << 4) | (d % 10); }

/* Both nibbles must be valid BCD digits, upper nibble ≤ max_tens.
 * Catches 0xFF (nibble = 0xF > 9) that appears when SDA is stuck high. */
static inline bool _bcd_ok(uint8_t val, uint8_t max_tens)
{
    return (val & 0x0F) <= 9 && (val >> 4) <= max_tens;
}

static bool s_ds1338_inited = false;

static bk_err_t _ds1338_read(uint8_t reg, uint8_t *buf, uint8_t len);
static bk_err_t _ds1338_write(uint8_t reg, const uint8_t *buf, uint8_t len);

static bk_err_t _ds1338_init(void)
{
    if (s_ds1338_inited) return BK_OK;
    i2c_config_t cfg = {
        .baud_rate  = I2C_BAUD_RATE_100KHZ,
        .addr_mode  = I2C_ADDR_MODE_7BIT,
        .slave_addr = 0,
    };
    bk_err_t r = bk_i2c_init((i2c_id_t)DS1338_I2C_ID, &cfg);
    if (r != BK_OK) {
        printf("[RTC] DS1338Z I2C init FAIL (id=%d err=%d)\n", DS1338_I2C_ID, (int)r);
        return r;
    }
    s_ds1338_inited = true;
    printf("[RTC] DS1338Z I2C init OK (id=%d addr=0x%02X)\n", DS1338_I2C_ID, DS1338_ADDR);

    /* SRAM write/read test (reg 0x08): checks whether I2C write path works at all.
     * DS1338Z SRAM is battery-backed static RAM — independent of timekeeping registers.
     * PASS → write path OK; if time-reg write still fails → VCC/write-protect issue.
     * FAIL → fundamental I2C write path broken (hardware, pull-up, or speed issue). */
    uint8_t sram_w = 0xA5, sram_r = 0x00;
    bk_err_t ws = _ds1338_write(0x08, &sram_w, 1);
    bk_err_t rs = _ds1338_read(0x08, &sram_r, 1);
    if (ws != BK_OK || rs != BK_OK) {
        printf("[RTC] DS1338Z SRAM test I/O FAIL (wr=%d rd=%d)\n", (int)ws, (int)rs);
    } else if (sram_r == sram_w) {
        printf("[RTC] DS1338Z SRAM test PASS (0x%02X OK) — write path functional\n", sram_r);
    } else {
        printf("[RTC] DS1338Z SRAM test FAIL wrote=0x%02X read=0x%02X — VCC or write-protect issue\n",
               sram_w, sram_r);
    }
    return BK_OK;
}

/* DS1338_I2C_ID=2 → Simulated I2C (GPIO_38=SCL, GPIO_39=SDA).
 * bk_i2c_master_write/read are hardware-I2C-only (i2c_driver.c) — return -11522 for id=2.
 * Use bk_i2c_memory_read/write which route through i2c_unified.c →
 * bk_i2c_memory_read/write_v2 → I2cMemRead/Write (GPIO bit-bang).
 * sim_i2c_driver.c is patched to propagate NACK (BK_FAIL) instead of always BK_OK. */
static bk_err_t _ds1338_read(uint8_t reg, uint8_t *buf, uint8_t len)
{
    for (uint8_t i = 0; i < len; i++) {
        i2c_mem_param_t p = {
            .dev_addr      = DS1338_ADDR,
            .mem_addr      = (uint8_t)(reg + i),
            .mem_addr_size = I2C_MEM_ADDR_SIZE_8BIT,
            .data          = &buf[i],
            .data_size     = 1,
            .timeout_ms    = 100,
        };
        bk_err_t r = bk_i2c_memory_read((i2c_id_t)DS1338_I2C_ID, &p);
        if (r != BK_OK) {
            printf("[RTC] DS1338Z read NACK byte=%d reg=0x%02X err=%d\n", i, reg + i, (int)r);
            return r;
        }
    }
    return BK_OK;
}

static bk_err_t _ds1338_write(uint8_t reg, const uint8_t *buf, uint8_t len)
{
    for (uint8_t i = 0; i < len; i++) {
        i2c_mem_param_t p = {
            .dev_addr      = DS1338_ADDR,
            .mem_addr      = (uint8_t)(reg + i),
            .mem_addr_size = I2C_MEM_ADDR_SIZE_8BIT,
            .data          = (uint8_t *)&buf[i],
            .data_size     = 1,
            .timeout_ms    = 100,
        };
        bk_err_t r = bk_i2c_memory_write((i2c_id_t)DS1338_I2C_ID, &p);
        if (r != BK_OK) {
            printf("[RTC] DS1338Z write NACK byte=%d reg=0x%02X val=0x%02X err=%d\n",
                   i, reg + i, buf[i], (int)r);
            return r;
        }
    }
    return BK_OK;
}

/* ----------------------------------------------------------------
 * RTC — DS1338Z primary, BK7258 AON RTC fallback
 * hal_rtc_get: reads DS1338Z; falls back to AON RTC on failure.
 *   Returns true  → time from DS1338Z (battery-backed, reliable).
 *   Returns false → time from AON RTC fallback (may be stale after power cycle).
 * hal_rtc_set: writes DS1338Z + syncs AON RTC for time() calls.
 * ----------------------------------------------------------------*/
bool hal_rtc_get(int *year, int *month, int *day,
                 int *hour, int *min,   int *sec)
{
    if (_ds1338_init() == BK_OK) {
        uint8_t regs[7];
        bk_err_t rd = _ds1338_read(0x00, regs, 7);
        if (rd == BK_OK) {
            printf("[RTC] DS1338Z raw: %02X %02X %02X %02X %02X %02X %02X  CH=%d\n",
                   regs[0], regs[1], regs[2], regs[3], regs[4], regs[5], regs[6],
                   (regs[0] >> 7) & 1);

            /* CH=1 (bit7 of seconds): oscillator stopped.
             * Try to restart: write seconds register with CH=0 then re-read. */
            if (regs[0] & 0x80) {
                uint8_t clr = regs[0] & 0x7F;
                printf("[RTC] DS1338Z CH=1 — clearing CH bit\n");
                if (_ds1338_write(0x00, &clr, 1) == BK_OK) {
                    rtos_delay_milliseconds(5);
                    _ds1338_read(0x00, regs, 7);
                    printf("[RTC] DS1338Z after CH clear: %02X CH=%d\n",
                           regs[0], (regs[0] >> 7) & 1);
                }
            }

            /* BCD nibble validation — each field's upper and lower nibbles must be
             * valid decimal digits (≤9). This catches 0xFF (nibble=0xF) that appears
             * when SDA is stuck high or VBAT missing (reference: DS1307 DS1307_GetTime_callback). */
            uint8_t sc_raw = regs[0] & 0x7F;   /* strip CH bit */
            uint8_t mn_raw = regs[1] & 0x7F;
            uint8_t hr_raw = regs[2] & 0x3F;   /* strip 12/24 bit */
            uint8_t dy_raw = regs[4] & 0x3F;
            uint8_t mo_raw = regs[5] & 0x1F;
            uint8_t yr_raw = regs[6];

            bool bcd_ok =
                _bcd_ok(sc_raw, 5) &&           /* seconds:  00–59, tens ≤ 5 */
                _bcd_ok(mn_raw, 5) &&           /* minutes:  00–59 */
                _bcd_ok(hr_raw, 2) &&           /* hours:    00–23, tens ≤ 2 */
                _bcd_ok(dy_raw, 3) && dy_raw != 0x00 &&  /* date: 01–31 */
                _bcd_ok(mo_raw, 1) && mo_raw != 0x00 &&  /* month: 01–12 */
                _bcd_ok(yr_raw, 9);             /* year:     00–99 */

            if (!bcd_ok) {
                printf("[RTC] DS1338Z BCD invalid — AON fallback\n");
                /* fall through */
            } else {
                uint8_t yr2 = _bcd2bin(yr_raw);
                uint8_t mo  = _bcd2bin(mo_raw);
                uint8_t dy  = _bcd2bin(dy_raw);
                if (yr2 >= 20 && yr2 <= 99) {
                    *sec   = _bcd2bin(sc_raw);
                    *min   = _bcd2bin(mn_raw);
                    *hour  = _bcd2bin(hr_raw);
                    *day   = dy;
                    *month = mo;
                    *year  = yr2 + 2000;
                    printf("[RTC] hal_rtc_get → DS1338Z: %04d-%02d-%02d %02d:%02d:%02d\n",
                           *year, *month, *day, *hour, *min, *sec);
                    return true;
                }
                printf("[RTC] DS1338Z year out of range: yr2=%d\n", yr2);
            }
        } else {
            printf("[RTC] DS1338Z read FAIL (err=%d)\n", (int)rd);
        }
    }

    /* DS1338Z not accessible or data invalid — fall back to BK7258 AON RTC */
    struct timeval tv;
    bk_rtc_gettimeofday(&tv, NULL);
    struct tm *t = localtime(&tv.tv_sec);
    *year  = t->tm_year + 1900;
    *month = t->tm_mon  + 1;
    *day   = t->tm_mday;
    *hour  = t->tm_hour;
    *min   = t->tm_min;
    *sec   = t->tm_sec;
    printf("[RTC] hal_rtc_get → AON: %04d-%02d-%02d %02d:%02d:%02d (epoch=%lu)\n",
           *year, *month, *day, *hour, *min, *sec, (unsigned long)tv.tv_sec);
    return false;
}

void hal_rtc_set(int year, int month, int day,
                 int hour, int min,   int sec)
{
    printf("[RTC] hal_rtc_set: %04d-%02d-%02d %02d:%02d:%02d\n",
           year, month, day, hour, min, sec);

    /* Write DS1338Z — sec BCD bit7=0 → CH=0 → oscillator starts */
    if (_ds1338_init() == BK_OK) {
        uint8_t regs[7];
        regs[0] = _bin2bcd((uint8_t)sec);          /* CH=0 implicit */
        regs[1] = _bin2bcd((uint8_t)min);
        regs[2] = _bin2bcd((uint8_t)hour);         /* 24h: bit6=0 */
        regs[3] = 1;                                /* day-of-week unused */
        regs[4] = _bin2bcd((uint8_t)day);
        regs[5] = _bin2bcd((uint8_t)month);
        regs[6] = _bin2bcd((uint8_t)(year - 2000));
        bk_err_t wr = _ds1338_write(0x00, regs, 7);
        printf("[RTC] DS1338Z write %s  regs: %02X %02X %02X %02X %02X %02X %02X\n",
               wr == BK_OK ? "OK" : "FAIL",
               regs[0], regs[1], regs[2], regs[3], regs[4], regs[5], regs[6]);

        /* Write 후 readback — BCD 일치 여부로 하드웨어 이상 / 배터리 불량 감지 */
        if (wr == BK_OK) {
            rtos_delay_milliseconds(5);
            uint8_t rb[7] = {0};
            bk_err_t rd = _ds1338_read(0x00, rb, 7);
            if (rd == BK_OK) {
                /* sec(CH 제거), min, hr(12/24 비트 제거), date, month, year 비교 */
                bool ok = ((rb[0] & 0x7F) == regs[0]) &&
                          ( rb[1]         == regs[1]) &&
                          ((rb[2] & 0x3F) == regs[2]) &&
                          ( rb[4]         == regs[4]) &&
                          ((rb[5] & 0x1F) == regs[5]) &&
                          ( rb[6]         == regs[6]);
                printf("[RTC] DS1338Z readback %s  regs: %02X %02X %02X %02X %02X %02X %02X\n",
                       ok ? "OK" : "MISMATCH",
                       rb[0], rb[1], rb[2], rb[3], rb[4], rb[5], rb[6]);
                if (!ok)
                    printf("[RTC] DS1338Z readback MISMATCH — 배터리 불량 또는 하드웨어 이상\n");
            } else {
                printf("[RTC] DS1338Z readback FAIL (err=%d)\n", (int)rd);
            }
        }
    } else {
        printf("[RTC] DS1338Z unavailable — AON only\n");
    }

    /* Sync BK7258 AON RTC so time() stays valid */
    struct tm t = {0};
    t.tm_year  = year  - 1900;
    t.tm_mon   = month - 1;
    t.tm_mday  = day;
    t.tm_hour  = hour;
    t.tm_min   = min;
    t.tm_sec   = sec;
    t.tm_isdst = -1;
    time_t epoch = mktime(&t);
    struct timeval  tv = { .tv_sec = epoch, .tv_usec = 0 };
    struct timezone tz = {0};
    bk_rtc_settimeofday(&tv, &tz);
    printf("[RTC] AON sync: epoch=%lu\n", (unsigned long)epoch);
}

#endif /* HAL_USE_EMULATOR */
