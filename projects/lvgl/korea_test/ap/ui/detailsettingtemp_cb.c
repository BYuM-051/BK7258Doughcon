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

#define TAG "[detailsettingtemp_cb.c] "
#define bk_printf(fmt, ...) do {if(0) printf(fmt, ##__VA_ARGS__); } while(0) // disable printf

extern bk_lv_ui_t bk_lv_tool_ui;

static uint32_t last_click_time = 0;
static int s_page = 0;   /* 0=page1(TempOff/Od), 1=page2(Revision/Fan) */
static int s_edit_field = -1;

/* Java 정의 string arrays */
static const char * const s_offTemp[] = {
    "0.0","0.5","1.0","1.5","2.0","2.5","3.0","3.5","4.0","4.5","5.0",
    "-5.0","-4.5","-4.0","-3.5","-3.0","-2.5","-2.0","-1.5","-1.0","-0.5"
}; /* 21 */

static const char * const s_odTemp[] = {
    "0.2","0.5","1.0","1.5","2.0","2.5","3.0","3.5","4.0","4.5",
    "5.0","5.5","6.0","6.5","7.0","7.5","8.0","8.5","9.0","9.5"
}; /* 20 */

static const char * const s_revisionTemp[] = {
    "0","1","2","3","4","5","-5","-4","-3","-2","-1"
}; /* 11 */

static const char * const s_onOff[] = {"ON","OFF"}; /* 2 */

/* F-mode offset/differential arrays */
static const char * const s_offTempF[] = {
    "0.0","0.9","1.8","2.7","3.6","4.5","5.4","6.3","7.2","8.1","9.0",
    "-9.0","-8.1","-7.2","-6.3","-5.4","-4.5","-3.6","-2.7","-1.8","-0.9"
}; /* 21 */
// static const char * const s_odTempF[] = {
//     "1","2","3","4","5","6","7","8","9","10",
//     "11","12","13","14","15","16","17"
// }; /* 17 */
static const char * const s_odTempF[] = {
    "0.4","0.9","1.8","2.7","3.6","4.5","5.4","6.3","7.2","8.1",
    "9.0","9.9","10.8","11.7","12.6","13.5","14.4","15.3","16.2","17.1"
}; /* 20 */
static const char * const s_revisionTempF[] = {
    "0","1.8","3.6","5.4","7.2","9.0",
    "-9.0","-7.2","-5.4","-3.6","-1.8",
}; /* 11 */

/* Pages: 2 pages, 4 fields each */
#define NUM_PAGES 2

static const char * const s_keys[NUM_PAGES][4] = {
    {"DetailTempOff", "DetailTempOd", "DetailFermentationTempOff", "DetailFermentationTempOd"},
    {"DetailTempRevision", "DetailFermentationTempRevision", "DetailFan", NULL},
};

typedef struct { const char * const *arr; int cnt; } _picker_opt_t;

static const _picker_opt_t s_picker_opts[NUM_PAGES][4] = {
    { {s_offTemp,21}, {s_odTemp,20}, {s_offTemp,21}, {s_odTemp,20} },
    { {s_revisionTemp,11}, {s_revisionTemp,11}, {s_onOff,2}, {NULL,0} },
};
static const _picker_opt_t s_picker_opts_f[NUM_PAGES][4] = {
    { {s_offTempF,21}, {s_odTempF,20}, {s_offTempF,21}, {s_odTempF,20} },
    { {s_revisionTempF,11}, {s_revisionTempF,11}, {s_onOff,2}, {NULL,0} },
};
static _picker_opt_t _picker_opt(int page, int field)
{
    int is_f = (strcmp(settings_get_str("Degree"), "\xc2\xb0""F") == 0);
    return is_f ? s_picker_opts_f[page][field] : s_picker_opts[page][field];
}

/* OFF/ON images [page][field(0-3)]; empty string = hidden row */
static char s_off_imgs[NUM_PAGES][4][128];
static char s_on_imgs[NUM_PAGES][4][128];
static int  s_roller_pg_field = -99; /* last page*4+field loaded; -99=none */

static void _build_img_arrays(void)
{
    int lang = settings_get_int("LANGUAGE");
    int is_f = (strcmp(settings_get_str("Degree"), "\xc2\xb0""F") == 0);
    const char *lsuf = (lang == 1) ? "_china" : (lang == 2) ? "_english" : "";
    const char *fsuf = is_f ? "_f" : "";
    /* page 0: detail_temp_2~5 (full _f + lang variants) */
    for (int i = 0; i < 4; i++) {
        snprintf(s_off_imgs[0][i], 128, "/images/detail_temp_%d_off%s%s.png", i + 2, fsuf, lsuf);
        snprintf(s_on_imgs[0][i],  128, "/images/detail_temp_%d_on%s%s.png",  i + 2, fsuf, lsuf);
    }
    /* page 1, fields 0,1: detail_temp_6,7 (full _f + lang variants) */
    for (int i = 0; i < 2; i++) {
        snprintf(s_off_imgs[1][i], 128, "/images/detail_temp_%d_off%s%s.png", i + 6, fsuf, lsuf);
        snprintf(s_on_imgs[1][i],  128, "/images/detail_temp_%d_on%s%s.png",  i + 6, fsuf, lsuf);
    }
    /* page 1, field 2: detail_temp_9 (no _f, full lang variants) */
    snprintf(s_off_imgs[1][2], 128, "/images/detail_temp_9_off%s.png", lsuf);
    snprintf(s_on_imgs[1][2],  128, "/images/detail_temp_9_on%s.png",  lsuf);
    /* page 1, field 3: hidden row */
    s_off_imgs[1][3][0] = '\0';
    s_on_imgs[1][3][0]  = '\0';
}

static void _build_img_arrays_if_changed(void)
{
    static int s_last_lang = -1;
    static int s_last_is_f = -1;
    int lang = settings_get_int("LANGUAGE");
    int is_f = (strcmp(settings_get_str("Degree"), "\xc2\xb0""F") == 0);
    if (lang == s_last_lang && is_f == s_last_is_f) return;
    s_last_lang = lang;
    s_last_is_f = is_f;
    _build_img_arrays();
    s_roller_pg_field = -99;   /* options may be language-dependent — force roller rebuild */
}


void detailsettingtemp_backbt_event_cb(lv_event_t *e);
void detailsettingtemp_settingbt1_event_cb(lv_event_t *e);
void detailsettingtemp_settingbt2_event_cb(lv_event_t *e);
void detailsettingtemp_settingbt3_event_cb(lv_event_t *e);
void detailsettingtemp_settingbt4_event_cb(lv_event_t *e);
void detailsettingtemp_leftbt_event_cb(lv_event_t *e);
void detailsettingtemp_rightbt_event_cb(lv_event_t *e);
void detailsettingtemp_changebt_event_cb(lv_event_t *e);
void detailsettingtemp_roller_event_cb(lv_event_t *e);
void detailsettingtemp_load_start_event_cb(lv_event_t *e);
void detailsettingtemp_loaded_event_cb(lv_event_t *e);
void detailsettingtemp_unload_start_event_cb(lv_event_t *e);
void detailsettingtemp_unloaded_event_cb(lv_event_t *e);

/* settingim/bt/txt 배열 헬퍼 */
static void _get_row_objs(bk_lv_ui_t *bk_ui, lv_obj_t *ims[4], lv_obj_t *bts[4], lv_obj_t *txts[4])
{
    ims[0]  = bk_ui->detailsettingtemp_settingim1;
    ims[1]  = bk_ui->detailsettingtemp_settingim2;
    ims[2]  = bk_ui->detailsettingtemp_settingim3;
    ims[3]  = bk_ui->detailsettingtemp_settingim4;
    bts[0]  = bk_ui->detailsettingtemp_settingbt1;
    bts[1]  = bk_ui->detailsettingtemp_settingbt2;
    bts[2]  = bk_ui->detailsettingtemp_settingbt3;
    bts[3]  = bk_ui->detailsettingtemp_settingbt4;
    txts[0] = bk_ui->detailsettingtemp_settingtxt1;
    txts[1] = bk_ui->detailsettingtemp_settingtxt2;
    txts[2] = bk_ui->detailsettingtemp_settingtxt3;
    txts[3] = bk_ui->detailsettingtemp_settingtxt4;
}

/* 활성화(편집 중)된 항목은 검정, 나머지는 기존 회색 유지 */
static void _update_txt_colors(bk_lv_ui_t *bk_ui, int active_field)
{
    lv_obj_t *ims[4], *bts[4], *txts[4];
    _get_row_objs(bk_ui, ims, bts, txts);
    for (int i = 0; i < 4; i++) {
        lv_obj_set_style_text_color(txts[i],
            (i == active_field) ? lv_color_hex(0x3C3A3D) : lv_color_hex(0xA6A6A6), 0);
    }
}

/* 모든 항목 OFF 상태로 리셋 + roller 숨김 */
static void _settingoff(bk_lv_ui_t *bk_ui)
{
    lv_obj_t *ims[4], *bts[4], *txts[4];
    _get_row_objs(bk_ui, ims, bts, txts);

    for (int i = 0; i < 4; i++) {
        if (s_off_imgs[s_page][i][0] != '\0')
            _img_set_src_timed(ims[i], s_off_imgs[s_page][i]);
    }
    lv_obj_add_flag(bk_ui->detailsettingtemp_settemp_setn1, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(bk_ui->detailsettingtemp_pickerbox, LV_OBJ_FLAG_HIDDEN);
    s_edit_field = -1;
    _update_txt_colors(bk_ui, -1);
}

static void _save_page(bk_lv_ui_t *bk_ui, int page)
{
    const char * const *k = s_keys[page];
    lv_obj_t *ims[4], *bts[4], *txts[4];
    _get_row_objs(bk_ui, ims, bts, txts);
    for (int i = 0; i < 4; i++) {
        if (!k[i]) continue;
        if (page == 1 && i == 2) continue;   /* Fan: 라벨이 중국어(开/关)로 표시 중일 수
             * 있어 lv_label_get_text()를 그대로 저장하면 설정에 开/关이 들어간다.
             * changebt_event_cb()가 이미 캐노니컬 "ON"/"OFF"로 저장해뒀으므로 여기서는
             * 건드리지 않는다 (재저장이 불필요하기도 함). */
        settings_set_str(k[i], lv_label_get_text(txts[i]));
    }
}

/* Return the display string for a field, converting between the C/F arrays by
 * parallel index regardless of which unit was active when the value was saved.
 * changebt() saves whatever the roller currently shows (C-array text in C mode,
 * F-array text in F mode) — so the stored string can be from either array.
 * Both directions must be handled symmetrically: previously the C-mode branch
 * (!is_f) returned the stored string unconverted, so a value saved while in F
 * mode stayed displayed as its raw F number after switching back to C. */
static const char *_display_val(int page, int field, int is_f)
{
    if (!s_keys[page][field]) return "-";
    const char *stored = settings_get_str(s_keys[page][field]);

    _picker_opt_t c_opt = s_picker_opts[page][field];
    _picker_opt_t f_opt = s_picker_opts_f[page][field];
    if (f_opt.arr == NULL || c_opt.arr == NULL) return stored;

    _picker_opt_t target = is_f ? f_opt : c_opt;
    _picker_opt_t other   = is_f ? c_opt : f_opt;

    /* Already stored as a value native to the current unit? Show directly. */
    for (int i = 0; i < target.cnt; i++) {
        if (strcmp(target.arr[i], stored) == 0) return stored;
    }
    /* Find index in the other unit's array, map to same index in target array. */
    for (int i = 0; i < other.cnt && i < target.cnt; i++) {
        if (strcmp(other.arr[i], stored) == 0) return target.arr[i];
    }
    return stored;
}

static void _refresh(bk_lv_ui_t *bk_ui)
{
    const char * const *k = s_keys[s_page];
    lv_obj_t *ims[4], *bts[4], *txts[4];
    _get_row_objs(bk_ui, ims, bts, txts);

    ui_lang_apply_next_bt(bk_ui->detailsettingtemp_currentpage, s_page + 1);

    int is_f = (strcmp(settings_get_str("Degree"), "\xc2\xb0""F") == 0);

    for (int i = 0; i < 4; i++) {
        if (s_off_imgs[s_page][i][0] != '\0') {
            _img_set_src_timed(ims[i], s_off_imgs[s_page][i]);
            _img_ensure_src(ims[i]);
            lv_obj_clear_flag(ims[i], LV_OBJ_FLAG_HIDDEN);
            _img_ensure_src(bts[i]);
            lv_obj_clear_flag(bts[i], LV_OBJ_FLAG_HIDDEN);
            if (s_page == 1 && i == 2) {   /* Fan: ON/OFF → 중국어는 开/关 폰트 */
                ui_lang_set_onoff_display(txts[i], _display_val(s_page, i, is_f));
            } else {
                /* txts[0..3]는 페이지 0/1이 공유하는 물리 위젯 — Fan(페이지1,행3)에서
                 * 중국어 전용 폰트(lv_font_onoff_cn_32, 숫자 글리프 없음)로 바뀐 채
                 * 남아있으면, 같은 행 인덱스를 쓰는 페이지0의 숫자 필드(발효off온도 등)
                 * 텍스트가 네모(tofu) 박스로 보인다. 매번 기본 폰트로 되돌린다. */
                lv_obj_set_style_text_font(txts[i], &lv_font_scdream_regular_32, 0);
                lv_label_set_text(txts[i], _display_val(s_page, i, is_f));
            }
        } else {
            /* page 2 row 4: hide */
            lv_obj_add_flag(ims[i], LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(bts[i], LV_OBJ_FLAG_HIDDEN);
            lv_label_set_text(txts[i], "");
        }
    }
}

static void _show_picker(bk_lv_ui_t *bk_ui, int field)
{
    if (_picker_opt(s_page, field).arr == NULL) return;

    _settingoff(bk_ui);
    s_edit_field = field;
    _update_txt_colors(bk_ui, field);
    _img_ensure_src(bk_ui->detailsettingtemp_pickerbox);
    lv_obj_clear_flag(bk_ui->detailsettingtemp_pickerbox, LV_OBJ_FLAG_HIDDEN);


    /* ON image */
    lv_obj_t *ims[4], *bts[4], *txts[4];
    _get_row_objs(bk_ui, ims, bts, txts);
    if (s_on_imgs[s_page][field][0] != '\0')
        _img_set_src_timed(ims[field], s_on_imgs[s_page][field]);

    /* build roller options — skip if same page+field already loaded */
    _picker_opt_t opt = _picker_opt(s_page, field);
    const char * const *arr = opt.arr;
    int cnt = opt.cnt;
    int pg_field = s_page * 4 + field;

    /* settemp_setn1은 8개 필드(2페이지 x 4)가 전부 공유하는 roller — Fan(중국어
     * 开/关)에서 남긴 전용 폰트가 다른 숫자 필드에도 남으면 숫자 글리프가 없어
     * 깨진다. 매번 진입 시 기본 폰트로 되돌린 뒤, Fan+중국어일 때만 다시 교체한다. */
    lv_obj_set_style_text_font(bk_ui->detailsettingtemp_settemp_setn1, &lv_font_scdream_regular_72, LV_PART_MAIN);
    lv_obj_set_style_text_font(bk_ui->detailsettingtemp_settemp_setn1, &lv_font_scdream_regular_90, LV_PART_SELECTED);
    bool cn_onoff = (s_page == 1 && field == 2 && settings_get_int("LANGUAGE") == 1);

    if (pg_field != s_roller_pg_field) {
        if (cn_onoff) {
            /* "开\n关" (U+5F00, U+5173) */
            lv_roller_set_options(bk_ui->detailsettingtemp_settemp_setn1,
                                   "\xe5\xbc\x80\n\xe5\x85\xb3", LV_ROLLER_MODE_NORMAL);
        } else {
            static char opts[512];
            char *p = opts;
            char * const end = opts + sizeof(opts) - 1;
            for (int i = 0; i < cnt && p < end; i++) {
                if (i > 0 && p < end) *p++ = '\n';
                size_t len = strlen(arr[i]);
                if (p + len >= end) break;
                memcpy(p, arr[i], len);
                p += len;
            }
            *p = '\0';
            uint32_t tr = lv_tick_get();
            lv_roller_set_options(bk_ui->detailsettingtemp_settemp_setn1, opts, cnt <= 2 ? LV_ROLLER_MODE_NORMAL : LV_ROLLER_MODE_INFINITE);
            bk_printf(TAG "[PERF]   roller_set_options(pg=%d,field=%d,cnt=%d) %lu ms\n", s_page, field, cnt, (unsigned long)lv_tick_elaps(tr));
        }
        lv_roller_set_visible_row_count(bk_ui->detailsettingtemp_settemp_setn1, 3);
        s_roller_pg_field = pg_field;
    }
    if (cn_onoff) {
        lv_obj_set_style_text_font(bk_ui->detailsettingtemp_settemp_setn1, &lv_font_onoff_cn_72, LV_PART_MAIN);
        lv_obj_set_style_text_font(bk_ui->detailsettingtemp_settemp_setn1, &lv_font_onoff_cn_90, LV_PART_SELECTED);
    }

    /* find current value index; fall back to opposite-unit array index mapping */
    int is_f = (strcmp(settings_get_str("Degree"), "\xc2\xb0""F") == 0);
    const char *cur = settings_get_str(s_keys[s_page][field]);
    int sel = 0;
    int found = 0;
    for (int i = 0; i < cnt; i++) {
        if (strcmp(arr[i], cur) == 0) { sel = i; found = 1; break; }
    }
    if (!found) {
        /* stored value is in the opposite unit — find its index via the other array */
        _picker_opt_t opp = is_f ? s_picker_opts[s_page][field] : s_picker_opts_f[s_page][field];
        if (opp.arr != NULL) {
            for (int i = 0; i < opp.cnt && i < cnt; i++) {
                if (strcmp(opp.arr[i], cur) == 0) { sel = i; break; }
            }
        }
    }
    lv_roller_set_selected(bk_ui->detailsettingtemp_settemp_setn1, (uint16_t)sel, LV_ANIM_OFF);
    _img_ensure_src(bk_ui->detailsettingtemp_settemp_setn1);
    lv_obj_clear_flag(bk_ui->detailsettingtemp_settemp_setn1, LV_OBJ_FLAG_HIDDEN);
}

void detailsettingtemp_roller_event_cb(lv_event_t *e)
{
    (void)e;
    /* roller scrolling only — value applied when changebt is pressed */
}

void detailsettingtemp_backbt_event_cb(lv_event_t *e)
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

void detailsettingtemp_settingbt1_event_cb(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    _settingbt(0);
}

void detailsettingtemp_settingbt2_event_cb(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    _settingbt(1);
}

void detailsettingtemp_settingbt3_event_cb(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    _settingbt(2);
}

void detailsettingtemp_settingbt4_event_cb(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    _settingbt(3);
}

static void _navigate(bk_lv_ui_t *bk_ui, int dir)
{
    _settingoff(bk_ui);
    _save_page(bk_ui, s_page);
    /* toggle between page 0 and 1 (left/right both toggle) */
    s_page = (s_page == 0) ? 1 : 0;
    _refresh(bk_ui);
}

void detailsettingtemp_leftbt_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    if (lv_tick_elaps(last_click_time) < 250) return;
    last_click_time = lv_tick_get();
    hal_buzzer_beep();
    _navigate(bk_ui, -1);
}

void detailsettingtemp_rightbt_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    if (lv_tick_elaps(last_click_time) < 250) return;
    last_click_time = lv_tick_get();
    hal_buzzer_beep();
    _navigate(bk_ui, 1);
}

void detailsettingtemp_changebt_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    device_state_t *state = &g_device_state;
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    if (lv_tick_elaps(last_click_time) < 250) return;
    last_click_time = lv_tick_get();
    hal_buzzer_beep();

    if (s_edit_field < 0 || s_edit_field >= 4) return;
    if (s_keys[s_page][s_edit_field] == NULL) return;

    char buf[32];   /* raw roller text — already the correct string for the CURRENT unit's display */
    lv_roller_get_selected_str(bk_ui->detailsettingtemp_settemp_setn1, buf, sizeof(buf));

    if (s_page == 1 && s_edit_field == 2) {
        /* Fan roller가 중국어(开/关)로 표시 중일 수 있어 텍스트 대신 선택
         * 인덱스로 판정한다(0=ON,1=OFF, s_onOff 순서 고정) — 텍스트를 그대로
         * 쓰면 설정에 开/关이 저장되어 uart_comm.c의 strcmp(...,"ON") 판정이
         * 항상 실패하고, 재진입 시 폰트 불일치로 빈칸처럼 보이는 문제가 있었다. */
        snprintf(buf, sizeof(buf), "%s",
                 (lv_roller_get_selected(bk_ui->detailsettingtemp_settemp_setn1) == 0) ? "ON" : "OFF");
    }

    /* 저장은 항상 섭씨(C) 기준 값으로 정규화한다 — uart_comm.c의 _rebuild_send_save_value1()이
     * settings_get_int()/_F2I()로 그대로 읽어 MCU에 보내므로, F 모드에서 고른 롤러 문자열
     * (예: "2.7", F-array 값)을 그대로 저장하면 하드웨어로 변환되지 않은 값이 전달된다. */
    char store_buf[32];
    strncpy(store_buf, buf, sizeof(store_buf) - 1);
    store_buf[sizeof(store_buf) - 1] = '\0';
    int is_f = (strcmp(settings_get_str("Degree"), "\xc2\xb0""F") == 0);
    if (is_f) {
        _picker_opt_t c_opt = s_picker_opts[s_page][s_edit_field];
        _picker_opt_t f_opt = s_picker_opts_f[s_page][s_edit_field];
        if (c_opt.arr && f_opt.arr) {
            for (int i = 0; i < f_opt.cnt && i < c_opt.cnt; i++) {
                if (strcmp(f_opt.arr[i], buf) == 0) {
                    strncpy(store_buf, c_opt.arr[i], sizeof(store_buf) - 1);
                    store_buf[sizeof(store_buf) - 1] = '\0';
                    break;
                }
            }
        }
    }

    lv_obj_t *ims[4], *bts[4], *txts[4];
    _get_row_objs(bk_ui, ims, bts, txts);
    if (s_page == 1 && s_edit_field == 2) {   /* Fan: ON/OFF → 중국어는 开/关 폰트 */
        ui_lang_set_onoff_display(txts[s_edit_field], buf);
    } else {
        /* txts[0..3]는 페이지 0/1 공유 위젯 — Fan에서 남긴 중국어 전용 폰트(숫자
         * 글리프 없음)가 남아있으면 숫자 필드가 네모(tofu) 박스로 보인다. */
        lv_obj_set_style_text_font(txts[s_edit_field], &lv_font_scdream_regular_32, 0);
        lv_label_set_text(txts[s_edit_field], buf);
    }
    settings_set_str(s_keys[s_page][s_edit_field], store_buf);
    settings_save_dirty();

    state->change_setting = true;
    state->start_run = false;
    uart_comm_trigger_change_setting();
}

void detailsettingtemp_loaded_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    ui_title_anim(bk_ui->detailsettingtemp_title);
}

void detailsettingtemp_unload_start_event_cb(lv_event_t *e)
{
    (void)e;
}

void detailsettingtemp_unloaded_event_cb(lv_event_t *e)
{
    (void)e;
}

void detailsettingtemp_load_start_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    uint32_t t0 = lv_tick_get();
    bk_printf(TAG "[PERF] detailtemp load_event start\n");

    _build_img_arrays_if_changed();
    bk_printf(TAG "[PERF]   _build_img_arrays +%lu ms\n", (unsigned long)lv_tick_elaps(t0));

    ui_lang_apply_detailsettingtemp(bk_ui);
    bk_printf(TAG "[PERF]   ui_lang_apply +%lu ms\n", (unsigned long)lv_tick_elaps(t0));
    s_page = 0;
    s_edit_field = -1;
    /* s_roller_pg_field: 언어/℃↔℉ 변경 시 _build_img_arrays_if_changed()에서만 초기화.
     * 스크린 재진입마다 초기화하면 첫 탭 시 lv_roller_set_options 가 불필요하게 재호출됨. */

    _refresh(bk_ui);
    _update_txt_colors(bk_ui, -1);   /* 재진입 시 이전 선택 색상(검정) 잔류 방지 */
    bk_printf(TAG "[PERF]   _refresh(page0) +%lu ms\n", (unsigned long)lv_tick_elaps(t0));

    lv_obj_add_flag(bk_ui->detailsettingtemp_settemp_setn1, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(bk_ui->detailsettingtemp_pickerbox,     LV_OBJ_FLAG_HIDDEN);
    bk_printf(TAG "[PERF] detailtemp load_event end total=%lu ms\n", (unsigned long)lv_tick_elaps(t0));
}
