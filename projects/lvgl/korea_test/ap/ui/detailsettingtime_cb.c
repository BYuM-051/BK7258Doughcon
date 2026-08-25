#include "lvgl.h"
#include "beken_ui.h"
#include "custom_func.h"
#include "event_runtime.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "ui_animations.h"
#include "ui_lang.h"
#include "settings.h"
#include "device_state.h"
#include "hardware_hal.h"
#include "uart_comm.h"

#define TAG "[detailsettingtime_cb.c] "
#define bk_printf(fmt, ...) do {if(0) printf(fmt, ##__VA_ARGS__); } while(0) // disable printf

extern bk_lv_ui_t bk_lv_tool_ui;

static uint32_t last_click_time = 0;
static int s_edit_field = -1;

/*
 * Field mapping:
 *  0 = HumidificationTime  (setn1+setn2=seconds XX, setn3=decimal Y)  picker_2
 *  1 = WaterInterval       (setn4+setn5=minutes, setn6+setn7=seconds)  pickerbox2
 *  2 = HumidificationHeaterTime (setn1=Water0, setn2+setn3=tens/units) picker_2
 *  3 = OverFermentationOnOff  (setn2 only, OnOff)                      picker_1
 *  4 = OverFermentation       (setn2 only, 1-60 min)                   picker_1
 */

/* Water = {1,2,3,4,5,6,7,8,9,0}: "0" is last (index 9) */
static const char * const s_Water[] = {
    "1","2","3","4","5","6","7","8","9","0"
}; /* 10 */

static const char * const s_Water0[] = {"0","1"};                             /* 2  */
static const char * const s_Water2[] = {"0","1","2","3","4","5"};              /* 6  */
static const char * const s_Water3[] = {"0","1","2","3","4","5","6","7","8","9"}; /* 10 */

/* Water1: 1-60 minutes */
static const char * const s_Water1[] = {
    "1","2","3","4","5","6","7","8","9","10",
    "11","12","13","14","15","16","17","18","19","20",
    "21","22","23","24","25","26","27","28","29","30",
    "31","32","33","34","35","36","37","38","39","40",
    "41","42","43","44","45","46","47","48","49","50",
    "51","52","53","54","55","56","57","58","59","60"
}; /* 60 */

static const char * const s_OnOff[] = {"ON","OFF"}; /* 2 */

static char s_off_imgs[5][128];
static char s_on_imgs[5][128];
static int  s_roller_field = -1;

static void _build_img_arrays(void)
{
    int lang = settings_get_int("LANGUAGE");
    const char *lsuf = (lang == 1) ? "_china" : (lang == 2) ? "_english" : "";
    for (int i = 0; i < 5; i++) {
        snprintf(s_off_imgs[i], 128, "/images/detail_time_%d_off%s.png", i + 1, lsuf);
        snprintf(s_on_imgs[i],  128, "/images/detail_time_%d_on%s.png",  i + 1, lsuf);
    }
}

static void _build_img_arrays_if_changed(void)
{
    static int s_last_lang = -1;
    int lang = settings_get_int("LANGUAGE");
    if (lang == s_last_lang) return;
    s_last_lang = lang;
    _build_img_arrays();
    s_roller_field = -1;
}

void detailsettingtime_backbt_event_cb(lv_event_t *e);
void detailsettingtime_settingbt1_event_cb(lv_event_t *e);
void detailsettingtime_settingbt2_event_cb(lv_event_t *e);
void detailsettingtime_settingbt3_event_cb(lv_event_t *e);
void detailsettingtime_settingbt4_event_cb(lv_event_t *e);
void detailsettingtime_settingbt5_event_cb(lv_event_t *e);
void detailsettingtime_changebt_event_cb(lv_event_t *e);
void detailsettingtime_roller_event_cb(lv_event_t *e);
void detailsettingtime_load_event_cb(lv_event_t *e);

/* Water array index: {1,2,...,9,0} — "0" is at index 9 */
static int _water_idx(char c)
{
    return (c == '0') ? 9 : (c - '1');
}

static void _build_opts(const char * const *arr, int cnt, char *buf, size_t bufsz)
{
    char *p = buf;
    char * const end = buf + bufsz - 1;
    for (int i = 0; i < cnt && p < end; i++) {
        if (i > 0 && p < end) *p++ = '\n';
        size_t len = strlen(arr[i]);
        if (p + len >= end) break;
        memcpy(p, arr[i], len);
        p += len;
    }
    *p = '\0';
}

static void _hide_all_rollers(bk_lv_ui_t *bk_ui)
{
    lv_obj_add_flag(bk_ui->detailsettingtime_settemp_setn1, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(bk_ui->detailsettingtime_settemp_setn2, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(bk_ui->detailsettingtime_settemp_setn3, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(bk_ui->detailsettingtime_settemp_setn4, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(bk_ui->detailsettingtime_settemp_setn5, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(bk_ui->detailsettingtime_settemp_setn6, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(bk_ui->detailsettingtime_settemp_setn7, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(bk_ui->detailsettingtime_pickerbox,  LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(bk_ui->detailsettingtime_pickerbox2, LV_OBJ_FLAG_HIDDEN);
}

/* 활성화(편집 중)된 항목은 검정, 나머지는 기존 회색 유지 */
static void _update_txt_colors(bk_lv_ui_t *bk_ui, int active_field)
{
    lv_color_t on  = lv_color_hex(0x3C3A3D);
    lv_color_t off = lv_color_hex(0xA6A6A6);
    lv_obj_set_style_text_color(bk_ui->detailsettingtime_settingtxt1_1, active_field == 0 ? on : off, 0);
    lv_obj_set_style_text_color(bk_ui->detailsettingtime_settingtxt1_2, active_field == 0 ? on : off, 0);
    lv_obj_set_style_text_color(bk_ui->detailsettingtime_settingtxt2_1, active_field == 1 ? on : off, 0);
    lv_obj_set_style_text_color(bk_ui->detailsettingtime_settingtxt2_2, active_field == 1 ? on : off, 0);
    lv_obj_set_style_text_color(bk_ui->detailsettingtime_settingtxt3,   active_field == 2 ? on : off, 0);
    lv_obj_set_style_text_color(bk_ui->detailsettingtime_settingtxt4,   active_field == 3 ? on : off, 0);
    lv_obj_set_style_text_color(bk_ui->detailsettingtime_settingtxt5,   active_field == 4 ? on : off, 0);
}

static void _settingoff(bk_lv_ui_t *bk_ui)
{
    _img_set_src_timed(bk_ui->detailsettingtime_settingim1, s_off_imgs[0]);
    _img_set_src_timed(bk_ui->detailsettingtime_settingim2, s_off_imgs[1]);
    _img_set_src_timed(bk_ui->detailsettingtime_settingim3, s_off_imgs[2]);
    _img_set_src_timed(bk_ui->detailsettingtime_settingim4, s_off_imgs[3]);
    _img_set_src_timed(bk_ui->detailsettingtime_settingim5, s_off_imgs[4]);
    _hide_all_rollers(bk_ui);
    s_edit_field = -1;
    _update_txt_colors(bk_ui, -1);
}

static void _refresh(bk_lv_ui_t *bk_ui)
{
    _img_set_src_timed(bk_ui->detailsettingtime_settingim1, s_off_imgs[0]);
    _img_set_src_timed(bk_ui->detailsettingtime_settingim2, s_off_imgs[1]);
    _img_set_src_timed(bk_ui->detailsettingtime_settingim3, s_off_imgs[2]);
    _img_set_src_timed(bk_ui->detailsettingtime_settingim4, s_off_imgs[3]);
    _img_set_src_timed(bk_ui->detailsettingtime_settingim5, s_off_imgs[4]);

    lv_label_set_text(bk_ui->detailsettingtime_settingtxt1_1, settings_get_str("DetailHumidificationTime0"));
    lv_label_set_text(bk_ui->detailsettingtime_settingtxt1_2, settings_get_str("DetailHumidificationTime1"));
    lv_label_set_text(bk_ui->detailsettingtime_settingtxt2_1, settings_get_str("DetailWaterInterval0"));
    lv_label_set_text(bk_ui->detailsettingtime_settingtxt2_2, settings_get_str("DetailWaterInterval1"));
    lv_label_set_text(bk_ui->detailsettingtime_settingtxt3,   settings_get_str("DetailHumidificationHeaterTime"));
    /* OverFermentationOnOff: ON/OFF → 중국어는 开/关 폰트 */
    ui_lang_set_onoff_display(bk_ui->detailsettingtime_settingtxt4,
                               settings_get_str("DetailOverFermentationOnOff"));
    lv_label_set_text(bk_ui->detailsettingtime_settingtxt5,   settings_get_str("DetailOverFermentation"));
}

/* set options + visible_row_count + selected on a roller */
#define _SET_ROLLER(roller, arr, cnt, sel) do { \
    static char _opts[512]; \
    _build_opts((arr), (cnt), _opts, sizeof(_opts)); \
    lv_roller_set_options((roller), _opts, (cnt) <= 2 ? LV_ROLLER_MODE_NORMAL : LV_ROLLER_MODE_INFINITE); \
    lv_roller_set_visible_row_count((roller), 3); \
    lv_roller_set_selected((roller), (uint16_t)(sel), LV_ANIM_OFF); \
} while (0)
/* set selected only (options already loaded) */
#define _SEL_ROLLER(roller, sel) lv_roller_set_selected((roller), (uint16_t)(sel), LV_ANIM_OFF)

/* Parse 1-2 char string into two digit chars (leading zero for short) */
static void _parse2(const char *s, char *c_tens, char *c_units)
{
    int len = (int)strlen(s);
    *c_tens  = (len >= 2) ? s[0] : '0';
    *c_units = (len >= 2) ? s[1] : (len == 1 ? s[0] : '0');
}

/* Parse 1-3 char string into three digit chars */
static void _parse3(const char *s, char *c0, char *c1, char *c2)
{
    int len = (int)strlen(s);
    *c0 = (len >= 3) ? s[0] : '0';
    *c1 = (len >= 3) ? s[1] : (len == 2 ? s[0] : '0');
    *c2 = (len >= 3) ? s[2] : (len == 2 ? s[1] : (len == 1 ? s[0] : '0'));
}

static void _show_picker(bk_lv_ui_t *bk_ui, int field)
{
    _settingoff(bk_ui);
    s_edit_field = field;
    _update_txt_colors(bk_ui, field);

    /* ON image */
    lv_obj_t *im_arr[5] = {
        bk_ui->detailsettingtime_settingim1,
        bk_ui->detailsettingtime_settingim2,
        bk_ui->detailsettingtime_settingim3,
        bk_ui->detailsettingtime_settingim4,
        bk_ui->detailsettingtime_settingim5,
    };
    _img_set_src_timed(im_arr[field], s_on_imgs[field]);
    /* pickerbox visibility is handled per-case in the switch below.
     * Do NOT call detailsettingtemp_pickerbox here — it belongs to a
     * different screen and is NULL in this context (caused assert). */




    bool need_opts = (s_roller_field != field);
    char ct, cu, c0, c1, c2;
    const char *val;

    /* setn2는 필드 0~4가 전부 공유하는 roller — case 3(중국어 开/关)에서 남긴
     * 전용 폰트가 다른 숫자 필드에도 남아있으면 숫자 글리프가 없어 깨진다.
     * 매번 진입 시 기본 폰트로 되돌린 뒤, case 3에서만 필요시 다시 교체한다. */
    lv_obj_set_style_text_font(bk_ui->detailsettingtime_settemp_setn2, &lv_font_scdream_regular_72, LV_PART_MAIN);
    lv_obj_set_style_text_font(bk_ui->detailsettingtime_settemp_setn2, &lv_font_scdream_regular_90, LV_PART_SELECTED);

    switch (field) {

    case 0: /* HumidificationTime: setn1=tens, setn2=units (of seconds), setn3=decimal */
        ui_lang_apply_picker(bk_ui->detailsettingtime_pickerbox, 3);
        _img_ensure_src(bk_ui->detailsettingtime_pickerbox);
        lv_obj_clear_flag(bk_ui->detailsettingtime_pickerbox, LV_OBJ_FLAG_HIDDEN);
        /* setn2는 case 3/4에서 단독 150px폭으로 재사용되므로, 3-roller 모드 복귀 시
         * 원래 폭/위치로 명시적으로 되돌린다. */
        lv_obj_set_width(bk_ui->detailsettingtime_settemp_setn2, 90);
        /* setn1/setn3(detailsettingtime_init.c)와 동일 오프셋으로 맞춰야 3-roller 그룹의
         * 가운데 롤러가 아래로 처져 보이지 않는다 — 예전엔 136 고정이라 init.c에서
         * setn2만 따로 올려도 여기서 매번 덮어써져 무효화됐음. */
        lv_obj_set_pos(bk_ui->detailsettingtime_settemp_setn2, 767-1, 136-2-1-5);
        val = settings_get_str("DetailHumidificationTime0");
        _parse2(val, &ct, &cu);
        val = settings_get_str("DetailHumidificationTime1");
        if (need_opts) {
            _SET_ROLLER(bk_ui->detailsettingtime_settemp_setn1, s_Water, 10, _water_idx(ct));
            _SET_ROLLER(bk_ui->detailsettingtime_settemp_setn2, s_Water, 10, _water_idx(cu));
            _SET_ROLLER(bk_ui->detailsettingtime_settemp_setn3, s_Water, 10, _water_idx(val[0]));
        } else {
            _SEL_ROLLER(bk_ui->detailsettingtime_settemp_setn1, _water_idx(ct));
            _SEL_ROLLER(bk_ui->detailsettingtime_settemp_setn2, _water_idx(cu));
            _SEL_ROLLER(bk_ui->detailsettingtime_settemp_setn3, _water_idx(val[0]));
        }
        _img_ensure_src(bk_ui->detailsettingtime_settemp_setn1);
        lv_obj_clear_flag(bk_ui->detailsettingtime_settemp_setn1, LV_OBJ_FLAG_HIDDEN);
        _img_ensure_src(bk_ui->detailsettingtime_settemp_setn2);
        lv_obj_clear_flag(bk_ui->detailsettingtime_settemp_setn2, LV_OBJ_FLAG_HIDDEN);
        _img_ensure_src(bk_ui->detailsettingtime_settemp_setn3);
        lv_obj_clear_flag(bk_ui->detailsettingtime_settemp_setn3, LV_OBJ_FLAG_HIDDEN);
        break;

    case 1: /* WaterInterval: setn4+setn5=minutes, setn6+setn7=seconds; uses pickerbox2 */
        ui_lang_apply_picker(bk_ui->detailsettingtime_pickerbox2, 4);
        _img_ensure_src(bk_ui->detailsettingtime_pickerbox2);
        lv_obj_clear_flag(bk_ui->detailsettingtime_pickerbox2, LV_OBJ_FLAG_HIDDEN);
        val = settings_get_str("DetailWaterInterval0");
        _parse2(val, &ct, &cu);
        if (need_opts) {
            _SET_ROLLER(bk_ui->detailsettingtime_settemp_setn4, s_Water, 10, _water_idx(ct));
            _SET_ROLLER(bk_ui->detailsettingtime_settemp_setn5, s_Water, 10, _water_idx(cu));
            val = settings_get_str("DetailWaterInterval1");
            _parse2(val, &ct, &cu);
            _SET_ROLLER(bk_ui->detailsettingtime_settemp_setn6, s_Water2, 6, (ct - '0'));
            _SET_ROLLER(bk_ui->detailsettingtime_settemp_setn7, s_Water3, 10, (cu - '0'));
        } else {
            _SEL_ROLLER(bk_ui->detailsettingtime_settemp_setn4, _water_idx(ct));
            _SEL_ROLLER(bk_ui->detailsettingtime_settemp_setn5, _water_idx(cu));
            val = settings_get_str("DetailWaterInterval1");
            _parse2(val, &ct, &cu);
            _SEL_ROLLER(bk_ui->detailsettingtime_settemp_setn6, (ct - '0'));
            _SEL_ROLLER(bk_ui->detailsettingtime_settemp_setn7, (cu - '0'));
        }
        _img_ensure_src(bk_ui->detailsettingtime_settemp_setn4);
        lv_obj_clear_flag(bk_ui->detailsettingtime_settemp_setn4, LV_OBJ_FLAG_HIDDEN);
        _img_ensure_src(bk_ui->detailsettingtime_settemp_setn5);
        lv_obj_clear_flag(bk_ui->detailsettingtime_settemp_setn5, LV_OBJ_FLAG_HIDDEN);
        _img_ensure_src(bk_ui->detailsettingtime_settemp_setn6);
        lv_obj_clear_flag(bk_ui->detailsettingtime_settemp_setn6, LV_OBJ_FLAG_HIDDEN);
        _img_ensure_src(bk_ui->detailsettingtime_settemp_setn7);
        lv_obj_clear_flag(bk_ui->detailsettingtime_settemp_setn7, LV_OBJ_FLAG_HIDDEN);
        break;

    case 2: /* HumidificationHeaterTime: setn1=Water0, setn2=Water(tens), setn3=Water(units) */
        ui_lang_apply_picker(bk_ui->detailsettingtime_pickerbox, 2);
        _img_ensure_src(bk_ui->detailsettingtime_pickerbox);
        lv_obj_clear_flag(bk_ui->detailsettingtime_pickerbox, LV_OBJ_FLAG_HIDDEN);
        /* setn2는 case 3/4에서 단독 150px폭으로 재사용되므로, 3-roller 모드 복귀 시
         * 원래 폭/위치로 명시적으로 되돌린다. */
        lv_obj_set_width(bk_ui->detailsettingtime_settemp_setn2, 90);
        /* setn1/setn3(detailsettingtime_init.c)와 동일 오프셋으로 맞춰야 3-roller 그룹의
         * 가운데 롤러가 아래로 처져 보이지 않는다 — 예전엔 136 고정이라 init.c에서
         * setn2만 따로 올려도 여기서 매번 덮어써져 무효화됐음. */
        lv_obj_set_pos(bk_ui->detailsettingtime_settemp_setn2, 767-1, 136-2-1-5);
        val = settings_get_str("DetailHumidificationHeaterTime");
        _parse3(val, &c0, &c1, &c2);
        if (need_opts) {
            _SET_ROLLER(bk_ui->detailsettingtime_settemp_setn1, s_Water0, 2, (c0 == '0') ? 0 : 1);
            _SET_ROLLER(bk_ui->detailsettingtime_settemp_setn2, s_Water, 10, _water_idx(c1));
            _SET_ROLLER(bk_ui->detailsettingtime_settemp_setn3, s_Water, 10, _water_idx(c2));
        } else {
            _SEL_ROLLER(bk_ui->detailsettingtime_settemp_setn1, (c0 == '0') ? 0 : 1);
            _SEL_ROLLER(bk_ui->detailsettingtime_settemp_setn2, _water_idx(c1));
            _SEL_ROLLER(bk_ui->detailsettingtime_settemp_setn3, _water_idx(c2));
        }
        _img_ensure_src(bk_ui->detailsettingtime_settemp_setn1);
        lv_obj_clear_flag(bk_ui->detailsettingtime_settemp_setn1, LV_OBJ_FLAG_HIDDEN);
        _img_ensure_src(bk_ui->detailsettingtime_settemp_setn2);
        lv_obj_clear_flag(bk_ui->detailsettingtime_settemp_setn2, LV_OBJ_FLAG_HIDDEN);
        _img_ensure_src(bk_ui->detailsettingtime_settemp_setn3);
        lv_obj_clear_flag(bk_ui->detailsettingtime_settemp_setn3, LV_OBJ_FLAG_HIDDEN);
        break;

    case 3: /* OverFermentationOnOff: setn2 with OnOff */
        ui_lang_apply_picker(bk_ui->detailsettingtime_pickerbox, 1);
        _img_ensure_src(bk_ui->detailsettingtime_pickerbox);
        lv_obj_clear_flag(bk_ui->detailsettingtime_pickerbox, LV_OBJ_FLAG_HIDDEN);
        val = settings_get_str("DetailOverFermentationOnOff");
        {
            int sel = (strcmp(val, "ON") == 0) ? 0 : 1;
            bool cn_onoff = (settings_get_int("LANGUAGE") == 1);
            if (need_opts) {
                if (cn_onoff) {
                    /* "开\n关" (U+5F00, U+5173) — s_OnOff 순서(ON,OFF)와 동일하게 매핑 */
                    lv_roller_set_options(bk_ui->detailsettingtime_settemp_setn2,
                                           "\xe5\xbc\x80\n\xe5\x85\xb3", LV_ROLLER_MODE_NORMAL);
                    lv_roller_set_visible_row_count(bk_ui->detailsettingtime_settemp_setn2, 3);
                    lv_roller_set_selected(bk_ui->detailsettingtime_settemp_setn2, (uint16_t)sel, LV_ANIM_OFF);
                } else {
                    _SET_ROLLER(bk_ui->detailsettingtime_settemp_setn2, s_OnOff, 2, sel);
                }
            } else {
                _SEL_ROLLER(bk_ui->detailsettingtime_settemp_setn2, sel);
            }
            if (cn_onoff) {
                lv_obj_set_style_text_font(bk_ui->detailsettingtime_settemp_setn2, &lv_font_onoff_cn_72, LV_PART_MAIN);
                lv_obj_set_style_text_font(bk_ui->detailsettingtime_settemp_setn2, &lv_font_onoff_cn_90, LV_PART_SELECTED);
            }
        }
        _img_ensure_src(bk_ui->detailsettingtime_settemp_setn2);
        lv_obj_clear_flag(bk_ui->detailsettingtime_settemp_setn2, LV_OBJ_FLAG_HIDDEN);
        /* 단독 사용 시 150px폭 — pickerbox(624~1000, 중심 812) 기준 중앙(737)에 위치 */
        lv_obj_set_width(bk_ui->detailsettingtime_settemp_setn2, 240);
        lv_obj_set_pos(bk_ui->detailsettingtime_settemp_setn2, 690, 128);
        break;

    case 4: /* OverFermentation: setn2 with Water1 (1-60 minutes) */
        ui_lang_apply_picker(bk_ui->detailsettingtime_pickerbox, 1);
        _img_ensure_src(bk_ui->detailsettingtime_pickerbox);
        lv_obj_clear_flag(bk_ui->detailsettingtime_pickerbox, LV_OBJ_FLAG_HIDDEN);
        val = settings_get_str("DetailOverFermentation");
        {
            int v = atoi(val);
            if (v < 1) v = 1;
            if (v > 60) v = 60;
            if (need_opts) {
                _SET_ROLLER(bk_ui->detailsettingtime_settemp_setn2, s_Water1, 60, v - 1);
            } else {
                _SEL_ROLLER(bk_ui->detailsettingtime_settemp_setn2, v - 1);
            }
        }
        /* 단독 사용 시 150px폭 — pickerbox(624~1000, 중심 812) 기준 중앙(737)에 위치 */
        lv_obj_set_width(bk_ui->detailsettingtime_settemp_setn2, 240);
        lv_obj_set_pos(bk_ui->detailsettingtime_settemp_setn2,690, 128);
        // _img_ensure_src(bk_ui->detailsettingtime_settemp_setn2);
        lv_obj_clear_flag(bk_ui->detailsettingtime_settemp_setn2, LV_OBJ_FLAG_HIDDEN);
        break;
    }

    s_roller_field = field;
}

#undef _SET_ROLLER
#undef _SEL_ROLLER

void detailsettingtime_roller_event_cb(lv_event_t *e) { (void)e; }

void detailsettingtime_backbt_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    if (lv_tick_elaps(last_click_time) < 250) return;
    last_click_time = lv_tick_get();
    hal_buzzer_beep();

    settings_save_dirty();
    if (bk_ui->settingmodedetailsetting == NULL || !lv_obj_is_valid(bk_ui->settingmodedetailsetting))
        init_page_settingmodedetailsetting(bk_ui);
    lv_scr_load(bk_ui->settingmodedetailsetting);
}

static void _settingbt(int field)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    if (lv_tick_elaps(last_click_time) < 250) return;
    last_click_time = lv_tick_get();
    hal_buzzer_beep();
    _show_picker(bk_ui, field);
}

void detailsettingtime_settingbt1_event_cb(lv_event_t *e)
{ if (lv_event_get_code(e) != LV_EVENT_CLICKED) return; _settingbt(0); }

void detailsettingtime_settingbt2_event_cb(lv_event_t *e)
{ if (lv_event_get_code(e) != LV_EVENT_CLICKED) return; _settingbt(1); }

void detailsettingtime_settingbt3_event_cb(lv_event_t *e)
{ if (lv_event_get_code(e) != LV_EVENT_CLICKED) return; _settingbt(2); }

void detailsettingtime_settingbt4_event_cb(lv_event_t *e)
{ if (lv_event_get_code(e) != LV_EVENT_CLICKED) return; _settingbt(3); }

void detailsettingtime_settingbt5_event_cb(lv_event_t *e)
{ if (lv_event_get_code(e) != LV_EVENT_CLICKED) return; _settingbt(4); }

void detailsettingtime_changebt_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    device_state_t *state = &g_device_state;
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    if (lv_tick_elaps(last_click_time) < 250) return;
    last_click_time = lv_tick_get();
    hal_buzzer_beep();

    if (s_edit_field < 0 || s_edit_field >= 5) return;

    char s1[8], s2[8], s3[8], s4[8];
    char result[16], result2[16];

    switch (s_edit_field) {

    case 0: /* HumidificationTime */
        lv_roller_get_selected_str(bk_ui->detailsettingtime_settemp_setn1, s1, sizeof(s1)); /* tens  */
        lv_roller_get_selected_str(bk_ui->detailsettingtime_settemp_setn2, s2, sizeof(s2)); /* units */
        lv_roller_get_selected_str(bk_ui->detailsettingtime_settemp_setn3, s3, sizeof(s3)); /* decimal */
        if (strcmp(s1, "0") == 0)
            snprintf(result, sizeof(result), "%s", s2);
        else
            snprintf(result, sizeof(result), "%s%s", s1, s2);
        lv_label_set_text(bk_ui->detailsettingtime_settingtxt1_1, result);
        lv_label_set_text(bk_ui->detailsettingtime_settingtxt1_2, s3);
        settings_set_str("DetailHumidificationTime0", result);
        settings_set_str("DetailHumidificationTime1", s3);
        break;

    case 1: /* WaterInterval */
        lv_roller_get_selected_str(bk_ui->detailsettingtime_settemp_setn4, s1, sizeof(s1)); /* min tens  */
        lv_roller_get_selected_str(bk_ui->detailsettingtime_settemp_setn5, s2, sizeof(s2)); /* min units */
        lv_roller_get_selected_str(bk_ui->detailsettingtime_settemp_setn6, s3, sizeof(s3)); /* sec tens  */
        lv_roller_get_selected_str(bk_ui->detailsettingtime_settemp_setn7, s4, sizeof(s4)); /* sec units */
        if (strcmp(s1, "0") == 0)
            snprintf(result, sizeof(result), "%s", s2);
        else
            snprintf(result, sizeof(result), "%s%s", s1, s2);
        if (strcmp(s3, "0") == 0)
            snprintf(result2, sizeof(result2), "%s", s4);
        else
            snprintf(result2, sizeof(result2), "%s%s", s3, s4);
        lv_label_set_text(bk_ui->detailsettingtime_settingtxt2_1, result);
        lv_label_set_text(bk_ui->detailsettingtime_settingtxt2_2, result2);
        settings_set_str("DetailWaterInterval0", result);
        settings_set_str("DetailWaterInterval1", result2);
        break;

    case 2: /* HumidificationHeaterTime */
        lv_roller_get_selected_str(bk_ui->detailsettingtime_settemp_setn1, s1, sizeof(s1)); /* hundreds */
        lv_roller_get_selected_str(bk_ui->detailsettingtime_settemp_setn2, s2, sizeof(s2)); /* tens     */
        lv_roller_get_selected_str(bk_ui->detailsettingtime_settemp_setn3, s3, sizeof(s3)); /* units    */
        if (strcmp(s1, "0") == 0) {
            if (strcmp(s2, "0") == 0)
                snprintf(result, sizeof(result), "%s", s3);
            else
                snprintf(result, sizeof(result), "%s%s", s2, s3);
        } else {
            snprintf(result, sizeof(result), "%s%s%s", s1, s2, s3);
        }
        { int v = atoi(result); if (v > 120) snprintf(result, sizeof(result), "120"); }
        lv_label_set_text(bk_ui->detailsettingtime_settingtxt3, result);
        settings_set_str("DetailHumidificationHeaterTime", result);
        break;

    case 3: /* OverFermentationOnOff */
        /* roller가 중국어(开/关)로 표시 중일 수 있어 텍스트 대신 선택 인덱스로
         * 판정한다(0=ON,1=OFF, s_OnOff 순서 고정) — 텍스트를 그대로 쓰면 설정에
         * "开"/"关"이 저장되어 uart_comm.c의 strcmp(...,"ON") 판정이 항상 실패하고,
         * 재진입 시 한국어/영어 폰트로 그 값을 그려 빈칸처럼 보이는 문제가 있었다. */
        snprintf(result, sizeof(result), "%s",
                 (lv_roller_get_selected(bk_ui->detailsettingtime_settemp_setn2) == 0) ? "ON" : "OFF");
        ui_lang_set_onoff_display(bk_ui->detailsettingtime_settingtxt4, result);
        settings_set_str("DetailOverFermentationOnOff", result);
        break;

    case 4: /* OverFermentation (minutes 1-60) */
        lv_roller_get_selected_str(bk_ui->detailsettingtime_settemp_setn2, result, sizeof(result));
        lv_label_set_text(bk_ui->detailsettingtime_settingtxt5, result);
        settings_set_str("DetailOverFermentation", result);
        break;
    }

    settings_save_dirty();
    state->change_setting = true;
    state->start_run = false;
    uart_comm_trigger_change_setting();
}

void detailsettingtime_load_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    if (code == LV_EVENT_SCREEN_LOADED) {
        ui_title_anim(bk_ui->detailsettingtime_title);
        return;
    }
    if (code != LV_EVENT_SCREEN_LOAD_START) return;
    uint32_t t0 = lv_tick_get();
    bk_printf(TAG "[PERF] detailtime load_event start\n");

    _build_img_arrays_if_changed();
    bk_printf(TAG "[PERF]   _build_img_arrays +%lu ms\n", (unsigned long)lv_tick_elaps(t0));

    ui_lang_apply_detailsettingtime(bk_ui);
    bk_printf(TAG "[PERF]   ui_lang_apply +%lu ms\n", (unsigned long)lv_tick_elaps(t0));
    s_edit_field   = -1;
    /* s_roller_field: 언어 변경 시 _build_img_arrays_if_changed()에서 초기화됨.
     * 스크린 재진입마다 초기화하면 첫 탭 시 lv_roller_set_options 가 불필요하게 재호출됨. */
    _refresh(bk_ui);
    _update_txt_colors(bk_ui, -1);   /* 재진입 시 이전 선택 색상(검정) 잔류 방지 */
    bk_printf(TAG "[PERF]   _refresh +%lu ms\n", (unsigned long)lv_tick_elaps(t0));

    _hide_all_rollers(bk_ui);
    bk_printf(TAG "[PERF] detailtime load_event end total=%lu ms\n", (unsigned long)lv_tick_elaps(t0));
}
