#include "lvgl.h"
#include "beken_ui.h"
#include "custom_func.h"
#include "event_runtime.h"
#include <stdio.h>
#include <string.h>

#include "beken_ui.h"
#include "settings.h"
#include "device_state.h"
#include "hardware_hal.h"
#include "preRenderer.h"

#define TAG "[popuppassword_cb.c] "
#define bk_printf(fmt, ...) do {if(0) printf(fmt, ##__VA_ARGS__); } while(0) // disable printf

extern bk_lv_ui_t bk_lv_tool_ui;
extern void destroy_page_popuppassword(bk_lv_ui_t *bk_ui);
extern void init_page_main(bk_lv_ui_t *bk_ui);

/* ── Color test mode (NeurosysPassword "71960") ──────────────────────
 * Android neurosysFragment 동일 흐름:
 *   빨강 → 클릭 → 초록 → 클릭 → 파랑 → 클릭
 *   → 4개 숨겨진 버튼을 좌상→좌하→우상→우하 순서로 터치
 *   → 메인 화면으로 복귀
 * ──────────────────────────────────────────────────────────────────── */
static lv_obj_t *s_ct_screen    = NULL;
static lv_obj_t *s_ct_bg        = NULL;
static lv_obj_t *s_ct_corners[4] = {NULL, NULL, NULL, NULL};
static int       s_ct_phase     = 0;   /* 0=red 1=green 2=blue 3=corners */
static int       s_ct_cidx      = 0;   /* 다음에 눌러야 할 corner 인덱스 */

/* 버튼 위치 (Android dp → px, 1024×600 기준) */
static const int16_t CT_CX[4] = {132, 132, 782, 782};
static const int16_t CT_CY[4] = {124, 384, 124, 384};

static void _ct_async_del(void *arg)
{
    lv_obj_t *obj = (lv_obj_t *)arg;
    if (obj && lv_obj_is_valid(obj))
        lv_obj_del(obj);
}

static void _ct_corner_cb(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    int idx = (int)(intptr_t)lv_event_get_user_data(e);
    if (idx != s_ct_cidx) return;   /* 순서 틀림 — 무시 */
    s_ct_cidx++;
    if (s_ct_cidx < 4) {
        /* 현재 버튼 숨기고 다음 버튼 표시 */
        lv_obj_add_flag(s_ct_corners[idx],         LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(s_ct_corners[s_ct_cidx], LV_OBJ_FLAG_HIDDEN);
    } else {
        /* 4개 모두 완료 → 메인으로 복귀 */
        bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
        lv_obj_t *old = s_ct_screen;
        s_ct_screen = NULL; s_ct_bg = NULL;
        s_ct_phase = 0;     s_ct_cidx = 0;
#if UI_PRENDERING_ENABLE
        ui_page_change(PAGE_MAIN);
#else
        if (bk_ui->main == NULL || !lv_obj_is_valid(bk_ui->main))
            init_page_main(bk_ui);
        lv_scr_load(bk_ui->main);
        lv_async_call(_ct_async_del, old);
#endif /* UI_PRENDERING_ENABLE */
    }
}

static void _ct_bg_cb(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    s_ct_phase++;
    if (s_ct_phase == 1) {
        lv_obj_set_style_bg_color(s_ct_bg, lv_color_make(0, 255, 0), 0);   /* 초록 */
    } else if (s_ct_phase == 2) {
        lv_obj_set_style_bg_color(s_ct_bg, lv_color_make(0, 0, 255), 0);   /* 파랑 */
    } else {
        /* 배경 숨기고 첫 번째 corner 버튼 표시 */
        lv_obj_add_flag(s_ct_bg, LV_OBJ_FLAG_HIDDEN);
        s_ct_cidx = 0;
        lv_obj_clear_flag(s_ct_corners[0], LV_OBJ_FLAG_HIDDEN);
    }
}

static void _ct_open(void)
{
    int i;
    if (s_ct_screen && lv_obj_is_valid(s_ct_screen)) return;
    s_ct_phase = 0; s_ct_cidx = 0;

    s_ct_screen = lv_obj_create(NULL);
    lv_obj_remove_style_all(s_ct_screen);
    lv_obj_set_size(s_ct_screen, 1024, 600);
    lv_obj_set_style_bg_color(s_ct_screen, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(s_ct_screen, LV_OPA_COVER, 0);

    /* 전체화면 컬러 배경 — 초기값 빨강 */
    s_ct_bg = lv_obj_create(s_ct_screen);
    lv_obj_remove_style_all(s_ct_bg);
    lv_obj_set_size(s_ct_bg, 1024, 600);
    lv_obj_set_pos(s_ct_bg, 0, 0);
    lv_obj_set_style_bg_color(s_ct_bg, lv_color_make(255, 0, 0), 0);
    lv_obj_set_style_bg_opa(s_ct_bg, LV_OPA_COVER, 0);
    lv_obj_add_event_cb(s_ct_bg, _ct_bg_cb, LV_EVENT_CLICKED, NULL);

    /* 4개 corner 버튼 — 처음에는 모두 숨김, 순서대로 하나씩 표시 */
    for (i = 0; i < 4; i++) {
        s_ct_corners[i] = lv_button_create(s_ct_screen);
        lv_obj_remove_style_all(s_ct_corners[i]);
        lv_obj_set_pos(s_ct_corners[i], CT_CX[i], CT_CY[i]);
        lv_obj_set_size(s_ct_corners[i], 100, 100);
        lv_obj_set_style_bg_color(s_ct_corners[i], lv_color_make(0, 0, 255), 0);
        lv_obj_set_style_bg_opa(s_ct_corners[i], LV_OPA_COVER, 0);
        lv_obj_set_style_radius(s_ct_corners[i], 8, 0);
        lv_obj_add_flag(s_ct_corners[i], LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_event_cb(s_ct_corners[i], _ct_corner_cb,
                            LV_EVENT_CLICKED, (void *)(intptr_t)i);
    }

    lv_scr_load(s_ct_screen);
}

/* Password entry buffer — shared across keypad callbacks */
static char s_pw_buf[16] = {0};
static uint32_t last_click_time  = 0;  /* enter/dismiss 중복 방지 (250ms) */
static uint32_t last_digit_time  = 0;  /* 숫자/backspace 디바운스 (80ms) */

/* Forward declarations */
void popuppassword_pop_keypad_event_cb(lv_event_t *e);
void popuppassword_pop_keypad2_event_cb(lv_event_t *e);
void popuppassword_pop_keypad3_event_cb(lv_event_t *e);
void popuppassword_pop_keypad4_event_cb(lv_event_t *e);
void popuppassword_pop_keypad5_event_cb(lv_event_t *e);
void popuppassword_pop_keypad6_event_cb(lv_event_t *e);
void popuppassword_pop_keypad7_event_cb(lv_event_t *e);
void popuppassword_pop_keypad8_event_cb(lv_event_t *e);
void popuppassword_pop_keypad9_event_cb(lv_event_t *e);
void popuppassword_pop_keypadbackspace_event_cb(lv_event_t *e);
void popuppassword_pop_keypad0_event_cb(lv_event_t *e);
void popuppassword_pop_keypadalldel_event_cb(lv_event_t *e);
void popuppassword_pop_dismiss_event_cb(lv_event_t *e);
void popuppassword_pop_enter_event_cb(lv_event_t *e);
void popuppassword_load_event_cb(lv_event_t *e);

void popuppassword_open(void)
{
    s_pw_buf[0] = '\0';
    last_digit_time = 0;
    last_click_time = 0;
}

/* Refresh the masked display (show '*' per digit typed) */
static void _refresh_display(bk_lv_ui_t *bk_ui)
{
    size_t len = strlen(s_pw_buf);
    char masked[16];
    size_t i;
    for (i = 0; i < len && i < sizeof(masked) - 1; i++)
        masked[i] = '*';
    masked[i] = '\0';
    /* Use the edt container's first child label, or a label we create on init */
    lv_obj_t *lbl = lv_obj_get_child(bk_ui->popuppassword_pop_edt, 0);
    if (lbl && lv_obj_check_type(lbl, &lv_label_class))
        lv_label_set_text(lbl, masked);
}

static void _pw_digit(bk_lv_ui_t *bk_ui, char d)
{
    uint32_t dt = lv_tick_elaps(last_digit_time);
    bk_printf(TAG "[KEYPAD] '%c' CLICKED  dt_since_last_clicked=%lums%s\n",
           d, (unsigned long)dt, (dt < 80) ? "  -> REJECTED(debounce)" : "");
    if (dt < 80) return;
    last_digit_time = lv_tick_get();
    size_t len = strlen(s_pw_buf);
    if (len < sizeof(s_pw_buf) - 1) {
        s_pw_buf[len]     = d;
        s_pw_buf[len + 1] = '\0';
    }
    hal_buzzer_beep();
    _refresh_display(bk_ui);
}
char* DetailPassword = "0603";
char* NeurosysPassword = "71960";
static void _pw_validate(bk_lv_ui_t *bk_ui)
{
    const char *det = settings_get_str("DetailPassword");
    if (!det || !*det) det = DetailPassword;
    const char *neu = settings_get_str("NeurosysPassword");
    if (!neu || !*neu) neu = NeurosysPassword;

    bk_printf(TAG "Input Password: [%s]\n", s_pw_buf);

    if (strcmp(s_pw_buf, det) == 0) {
        s_pw_buf[0] = '\0';
        destroy_page_popuppassword(bk_ui);
        _ct_open();
    } else if (strcmp(s_pw_buf, neu) == 0) {
        s_pw_buf[0] = '\0';
        destroy_page_popuppassword(bk_ui);
#if UI_PRENDERING_ENABLE
        ui_page_change(PAGE_SETTINGMODEDETAILSETTING);
#else
        if (bk_ui->settingmodedetailsetting == NULL || !lv_obj_is_valid(bk_ui->settingmodedetailsetting))
            init_page_settingmodedetailsetting(bk_ui);
        lv_scr_load(bk_ui->settingmodedetailsetting);
#endif /* UI_PRENDERING_ENABLE */
    } else {
        _img_ensure_src(bk_ui->popuppassword_pop_cautionim);
        lv_obj_clear_flag(bk_ui->popuppassword_pop_cautionim, LV_OBJ_FLAG_HIDDEN);
        s_pw_buf[0] = '\0';
        _refresh_display(bk_ui);
    }
}

/* ---------------------------------------------------------------------------
 * Keypad callbacks
 * ---------------------------------------------------------------------------*/
/* 눌러진 동안만 pop_keypadNim 오버레이(파란 강조 이미지)를 표시.
 * settingmodetime_keypad_event_cb와 동일한 패턴 — PRESSED에서 clear_flag(HIDDEN),
 * RELEASED/PRESS_LOST에서 다시 add_flag(HIDDEN). 이 처리가 없어서 ON 이미지가
 * 전혀 적용되지 않던 문제를 수정.
 *
 * 빠른 탭(짧은 press~release)에서는 순간적으로 켜졌다 꺼져 눈에 잘 안 보인다는
 * 피드백에 따라, RELEASED 시 즉시 숨기지 않고 최소 노출시간(300ms, 기존 체감
 * 순간표시 대비 2배)만큼 유지한 뒤 숨기는 one-shot 타이머를 사용. */
#define _KEYPAD_ON_MIN_MS   450u
#define _KEYPAD_IM_SLOTS    12

/* im 객체의 user_data는 _img_set_src_deferred()가 deferred 경로 문자열을
 * 저장하는 데 이미 쓰고 있으므로(custom_func.c), 숨김 타이머는 별도의
 * 작은 lookup 배열로 추적한다 (user_data를 덮어쓰면 첫 press 때
 * _img_ensure_src()가 문자열 대신 타이머 포인터를 읽어 깨짐). */
static lv_obj_t  *s_keypad_hide_im[_KEYPAD_IM_SLOTS];
static lv_timer_t *s_keypad_hide_timer[_KEYPAD_IM_SLOTS];

static void _keypad_hide_timer_cb(lv_timer_t *t)
{
    lv_obj_t *im = (lv_obj_t *)lv_timer_get_user_data(t);
    if (im && lv_obj_is_valid(im)) lv_obj_add_flag(im, LV_OBJ_FLAG_HIDDEN);
    for (int i = 0; i < _KEYPAD_IM_SLOTS; i++) {
        if (s_keypad_hide_timer[i] == t) { s_keypad_hide_im[i] = NULL; s_keypad_hide_timer[i] = NULL; break; }
    }
}

static void _keypad_cancel_hide_timer(lv_obj_t *im)
{
    for (int i = 0; i < _KEYPAD_IM_SLOTS; i++) {
        if (s_keypad_hide_im[i] == im) {
            lv_timer_delete(s_keypad_hide_timer[i]);
            s_keypad_hide_im[i] = NULL;
            s_keypad_hide_timer[i] = NULL;
            return;
        }
    }
}

static void _keypad_schedule_hide(lv_obj_t *im)
{
    _keypad_cancel_hide_timer(im);
    for (int i = 0; i < _KEYPAD_IM_SLOTS; i++) {
        if (s_keypad_hide_im[i] == NULL) {
            s_keypad_hide_im[i]    = im;
            s_keypad_hide_timer[i] = lv_timer_create(_keypad_hide_timer_cb, _KEYPAD_ON_MIN_MS, im);
            lv_timer_set_repeat_count(s_keypad_hide_timer[i], 1);
            return;
        }
    }
}

/* [KEYPAD] 진단: PRESSED 이벤트 자체가 얼마나 자주 들어오는지 실측 —
 * "연속으로 눌러도 1초에 한 번만 반응"이 이 파일의 디바운스(80ms) 때문이
 * 아니라면, LVGL이 PRESSED를 실제로 얼마나 드물게 주는지부터 확인해야 함. */
static uint32_t s_kp_last_pressed_t = 0;

/* 빠른 탭에서도 최소 300ms(기존 순간표시 대비 2배)는 켜져 있도록 유지 */
static void _keypad_im_track(lv_event_t *e, lv_obj_t *im)
{
    lv_event_code_t code = lv_event_get_code(e);
    /* PRESSED dt 진단 로그는 하이라이트 기능 on/off와 무관하게 항상 남긴다 */
    if (code == LV_EVENT_PRESSED) {
        uint32_t now = lv_tick_get();
        bk_printf(TAG "[KEYPAD] PRESSED  dt_since_last_pressed=%lums\n",
               (unsigned long)(now - s_kp_last_pressed_t));
        s_kp_last_pressed_t = now;
    }
#if !UI_POPUPPASSWORD_KEYPAD_HIGHLIGHT_ENABLE
    (void)im;
    return;  /* 하이라이트 기능 OFF — flag 토글로 인한 invalidate/flush 자체를 없앰 */
#else
    if (!im) return;
    if (code == LV_EVENT_PRESSED) {
        _keypad_cancel_hide_timer(im);  /* 재-press 시 대기 중이던 숨김 취소 */
#if UI_POPUPPASSWORD_KEYPAD_LAZY_PRESS_ENABLE
        /* 이 버튼을 처음 누르는 순간에만 실제 decode (이후 재사용) */
        _img_ensure_src(im);
#else
        /* _img_ensure_src(im) 불필요 — SCREEN_LOAD_START에서 이미 _img_set_src_timed로
         * 즉시 로드되어 있음. automode/org와 동일하게 flag 토글만 수행. */
#endif
        lv_obj_clear_flag(im, LV_OBJ_FLAG_HIDDEN);
    } else if (code == LV_EVENT_RELEASED || code == LV_EVENT_PRESS_LOST) {
        lv_obj_add_flag(im, LV_OBJ_FLAG_HIDDEN);
    }
#endif
}

void popuppassword_pop_keypad1_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    _keypad_im_track(e, bk_ui->popuppassword_pop_keypad1im);
    if (lv_event_get_code(e) != LV_EVENT_RELEASED) return;
    _pw_digit(bk_ui, '1');
}

void popuppassword_pop_keypad2_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    _keypad_im_track(e, bk_ui->popuppassword_pop_keypad2im);
    if (lv_event_get_code(e) != LV_EVENT_RELEASED) return;
    _pw_digit(bk_ui, '2');
}

void popuppassword_pop_keypad3_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    _keypad_im_track(e, bk_ui->popuppassword_pop_keypad3im);
    if (lv_event_get_code(e) != LV_EVENT_RELEASED) return;
    _pw_digit(bk_ui, '3');
}

void popuppassword_pop_keypad4_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    _keypad_im_track(e, bk_ui->popuppassword_pop_keypad4im);
    if (lv_event_get_code(e) != LV_EVENT_RELEASED) return;
    _pw_digit(bk_ui, '4');
}

void popuppassword_pop_keypad5_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    _keypad_im_track(e, bk_ui->popuppassword_pop_keypad5im);
    if (lv_event_get_code(e) != LV_EVENT_RELEASED) return;
    _pw_digit(bk_ui, '5');
}

void popuppassword_pop_keypad6_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    _keypad_im_track(e, bk_ui->popuppassword_pop_keypad6im);
    if (lv_event_get_code(e) != LV_EVENT_RELEASED) return;
    _pw_digit(bk_ui, '6');
}

void popuppassword_pop_keypad7_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    _keypad_im_track(e, bk_ui->popuppassword_pop_keypad7im);
    if (lv_event_get_code(e) != LV_EVENT_RELEASED) return;
    _pw_digit(bk_ui, '7');
}

void popuppassword_pop_keypad8_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    _keypad_im_track(e, bk_ui->popuppassword_pop_keypad8im);
    if (lv_event_get_code(e) != LV_EVENT_RELEASED) return;
    _pw_digit(bk_ui, '8');
}

void popuppassword_pop_keypad9_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    _keypad_im_track(e, bk_ui->popuppassword_pop_keypad9im);
    if (lv_event_get_code(e) != LV_EVENT_RELEASED) return;
    _pw_digit(bk_ui, '9');
}

void popuppassword_pop_keypad0_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    _keypad_im_track(e, bk_ui->popuppassword_pop_keypad0im);
    if (lv_event_get_code(e) != LV_EVENT_RELEASED) return;
    _pw_digit(bk_ui, '0');
}

void popuppassword_pop_keypadbackspace_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    _keypad_im_track(e, bk_ui->popuppassword_pop_keypadbackspaceim);
    if (lv_event_get_code(e) != LV_EVENT_RELEASED) return;
    if (lv_tick_elaps(last_digit_time) < 80) return;
    last_digit_time = lv_tick_get();
    size_t len = strlen(s_pw_buf);
    if (len > 0) s_pw_buf[len - 1] = '\0';
    hal_buzzer_beep();
    _refresh_display(bk_ui);
}

void popuppassword_pop_keypadalldel_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    _keypad_im_track(e, bk_ui->popuppassword_pop_keypadalldelim);
    if (lv_event_get_code(e) != LV_EVENT_RELEASED) return;
    if (lv_tick_elaps(last_digit_time) < 80) return;
    last_digit_time = lv_tick_get();
    s_pw_buf[0] = '\0';
    hal_buzzer_beep();
    _refresh_display(bk_ui);
}

void popuppassword_pop_dismiss_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    if (lv_tick_elaps(last_click_time) < 250) return;
    last_click_time = lv_tick_get();
    s_pw_buf[0] = '\0';
    hal_buzzer_beep();
    /* lv_layer_top() 오버레이 제거 — active screen(settingmode)이 그대로 표시됨 */
    destroy_page_popuppassword(bk_ui);
}

void popuppassword_pop_enter_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    if (lv_tick_elaps(last_click_time) < 250) return;
    last_click_time = lv_tick_get();
    hal_buzzer_beep();
    _pw_validate(bk_ui);
}

void popuppassword_load_event_cb(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_SCREEN_LOAD_START) return;
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;

#if UI_POPUPPASSWORD_KEYPAD_LAZY_PRESS_ENABLE
    /* 실제 decode는 하지 않고 경로만 user_data에 저장 — 각 버튼을 처음 누를 때
     * _keypad_im_track()의 _img_ensure_src()가 그 순간 디코드한다. */
    if (!lv_obj_get_user_data(bk_ui->popuppassword_pop_keypad1im)) {
        _img_set_src_deferred(bk_ui->popuppassword_pop_keypad1im,         "/images/pop_keypad1.png");
        _img_set_src_deferred(bk_ui->popuppassword_pop_keypad2im,         "/images/pop_keypad2.png");
        _img_set_src_deferred(bk_ui->popuppassword_pop_keypad3im,         "/images/pop_keypad3.png");
        _img_set_src_deferred(bk_ui->popuppassword_pop_keypad4im,         "/images/pop_keypad4.png");
        _img_set_src_deferred(bk_ui->popuppassword_pop_keypad5im,         "/images/pop_keypad5.png");
        _img_set_src_deferred(bk_ui->popuppassword_pop_keypad6im,         "/images/pop_keypad6.png");
        _img_set_src_deferred(bk_ui->popuppassword_pop_keypad7im,         "/images/pop_keypad7.png");
        _img_set_src_deferred(bk_ui->popuppassword_pop_keypad8im,         "/images/pop_keypad8.png");
        _img_set_src_deferred(bk_ui->popuppassword_pop_keypad9im,         "/images/pop_keypad9.png");
        _img_set_src_deferred(bk_ui->popuppassword_pop_keypadbackspaceim, "/images/pop_keypadback.png");
        _img_set_src_deferred(bk_ui->popuppassword_pop_keypad0im,         "/images/pop_keypad0.png");
        _img_set_src_deferred(bk_ui->popuppassword_pop_keypadalldelim,    "/images/pop_keypadalldel.png");
    }
#else
    /* Lazy-load 12 keypad images on first show (deferred from init — saves ~960ms) */
    if (!lv_image_get_src(bk_ui->popuppassword_pop_keypad1im)) {
        _img_set_src_timed(bk_ui->popuppassword_pop_keypad1im,         "/images/pop_keypad1.png");
        _img_set_src_timed(bk_ui->popuppassword_pop_keypad2im,         "/images/pop_keypad2.png");
        _img_set_src_timed(bk_ui->popuppassword_pop_keypad3im,         "/images/pop_keypad3.png");
        _img_set_src_timed(bk_ui->popuppassword_pop_keypad4im,         "/images/pop_keypad4.png");
        _img_set_src_timed(bk_ui->popuppassword_pop_keypad5im,         "/images/pop_keypad5.png");
        _img_set_src_timed(bk_ui->popuppassword_pop_keypad6im,         "/images/pop_keypad6.png");
        _img_set_src_timed(bk_ui->popuppassword_pop_keypad7im,         "/images/pop_keypad7.png");
        _img_set_src_timed(bk_ui->popuppassword_pop_keypad8im,         "/images/pop_keypad8.png");
        _img_set_src_timed(bk_ui->popuppassword_pop_keypad9im,         "/images/pop_keypad9.png");
        _img_set_src_timed(bk_ui->popuppassword_pop_keypadbackspaceim, "/images/pop_keypadback.png");
        _img_set_src_timed(bk_ui->popuppassword_pop_keypad0im,         "/images/pop_keypad0.png");
        _img_set_src_timed(bk_ui->popuppassword_pop_keypadalldelim,    "/images/pop_keypadalldel.png");
    }
#endif

    /* Clear display and caution on every open */
    lv_obj_t *lbl = lv_obj_get_child(bk_ui->popuppassword_pop_edt, 0);
    if (!lbl || !lv_obj_check_type(lbl, &lv_label_class)) {
        /* Create display label inside the edt container if not already present */
        lbl = lv_label_create(bk_ui->popuppassword_pop_edt);
        lv_obj_align(lbl, LV_ALIGN_LEFT_MID, 4, 0);
    }
    lv_label_set_text(lbl, "");
    lv_obj_add_flag(bk_ui->popuppassword_pop_cautionim, LV_OBJ_FLAG_HIDDEN);
    s_pw_buf[0] = '\0';
}
