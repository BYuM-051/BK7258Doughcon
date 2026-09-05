#include "lvgl.h"
#include "beken_ui.h"
#include "custom_func.h"
#include "event_runtime.h"
#include <stdio.h>
#include <string.h>
#include <os/os.h>

#include "beken_ui.h"
#include "ui_animations.h"
#include "ui_lang.h"
#include "settings.h"
#include "device_state.h"
#include "uart_comm.h"
#include "hardware_hal.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "pageManager.h"
#define TAG "[manualmodestart_cb.c] "
// #define bk_printf(fmt, ...) do {if(0) printf(fmt, ##__VA_ARGS__); } while(0) // disable printf

extern bk_lv_ui_t bk_lv_tool_ui;

static uint32_t s_last_click_manualmodestart = 0;
static uint32_t s_last_click_hide_mms        = 0;
static int      s_tci_manualmodestart        = 0;
static char     s_save_manualmodestart[32]   = {0};
static char     s_edit_buf_manualmodestart[32] = {0};
static lv_timer_t *s_ui_timer_mms    = NULL;
/* 조건부 갱신 캐시 — 동일값 반복 set 시 lv_malloc+lv_obj_invalidate 방지 */
static int         s_last_temp_mms   = 0x7FFFFFFF;
static int         s_last_hum_mms    = 0x7FFFFFFF;
static lv_obj_t   *s_mms_drop_clip   = NULL;   /* 해동/발효 상단 클립 컨테이너 */
static lv_obj_t   *s_mms_drop_img    = NULL;   /* 클립 안 이미지 */
static lv_obj_t   *s_mms_ferm_btm_clip  = NULL; /* 발효 하단 고정 클립 컨테이너 */
static lv_obj_t   *s_mms_ferm_btm_img   = NULL; /* 하단 고정 이미지 */
static lv_obj_t   *s_mms_ferm_inner     = NULL; /* 발효 상단 내부 클립 (애니메이션 대상) */
static lv_obj_t   *s_mms_ferm_top_img   = NULL; /* 발효 상단 이미지 */
static lv_timer_t *s_run_anim_timer_mms = NULL;
static int32_t     s_arc_angle_mms      = 0;
static int32_t     s_gif_val_mms        = 0;
static int         s_gif_type_mms       = 0;    /* 1=냉동회전 2=해동낙하 3=발효상승 */
/* 저온발효 bg: 최초 1회 JPEG decode → raw buffer 상주
 * canvas는 HIDDEN (buffer 소유자 역할), bg_img는 (0,0) lv_image child (arc보다 z-order 뒤) */
static lv_obj_t *s_ferm2_bg_canvas  = NULL;
static lv_obj_t *s_ferm2_bg_img     = NULL;
static void     *s_ferm2_canvas_buf  = NULL;
static int       s_ferm2_buf_lang    = -1;
static lv_obj_t *s_run_arc_bg_white  = NULL; /* 회색 배경 링 바깥쪽 5px 흰색 테두리 */
static lv_obj_t *s_run_arc_inner_white = NULL; /* 회색 배경 링 안쪽 7px 흰색 테두리 */

/* Keypad helper declarations */
static void _keypad_input_manualmodestart(bk_lv_ui_t *bk_ui, char digit);
static void _keypad_minor_manualmodestart(bk_lv_ui_t *bk_ui);
static void _keypad_backspace_manualmodestart(bk_lv_ui_t *bk_ui);
static void _keypad_hide_manualmodestart(bk_lv_ui_t *bk_ui);

void manualmodestart_backbt_event_cb(lv_event_t *e);
void manualmodestart_startbt_event_cb(lv_event_t *e);
void manualmodestart_manual_freeze_temp_bt_event_cb(lv_event_t *e);
void manualmodestart_manual_defrost_temp_bt_event_cb(lv_event_t *e);
void manualmodestart_manual_fermentation_temp_bt_event_cb(lv_event_t *e);
void manualmodestart_manual_fermentation_humidity_bt_event_cb(lv_event_t *e);
void manualmodestart_keypad_event_cb(lv_event_t *e);
void manualmodestart_keypadhide_event_cb(lv_event_t *e);
void manualmodestart_load_event_cb(lv_event_t *e);
static void _keypad_on_manualmodestart(bk_lv_ui_t *bk_ui);
static void _keypad_off_manualmodestart(bk_lv_ui_t *bk_ui);


static lv_obj_t *_get_target_label_mms(bk_lv_ui_t *bk_ui)
{
    switch (s_tci_manualmodestart) {
        case 1: return bk_ui->manualmodestart_manual_freeze_temp_txt;
        case 2: return bk_ui->manualmodestart_manual_defrost_temp_txt;
        case 3: return bk_ui->manualmodestart_manual_fermentation_temp_txt;
        case 4: return bk_ui->manualmodestart_manual_fermentation_humidity_txt;
        default: return NULL;
    }
}

/* Hide all underbars. They are created by the step initializer. */
static void _underbar_all_hide_mms(bk_lv_ui_t *bk_ui)
{
    if (bk_ui->manualmodestart_manual_freeze_temp_underbar)
        lv_obj_add_flag(bk_ui->manualmodestart_manual_freeze_temp_underbar,            LV_OBJ_FLAG_HIDDEN);
    if (bk_ui->manualmodestart_manual_defrost_temp_underbar)
        lv_obj_add_flag(bk_ui->manualmodestart_manual_defrost_temp_underbar,           LV_OBJ_FLAG_HIDDEN);
    if (bk_ui->manualmodestart_manual_fermentation_temp_underbar)
        lv_obj_add_flag(bk_ui->manualmodestart_manual_fermentation_temp_underbar,      LV_OBJ_FLAG_HIDDEN);
    if (bk_ui->manualmodestart_manual_fermentation_humidity_underbar)
        lv_obj_add_flag(bk_ui->manualmodestart_manual_fermentation_humidity_underbar,  LV_OBJ_FLAG_HIDDEN);
}

/* Show only the underbar for the active field */
static void _underbar_show_mms(bk_lv_ui_t *bk_ui)
{
    switch (s_tci_manualmodestart) {
        case 1: lv_obj_clear_flag(bk_ui->manualmodestart_manual_freeze_temp_underbar,           LV_OBJ_FLAG_HIDDEN); break;
        case 2: lv_obj_clear_flag(bk_ui->manualmodestart_manual_defrost_temp_underbar,          LV_OBJ_FLAG_HIDDEN); break;
        case 3: lv_obj_clear_flag(bk_ui->manualmodestart_manual_fermentation_temp_underbar,     LV_OBJ_FLAG_HIDDEN); break;
        case 4: lv_obj_clear_flag(bk_ui->manualmodestart_manual_fermentation_humidity_underbar, LV_OBJ_FLAG_HIDDEN); break;
        default: break;
    }
}

static void _maxmin_mms(bk_lv_ui_t *bk_ui)
{
    device_state_t *state = &g_device_state;

    int is_f = (strcmp(settings_get_str("Degree"), "\xc2\xb0""F") == 0);
    char _buf[16];
    int _v;
    _v = atoi(lv_label_get_text(bk_ui->manualmodestart_manual_freeze_temp_txt));
    if (is_f) { if (_v < 5)   _v = 5;   if (_v > 32)  _v = 32; }  /* F: 5~32 */
    else       { if (_v < -15) _v = -15; if (_v > 0)   _v = 0;  }  /* C:-15~0 */
    snprintf(_buf, sizeof(_buf), "%02d", _v);
    lv_label_set_text(bk_ui->manualmodestart_manual_freeze_temp_txt, _buf);

    _v = atoi(lv_label_get_text(bk_ui->manualmodestart_manual_defrost_temp_txt));
    if (is_f) { if (_v < 32) _v = 32; if (_v > 73) _v = 73; }     /* F: 32~73 */
    else       { if (_v < 0)  _v = 0;  if (_v > 23) _v = 23; }     /* C:  0~23 */
    snprintf(_buf, sizeof(_buf), "%02d", _v);
    lv_label_set_text(bk_ui->manualmodestart_manual_defrost_temp_txt, _buf);

    _v = atoi(lv_label_get_text(bk_ui->manualmodestart_manual_fermentation_temp_txt));
    if (is_f) { if (_v < 41) _v = 41; if (_v > 104) _v = 104; }   /* F: 41~104 */
    else       { if (_v < 5) _v = 5; if (_v > 40)  _v = 40;  }   /* C: 5~40 */
    snprintf(_buf, sizeof(_buf), "%d", _v);
    lv_label_set_text(bk_ui->manualmodestart_manual_fermentation_temp_txt, _buf);

    _v = atoi(lv_label_get_text(bk_ui->manualmodestart_manual_fermentation_humidity_txt));
    if (_v < 30) _v = 30;
    if (_v > 90) _v = 90;
    snprintf(_buf, sizeof(_buf), "%02d", _v);
    lv_label_set_text(bk_ui->manualmodestart_manual_fermentation_humidity_txt, _buf);
    /* reset all label colors to normal after clamping */
    lv_obj_set_style_text_color(bk_ui->manualmodestart_manual_freeze_temp_txt,        lv_color_hex(0x162A9E), 0);
    lv_obj_set_style_text_color(bk_ui->manualmodestart_manual_defrost_temp_txt,       lv_color_hex(0x53BAE4), 0);

    lv_obj_set_style_text_color(bk_ui->manualmodestart_manual_fermentation_temp_txt,  lv_color_hex(0xD1232A), 0);
    lv_obj_set_style_text_color(bk_ui->manualmodestart_manual_fermentation_humidity_txt, lv_color_hex(0xD1232A), 0);
    if(state->manual_current_mode == MANUAL_MODE_FERM2){
        lv_obj_set_style_text_color(bk_ui->manualmodestart_manual_fermentation_temp_txt,  lv_color_hex(0xD4A020), 0);
        lv_obj_set_style_text_color(bk_ui->manualmodestart_manual_fermentation_humidity_txt, lv_color_hex(0xD4A020), 0);
    }
}

/* Apply mode-specific image for each keypad button */
static void _keypad_set_images_mms(bk_lv_ui_t *bk_ui)
{
    device_state_t *state = &g_device_state;
    const char *pfx = (state->manual_current_mode == 1) ? "freeze" :
                      (state->manual_current_mode == 2) ? "defrost" : "fermentation";
    static const char * const kp_names[12] = {
        "keypad1","keypad2","keypad3","keypad4","keypad5","keypad6",
        "keypad7","keypad8","keypad9","keypad0","keypadminor","keypadback"
    };
    lv_obj_t *kp_imgs[12] = {
        bk_ui->manualmodestart_keypad1_im,
        bk_ui->manualmodestart_keypad2_im,
        bk_ui->manualmodestart_keypad3_im,
        bk_ui->manualmodestart_keypad4_im,
        bk_ui->manualmodestart_keypad5_im,
        bk_ui->manualmodestart_keypad6_im,
        bk_ui->manualmodestart_keypad7_im,
        bk_ui->manualmodestart_keypad8_im,
        bk_ui->manualmodestart_keypad9_im,
        bk_ui->manualmodestart_keypad0_im,
        bk_ui->manualmodestart_keypadminor_im,
        bk_ui->manualmodestart_keypadbackspace_im,
    };
    char buf[64];
    int i;
    for (i = 0; i < 12; i++) {
     //   snprintf(buf, sizeof(buf), "/images/%s_%s.png", pfx, kp_names[i]);
        snprintf(buf, sizeof(buf), "/images/%s_%s.png", pfx, kp_names[i]);
        _img_set_src_timed(kp_imgs[i], buf);
    }
}

static void _keypad_on_manualmodestart(bk_lv_ui_t *bk_ui)
{
    /* Objects and language-specific fixed images are created by the step initializer. */
    _keypad_set_images_mms(bk_ui);

    lv_obj_t *buttons[12] = {
        bk_ui->manualmodestart_keypad1,     bk_ui->manualmodestart_keypad2,
        bk_ui->manualmodestart_keypad3,     bk_ui->manualmodestart_keypad4,
        bk_ui->manualmodestart_keypad5,     bk_ui->manualmodestart_keypad6,
        bk_ui->manualmodestart_keypad7,     bk_ui->manualmodestart_keypad8,
        bk_ui->manualmodestart_keypad9,     bk_ui->manualmodestart_keypad0,
        bk_ui->manualmodestart_keypadminor, bk_ui->manualmodestart_keypadbackspace,
    };
    for (int i = 0; i < 12; i++)
        lv_obj_add_flag(buttons[i], LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(bk_ui->manualmodestart_keypadhide,   LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(bk_ui->manualmodestart_keypadhide, LV_OBJ_FLAG_HIDDEN);
    /* keypadhide_im: 항상 보이지 않고 눌렀을 때만 표시(press feedback) — event_cb에서 처리 */
    ui_keypad_slide_on(bk_ui->manualmodestart_keypadbaseim);
}

static void _keypad_off_manualmodestart(bk_lv_ui_t *bk_ui)
{
    /* Not yet created (e.g. called from load_event_cb on first entry) */
    if (!bk_ui->manualmodestart_keypad1) {
        if (bk_ui->manualmodestart_keypadbaseim)
            ui_keypad_slide_off(bk_ui->manualmodestart_keypadbaseim);
        return;
    }
    lv_obj_t **bt[12] = {
        &bk_ui->manualmodestart_keypad1,     &bk_ui->manualmodestart_keypad2,
        &bk_ui->manualmodestart_keypad3,     &bk_ui->manualmodestart_keypad4,
        &bk_ui->manualmodestart_keypad5,     &bk_ui->manualmodestart_keypad6,
        &bk_ui->manualmodestart_keypad7,     &bk_ui->manualmodestart_keypad8,
        &bk_ui->manualmodestart_keypad9,     &bk_ui->manualmodestart_keypad0,
        &bk_ui->manualmodestart_keypadminor, &bk_ui->manualmodestart_keypadbackspace,
    };
    lv_obj_t **im[12] = {
        &bk_ui->manualmodestart_keypad1_im,        &bk_ui->manualmodestart_keypad2_im,
        &bk_ui->manualmodestart_keypad3_im,        &bk_ui->manualmodestart_keypad4_im,
        &bk_ui->manualmodestart_keypad5_im,        &bk_ui->manualmodestart_keypad6_im,
        &bk_ui->manualmodestart_keypad7_im,        &bk_ui->manualmodestart_keypad8_im,
        &bk_ui->manualmodestart_keypad9_im,        &bk_ui->manualmodestart_keypad0_im,
        &bk_ui->manualmodestart_keypadminor_im,    &bk_ui->manualmodestart_keypadbackspace_im,
    };
    for (int i = 0; i < 12; i++) {
        lv_obj_clear_flag(*bt[i], LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_flag(*im[i],   LV_OBJ_FLAG_HIDDEN);
    }
    lv_obj_add_flag(bk_ui->manualmodestart_keypadhide, LV_OBJ_FLAG_HIDDEN);
    if (bk_ui->manualmodestart_keypadhide_im)
        lv_obj_add_flag(bk_ui->manualmodestart_keypadhide_im, LV_OBJ_FLAG_HIDDEN);
    ui_keypad_slide_off(bk_ui->manualmodestart_keypadbaseim);
}

static void _common_click_mms(bk_lv_ui_t *bk_ui)
{
    /* 운전 중에는 값 입력(keypad) 자체를 막음 — 설정값 변경은 운전 정지 후에만.
     * FERM2(저온발효/과발효방지)는 버튼 자체를 숨겨서 원래도 이 함수까지 못 오지만,
     * operation 플래그가 그 시점에 false일 가능성까지 대비해 모드로도 한 번 더 막음. */
    if (g_device_state.operation) return;
    if (g_device_state.manual_current_mode == MANUAL_MODE_FERM2) return;
    if (lv_tick_elaps(s_last_click_manualmodestart) < 250) return;
    s_last_click_manualmodestart = lv_tick_get();
    lv_obj_t *_lbl = _get_target_label_mms(bk_ui);
    if (_lbl) {
        strncpy(s_save_manualmodestart, lv_label_get_text(_lbl),
                sizeof(s_save_manualmodestart) - 1);
        s_save_manualmodestart[sizeof(s_save_manualmodestart) - 1] = '\0';
    }
    s_edit_buf_manualmodestart[0] = '\0';
    _underbar_all_hide_mms(bk_ui);
    _underbar_show_mms(bk_ui);
    _keypad_on_manualmodestart(bk_ui);
    _maxmin_mms(bk_ui);
    if (_lbl && lv_label_get_text(_lbl)[0] == '\0')
        lv_label_set_text(_lbl, s_save_manualmodestart);
    hal_buzzer_beep();
}

static void _keypad_input_manualmodestart(bk_lv_ui_t *bk_ui, char digit)
{
    if (lv_tick_elaps(s_last_click_manualmodestart) < 250) return;
    s_last_click_manualmodestart = lv_tick_get();
    if (s_tci_manualmodestart == 0) return;
    /* 온도(1~3): °F는 세 자리까지("100" 등), °C는 두 자리까지 허용.
     * 습도(4): 단위 무관하게 항상 두 자리까지. */
    int _is_f_max = (strcmp(settings_get_str("Degree"), "\xc2\xb0""F") == 0);
    int _max = (s_tci_manualmodestart <= 3) ? (_is_f_max ? 3 : 2) : 2;
    /* 냉동 온도 필드는 '-' 부호가 자릿수와 별개로 앞에 붙으므로, 부호가 있으면
     * 자리수 제한에서 제외 — 안 그러면 "-1" 상태에서 "5"를 눌렀을 때 최고참
     * 문자(부호)가 밀려나 "-15"가 아니라 "15"로 표시되는 버그가 생긴다. */
    int _has_sign = (s_edit_buf_manualmodestart[0] == '-');
    if (_has_sign) _max += 1;
    size_t _len = strlen(s_edit_buf_manualmodestart);
    if ((int)_len >= _max) {
        int _start = _has_sign ? 1 : 0;
        memmove(s_edit_buf_manualmodestart + _start,
                s_edit_buf_manualmodestart + _start + 1, _len - _start - 1);
        s_edit_buf_manualmodestart[_len - 1] = digit;
        s_edit_buf_manualmodestart[_len]     = '\0';
    } else {
        s_edit_buf_manualmodestart[_len]     = digit;
        s_edit_buf_manualmodestart[_len + 1] = '\0';
    }
    lv_obj_t *_lbl = _get_target_label_mms(bk_ui);
    if (_lbl) lv_label_set_text(_lbl, s_edit_buf_manualmodestart);
    hal_buzzer_beep();
}

static void _keypad_minor_manualmodestart(bk_lv_ui_t *bk_ui)
{
    if (s_tci_manualmodestart != 1) return;   /* 냉동 온도만 음수 부호 필요 */
    if (lv_tick_elaps(s_last_click_manualmodestart) < 250) return;
    s_last_click_manualmodestart = lv_tick_get();

    size_t _len = strlen(s_edit_buf_manualmodestart);
    if (s_edit_buf_manualmodestart[0] == '-') {
        /* '-' 제거 (토글 off) */
        memmove(s_edit_buf_manualmodestart,
                s_edit_buf_manualmodestart + 1, _len);
    } else if (_len < sizeof(s_edit_buf_manualmodestart) - 2) {
        /* '-' 맨 앞에 삽입 (토글 on) */
        memmove(s_edit_buf_manualmodestart + 1,
                s_edit_buf_manualmodestart, _len + 1);
        s_edit_buf_manualmodestart[0] = '-';
    }
    lv_obj_t *_lbl = _get_target_label_mms(bk_ui);
    if (_lbl) lv_label_set_text(_lbl, s_edit_buf_manualmodestart);
    hal_buzzer_beep();
}

static void _keypad_backspace_manualmodestart(bk_lv_ui_t *bk_ui)
{
    if (lv_tick_elaps(s_last_click_manualmodestart) < 250) return;
    s_last_click_manualmodestart = lv_tick_get();
    if (s_tci_manualmodestart == 0) return;
    size_t _len = strlen(s_edit_buf_manualmodestart);
    if (_len > 0) s_edit_buf_manualmodestart[_len - 1] = '\0';
    lv_obj_t *_lbl = _get_target_label_mms(bk_ui);
    if (_lbl) lv_label_set_text(_lbl,
        s_edit_buf_manualmodestart[0] != '\0'
            ? s_edit_buf_manualmodestart : s_save_manualmodestart);
}

static void _keypad_hide_manualmodestart(bk_lv_ui_t *bk_ui)
{
    if (lv_tick_elaps(s_last_click_hide_mms) < 250) return;
    s_last_click_hide_mms = lv_tick_get();
    _maxmin_mms(bk_ui);
    {
        int _is_f = (strcmp(settings_get_str("Degree"), "\xc2\xb0""F") == 0);
#define _ST(key, obj) do { \
    const char *_sv = lv_label_get_text(obj); \
    if (_is_f && _sv && _sv[0]) { \
        char _cb[16]; snprintf(_cb, sizeof(_cb), "%d", (atoi(_sv) - 32) * 5 / 9); \
        settings_set_str(key, _cb); \
    } else settings_set_str(key, _sv); \
} while(0)
        _ST("ManualFreezeTemp",       bk_ui->manualmodestart_manual_freeze_temp_txt);
        _ST("ManualDefrostTemp",      bk_ui->manualmodestart_manual_defrost_temp_txt);
        _ST("ManualFermentationTemp", bk_ui->manualmodestart_manual_fermentation_temp_txt);
        settings_set_str("ManualFermentationHumidity",
            lv_label_get_text(bk_ui->manualmodestart_manual_fermentation_humidity_txt));
#undef _ST
    }
    uart_comm_trigger_change_setting();
    _underbar_all_hide_mms(bk_ui);
    _keypad_off_manualmodestart(bk_ui);
    s_tci_manualmodestart = 0;
    s_edit_buf_manualmodestart[0] = '\0';
}

/* ── lv_arc 기반 서클 ────────────────────────────────────────────────
 * 정지: 40° 세그먼트 고정 표시
 * 운전: 세그먼트가 360° 무한 회전
 * ──────────────────────────────────────────────────────────────────── */
static void _arc_anim_cb(void *obj, int32_t val)
{
    /* arc_rounded=false 시 LVGL의 partial invalidation이 arc 끝 픽셀을 놓침
     * → 구 위치 찌꺼기 발생. 각도 변경 전에 전체 영역 먼저 무효화. */
    lv_obj_invalidate((lv_obj_t *)obj);
    uint16_t start = (uint16_t)(val % 360);
    uint16_t end   = (uint16_t)((val + 90) % 360);
    lv_arc_set_angles((lv_obj_t *)obj, start, end);
}

static lv_obj_t *_create_run_arc(lv_obj_t *scr, lv_color_t color)
{
    /* 회색 배경 링 바깥쪽 5px를 흰색으로 — 전체 원 크기(반지름150)는 그대로 두고,
     * 회색 링 자체를 5px 줄여(반지름 145까지만) 비운 바깥쪽 5px(145~150)를
     * 원래 크기 그대로인 흰색 링으로 채운다. 안쪽 경계(반지름120, circle_basic 구멍
     * 크기)는 두 경우 모두 동일하게 유지.
     * 회색 링: outer=145(size 290), width=25 → inner=120
     * 흰색 링: outer=150(size 300, 기존 전체 크기 그대로), width=5  → 145~150만 칠함 */
    lv_obj_t *arc = lv_arc_create(scr);
    lv_obj_set_size(arc, 290, 290);
    lv_obj_set_pos(arc, 367, 122);

    /* 배경 ring: 360° 회색 (반지름 120~145) */
    lv_arc_set_bg_angles(arc, 0, 360);
    lv_obj_set_style_arc_color(arc, lv_color_hex(0xD8D8D8), LV_PART_MAIN);
    lv_obj_set_style_arc_width(arc, 26, LV_PART_MAIN);
    lv_obj_set_style_arc_rounded(arc, false, LV_PART_MAIN);

    /* 지시자 arc: 모드 색상, 90° — 12시 방향 중앙. 회색 링(120~145) 안쪽 경계에서
     * 5px 여백(반지름 125~145) — 바깥쪽은 회색 링 새 외곽과 맞닿음 */
    lv_arc_set_angles(arc, 225, 315);
    lv_obj_set_style_arc_color(arc, color, LV_PART_INDICATOR);
    lv_obj_set_style_arc_width(arc, 20, LV_PART_INDICATOR);
    lv_obj_set_style_arc_rounded(arc, false, LV_PART_INDICATOR);
    lv_obj_set_style_pad_all(arc, 0, LV_PART_INDICATOR);

    /* 중앙 투명 — circle_basic PNG가 뒤에서 보임 */
    lv_obj_set_style_bg_opa(arc, LV_OPA_TRANSP, LV_PART_MAIN);

    /* 손잡이 투명 */
    lv_obj_set_style_opa(arc, LV_OPA_TRANSP, LV_PART_KNOB);
    lv_obj_set_style_pad_all(arc, 0, LV_PART_KNOB);

    lv_obj_remove_flag(arc, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(arc, LV_OBJ_FLAG_HIDDEN);   /* 정지 상태에서는 숨김 */
    lv_obj_move_background(arc);

    /* 바깥쪽 5px 흰색 링 — 원래 전체 크기(300x300, 반지름150) 그대로, 두께만 5 */
    s_run_arc_bg_white = lv_arc_create(scr);
    lv_obj_set_size(s_run_arc_bg_white, 300, 300);
    lv_obj_set_pos(s_run_arc_bg_white, 362, 117);
    lv_arc_set_bg_angles(s_run_arc_bg_white, 0, 360);
    lv_obj_set_style_arc_color(s_run_arc_bg_white, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_arc_width(s_run_arc_bg_white, 6, LV_PART_MAIN);
    lv_obj_set_style_arc_rounded(s_run_arc_bg_white, false, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(s_run_arc_bg_white, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_opa(s_run_arc_bg_white, LV_OPA_TRANSP, LV_PART_INDICATOR);
    lv_obj_set_style_opa(s_run_arc_bg_white, LV_OPA_TRANSP, LV_PART_KNOB);
    lv_obj_remove_flag(s_run_arc_bg_white, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(s_run_arc_bg_white, LV_OBJ_FLAG_HIDDEN);
    lv_obj_move_background(s_run_arc_bg_white);

    /* 안쪽 7px 흰색 링 — 회색 링 안쪽 경계(반지름120)에 붙여서, 그 안쪽으로
     * 7px(반지름 113~120)를 흰색으로 채운다. 중심은 위 두 arc와 동일(512,267). */
    s_run_arc_inner_white = lv_arc_create(scr);
    lv_obj_set_size(s_run_arc_inner_white, 240, 240);
    lv_obj_set_pos(s_run_arc_inner_white, 392, 147);
    lv_arc_set_bg_angles(s_run_arc_inner_white, 0, 360);
    lv_obj_set_style_arc_color(s_run_arc_inner_white, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_arc_width(s_run_arc_inner_white, 7, LV_PART_MAIN);
    lv_obj_set_style_arc_rounded(s_run_arc_inner_white, false, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(s_run_arc_inner_white, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_opa(s_run_arc_inner_white, LV_OPA_TRANSP, LV_PART_INDICATOR);
    lv_obj_set_style_opa(s_run_arc_inner_white, LV_OPA_TRANSP, LV_PART_KNOB);
    lv_obj_remove_flag(s_run_arc_inner_white, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(s_run_arc_inner_white, LV_OBJ_FLAG_HIDDEN);
    lv_obj_move_background(s_run_arc_inner_white);

    return arc;
}

static void _arc_anim_start(lv_obj_t *arc)
{
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, arc);
    lv_anim_set_exec_cb(&a, _arc_anim_cb);
    lv_anim_set_values(&a, 0, 360);
    lv_anim_set_duration(&a, 4000);
    lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
    lv_anim_start(&a);
}

static void _arc_anim_stop(lv_obj_t *arc)
{
    if (arc == NULL || !lv_obj_is_valid(arc))
    {
        return;
    }

    lv_anim_delete(arc, _arc_anim_cb);
    lv_arc_set_angles(arc, 225, 315);   /* 원점 복귀 — 12시 중앙 (90°) */
}


/* ── manual_gif PNG 애니메이션 (냉동/발효: 회전, 해동: 물방울 낙하) ── */
static void _gif_rotate_cb_mms(void *obj, int32_t val)
{
    lv_image_set_rotation((lv_obj_t *)obj, (uint16_t)(val % 3600));
}

static void _drop_anim_cb_mms(void *obj, int32_t val)
{
    lv_obj_set_y((lv_obj_t *)obj, val);
}

static void _gif_anim_start_mms(bk_lv_ui_t *bk_ui)
{
    if (bk_ui->manualmodestart_manual_gif_basic)
        lv_obj_add_flag(bk_ui->manualmodestart_manual_gif_basic, LV_OBJ_FLAG_HIDDEN);

    int mode = g_device_state.manual_current_mode;

    if (mode == MANUAL_MODE_FREEZE) {
        /* 냉동: 360° 무한 회전 */
        if (!bk_ui->manualmodestart_manual_gif) return;
        lv_obj_t *gif = bk_ui->manualmodestart_manual_gif;
        _img_ensure_src(gif);
        lv_obj_clear_flag(gif, LV_OBJ_FLAG_HIDDEN);
        lv_anim_t a;
        lv_anim_init(&a);
        lv_anim_set_var(&a, gif);
        lv_anim_set_exec_cb(&a, _gif_rotate_cb_mms);
        lv_anim_set_values(&a, 0, 3600);
        lv_anim_set_duration(&a, 4000);
        lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
        lv_anim_start(&a);
    } else {
        /* 해동: 위→아래 낙하 (76×55) / 발효: 아래→위 상승 (76×24, 상단 24px만) */
        if (!s_mms_drop_clip || !lv_obj_is_valid(s_mms_drop_clip)) return;
        lv_anim_t a;
        lv_anim_init(&a);
        lv_anim_set_exec_cb(&a, _drop_anim_cb_mms);
        lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
        if (mode == MANUAL_MODE_DEFROST) {
            /* 해동: 76×55 창, 물방울 전체 위→아래 낙하 */
            lv_obj_set_size(s_mms_drop_clip, 76, 55);
            lv_anim_set_var(&a, s_mms_drop_img);
            lv_anim_set_values(&a, -55, 55);
            lv_anim_set_duration(&a, 2000);
        } else {
            /* 발효: 외부 클립 76×24, 내부 컨테이너(76×24) 가 애니메이션
             * ferm_inner 가 24px 이므로 이미지 상단 24px 모양만 오르내림
             * y=26 : 내부가 외부 아래 → 안 보임
             * y=0  : 내부가 외부와 일치 → 이미지 상단 26px 표시
             * y=-26: 내부가 외부 위로 이탈 → 안 보임 */
            lv_obj_set_size(s_mms_drop_clip, 76, 24);
            lv_anim_set_var(&a, s_mms_ferm_inner);
            lv_anim_set_values(&a, 25, -25);
            lv_anim_set_duration(&a, 2400);
            if (s_mms_ferm_btm_clip && lv_obj_is_valid(s_mms_ferm_btm_clip)) {
                if (s_mms_ferm_btm_img) _img_ensure_src(s_mms_ferm_btm_img);
                lv_obj_clear_flag(s_mms_ferm_btm_clip, LV_OBJ_FLAG_HIDDEN);
            }
        }
        _img_ensure_src(s_mms_drop_clip);
        lv_obj_clear_flag(s_mms_drop_clip, LV_OBJ_FLAG_HIDDEN);
        lv_anim_start(&a);
    }
}

static void _gif_anim_stop_mms(bk_lv_ui_t *bk_ui)
{
    /* 회전 정지 */
    if (bk_ui->manualmodestart_manual_gif) {
        lv_anim_delete(bk_ui->manualmodestart_manual_gif, _gif_rotate_cb_mms);
        lv_image_set_rotation(bk_ui->manualmodestart_manual_gif, 0);
        lv_obj_add_flag(bk_ui->manualmodestart_manual_gif, LV_OBJ_FLAG_HIDDEN);
    }
    /* 낙하/상승 정지 */
    if (s_mms_drop_clip && lv_obj_is_valid(s_mms_drop_clip)) {
        if (s_mms_drop_img) {
            lv_anim_delete(s_mms_drop_img, _drop_anim_cb_mms);
            lv_obj_set_y(s_mms_drop_img, 0);
        }
        if (s_mms_ferm_inner && lv_obj_is_valid(s_mms_ferm_inner)) {
            lv_anim_delete(s_mms_ferm_inner, _drop_anim_cb_mms);
            lv_obj_set_y(s_mms_ferm_inner, 0);
        }
        lv_obj_add_flag(s_mms_drop_clip, LV_OBJ_FLAG_HIDDEN);
    }
    /* 발효 하단 고정 숨김 */
    if (s_mms_ferm_btm_clip && lv_obj_is_valid(s_mms_ferm_btm_clip))
        lv_obj_add_flag(s_mms_ferm_btm_clip, LV_OBJ_FLAG_HIDDEN);
    /* gif_basic 복귀 */
    if (bk_ui->manualmodestart_manual_gif_basic) {
        _img_ensure_src(bk_ui->manualmodestart_manual_gif_basic);
        lv_obj_clear_flag(bk_ui->manualmodestart_manual_gif_basic, LV_OBJ_FLAG_HIDDEN);
    }
}

/* ── 저온발효 배경: 최초 1회 JPEG decode → raw buffer 상주, 화면 bg_img_src 설정 ──
 * canvas는 HIDDEN 상태(렌더링 제외)로만 유지 — buffer 소유자 + image dsc 제공 역할
 * bg_img_src = 화면 자체의 배경 → dirty region에서 자식 위젯보다 항상 먼저 렌더링
 * → arc 이동 시 구 위치가 bg_img_src raw픽셀로 덮임 → 찌꺼기 없음              */
static void _ferm2_bg_load(bk_lv_ui_t *bk_ui, const char *bg_path, int lang)
{
    if (bk_ui == NULL || bg_path == NULL ||
        bk_ui->manualmodestart == NULL || !lv_obj_is_valid(bk_ui->manualmodestart))
    {
        return;
    }

    lv_obj_t *scr = bk_ui->manualmodestart;

    size_t freePsram = rtos_get_psram_free_heap_size();
    bk_printf(TAG "[FERM2] psram free = %u B\n", (unsigned)freePsram);

    if (s_ferm2_canvas_buf == NULL)
    {
        if (freePsram < 1300 * 1024)
        {
            bk_printf(TAG "[FERM2] psram low, fallback to direct JPEG\n");
            lv_obj_set_style_bg_img_src(scr, bg_path, 0);
            lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);
            return;
        }

        uint32_t bufSize = LV_CANVAS_BUF_SIZE(1024, 540, 16, LV_DRAW_BUF_ALIGN);
        s_ferm2_canvas_buf = lv_malloc(bufSize);
        if (s_ferm2_canvas_buf == NULL)
        {
            bk_printf(TAG "[FERM2] canvas buffer alloc failed, fallback to direct JPEG\n");
            lv_obj_set_style_bg_img_src(scr, bg_path, 0);
            lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);
            return;
        }
    }

    /* canvas/image는 페이지가 살아 있는 동안 재사용한다. */
    if (s_ferm2_bg_canvas == NULL || !lv_obj_is_valid(s_ferm2_bg_canvas))
    {
        s_ferm2_bg_canvas = lv_canvas_create(scr);
        lv_canvas_set_buffer(s_ferm2_bg_canvas,
                             s_ferm2_canvas_buf,
                             1024,
                             540,
                             LV_COLOR_FORMAT_RGB565);
        lv_obj_add_flag(s_ferm2_bg_canvas, LV_OBJ_FLAG_HIDDEN);
        s_ferm2_buf_lang = -1;
    }

    if (s_ferm2_buf_lang != lang)
    {
        lv_layer_t layer;
        lv_canvas_init_layer(s_ferm2_bg_canvas, &layer);

        lv_draw_image_dsc_t imgDsc;
        lv_draw_image_dsc_init(&imgDsc);
        imgDsc.src = bg_path;

        lv_area_t area = {0, 0, 1023, 539};
        lv_draw_image(&layer, &imgDsc, &area);
        lv_canvas_finish_layer(s_ferm2_bg_canvas, &layer);
        s_ferm2_buf_lang = lang;
    }

    if (s_ferm2_bg_img == NULL || !lv_obj_is_valid(s_ferm2_bg_img))
    {
        s_ferm2_bg_img = lv_image_create(scr);
        lv_obj_set_pos(s_ferm2_bg_img, 0, 0);
        lv_obj_remove_flag(s_ferm2_bg_img, LV_OBJ_FLAG_CLICKABLE);
    }

    lv_image_set_src(s_ferm2_bg_img, lv_canvas_get_image(s_ferm2_bg_canvas));
    lv_obj_clear_flag(s_ferm2_bg_img, LV_OBJ_FLAG_HIDDEN);
    lv_obj_move_background(s_ferm2_bg_img);

    /* raw canvas를 쓰는 경우 screen style의 direct JPEG fallback은 제거한다. */
    lv_obj_set_style_bg_img_src(scr, NULL, 0);
}


/* ── 실시간 온도/습도 갱신 ─────────────────────────────────────────── */
static void _refresh_running_ui_mms(bk_lv_ui_t *bk_ui)
{
    device_state_t *state = &g_device_state;
    char buf[16];

    if (state->current_temp != s_last_temp_mms) {
        s_last_temp_mms = state->current_temp;
        int _is_f = (strcmp(settings_get_str("Degree"), "\xc2\xb0""F") == 0);
        int _t = _is_f ? (state->current_temp * 9 / 5 + 32) : state->current_temp;
        snprintf(buf, sizeof(buf), "%d", _t);
        lv_label_set_text(bk_ui->manualmodestart_tempbox_current_temp, buf);
    }

    if (state->current_humidity != s_last_hum_mms) {
        s_last_hum_mms = state->current_humidity;
        snprintf(buf, sizeof(buf), "%d", state->current_humidity);
        lv_label_set_text(bk_ui->manualmodestart_tempbox_current_humidity, buf);
    }
}

static void _ui_timer_cb_mms(lv_timer_t *timer)
{
    (void)timer;
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    if (!bk_ui->manualmodestart || !lv_obj_is_valid(bk_ui->manualmodestart)) return;
    if (lv_scr_act() != bk_ui->manualmodestart) return;
    /* UART 신규 패킷 없으면 건너뜀 */
    static uint32_t s_last_rx_seq_mms = 0;
    uint32_t _cur_seq = g_uart_rx_seq;
    if (_cur_seq == s_last_rx_seq_mms) return;
    s_last_rx_seq_mms = _cur_seq;
    _refresh_running_ui_mms(bk_ui);
}

/* ── 운전 중 UI 적용 (arc 회전, 나가기 숨김, 온도/습도 박스, STOP 버튼) ── */
static void _ui_apply_running_mms(bk_lv_ui_t *bk_ui)
{
    /* 화면 재진입 시 레이블은 ""로 초기화되지만 캐시는 이전 값 유지 → 갱신 skip 방지 */
    s_last_temp_mms = 0x7FFFFFFF;
    s_last_hum_mms  = 0x7FFFFFFF;

    int lang = settings_get_int("LANGUAGE");

    /* arc + gif 표시 + 회전 시작 */
    if (s_run_arc_bg_white) lv_obj_clear_flag(s_run_arc_bg_white, LV_OBJ_FLAG_HIDDEN);
    if (s_run_arc_inner_white) lv_obj_clear_flag(s_run_arc_inner_white, LV_OBJ_FLAG_HIDDEN);
    if (bk_ui->manualmodestart_run_arc) {
        _img_ensure_src(bk_ui->manualmodestart_run_arc);
        lv_obj_clear_flag(bk_ui->manualmodestart_run_arc, LV_OBJ_FLAG_HIDDEN);
        _arc_anim_start(bk_ui->manualmodestart_run_arc);
    }
    /* circle_basic(링 baked-in PNG)은 운전 중엔 숨김 — arc가 그리는 링과
     * 겹쳐 보이던 문제(경계 색 잔상, 두께 불일치) 방지. 초기 화면에서만 표시. */
    if (bk_ui->manualmodestart_manual_circle_basic)
        lv_obj_add_flag(bk_ui->manualmodestart_manual_circle_basic, LV_OBJ_FLAG_HIDDEN);
    _gif_anim_start_mms(bk_ui);

    /* STOP 버튼 이미지 (언어별) */
    if      (lang == 1) _img_set_src_timed(bk_ui->manualmodestart_startim, "/images/stop_bt_china.png");
    else if (lang == 2) _img_set_src_timed(bk_ui->manualmodestart_startim, "/images/stop_bt_english.png");
    else                _img_set_src_timed(bk_ui->manualmodestart_startim, "/images/stop_bt.png");

    /* 나가기 버튼 숨김 */
    lv_obj_add_flag(bk_ui->manualmodestart_backim, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(bk_ui->manualmodestart_backbt, LV_OBJ_FLAG_HIDDEN);

    /* 온도 박스 + 현재값 표시 (냉동/해동은 습도 숨김) */
    int mode = g_device_state.manual_current_mode;
    bool show_humidity = (mode == MANUAL_MODE_FERM || mode == MANUAL_MODE_FERM2);
    {
        int is_f = (strcmp(settings_get_str("Degree"), "\xc2\xb0""F") == 0) ? 1 : 0;
        const char *fsuf = is_f ? "_f" : "";
        const char *lsuf = (lang == 1) ? "_china" : (lang == 2) ? "_english" : "";
        char _tb[128];
        if (show_humidity)
            snprintf(_tb, sizeof(_tb), "/images/tempbox%s%s.png", fsuf, lsuf);
        else
            snprintf(_tb, sizeof(_tb), "/images/tempbox%s_zero%s.png", fsuf, lsuf);
        _img_set_src_timed(bk_ui->manualmodestart_tempbox, _tb);
    }
    _img_ensure_src(bk_ui->manualmodestart_tempbox);
    lv_obj_clear_flag(bk_ui->manualmodestart_tempbox,              LV_OBJ_FLAG_HIDDEN);
    _img_ensure_src(bk_ui->manualmodestart_tempbox_current_temp);
    lv_obj_clear_flag(bk_ui->manualmodestart_tempbox_current_temp, LV_OBJ_FLAG_HIDDEN);
    if (show_humidity) {
        _img_ensure_src(bk_ui->manualmodestart_tempbox_current_humidity);
        lv_obj_clear_flag(bk_ui->manualmodestart_tempbox_current_humidity, LV_OBJ_FLAG_HIDDEN);
    } else
        lv_obj_add_flag(bk_ui->manualmodestart_tempbox_current_humidity,   LV_OBJ_FLAG_HIDDEN);

    /* 현재 온도/습도 즉시 갱신 */
    _refresh_running_ui_mms(bk_ui);

    /* 1초 갱신 타이머 */
    if (s_ui_timer_mms) { lv_timer_delete(s_ui_timer_mms); s_ui_timer_mms = NULL; }
    s_ui_timer_mms = lv_timer_create(_ui_timer_cb_mms, 1000, NULL);
}

/* ── 정지 UI 적용 (arc 정지, 나가기 복구, 온도박스 숨김, START 버튼) ── */
static void _ui_apply_stopped_mms(bk_lv_ui_t *bk_ui)
{
    int lang = settings_get_int("LANGUAGE");

    /* arc + gif 애니메이션 정지 + 숨김 */
    if (bk_ui->manualmodestart_run_arc) {
        _arc_anim_stop(bk_ui->manualmodestart_run_arc);
        lv_obj_add_flag(bk_ui->manualmodestart_run_arc, LV_OBJ_FLAG_HIDDEN);
    }
    if (s_run_arc_bg_white) lv_obj_add_flag(s_run_arc_bg_white, LV_OBJ_FLAG_HIDDEN);
    if (s_run_arc_inner_white) lv_obj_add_flag(s_run_arc_inner_white, LV_OBJ_FLAG_HIDDEN);
    /* 초기(정지) 화면으로 복귀 — 링이 baked-in된 circle_basic 다시 표시 */
    if (bk_ui->manualmodestart_manual_circle_basic)
        lv_obj_clear_flag(bk_ui->manualmodestart_manual_circle_basic, LV_OBJ_FLAG_HIDDEN);
    _gif_anim_stop_mms(bk_ui);

    /* START 버튼 이미지 (언어별) */
    if      (lang == 1) _img_set_src_timed(bk_ui->manualmodestart_startim, "/images/start_bt_china.png");
    else if (lang == 2) _img_set_src_timed(bk_ui->manualmodestart_startim, "/images/start_bt_english.png");
    else                _img_set_src_timed(bk_ui->manualmodestart_startim, "/images/start_bt.png");

    /* 나가기 버튼 복구 */
    _img_ensure_src(bk_ui->manualmodestart_backim);
    lv_obj_clear_flag(bk_ui->manualmodestart_backim, LV_OBJ_FLAG_HIDDEN);
    _img_ensure_src(bk_ui->manualmodestart_backbt);
    lv_obj_clear_flag(bk_ui->manualmodestart_backbt, LV_OBJ_FLAG_HIDDEN);

    /* 온도/습도 박스 숨김 (운전 중에만 표시) */
    lv_obj_add_flag(bk_ui->manualmodestart_tempbox,              LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(bk_ui->manualmodestart_tempbox_current_temp, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(bk_ui->manualmodestart_tempbox_current_humidity, LV_OBJ_FLAG_HIDDEN);

    /* 타이머 불필요 (표시할 온도 박스 없음) */
    if (s_ui_timer_mms) { lv_timer_delete(s_ui_timer_mms); s_ui_timer_mms = NULL; }
}

/* ── page runtime lifecycle ------------------------------------------------- */

static bool _manualmodestart_runtime_ready(bk_lv_ui_t *bk_ui)
{
    return bk_ui != NULL &&
           bk_ui->manualmodestart != NULL && lv_obj_is_valid(bk_ui->manualmodestart) &&
           bk_ui->manualmodestart_run_arc != NULL && lv_obj_is_valid(bk_ui->manualmodestart_run_arc) &&
           s_run_arc_bg_white != NULL && lv_obj_is_valid(s_run_arc_bg_white) &&
           s_run_arc_inner_white != NULL && lv_obj_is_valid(s_run_arc_inner_white) &&
           s_mms_drop_clip != NULL && lv_obj_is_valid(s_mms_drop_clip) &&
           s_mms_drop_img != NULL && lv_obj_is_valid(s_mms_drop_img) &&
           s_mms_ferm_inner != NULL && lv_obj_is_valid(s_mms_ferm_inner) &&
           s_mms_ferm_top_img != NULL && lv_obj_is_valid(s_mms_ferm_top_img) &&
           s_mms_ferm_btm_clip != NULL && lv_obj_is_valid(s_mms_ferm_btm_clip) &&
           s_mms_ferm_btm_img != NULL && lv_obj_is_valid(s_mms_ferm_btm_img);
}

bool manualmodestart_runtime_create(bk_lv_ui_t *bk_ui)
{
    if (bk_ui == NULL || bk_ui->manualmodestart == NULL ||
        !lv_obj_is_valid(bk_ui->manualmodestart))
    {
        return false;
    }

    if (_manualmodestart_runtime_ready(bk_ui))
    {
        return true;
    }

    /* 일부만 살아 있는 상태에서 덧붙이면 중복 child와 dangling pointer가 생긴다. */
    if (bk_ui->manualmodestart_run_arc != NULL ||
        s_run_arc_bg_white != NULL || s_run_arc_inner_white != NULL ||
        s_mms_drop_clip != NULL || s_mms_drop_img != NULL ||
        s_mms_ferm_inner != NULL || s_mms_ferm_top_img != NULL ||
        s_mms_ferm_btm_clip != NULL || s_mms_ferm_btm_img != NULL)
    {
        bk_printf(TAG "[ERROR] manualmodestart runtime objects are partially initialized\n");
        return false;
    }

    lv_obj_t *scr = bk_ui->manualmodestart;

    s_mms_drop_clip = lv_obj_create(scr);
    lv_obj_set_pos(s_mms_drop_clip, 474, 170);
    lv_obj_set_size(s_mms_drop_clip, 76, 55);
    lv_obj_add_flag(s_mms_drop_clip, LV_OBJ_FLAG_HIDDEN);
    lv_obj_remove_flag(s_mms_drop_clip, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_opa(s_mms_drop_clip, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(s_mms_drop_clip, 0, 0);
    lv_obj_set_style_pad_all(s_mms_drop_clip, 0, 0);

    s_mms_drop_img = lv_image_create(s_mms_drop_clip);
    lv_obj_set_pos(s_mms_drop_img, 0, 0);
    lv_obj_remove_flag(s_mms_drop_img, LV_OBJ_FLAG_CLICKABLE);

    s_mms_ferm_inner = lv_obj_create(s_mms_drop_clip);
    lv_obj_set_pos(s_mms_ferm_inner, 0, 0);
    lv_obj_set_size(s_mms_ferm_inner, 76, 24);
    lv_obj_remove_flag(s_mms_ferm_inner, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_opa(s_mms_ferm_inner, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(s_mms_ferm_inner, 0, 0);
    lv_obj_set_style_pad_all(s_mms_ferm_inner, 0, 0);

    s_mms_ferm_top_img = lv_image_create(s_mms_ferm_inner);
    lv_obj_set_pos(s_mms_ferm_top_img, 0, 0);
    lv_obj_remove_flag(s_mms_ferm_top_img, LV_OBJ_FLAG_CLICKABLE);

    s_mms_ferm_btm_clip = lv_obj_create(scr);
    lv_obj_set_pos(s_mms_ferm_btm_clip, 474, 194);
    lv_obj_set_size(s_mms_ferm_btm_clip, 76, 31);
    lv_obj_add_flag(s_mms_ferm_btm_clip, LV_OBJ_FLAG_HIDDEN);
    lv_obj_remove_flag(s_mms_ferm_btm_clip, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_opa(s_mms_ferm_btm_clip, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(s_mms_ferm_btm_clip, 0, 0);
    lv_obj_set_style_pad_all(s_mms_ferm_btm_clip, 0, 0);

    s_mms_ferm_btm_img = lv_image_create(s_mms_ferm_btm_clip);
    lv_obj_set_pos(s_mms_ferm_btm_img, 0, -24);
    lv_obj_remove_flag(s_mms_ferm_btm_img, LV_OBJ_FLAG_CLICKABLE);

    /* 실제 색은 SHOW_START에서 현재 mode에 맞춰 갱신한다. */
    bk_ui->manualmodestart_run_arc = _create_run_arc(scr, lv_color_hex(0xD8D8D8));

    if (!_manualmodestart_runtime_ready(bk_ui))
    {
        bk_printf(TAG "[ERROR] manualmodestart runtime object creation failed\n");
        return false;
    }

    /* 고정 child보다 뒤쪽에 배치. FERM2 background는 load 시 더 뒤로 이동한다. */
    lv_obj_move_background(s_mms_drop_clip);
    lv_obj_move_background(s_mms_ferm_btm_clip);
    lv_obj_move_background(bk_ui->manualmodestart_manual_gif);
    lv_obj_move_background(bk_ui->manualmodestart_manual_gif_basic);
    lv_obj_move_background(bk_ui->manualmodestart_manual_txt_basic);
    lv_obj_move_background(bk_ui->manualmodestart_manual_circle_basic);
    lv_obj_move_background(bk_ui->manualmodestart_manual_circle_gif);

    return true;
}

void manualmodestart_runtime_stop(bk_lv_ui_t *bk_ui)
{
    if (bk_ui == NULL)
    {
        return;
    }

    _gif_anim_stop_mms(bk_ui);
    _arc_anim_stop(bk_ui->manualmodestart_run_arc);

    if (bk_ui->manualmodestart_run_arc && lv_obj_is_valid(bk_ui->manualmodestart_run_arc))
    {
        lv_obj_add_flag(bk_ui->manualmodestart_run_arc, LV_OBJ_FLAG_HIDDEN);
    }
    if (s_run_arc_bg_white && lv_obj_is_valid(s_run_arc_bg_white))
    {
        lv_obj_add_flag(s_run_arc_bg_white, LV_OBJ_FLAG_HIDDEN);
    }
    if (s_run_arc_inner_white && lv_obj_is_valid(s_run_arc_inner_white))
    {
        lv_obj_add_flag(s_run_arc_inner_white, LV_OBJ_FLAG_HIDDEN);
    }

    if (s_run_anim_timer_mms != NULL)
    {
        lv_timer_delete(s_run_anim_timer_mms);
        s_run_anim_timer_mms = NULL;
    }

    if (s_ui_timer_mms != NULL)
    {
        lv_timer_delete(s_ui_timer_mms);
        s_ui_timer_mms = NULL;
    }
}

void manualmodestart_runtime_reset(void)
{
    /* 이 함수는 page root 삭제 후 호출한다. */
    s_mms_drop_clip       = NULL;
    s_mms_drop_img        = NULL;
    s_mms_ferm_inner      = NULL;
    s_mms_ferm_top_img    = NULL;
    s_mms_ferm_btm_clip   = NULL;
    s_mms_ferm_btm_img    = NULL;
    s_run_arc_bg_white    = NULL;
    s_run_arc_inner_white = NULL;
    s_ferm2_bg_canvas     = NULL;
    s_ferm2_bg_img        = NULL;

    if (s_ferm2_canvas_buf != NULL)
    {
        lv_free(s_ferm2_canvas_buf);
        s_ferm2_canvas_buf = NULL;
    }

    s_ferm2_buf_lang = -1;
    s_last_temp_mms = 0x7FFFFFFF;
    s_last_hum_mms  = 0x7FFFFFFF;
    s_tci_manualmodestart = 0;
    s_save_manualmodestart[0] = '\0';
    s_edit_buf_manualmodestart[0] = '\0';
    s_arc_angle_mms = 0;
    s_gif_val_mms = 0;
    s_gif_type_mms = 0;
}

static bool _set_mode_image_mms(lv_obj_t *obj,
                                const char *basePath,
                                bool hasLanguageVariant,
                                bool hasDegreeVariant)
{
    if (obj == NULL || !lv_obj_is_valid(obj))
    {
        bk_printf(TAG "[ERROR] image target is NULL/invalid: %s\n", basePath);
        return false;
    }

    char fullPath[128] = {0};
    if (!getImageFullPath(basePath,
                          hasLanguageVariant,
                          hasDegreeVariant,
                          ".png",
                          fullPath,
                          sizeof(fullPath)))
    {
        bk_printf(TAG "[ERROR] failed to resolve image path: %s\n", basePath);
        return false;
    }

    lv_image_set_src(obj, fullPath);
    return true;
}

static void _reset_show_state_mms(bk_lv_ui_t *bk_ui)
{
    lv_obj_t *modeObjects[] =
    {
        bk_ui->manualmodestart_manual_freeze_temp_txt,
        bk_ui->manualmodestart_manual_freeze_temp_bt,
        bk_ui->manualmodestart_manual_defrost_temp_txt,
        bk_ui->manualmodestart_manual_defrost_temp_bt,
        bk_ui->manualmodestart_manual_fermentation_temp_txt,
        bk_ui->manualmodestart_manual_fermentation_humidity_txt,
        bk_ui->manualmodestart_manual_fermentation_temp_bt,
        bk_ui->manualmodestart_manual_fermentation_humidity_bt,
        bk_ui->manualmodestart_over_time_korea,
        bk_ui->manualmodestart_over_time_china,
        bk_ui->manualmodestart_over_time_english,
    };

    for (uint32_t i = 0; i < sizeof(modeObjects) / sizeof(modeObjects[0]); i++)
    {
        if (modeObjects[i] != NULL && lv_obj_is_valid(modeObjects[i]))
        {
            lv_obj_add_flag(modeObjects[i], LV_OBJ_FLAG_HIDDEN);
        }
    }

    _underbar_all_hide_mms(bk_ui);
    _keypad_off_manualmodestart(bk_ui);

    if (s_ferm2_bg_img != NULL && lv_obj_is_valid(s_ferm2_bg_img))
    {
        lv_obj_add_flag(s_ferm2_bg_img, LV_OBJ_FLAG_HIDDEN);
    }

    /* direct-JPEG fallback가 남아 있을 수 있으므로 일반 mode 시작 전에 제거한다. */
    lv_obj_set_style_bg_img_src(bk_ui->manualmodestart, NULL, 0);

    lv_obj_set_style_text_color(bk_ui->manualmodestart_manual_fermentation_temp_txt,
                                lv_color_hex(0xD1232A), 0);
    lv_obj_set_style_text_color(bk_ui->manualmodestart_manual_fermentation_humidity_txt,
                                lv_color_hex(0xD1232A), 0);

    s_tci_manualmodestart = 0;
    s_edit_buf_manualmodestart[0] = '\0';
}

/* ─────────────────────────────────────────────────────────────────────────── */

void manualmodestart_backbt_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    device_state_t *state = &g_device_state;
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    if (lv_tick_elaps(s_last_click_manualmodestart) < 250) return;
    s_last_click_manualmodestart = lv_tick_get();
    hal_buzzer_beep();
    /* 타이머 정지 */
    if (s_ui_timer_mms) 
    { 
        lv_timer_delete(s_ui_timer_mms); 
        s_ui_timer_mms = NULL; 
    }

    if (state->auto_mode_over) 
    {
        /* 과발효방지 저온발효 중 Stop:
         * Android: AutoModeOver=false, Operation=false, AutoModeStart=false
         *          blackOutChecking=false, saveChecking=0 → AutoModeFragment */
        /* 화면 전환 전에 발효 상승/하강 애니메이션부터 정지 + 원본 아이콘 복구.
         * 그냥 destroy_page로 넘기면 전환 직전 마지막으로 그려진 프레임이
         * 애니메이션 중간(클립 일부만 보이는 상태)일 수 있어 깨진 것처럼 보인다. */
        _gif_anim_stop_mms(bk_ui);
        state->auto_mode_over         = false;
        state->over_ferm_active       = false;
        state->over_ferm_jeon_started = false;
        state->operation              = false;
        /* start_run=false로 두면 _write_process()의 모든 분기 조건이 거짓이 되어
         * 이후 어떤 TX도 나가지 않는다(에코할 게 없으니 RX no data가 계속 쌓임).
         * automodestart_cb.c의 정상 Stop과 동일하게 true로 유지해야 다음 사이클에
         * drive=0x00 op=0 STATUS가 정상적으로 나간다. */
        state->start_run              = true;
        state->auto_mode_start        = false;
        state->black_out_checking     = false;
        state->manual_current_mode    = 0;
        settings_set_str("saveChecking", "0");
        settings_set_str("SaveWriting",  "0");
        ui_page_change(PAGE_AUTOMODE);
        //destroy_page_manualmodestart(bk_ui);
        bk_printf(TAG "[OVER_FERM] Stop → AutoModeOver 해제, automode 화면\n");
        return;
    }
    state->operation           = false;
    /* start_run=false로 두면 이후 _write_process()가 아무 TX도 보내지 않게 되어
     * RX no data가 계속 쌓인다 — over_ferm Stop 경로와 동일한 문제이므로 true 유지. */
    state->start_run           = true;
    state->manual_start        = false;
    state->auto_mode_start     = false;
    state->manual_current_mode = 0;
    settings_set_str("saveChecking", "0");
    ui_page_change(PAGE_MANUALMODE);
}

void manualmodestart_startbt_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    device_state_t *state = &g_device_state;
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    if (lv_tick_elaps(s_last_click_manualmodestart) < 250) return;
    s_last_click_manualmodestart = lv_tick_get();

    hal_buzzer_beep();

    if (!state->manual_start) {
        /* ── 운전 시작 ────────────────────────────────────────────── */
        _maxmin_mms(bk_ui);
        {
            int _is_f = (strcmp(settings_get_str("Degree"), "\xc2\xb0""F") == 0);
#define _ST(key, obj) do { \
    const char *_sv = lv_label_get_text(obj); \
    if (_is_f && _sv && _sv[0]) { \
        char _cb[16]; snprintf(_cb, sizeof(_cb), "%d", (atoi(_sv) - 32) * 5 / 9); \
        settings_set_str(key, _cb); \
    } else settings_set_str(key, _sv); \
} while(0)
            _ST("ManualFreezeTemp",       bk_ui->manualmodestart_manual_freeze_temp_txt);
            _ST("ManualDefrostTemp",      bk_ui->manualmodestart_manual_defrost_temp_txt);
            _ST("ManualFermentationTemp", bk_ui->manualmodestart_manual_fermentation_temp_txt);
            settings_set_str("ManualFermentationHumidity",
                lv_label_get_text(bk_ui->manualmodestart_manual_fermentation_humidity_txt));
#undef _ST
        }

        /* saveOperationTemp: 0=freeze, 1=defrost, 2=fermentation.
         * current_op_mode도 동기화: uart_comm RX가 매 주기 saveOperationTemp에 덮어쓰므로
         * 이전 dry 세션의 OP_MODE_DRY(4)가 남아있으면 정전복구시 dry 경로로 잘못 진입함. */
        if (state->manual_current_mode == 1) {
            settings_set_str("saveOperationTemp", "0");
            state->current_op_mode = OP_MODE_FREEZE;
        } else if (state->manual_current_mode == 2) {
            settings_set_str("saveOperationTemp", "1");
            state->current_op_mode = OP_MODE_DEFROST;
        } else if (state->manual_current_mode == 3) {
            settings_set_str("saveOperationTemp", "2");
            state->current_op_mode = OP_MODE_FERM1;
        }

        /* RTC 시작 시각 기록 */
        {
            int sy, sm, sd, sh, smn, ss;
            hal_rtc_get(&sy, &sm, &sd, &sh, &smn, &ss);
            char buf[8];
            snprintf(buf, sizeof(buf), "%04d", sy); settings_set_str("originYear",  buf);
            snprintf(buf, sizeof(buf), "%02d", sm); settings_set_str("originMonth", buf);
            snprintf(buf, sizeof(buf), "%02d", sd); settings_set_str("originDay",   buf);
            snprintf(buf, sizeof(buf), "%02d", sh); settings_set_str("originHour",  buf);
            snprintf(buf, sizeof(buf), "%02d", smn);settings_set_str("originMin",   buf);
            state->send_start_year  = sy;
            state->send_start_month = sm;
            state->send_start_day   = sd;
            state->send_start_hour  = sh;
            state->send_start_min   = smn;
        }

        /* 0x31 FIRST_START payload는 send_freeze/defreeze/ferm1_* 필드를 직접 사용한다.
         * automode_cb은 CurrentSave* → send_* 동기화를 직접 하지만, manual은 없어서
         * 직전 0x21 RX 에코값(MCU NVRAM)이 그대로 전송된다. 여기서 동기화한다. */
        state->send_freeze_temp    = atoi(settings_get_str("ManualFreezeTemp"));
        state->send_defreeze_temp  = atoi(settings_get_str("ManualDefrostTemp"));
        state->send_ferm1_temp     = atoi(settings_get_str("ManualFermentationTemp"));
        state->send_ferm1_humidity = atoi(settings_get_str("ManualFermentationHumidity"));

        settings_set_str("saveDayPeriod", "0");  /* 수동운전: day_period 항상 0 저장 */
        state->day_period   = 0;
        state->operation    = true;
        uart_comm_trigger_first_start();   /* first_start=true + s_first_start_count 리셋 */
        state->manual_start = true;
        settings_set_str("saveChecking", "1");
        settings_set_str("SaveWriting",  "0");
        uart_comm_trigger_change_setting();
        settings_save_dirty();

        _ui_apply_running_mms(bk_ui);

    } else {
        /* ── 운전 정지 ────────────────────────────────────────────── */
        state->operation          = false;
        state->start_run          = true;
        state->first_start        = false;
        state->manual_start       = false;
        state->black_out_checking = false;
        settings_set_str("SaveWriting",  "0");
        settings_set_str("saveChecking", "0");
        uart_comm_trigger_change_setting();
        settings_save_dirty();

        _ui_apply_stopped_mms(bk_ui);
    }
}

void manualmodestart_manual_freeze_temp_bt_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    s_tci_manualmodestart = 1;
    _common_click_mms(bk_ui);
}

void manualmodestart_manual_defrost_temp_bt_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    s_tci_manualmodestart = 2;
    _common_click_mms(bk_ui);
}

void manualmodestart_manual_fermentation_temp_bt_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    s_tci_manualmodestart = 3;
    _common_click_mms(bk_ui);
}

void manualmodestart_manual_fermentation_humidity_bt_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    s_tci_manualmodestart = 4;
    _common_click_mms(bk_ui);
}

void manualmodestart_keypad_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    lv_event_code_t code = lv_event_get_code(e);
    intptr_t idx = (intptr_t)lv_event_get_user_data(e);
    static const char digits[12] = {'1','2','3','4','5','6','7','8','9','0',0,0};
    lv_obj_t *im_ptrs[12] = {
        bk_ui->manualmodestart_keypad1_im,        bk_ui->manualmodestart_keypad2_im,
        bk_ui->manualmodestart_keypad3_im,        bk_ui->manualmodestart_keypad4_im,
        bk_ui->manualmodestart_keypad5_im,        bk_ui->manualmodestart_keypad6_im,
        bk_ui->manualmodestart_keypad7_im,        bk_ui->manualmodestart_keypad8_im,
        bk_ui->manualmodestart_keypad9_im,        bk_ui->manualmodestart_keypad0_im,
        bk_ui->manualmodestart_keypadminor_im,    bk_ui->manualmodestart_keypadbackspace_im,
    };
    lv_obj_t *im = (idx >= 0 && idx < 12) ? im_ptrs[idx] : NULL;
    if (code == LV_EVENT_PRESSED && im) {
        _img_ensure_src(im);
        lv_obj_clear_flag(im, LV_OBJ_FLAG_HIDDEN);
    } else if ((code == LV_EVENT_RELEASED || code == LV_EVENT_PRESS_LOST) && im)
        lv_obj_add_flag(im, LV_OBJ_FLAG_HIDDEN);
    else if (code == LV_EVENT_CLICKED) {
        if      (idx == 10) _keypad_minor_manualmodestart(bk_ui);
        else if (idx == 11) _keypad_backspace_manualmodestart(bk_ui);
        else if (idx <= 9)  _keypad_input_manualmodestart(bk_ui, digits[idx]);
    }
}

void manualmodestart_keypadhide_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    lv_event_code_t code = lv_event_get_code(e);

    /* keypadback_close.png(keypadhide_im)를 눌렀을 때만 표시 — press feedback */
    if (code == LV_EVENT_PRESSED) {
        if (bk_ui->manualmodestart_keypadhide_im) lv_obj_clear_flag(bk_ui->manualmodestart_keypadhide_im, LV_OBJ_FLAG_HIDDEN);
        return;
    }
    if (code == LV_EVENT_RELEASED || code == LV_EVENT_PRESS_LOST) {
        if (bk_ui->manualmodestart_keypadhide_im) lv_obj_add_flag(bk_ui->manualmodestart_keypadhide_im, LV_OBJ_FLAG_HIDDEN);
    }

    if (code != LV_EVENT_CLICKED) return;
    hal_buzzer_beep();
    _keypad_hide_manualmodestart(bk_ui);
}

void manualmodestart_load_start_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code != LV_EVENT_SCREEN_LOAD_START && code != UI_EVENT_PAGE_SHOW_START)
    {
        bk_printf(TAG "[WARN] manualmodestart_load_start_event_cb: unexpected event code %d\n", code);
        return;
    }

    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    device_state_t *state = &g_device_state;

    if (bk_ui->manualmodestart == NULL || !lv_obj_is_valid(bk_ui->manualmodestart))
    {
        bk_printf(TAG "[ERROR] SHOW_START with invalid page root\n");
        return;
    }

    /* SHOW callback에서는 child를 만들지 않는다. init이 끝났다는 invariant를 확인만 한다. */
    if (!_manualmodestart_runtime_ready(bk_ui))
    {
        bk_printf(TAG "[ERROR] SHOW_START before runtime objects are ready, mode=%d\n",
                  state->manual_current_mode);
        return;
    }

    /* prerender 상태로 살아 있던 page의 이전 animation/UI 상태를 먼저 지운다. */
    manualmodestart_runtime_stop(bk_ui);
    _reset_show_state_mms(bk_ui);

    /* 기본 title은 manual mode. FERM2만 아래에서 automode title로 덮는다. */
    _set_mode_image_mms(bk_ui->manualmodestart_title,
                        "/images/manualmode_title",
                        true,
                        false);

    /* mode-dependent fixed images. 모든 target은 init에서 이미 생성되어 있어야 한다. */
    switch (state->manual_current_mode)
    {
        case MANUAL_MODE_FREEZE:
            _set_mode_image_mms(bk_ui->manualmodestart_manual_txt_basic,
                                "/images/manual_freeze_circle_txt", true, true);
            _set_mode_image_mms(bk_ui->manualmodestart_manual_circle_basic,
                                "/images/manual_freeze_circle_basic", true, true);
            _set_mode_image_mms(bk_ui->manualmodestart_manual_gif_basic,
                                "/images/manual_freeze_gif", false, false);
            _set_mode_image_mms(bk_ui->manualmodestart_manual_gif,
                                "/images/manual_freeze_gif", false, false);
            lv_obj_set_size(bk_ui->manualmodestart_manual_gif_basic, 52, 52);
            lv_obj_set_pos(bk_ui->manualmodestart_manual_gif_basic, 486, 171);
            break;

        case MANUAL_MODE_DEFROST:
            _set_mode_image_mms(bk_ui->manualmodestart_manual_txt_basic,
                                "/images/manual_defrost_circle_txt", true, true);
            _set_mode_image_mms(bk_ui->manualmodestart_manual_circle_basic,
                                "/images/manual_defrost_circle_basic", true, true);
            _set_mode_image_mms(bk_ui->manualmodestart_manual_gif_basic,
                                "/images/manual_defrost_gif", false, false);
            _set_mode_image_mms(s_mms_drop_img,
                                "/images/manual_defrost_gif", false, false);
            lv_obj_set_size(bk_ui->manualmodestart_manual_gif_basic, 76, 55);
            lv_obj_set_pos(bk_ui->manualmodestart_manual_gif_basic, 474, 170);
            break;

        case MANUAL_MODE_FERM:
            _set_mode_image_mms(bk_ui->manualmodestart_manual_txt_basic,
                                "/images/manual_fermentation2_circle_txt", true, true);
            _set_mode_image_mms(bk_ui->manualmodestart_manual_circle_basic,
                                "/images/manual_fermentation2_circle_basic", true, true);
            _set_mode_image_mms(bk_ui->manualmodestart_manual_gif_basic,
                                "/images/manual_fermentation2_gif", false, false);
            _set_mode_image_mms(s_mms_ferm_top_img,
                                "/images/manual_fermentation2_gif", false, false);
            _set_mode_image_mms(s_mms_ferm_btm_img,
                                "/images/manual_fermentation2_gif", false, false);
            lv_obj_set_size(bk_ui->manualmodestart_manual_gif_basic, 76, 55);
            lv_obj_set_pos(bk_ui->manualmodestart_manual_gif_basic, 474, 170);
            break;

        case MANUAL_MODE_FERM2:
            _set_mode_image_mms(bk_ui->manualmodestart_title,
                                "/images/automode_title", true, false);
            _set_mode_image_mms(bk_ui->manualmodestart_manual_txt_basic,
                                "/images/manual_fermentation1_circle_txt", true, true);
            _set_mode_image_mms(bk_ui->manualmodestart_manual_circle_basic,
                                "/images/manual_fermentation1_circle_basic", true, true);
            _set_mode_image_mms(bk_ui->manualmodestart_manual_gif_basic,
                                "/images/manual_fermentation1_gif", false, false);
            _set_mode_image_mms(s_mms_ferm_top_img,
                                "/images/manual_fermentation1_gif", false, false);
            _set_mode_image_mms(s_mms_ferm_btm_img,
                                "/images/manual_fermentation1_gif", false, false);
            lv_obj_set_size(bk_ui->manualmodestart_manual_gif_basic, 76, 55);
            lv_obj_set_pos(bk_ui->manualmodestart_manual_gif_basic, 474, 170);
            break;

        default:
            bk_printf(TAG "[ERROR] invalid manual_current_mode=%d\n", state->manual_current_mode);
            return;
    }

    if (state->black_out_checking)
    {
        lv_obj_clear_flag(bk_ui->manualmodestart_blackout, LV_OBJ_FLAG_HIDDEN);
    }
    else
    {
        lv_obj_add_flag(bk_ui->manualmodestart_blackout, LV_OBJ_FLAG_HIDDEN);
    }

    /* 설정값 -> label */
    {
        int isF = (strcmp(settings_get_str("Degree"), "\xc2\xb0""F") == 0);
#define _LT(obj, key) do { \
    const char *sv = settings_get_str(key); \
    if (isF && sv && sv[0]) { \
        char fb[16]; \
        snprintf(fb, sizeof(fb), "%02d", atoi(sv) * 9 / 5 + 32); \
        lv_label_set_text(obj, fb); \
    } else { \
        lv_label_set_text(obj, sv ? sv : ""); \
    } \
} while (0)

        _LT(bk_ui->manualmodestart_manual_freeze_temp_txt,  "ManualFreezeTemp");
        _LT(bk_ui->manualmodestart_manual_defrost_temp_txt, "ManualDefrostTemp");

        if (state->manual_current_mode == MANUAL_MODE_FERM2)
        {
            char fb[16];
            int temp = isF ? (state->send_ferm1_temp * 9 / 5 + 32) : state->send_ferm1_temp;
            snprintf(fb, sizeof(fb), "%d", temp);
            lv_label_set_text(bk_ui->manualmodestart_manual_fermentation_temp_txt, fb);
            snprintf(fb, sizeof(fb), "%d", state->send_ferm1_humidity);
            lv_label_set_text(bk_ui->manualmodestart_manual_fermentation_humidity_txt, fb);
        }
        else
        {
            _LT(bk_ui->manualmodestart_manual_fermentation_temp_txt, "ManualFermentationTemp");
            lv_label_set_text(bk_ui->manualmodestart_manual_fermentation_humidity_txt,
                              settings_get_str("ManualFermentationHumidity"));
        }
#undef _LT
    }

    lv_color_t arcColor = lv_color_hex(0xC81D25);

    switch (state->manual_current_mode)
    {
        case MANUAL_MODE_FREEZE:
            lv_obj_set_style_bg_color(bk_ui->manualmodestart, lv_color_hex(0x162A9E), 0);
            arcColor = lv_color_hex(0x162A9E);
            lv_obj_clear_flag(bk_ui->manualmodestart_manual_freeze_temp_txt, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(bk_ui->manualmodestart_manual_freeze_temp_bt, LV_OBJ_FLAG_HIDDEN);
            break;

        case MANUAL_MODE_DEFROST:
            lv_obj_set_style_bg_color(bk_ui->manualmodestart, lv_color_hex(0x53BAE4), 0);
            arcColor = lv_color_hex(0x53BAE4);
            lv_obj_clear_flag(bk_ui->manualmodestart_manual_defrost_temp_txt, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(bk_ui->manualmodestart_manual_defrost_temp_bt, LV_OBJ_FLAG_HIDDEN);
            break;

        case MANUAL_MODE_FERM:
            lv_obj_set_style_bg_color(bk_ui->manualmodestart, lv_color_hex(0xC81D25), 0);
            arcColor = lv_color_hex(0xC81D25);
            lv_obj_clear_flag(bk_ui->manualmodestart_manual_fermentation_temp_txt, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(bk_ui->manualmodestart_manual_fermentation_humidity_txt, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(bk_ui->manualmodestart_manual_fermentation_temp_bt, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(bk_ui->manualmodestart_manual_fermentation_humidity_bt, LV_OBJ_FLAG_HIDDEN);
            break;

        case MANUAL_MODE_FERM2:
        {
            int lang = settings_get_int("LANGUAGE");
            const char *bgPath = (lang == 1) ? "/images/fermentation_bg_china.jpg" :
                                 (lang == 2) ? "/images/fermentation_bg_english.jpg" :
                                               "/images/fermentation_bg.jpg";

            _ferm2_bg_load(bk_ui, bgPath, lang);
            arcColor = lv_color_hex(0xD4A020);

            lv_obj_clear_flag(bk_ui->manualmodestart_manual_fermentation_temp_txt, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(bk_ui->manualmodestart_manual_fermentation_humidity_txt, LV_OBJ_FLAG_HIDDEN);
            lv_obj_set_style_text_color(bk_ui->manualmodestart_manual_fermentation_temp_txt,
                                        arcColor, 0);
            lv_obj_set_style_text_color(bk_ui->manualmodestart_manual_fermentation_humidity_txt,
                                        arcColor, 0);

            int overMin = atoi(settings_get_str("DetailOverFermentation"));
            if (overMin > 0)
            {
                char overText[8];
                snprintf(overText, sizeof(overText), "%d", overMin);

                lv_obj_t *overLabel = (lang == 1) ? bk_ui->manualmodestart_over_time_china :
                                      (lang == 2) ? bk_ui->manualmodestart_over_time_english :
                                                    bk_ui->manualmodestart_over_time_korea;
                if (overLabel != NULL)
                {
                    lv_label_set_text(overLabel, overText);
                    lv_obj_clear_flag(overLabel, LV_OBJ_FLAG_HIDDEN);
                }
            }
            break;
        }

        default:
            return;
    }

    lv_obj_set_style_arc_color(bk_ui->manualmodestart_run_arc, arcColor, LV_PART_INDICATOR);
    _arc_anim_stop(bk_ui->manualmodestart_run_arc);
    lv_obj_add_flag(bk_ui->manualmodestart_run_arc, LV_OBJ_FLAG_HIDDEN);

    if (state->manual_start || state->auto_mode_over)
    {
        _ui_apply_running_mms(bk_ui);

        if (state->auto_mode_over)
        {
            lv_obj_add_flag(bk_ui->manualmodestart_startbt, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(bk_ui->manualmodestart_startim, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(bk_ui->manualmodestart_backim, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(bk_ui->manualmodestart_backbt, LV_OBJ_FLAG_HIDDEN);
        }
    }
    else
    {
        _ui_apply_stopped_mms(bk_ui);
    }
}


void manualmodestart_unload_start_event_cb(lv_event_t *e)
{
    (void)e;
    manualmodestart_runtime_stop(&bk_lv_tool_ui);
}



void manualmodestart_unloaded_event_cb(lv_event_t *e)
{
    (void)e;
    return;
}


void manualmodestart_loaded_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    ui_title_anim(bk_ui->manualmodestart_title);
    return;
}