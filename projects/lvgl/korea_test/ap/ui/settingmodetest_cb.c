
#include "lvgl.h"
#include "beken_ui.h"
#include "custom_func.h"
#include "event_runtime.h"
#include <stdio.h>
#include <string.h>

#include "beken_ui.h"
#include "ui_animations.h"
#include "ui_lang.h"
#include "settings.h"
#include "device_state.h"
#include "uart_comm.h"
#include "hardware_hal.h"

#define TAG "[settingmodetest_cb.c] "
#define bk_printf(fmt, ...) do {if(0) printf(fmt, ##__VA_ARGS__); } while(0) // disable printf

extern bk_lv_ui_t bk_lv_tool_ui;
extern void popuperror_toggle(bk_lv_ui_t *bk_ui);

static uint32_t last_click_time = 0;
static lv_timer_t *s_test_timer    = NULL;

static void _test_set_img(bk_lv_ui_t *bk_ui, int idx, int on);  /* forward decl */

void settingmodetest_backbt_event_cb(lv_event_t *e);
void settingmodetest_compbt_event_cb(lv_event_t *e);
void settingmodetest_roomfanbt_event_cb(lv_event_t *e);
void settingmodetest_fireheaterbt_event_cb(lv_event_t *e);
void settingmodetest_humidityheaterbt_event_cb(lv_event_t *e);
void settingmodetest_watervalvebt_event_cb(lv_event_t *e);
void settingmodetest_defrostheaterbt_event_cb(lv_event_t *e);
void settingmodetest_ledbt_event_cb(lv_event_t *e);
void settingmodetest_cabinetheaterbt_event_cb(lv_event_t *e);
void settingmodetest_damperbt_event_cb(lv_event_t *e);
void settingmodetest_test_error_check_bt_event_cb(lv_event_t *e);
void settingmodetest_load_event_cb(lv_event_t *e);

static void _stop_test_timer(void)
{
    if (s_test_timer) { lv_timer_delete(s_test_timer); s_test_timer = NULL; }
}


void settingmodetest_backbt_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    device_state_t *state = &g_device_state;
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    if (lv_tick_elaps(last_click_time) < 250) return;
    last_click_time = lv_tick_get();
    hal_buzzer_beep();

    _stop_test_timer();
    if (bk_ui->settingmode == NULL || !lv_obj_is_valid(bk_ui->settingmode))
        init_page_settingmode(bk_ui);
    lv_scr_load(bk_ui->settingmode);
    state->start_run = true;
    state->test_mode = false;
}

static const char * const s_test_bases[9] = {
    "test_comp", "test_roomfan", "test_fireheater",
    "test_humidityheater", "test_water", "test_defrost",
    "test_led", "test_cabinetheater", "test_damper"
};

static void _test_set_img_lang(bk_lv_ui_t *bk_ui, int idx, int on, int lang);

static void _test_set_img(bk_lv_ui_t *bk_ui, int idx, int on)
{
    _test_set_img_lang(bk_ui, idx, on, settings_get_int("LANGUAGE"));
}

static void _test_set_img_lang(bk_lv_ui_t *bk_ui, int idx, int on, int lang)
{
    lv_obj_t *ims[9] = {
        bk_ui->settingmodetest_compim,
        bk_ui->settingmodetest_roomfanim,
        bk_ui->settingmodetest_fireheaterim,
        bk_ui->settingmodetest_humidityheaterim,
        bk_ui->settingmodetest_watervalveim,
        bk_ui->settingmodetest_defrostheaterim,
        bk_ui->settingmodetest_ledim,
        bk_ui->settingmodetest_cabinetheaterim,
        bk_ui->settingmodetest_damperim,
    };
    const char *lsuf = (lang == 1) ? "_china" : (lang == 2) ? "_english" : "";
    char path[128];
    snprintf(path, sizeof(path), "/images/%s%s%s.png",
             s_test_bases[idx], on ? "_on" : "_off", lsuf);
    _img_set_src_timed(ims[idx], path);
}

/* Only set label text when it actually changes — lv_label_set_text always
 * invalidates in LVGL v9 even when the text is identical, which would force
 * a re-composite of the testmode_box JPEG beneath every label every frame. */
static void _label_set_if_changed(lv_obj_t *label, const char *text)
{
    if (strcmp(lv_label_get_text(label), text) != 0)
        lv_label_set_text(label, text);
}

/* timebar_cb.c의 _ICON 매크로와 동일한 표시/숨김 규칙 —
 * 상태 변경 시에만 flag 변경(중복 lv_obj_invalidate 방지) */
static void _update_test_error_icon(bk_lv_ui_t *bk_ui)
{
    bool want_vis   = g_device_state.op_error;
    bool is_hidden  = lv_obj_has_flag(bk_ui->settingmodetest_test_error_check_im, LV_OBJ_FLAG_HIDDEN);
    if (want_vis && is_hidden) {
        _img_ensure_src(bk_ui->settingmodetest_test_error_check_im);
        lv_obj_clear_flag(bk_ui->settingmodetest_test_error_check_im, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(bk_ui->settingmodetest_test_error_check_bt, LV_OBJ_FLAG_HIDDEN);
    } else if (!want_vis && !is_hidden) {
        lv_obj_add_flag(bk_ui->settingmodetest_test_error_check_im, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(bk_ui->settingmodetest_test_error_check_bt, LV_OBJ_FLAG_HIDDEN);
    }
}

static void _update_test_labels(lv_timer_t *timer)
{
    (void)timer;
    device_state_t *state = &g_device_state;
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    _update_test_error_icon(bk_ui);
    if (!state->testreceive) return;
    state->testreceive = false;

    char buf[8];

#define _SET(label, val)      do { snprintf(buf, sizeof(buf), "%d", (int)(val)); _label_set_if_changed(bk_ui->label, buf); } while(0)
#define _SET_BOOL(label, val) _label_set_if_changed(bk_ui->label, (val) ? "ON" : "OFF")
#define _SET_TEMP(label, val) do { \
    int _cv = (int)(val); \
    int _dv = _is_f ? (_cv * 9 / 5 + 32) : _cv; \
    snprintf(buf, sizeof(buf), "%d", _dv); \
    _label_set_if_changed(bk_ui->label, buf); \
} while(0)
    int _is_f = (strcmp(settings_get_str("Degree"), "\xc2\xb0""F") == 0);
    _SET_BOOL(settingmodetest_TestComp,           state->hw_test.comp);
    _SET_BOOL(settingmodetest_TestRoomFan,        state->hw_test.fan);
    _SET_BOOL(settingmodetest_TestFireHeater,     state->hw_test.fire_heater);
    _SET_BOOL(settingmodetest_TestHumidityHeater, state->hw_test.humid_heater);
    _SET_BOOL(settingmodetest_TestWaterValve,     state->hw_test.water_valve);
    _SET_BOOL(settingmodetest_TestDefrostHeater,  state->hw_test.defrost_heater);
    _SET_BOOL(settingmodetest_TestDcLed,          state->hw_test.dc_led);
    _SET_BOOL(settingmodetest_TestCabinetHeater,  state->hw_test.cabinet_heater);
    _SET_BOOL(settingmodetest_TestDamper,         state->hw_test.damper);
    _SET_TEMP(settingmodetest_TestFreezeTemp,          state->hw_test.f_temp);
    _SET_TEMP(settingmodetest_TestDefrostSensorTemp,   state->hw_test.defrost_temp);
    _SET_TEMP(settingmodetest_TestRTSensorTemp,        state->hw_test.rt_temp);
    _SET(settingmodetest_TestHumidity,            state->hw_test.humidity);
#undef _SET
#undef _SET_BOOL
#undef _SET_TEMP
    /* Android 양산모델(ButtonBroadcastReceiver.java: ErrorCodeTxt.setText(String.valueOf(savetest[5])))과
     * 동일하게 10진수로 표기 — 이전엔 %02X(16진수)라서 같은 원시값(예: 16)이 "10"으로 잘못 보였음. */
    snprintf(buf, sizeof(buf), "%d", state->hw_test.error_code);
    _label_set_if_changed(bk_ui->settingmodetest_TestErrorCode, buf);
}

static void _restore_button_imgs(bk_lv_ui_t *bk_ui)
{
    device_state_t *state = &g_device_state;
    int lang = settings_get_int("LANGUAGE");   /* lookup once, not 9× */
    for (int i = 0; i < 9; i++)
        _test_set_img_lang(bk_ui, i, state->savetesttest[i], lang);
}


/* Toggle savetesttest[idx] between 0 and 1, then trigger 0x50 UART packet */
static void _test_toggle(int idx)
{
    if (lv_tick_elaps(last_click_time) < 250) return;
    last_click_time = lv_tick_get();
    hal_buzzer_beep();
    device_state_t *state = &g_device_state;
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    bk_printf(TAG "test mode %d", idx);
    state->savetesttest[idx] = state->savetesttest[idx] ? 0 : 1;
    {
        uint32_t _pt = lv_tick_get();
        _test_set_img(bk_ui, idx, state->savetesttest[idx]);
        uint32_t dt = lv_tick_elaps(_pt);
        if (dt > 20) bk_printf(TAG "[PERF] _test_set_img[%d]=%lu ms (cache miss?)\n", idx, (unsigned long)dt);
    }
    // uart_comm_trigger_hw_test();
}

void settingmodetest_compbt_event_cb(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    _test_toggle(0);   /* Compressor — bit 0 */
}

void settingmodetest_roomfanbt_event_cb(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    _test_toggle(1);   /* Room fan — bit 1 */
}

void settingmodetest_fireheaterbt_event_cb(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    _test_toggle(2);   /* Fire heater — bit 2 */
}

void settingmodetest_humidityheaterbt_event_cb(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    _test_toggle(3);   /* Humidity heater — bit 3 */
}

void settingmodetest_watervalvebt_event_cb(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    _test_toggle(4);   /* Water valve — bit 4 */
}

void settingmodetest_defrostheaterbt_event_cb(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    _test_toggle(5);   /* Defrost heater — bit 5 */
}

void settingmodetest_ledbt_event_cb(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    _test_toggle(6);   /* LED — bit 6 */
}

void settingmodetest_cabinetheaterbt_event_cb(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    _test_toggle(7);   /* Cabinet heater — bit 7 */
}

void settingmodetest_damperbt_event_cb(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    _test_toggle(8);   /* Damper — bit 8 */
}

void settingmodetest_test_error_check_bt_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    if (!g_device_state.op_error) return;
    if (lv_tick_elaps(last_click_time) < 250) return;
    last_click_time = lv_tick_get();
    hal_buzzer_beep();
    popuperror_toggle(bk_ui);
}

void settingmodetest_unload_start_event_cb(lv_event_t *e)
{
    (void)e;

    _stop_test_timer();

    /* testmode_box canvas는 이 화면을 보는 동안만 필요 — 나갈 때 반납.
     * settingmode로 복귀하면 그 화면의 SCREEN_LOADED가 다시 preload한다. */
    settingmodetest_canvas_free();
}

void settingmodetest_unloaded_event_cb(lv_event_t *e)
{
    (void)e;
    return;
}

void settingmodetest_loaded_event_cb(lv_event_t *e)
{
    (void)e;

    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;

    ui_title_anim(bk_ui->settingmodetest_title);

    uint32_t tb = lv_tick_get();

    /* 화면 표시 후 버튼 9개 일괄 로드 */
    _restore_button_imgs(bk_ui);

    bk_printf(
        TAG "[PERF] settingmodetest btn imgs (LOADED) +%lu ms\n",
        (unsigned long)lv_tick_elaps(tb)
    );
}


void settingmodetest_load_start_event_cb(lv_event_t *e)
{
    (void)e;

    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;

    /* keep-alive라 오브젝트는 재사용되지만 canvas는 나갈 때마다 free되므로,
     * 매 진입마다 canvas를 보장하고 imageview4 src를 다시 맞춘다
     * (settingmodetest_init.c의 settingmodetest_apply_bg 주석 참고). */
    settingmodetest_apply_bg(bk_ui);

    uint32_t t0 = lv_tick_get();
    bk_printf(TAG "[PERF] settingmodetest load_event start\n");

    /* 재진입 시 모든 버튼 OFF 상태로 초기화 */
    memset(
        g_device_state.savetesttest,
        0,
        sizeof(g_device_state.savetesttest)
    );

    /* loads testmode_box.jpg, title, exit_bt */
    ui_lang_apply_settingmodetest(bk_ui);

    bk_printf(
        TAG "[PERF]   ui_lang_apply +%lu ms\n",
        (unsigned long)lv_tick_elaps(t0)
    );

    /* clear result labels */
    lv_label_set_text(bk_ui->settingmodetest_TestComp,              "");
    lv_label_set_text(bk_ui->settingmodetest_TestRoomFan,           "");
    lv_label_set_text(bk_ui->settingmodetest_TestFireHeater,        "");
    lv_label_set_text(bk_ui->settingmodetest_TestHumidityHeater,    "");
    lv_label_set_text(bk_ui->settingmodetest_TestWaterValve,        "");
    lv_label_set_text(bk_ui->settingmodetest_TestDefrostHeater,     "");
    lv_label_set_text(bk_ui->settingmodetest_TestDcLed,             "");
    lv_label_set_text(bk_ui->settingmodetest_TestCabinetHeater,     "");
    lv_label_set_text(bk_ui->settingmodetest_TestDamper,            "");
    lv_label_set_text(bk_ui->settingmodetest_TestFreezeTemp,        "");
    lv_label_set_text(bk_ui->settingmodetest_TestDefrostSensorTemp, "");
    lv_label_set_text(bk_ui->settingmodetest_TestRTSensorTemp,      "");
    lv_label_set_text(bk_ui->settingmodetest_TestHumidity,          "");
    lv_label_set_text(bk_ui->settingmodetest_TestErrorCode,         "");

    /* 화면 진입 즉시 0x50 HW_TEST 요청
     * → UART 모듈 폴링 주기(~1200ms) 대기 없음 */
    uart_comm_trigger_hw_test();

    _stop_test_timer();

    s_test_timer = lv_timer_create(
        _update_test_labels,
        200,
        NULL
    );

    /* 다음 lv_task_handler에서 즉시 1회 발화 */
    lv_timer_ready(s_test_timer);

    bk_printf(
        TAG "[PERF] settingmodetest load_event end total=%lu ms\n",
        (unsigned long)lv_tick_elaps(t0)
    );
}