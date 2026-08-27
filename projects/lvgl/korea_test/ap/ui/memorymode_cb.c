#include "lvgl.h"
#include "beken_ui.h"
#include "custom_func.h"
#include "event_runtime.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "beken_ui.h"
#include "ui_animations.h"
#include "ui_lang.h"
#include "device_state.h"
#include "settings.h"
#include "hardware_hal.h"

#include "preRenderer.h"
#define TAG "[memorymode_cb.c] "
#define bk_printf(fmt, ...) do {if(0) printf(fmt, ##__VA_ARGS__); } while(0) // disable printf

extern bk_lv_ui_t bk_lv_tool_ui;
extern double degree_basic_change(double ch);

static uint32_t last_click_time = 0;
static uint32_t s_check_t[4]   = {0};  /* per-row debounce for check buttons */
#define s_page     (g_device_state.memory_slot_page)
#define s_checking (g_device_state.memory_slot_checking)

void memorymode_backbt_event_cb(lv_event_t *e);
void memorymode_memory_check1bt_event_cb(lv_event_t *e);
void memorymode_memory_check2bt_event_cb(lv_event_t *e);
void memorymode_memory_check3bt_event_cb(lv_event_t *e);
void memorymode_memory_check4bt_event_cb(lv_event_t *e);
void memorymode_memoryleftbt_event_cb(lv_event_t *e);
void memorymode_memory1bt_event_cb(lv_event_t *e);
void memorymode_memory2bt_event_cb(lv_event_t *e);
void memorymode_memory3bt_event_cb(lv_event_t *e);
void memorymode_memoryrightbt_event_cb(lv_event_t *e);
void memorymode_okbt_event_cb(lv_event_t *e);
void memorymode_deletebt_event_cb(lv_event_t *e);
void memorymode_load_start_event_cb(lv_event_t *e);
void memorymode_loaded_event_cb(lv_event_t *e);
void memorymode_unload_start_event_cb(lv_event_t *e);
void memorymode_unloaded_event_cb(lv_event_t *e);

int memorymode_get_selected_slot(void)
{
    return s_page * 4 + (s_checking - 1);
}

static int s_slot_to_delete = -1;

int memorymode_get_pending_delete_slot(void)
{
    return s_slot_to_delete;
}

/* Helper struct: all label/image pointers for one display row */
typedef struct {
    lv_obj_t *number;
    lv_obj_t *day_period;
    lv_obj_t *freeze_temp;
    lv_obj_t *defrost_temp;
    lv_obj_t *defrost_hour;
    lv_obj_t *defrost_min;
    lv_obj_t *ferm1_temp;
    lv_obj_t *ferm1_humidity;
    lv_obj_t *ferm1_hour;
    lv_obj_t *ferm1_min;
    lv_obj_t *ferm2_temp;
    lv_obj_t *ferm2_humidity;
    lv_obj_t *ferm2_hour;
    lv_obj_t *ferm2_min;
    lv_obj_t *complete_hour;
    lv_obj_t *complete_min;
    lv_obj_t *check_im;
} _mm_row_t;

static void _get_row_widgets(bk_lv_ui_t *bk_ui, int row, _mm_row_t *r)
{
    switch (row) {
    default:
    case 0:
        r->number         = bk_ui->memorymode_memory_number0;
        r->day_period     = bk_ui->memorymode_memory_Day_Period0;
        r->freeze_temp    = bk_ui->memorymode_memory_freeze_temp0;
        r->defrost_temp   = bk_ui->memorymode_memory_defrost_temp0;
        r->defrost_hour   = bk_ui->memorymode_memory_defrost_hour0;
        r->defrost_min    = bk_ui->memorymode_memory_defrost_min0;
        r->ferm1_temp     = bk_ui->memorymode_memory_fermentation1_temp0;
        r->ferm1_humidity = bk_ui->memorymode_memory_fermentation1_humidity0;
        r->ferm1_hour     = bk_ui->memorymode_memory_fermentation1_hour0;
        r->ferm1_min      = bk_ui->memorymode_memory_fermentation1_min0;
        r->ferm2_temp     = bk_ui->memorymode_memory_fermentation2_temp0;
        r->ferm2_humidity = bk_ui->memorymode_memory_fermentation2_humidity0;
        r->ferm2_hour     = bk_ui->memorymode_memory_fermentation2_hour0;
        r->ferm2_min      = bk_ui->memorymode_memory_fermentation2_min0;
        r->complete_hour  = bk_ui->memorymode_memory_completehour0;
        r->complete_min   = bk_ui->memorymode_memory_completemin0;
        r->check_im       = bk_ui->memorymode_memory_check1im;
        break;
    case 1:
        r->number         = bk_ui->memorymode_memory_number1;
        r->day_period     = bk_ui->memorymode_memory_Day_Period1;
        r->freeze_temp    = bk_ui->memorymode_memory_freeze_temp1;
        r->defrost_temp   = bk_ui->memorymode_memory_defrost_temp1;
        r->defrost_hour   = bk_ui->memorymode_memory_defrost_hour1;
        r->defrost_min    = bk_ui->memorymode_memory_defrost_min1;
        r->ferm1_temp     = bk_ui->memorymode_memory_fermentation1_temp1;
        r->ferm1_humidity = bk_ui->memorymode_memory_fermentation1_humidity1;
        r->ferm1_hour     = bk_ui->memorymode_memory_fermentation1_hour1;
        r->ferm1_min      = bk_ui->memorymode_memory_fermentation1_min1;
        r->ferm2_temp     = bk_ui->memorymode_memory_fermentation2_temp1;
        r->ferm2_humidity = bk_ui->memorymode_memory_fermentation2_humidity1;
        r->ferm2_hour     = bk_ui->memorymode_memory_fermentation2_hour1;
        r->ferm2_min      = bk_ui->memorymode_memory_fermentation2_min1;
        r->complete_hour  = bk_ui->memorymode_memory_completehour1;
        r->complete_min   = bk_ui->memorymode_memory_completemin1;
        r->check_im       = bk_ui->memorymode_memory_check2im;
        break;
    case 2:
        r->number         = bk_ui->memorymode_memory_number2;
        r->day_period     = bk_ui->memorymode_memory_Day_Period2;
        r->freeze_temp    = bk_ui->memorymode_memory_freeze_temp2;
        r->defrost_temp   = bk_ui->memorymode_memory_defrost_temp2;
        r->defrost_hour   = bk_ui->memorymode_memory_defrost_hour2;
        r->defrost_min    = bk_ui->memorymode_memory_defrost_min2;
        r->ferm1_temp     = bk_ui->memorymode_memory_fermentation1_temp2;
        r->ferm1_humidity = bk_ui->memorymode_memory_fermentation1_humidity2;
        r->ferm1_hour     = bk_ui->memorymode_memory_fermentation1_hour2;
        r->ferm1_min      = bk_ui->memorymode_memory_fermentation1_min2;
        r->ferm2_temp     = bk_ui->memorymode_memory_fermentation2_temp2;
        r->ferm2_humidity = bk_ui->memorymode_memory_fermentation2_humidity2;
        r->ferm2_hour     = bk_ui->memorymode_memory_fermentation2_hour2;
        r->ferm2_min      = bk_ui->memorymode_memory_fermentation2_min2;
        r->complete_hour  = bk_ui->memorymode_memory_completehour2;
        r->complete_min   = bk_ui->memorymode_memory_completemin2;
        r->check_im       = bk_ui->memorymode_memory_check3im;
        break;
    case 3:
        r->number         = bk_ui->memorymode_memory_number3;
        r->day_period     = bk_ui->memorymode_memory_Day_Period3;
        r->freeze_temp    = bk_ui->memorymode_memory_freeze_temp3;
        r->defrost_temp   = bk_ui->memorymode_memory_defrost_temp3;
        r->defrost_hour   = bk_ui->memorymode_memory_defrost_hour3;
        r->defrost_min    = bk_ui->memorymode_memory_defrost_min3;
        r->ferm1_temp     = bk_ui->memorymode_memory_fermentation1_temp3;
        r->ferm1_humidity = bk_ui->memorymode_memory_fermentation1_humidity3;
        r->ferm1_hour     = bk_ui->memorymode_memory_fermentation1_hour3;
        r->ferm1_min      = bk_ui->memorymode_memory_fermentation1_min3;
        r->ferm2_temp     = bk_ui->memorymode_memory_fermentation2_temp3;
        r->ferm2_humidity = bk_ui->memorymode_memory_fermentation2_humidity3;
        r->ferm2_hour     = bk_ui->memorymode_memory_fermentation2_hour3;
        r->ferm2_min      = bk_ui->memorymode_memory_fermentation2_min3;
        r->complete_hour  = bk_ui->memorymode_memory_completehour3;
        r->complete_min   = bk_ui->memorymode_memory_completemin3;
        r->check_im       = bk_ui->memorymode_memory_check4im;
        break;
    }
}

/* memory_auto_save[] index → Memory* settings key suffix */
const char * const g_mem_keys[15] = {
    "MemoryDayPeriod", "MemoryFreezeTemp", "MemoryDefrostTemp",
    "MemoryDefrostHour", "MemoryDefrostMin",
    "MemoryFermentation1Temp", "MemoryFermentation1Humidity",
    "MemoryFermentation1Hour", "MemoryFermentation1Min",
    "MemoryFermentation2Temp", "MemoryFermentation2Humidity",
    "MemoryFermentation2Hour", "MemoryFermentation2Min",
    "MemoryCompleteHour", "MemoryCompleteMin",
};

/* Corresponding CurrentSave* keys for load-back path */
const char * const g_mem_cur_keys[15] = {
    "saveDayPeriod",
    "CurrentSaveFreezeTemp",
    "CurrentSaveDefreezeTemp",   "CurrentSaveDefreezeTimeHour", "CurrentSaveDefreezeTimeMin",
    "CurrentSaveFermentation1Temp", "CurrentSaveFermentation1Humidity",
    "CurrentSaveFermentation1TimeHour", "CurrentSaveFermentation1TimeMin",
    "CurrentSaveFermentation2Temp", "CurrentSaveFermentation2Humidity",
    "CurrentSaveFermentation2TimeHour", "CurrentSaveFermentation2TimeMin",
    "CurrentCompleteHour", "CurrentCompleteMin",
};

static bool _slot_is_used(int slot)
{
    char k[40];
    snprintf(k, sizeof(k), "%s%d", g_mem_keys[0], slot); /* MemoryDayPeriod{slot} */
    const char *v = settings_get_str(k);
    return (v && v[0] != '\0');
}

static void _refresh_display(bk_lv_ui_t *bk_ui)
{
    int is_f = (strcmp(settings_get_str("Degree"), "\xc2\xb0""F") == 0);

    for (int row = 0; row < 4; row++) {
        int slot = s_page * 4 + row;
        _mm_row_t r;
        _get_row_widgets(bk_ui, row, &r);

        char k[40], num[8];
        snprintf(num, sizeof(num), "%d", slot + 1);
        lv_label_set_text(r.number, num);

        /* Display fields in same order as g_mem_keys[] */
        lv_obj_t *labels[14] = {
            r.day_period, r.freeze_temp, r.defrost_temp,
            r.defrost_hour, r.defrost_min,
            r.ferm1_temp, r.ferm1_humidity, r.ferm1_hour, r.ferm1_min,
            r.ferm2_temp, r.ferm2_humidity, r.ferm2_hour, r.ferm2_min,
            r.complete_hour,   /* [13] */
        };
        for (int fi = 1; fi < 14; fi++) {  /* fi=0 (DayPeriod) handled below — avoids duplicate read */
            snprintf(k, sizeof(k), "%s%d", g_mem_keys[fi], slot);
            const char *fv = settings_get_str(k);
            if (!fv || !fv[0]) { lv_label_set_text(labels[fi], ""); continue; }

            /* F 모드: 온도 항목만 변환 (저장값은 항상 C) */
            if (is_f && (fi == 1 || fi == 2)) {
                /* freeze/defrost: C→F 절대온도 변환 */
                char tmp[16];
                snprintf(tmp, sizeof(tmp), "%.0f", atof(fv) * 1.8 + 32.0);
                lv_label_set_text(labels[fi], tmp);
            } else if (is_f && (fi == 5 || fi == 9)) {
                /* ferm1/ferm2: C→F 절대온도 변환 (freeze/defrost와 동일) */
                char tmp[16];
                snprintf(tmp, sizeof(tmp), "%.0f", atof(fv) * 1.8 + 32.0);
                lv_label_set_text(labels[fi], tmp);
            } else {
                lv_label_set_text(labels[fi], fv);
            }
        }
        snprintf(k, sizeof(k), "%s%d", g_mem_keys[14], slot);
        const char *cmv = settings_get_str(k);
        lv_label_set_text(r.complete_min, cmv ? cmv : "");

        /* DayPeriod: stored value is already the actual day count (computed as diff/86400 + 1) */
        {
            snprintf(k, sizeof(k), "%s%d", g_mem_keys[0], slot);
            const char *dp = settings_get_str(k);
            lv_label_set_text(r.day_period, (dp && dp[0]) ? dp : "");
        }


        /* check icon — 항상 표시, 선택된 행만 on 이미지, 나머지는 off 이미지 */
        bool selected = (s_checking != 0 && s_checking == row + 1);
        _img_ensure_src(r.check_im);
        lv_obj_clear_flag(r.check_im, LV_OBJ_FLAG_HIDDEN);
        _img_set_src_timed(r.check_im,
            selected ? "/images/memory_check_on.png" : "/images/memory_check_off.png");
    }

    /* Page indicator: switch on/off images to reflect current s_page */
    _img_set_src_timed(bk_ui->memorymode_memory1im,
                     s_page == 0 ? "/images/memroy1_on.png" : "/images/memroy1_off.png");
    _img_set_src_timed(bk_ui->memorymode_memory2im,
                     s_page == 1 ? "/images/memroy2_on.png" : "/images/memroy2_off.png");
    _img_set_src_timed(bk_ui->memorymode_memory3im,
                     s_page == 2 ? "/images/memroy3_on.png" : "/images/memroy3_off.png");
}

void memory_save_to_slot(int slot)
{
    char k[40];
    bk_printf(TAG "[MEM] SAVE → slot %d\n", slot);
    for (int i = 0; i < 15; i++) {
        snprintf(k, sizeof(k), "%s%d", g_mem_keys[i], slot);
        settings_set_str(k, g_device_state.memory_auto_save[i]);
        bk_printf(TAG "[MEM]   [%d] %s = \"%s\"\n", i, k, g_device_state.memory_auto_save[i]);
    }
}

void memory_load_from_slot(int slot)
{
    char k[40];
    int is_f = (strcmp(settings_get_str("Degree"), "\xc2\xb0""F") == 0);
    bk_printf(TAG "[MEM] LOAD ← slot %d (is_f=%d)\n", slot, is_f);
    for (int i = 0; i < 15; i++) {
        snprintf(k, sizeof(k), "%s%d", g_mem_keys[i], slot);
        const char *val = settings_get_str(k);
        strncpy(g_device_state.memory_auto_load[i], val, 15);
        g_device_state.memory_auto_load[i][15] = '\0';
        /* °F 모드: CurrentSave*는 표시 단위(F)로 저장해야 함 — C→F 절대 변환 */
        if (is_f && (i == 1 || i == 2 || i == 5 || i == 9)) {
            char fbuf[16];
            snprintf(fbuf, sizeof(fbuf), "%.0f", degree_basic_change(atof(val)));
            settings_set_str(g_mem_cur_keys[i], fbuf);
            bk_printf(TAG "[MEM]   [%d] %s = \"%s\" -> %s (F: %s)\n", i, k, val, g_mem_cur_keys[i], fbuf);
        } else {
            settings_set_str(g_mem_cur_keys[i], val);
            bk_printf(TAG "[MEM]   [%d] %s = \"%s\" -> %s\n", i, k, val, g_mem_cur_keys[i]);
        }
    }
}

/* ----------------------------------------------------------------------- */

void memorymode_backbt_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    device_state_t *state = &g_device_state;
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    if (lv_tick_elaps(last_click_time) < 250) return;
    last_click_time = lv_tick_get();
    hal_buzzer_beep();

    int mode = state->memory_mode_check;
    bk_printf(TAG "[MEM] BACK btn: mode=%d -> automode\n", mode);
    state->memory_mode_check = MEMORY_MODE_NONE;
    if (mode == MEMORY_MODE_SAVE || mode == MEMORY_MODE_LOAD) 
    {
#if UI_PRENDERING_ENABLE
        ui_page_change(PAGE_AUTOMODE);
#else
        if (bk_ui->automode == NULL || !lv_obj_is_valid(bk_ui->automode))
            init_page_automode(bk_ui);
        lv_scr_load(bk_ui->automode);
#endif /* UI_PRENDERING_ENABLE */
    } 
    else 
    {
#if UI_PRENDERING_ENABLE
        ui_page_change(PAGE_MAIN);
#else
        init_page_main(bk_ui);
        lv_scr_load(bk_ui->main);
#endif /* UI_PRENDERING_ENABLE */
    }
    /* keep-alive: memorymode screen retained for fast re-entry */
}

void memorymode_memory_check1bt_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    if (lv_tick_elaps(s_check_t[0]) < 150) return;
    s_check_t[0] = lv_tick_get();
    hal_buzzer_beep();
    s_checking = 1;
    bk_printf(TAG "[MEM] check1 -> slot=%d\n", memorymode_get_selected_slot());
    _refresh_display(bk_ui);
}

void memorymode_memory_check2bt_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    if (lv_tick_elaps(s_check_t[1]) < 150) return;
    s_check_t[1] = lv_tick_get();
    hal_buzzer_beep();
    s_checking = 2;
    bk_printf(TAG "[MEM] check2 -> slot=%d\n", memorymode_get_selected_slot());
    _refresh_display(bk_ui);
}

void memorymode_memory_check3bt_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    if (lv_tick_elaps(s_check_t[2]) < 150) return;
    s_check_t[2] = lv_tick_get();
    hal_buzzer_beep();
    s_checking = 3;
    bk_printf(TAG "[MEM] check3 -> slot=%d\n", memorymode_get_selected_slot());
    _refresh_display(bk_ui);
}

void memorymode_memory_check4bt_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    if (lv_tick_elaps(s_check_t[3]) < 150) return;
    s_check_t[3] = lv_tick_get();
    hal_buzzer_beep();
    s_checking = 4;
    bk_printf(TAG "[MEM] check4 -> slot=%d\n", memorymode_get_selected_slot());
    _refresh_display(bk_ui);
}

void memorymode_memoryleftbt_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    if (lv_tick_elaps(last_click_time) < 250) return;
    last_click_time = lv_tick_get();
    hal_buzzer_beep();
    s_page = (s_page > 0) ? s_page - 1 : 2;  /* 0->2 순환 */
    s_checking = 0;
    bk_printf(TAG "[MEM] page LEFT -> page=%d slot=%d\n", s_page, memorymode_get_selected_slot());
    _refresh_display(bk_ui);
}

void memorymode_memory1bt_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    if (lv_tick_elaps(last_click_time) < 250) return;
    last_click_time = lv_tick_get();
    hal_buzzer_beep();
    s_page = 0; s_checking = 0;
    bk_printf(TAG "[MEM] page 1 -> page=%d slot=%d\n", s_page, memorymode_get_selected_slot());
    _refresh_display(bk_ui);
}

void memorymode_memory2bt_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    if (lv_tick_elaps(last_click_time) < 250) return;
    last_click_time = lv_tick_get();
    hal_buzzer_beep();
    s_page = 1; s_checking = 0;
    bk_printf(TAG "[MEM] page 2 -> page=%d slot=%d\n", s_page, memorymode_get_selected_slot());
    _refresh_display(bk_ui);
}

void memorymode_memory3bt_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    if (lv_tick_elaps(last_click_time) < 250) return;
    last_click_time = lv_tick_get();
    hal_buzzer_beep();
    s_page = 2; s_checking = 0;
    bk_printf(TAG "[MEM] page 3 -> page=%d slot=%d\n", s_page, memorymode_get_selected_slot());
    _refresh_display(bk_ui);
}

void memorymode_memoryrightbt_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    if (lv_tick_elaps(last_click_time) < 250) return;
    last_click_time = lv_tick_get();
    hal_buzzer_beep();
    s_page = (s_page < 2) ? s_page + 1 : 0;  /* 2->0 순환 */
    s_checking = 0;
    bk_printf(TAG "[MEM] page RIGHT -> page=%d slot=%d\n", s_page, memorymode_get_selected_slot());
    _refresh_display(bk_ui);
}

void memorymode_okbt_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    device_state_t *state = &g_device_state;
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    if (lv_tick_elaps(last_click_time) < 250) return;
    last_click_time = lv_tick_get();
    hal_buzzer_beep();

    if (s_checking == 0) {
        bk_printf(TAG "[MEM] OK: no slot selected (mode=%d)\n", state->memory_mode_check);
        return;
    }
    int slot = memorymode_get_selected_slot();
    bk_printf(TAG "[MEM] OK btn: mode=%d slot=%d (page=%d checking=%d)\n",
           state->memory_mode_check, slot, s_page, s_checking);
    if (state->memory_mode_check == MEMORY_MODE_SAVE) {
        if (!settings_is_loaded()) {
            bk_printf(TAG "[MEM] SAVE blocked — settings not loaded yet\n");
            return;
        }
        memory_save_to_slot(slot);
        settings_save_all_sync();
        bk_printf(TAG "[MEM] save complete, display refreshed\n");
        /* 저장 후 다음 빈 칸으로 자동 선택 이동하던 로직 제거 — ON 체크박스는
         * 사용자가 명시적으로 항목을 탭했을 때만 나타나야 함 */
        s_checking = 0;
        _refresh_display(bk_ui);
    } else if (state->memory_mode_check == MEMORY_MODE_LOAD) {
        if (!_slot_is_used(slot)) {
            bk_printf(TAG "[MEM] LOAD: slot %d empty — caution popup\n", slot);
            init_page_popupcaution(bk_ui);
            return;
        }
        memory_load_from_slot(slot);
        settings_save_dirty();
        bk_printf(TAG "[MEM] load complete, navigate to automode\n");
        /* memory_mode_check는 automode_load_event_cb 진입 후 클리어 — 메모리 불러오기 판별용 */
#if UI_PRENDERING_ENABLE
        ui_page_change(PAGE_AUTOMODE);
#else
        if (bk_ui->automode == NULL || !lv_obj_is_valid(bk_ui->automode))
            init_page_automode(bk_ui);
        lv_scr_load(bk_ui->automode);
#endif /* UI_PRENDERING_ENABLE */
    } else {
        /* Accessed from main page (MEMORY_MODE_NONE): load selected slot → automode */
        if (!_slot_is_used(slot)) {
            bk_printf(TAG "[MEM] OK: NONE mode, slot %d is empty — caution popup\n", slot);
            init_page_popupcaution(bk_ui);
            return;
        }
        memory_load_from_slot(slot);
        settings_save_dirty();
        bk_printf(TAG "[MEM] NONE mode load complete (slot %d), navigate to automode\n", slot);
        state->memory_mode_check = MEMORY_MODE_LOAD;  /* automode_load_event_cb에서 클리어 */
#if UI_PRENDERING_ENABLE
        ui_page_change(PAGE_AUTOMODE);
#else
        if (bk_ui->automode == NULL || !lv_obj_is_valid(bk_ui->automode))
            init_page_automode(bk_ui);
        lv_scr_load(bk_ui->automode);
#endif /* UI_PRENDERING_ENABLE */
    }
}

void memorymode_deletebt_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    if (lv_tick_elaps(last_click_time) < 250) return;
    last_click_time = lv_tick_get();
    hal_buzzer_beep();

    if (s_checking < 1 || s_checking > 4) return;   /* 선택 없으면 무시 */
    s_slot_to_delete = s_page * 4 + (s_checking - 1);
    bk_printf(TAG "[DEL] pending slot=%d (page=%d checking=%d)\n",
           s_slot_to_delete, s_page, s_checking);
    init_page_popupdelete(bk_ui);   /* memorymode 위에 오버레이 — 화면 전환 없음 */
}

extern void destroy_page_popupdelete(bk_lv_ui_t *bk_ui);

void memorymode_refresh_display(bk_lv_ui_t *bk_ui)
{
    _refresh_display(bk_ui);
}

/* 슬롯 삭제 후 호출 — 방금 지운 행이 checked(on) 상태로 남아있지 않도록
 * 선택 해제. 호출자가 memorymode_refresh_display()보다 먼저 호출해야
 * 그 갱신에 반영됨. */
void memorymode_clear_checking(void)
{
    s_checking = 0;
}

void memorymode_loaded_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;

    ui_title_anim(bk_ui->memorymode_title);
    /* Ensure deferred static images — 0ms cache hit if prewarm completed */
    _img_ensure_src(bk_ui->memorymode_title);
    _img_ensure_src(bk_ui->memorymode_imageview4);   /* memory_title_line */
    _img_ensure_src(bk_ui->memorymode_memorybox0);
    _img_ensure_src(bk_ui->memorymode_imageview81);  /* memory_left */
    _img_ensure_src(bk_ui->memorymode_imageview89);  /* memory_right */
    _img_ensure_src(bk_ui->memorymode_imageview92);  /* ok */
    _img_ensure_src(bk_ui->memorymode_deleteim);
    /* 자동설정(불러오기/저장)에서 진입한 메모리모드에서는 삭제 버튼 숨김 —
     * 메모리모드 메뉴로 직접 들어왔을 때(NONE)만 삭제 가능 */
    if (g_device_state.memory_mode_check == MEMORY_MODE_NONE) {
        lv_obj_clear_flag(bk_ui->memorymode_deleteim, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(bk_ui->memorymode_deletebt, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(bk_ui->memorymode_deleteim, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(bk_ui->memorymode_deletebt, LV_OBJ_FLAG_HIDDEN);
    }
    /* memorybox1-3: init에서 deferred 없이 생성 — 첫 로드 시 설정 (cache hit) */
    if (!lv_image_get_src(bk_ui->memorymode_memorybox1)) {
        _img_set_src_timed(bk_ui->memorymode_memorybox1, "/images/memory_box.png");
        _img_set_src_timed(bk_ui->memorymode_memorybox2, "/images/memory_box.png");
        _img_set_src_timed(bk_ui->memorymode_memorybox3, "/images/memory_box.png");
    }
    _refresh_display(bk_ui);
    ui_lang_apply_memorymode(bk_ui);
}

void memorymode_unload_start_event_cb(lv_event_t *e)
{
    (void)e;
}

void memorymode_unloaded_event_cb(lv_event_t *e)
{
    (void)e;
}

void memorymode_load_start_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;

    /* 잔재 오버레이 제거 (오버레이는 memorymode의 자식 — 화면 전환 없음) */
    destroy_page_popupdelete(bk_ui);

    /* 진입 시 항상 아무 항목도 선택되지 않은 상태로 표시 (모드 무관).
     * s_checking은 device_state에 저장되어 화면을 나갔다 다시 들어와도
     * 이전 선택값(1~4)이 유효 범위라 그대로 남아있었음 → 무조건 0으로 리셋. */
    s_checking = 0;
    bk_printf(TAG "[MEM] screen load: mode=%d page=%d checking=%d\n",
           g_device_state.memory_mode_check, s_page, s_checking);
}
