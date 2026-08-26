#include "lvgl.h"
#include "beken_ui.h"
#include "custom_func.h"
#include "event_runtime.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "beken_ui.h"
#include "ui_animations.h"
#include "ui_lang.h"
#include "settings.h"
#include "hardware_hal.h"

extern bk_lv_ui_t bk_lv_tool_ui;

static uint32_t last_click_time = 0;

void settingmoderecord_deletebt_event_cb(lv_event_t *e);
void settingmoderecord_backbt_event_cb(lv_event_t *e);
void settingmoderecord_load_start_event_cb(lv_event_t *e);
void settingmoderecord_loaded_event_cb(lv_event_t *e);
void settingmoderecord_unload_start_event_cb(lv_event_t *e);
void settingmoderecord_unloaded_event_cb(lv_event_t *e);

static void _apply_record_labels(bk_lv_ui_t *bk_ui);

void settingmoderecord_deletebt_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    if (lv_tick_elaps(last_click_time) < 250) return;
    last_click_time = lv_tick_get();
    hal_buzzer_beep();

    settings_set_str("RecordFreezeTemp0", "");
    settings_set_str("RecordFreezeTimeHour0", "");
    settings_set_str("RecordFreezeTimeMin0", "");
    settings_set_str("RecordDeFreezeTemp0", "");
    settings_set_str("RecordDeFreezeTimeHour0", "");
    settings_set_str("RecordDeFreezeTimeMin0", "");
    settings_set_str("RecordFermentation1Temp0", "");
    settings_set_str("RecordFermentation1Humidity0", "");
    settings_set_str("RecordFermentation1TimeHour0", "");
    settings_set_str("RecordFermentation1TimeMin0", "");
    settings_set_str("RecordFermentation2Temp0", "");
    settings_set_str("RecordFermentation2Humidity0", "");
    settings_set_str("RecordFermentation2TimeHour0", "");
    settings_set_str("RecordFermentation2TimeMin0", "");
    settings_set_str("RecordStartMonth0", "");
    settings_set_str("RecordStartDay0", "");
    settings_set_str("RecordStartHour0", "");
    settings_set_str("RecordStartMin0", "");
    settings_set_str("RecordEndMonth0", "");
    settings_set_str("RecordEndDay0", "");
    settings_set_str("RecordEndHour0", "");
    settings_set_str("RecordEndMin0", "");
    settings_set_str("RecordFreezeTemp1", "");
    settings_set_str("RecordFreezeTimeHour1", "");
    settings_set_str("RecordFreezeTimeMin1", "");
    settings_set_str("RecordDeFreezeTemp1", "");
    settings_set_str("RecordDeFreezeTimeHour1", "");
    settings_set_str("RecordDeFreezeTimeMin1", "");
    settings_set_str("RecordFermentation1Temp1", "");
    settings_set_str("RecordFermentation1Humidity1", "");
    settings_set_str("RecordFermentation1TimeHour1", "");
    settings_set_str("RecordFermentation1TimeMin1", "");
    settings_set_str("RecordFermentation2Temp1", "");
    settings_set_str("RecordFermentation2Humidity1", "");
    settings_set_str("RecordFermentation2TimeHour1", "");
    settings_set_str("RecordFermentation2TimeMin1", "");
    settings_set_str("RecordStartMonth1", "");
    settings_set_str("RecordStartDay1", "");
    settings_set_str("RecordStartHour1", "");
    settings_set_str("RecordStartMin1", "");
    settings_set_str("RecordEndMonth1", "");
    settings_set_str("RecordEndDay1", "");
    settings_set_str("RecordEndHour1", "");
    settings_set_str("RecordEndMin1", "");
    settings_set_str("RecordFreezeTemp2", "");
    settings_set_str("RecordFreezeTimeHour2", "");
    settings_set_str("RecordFreezeTimeMin2", "");
    settings_set_str("RecordDeFreezeTemp2", "");
    settings_set_str("RecordDeFreezeTimeHour2", "");
    settings_set_str("RecordDeFreezeTimeMin2", "");
    settings_set_str("RecordFermentation1Temp2", "");
    settings_set_str("RecordFermentation1Humidity2", "");
    settings_set_str("RecordFermentation1TimeHour2", "");
    settings_set_str("RecordFermentation1TimeMin2", "");
    settings_set_str("RecordFermentation2Temp2", "");
    settings_set_str("RecordFermentation2Humidity2", "");
    settings_set_str("RecordFermentation2TimeHour2", "");
    settings_set_str("RecordFermentation2TimeMin2", "");
    settings_set_str("RecordStartMonth2", "");
    settings_set_str("RecordStartDay2", "");
    settings_set_str("RecordStartHour2", "");
    settings_set_str("RecordStartMin2", "");
    settings_set_str("RecordEndMonth2", "");
    settings_set_str("RecordEndDay2", "");
    settings_set_str("RecordEndHour2", "");
    settings_set_str("RecordEndMin2", "");
    settings_set_str("RecordFreezeTemp3", "");
    settings_set_str("RecordFreezeTimeHour3", "");
    settings_set_str("RecordFreezeTimeMin3", "");
    settings_set_str("RecordDeFreezeTemp3", "");
    settings_set_str("RecordDeFreezeTimeHour3", "");
    settings_set_str("RecordDeFreezeTimeMin3", "");
    settings_set_str("RecordFermentation1Temp3", "");
    settings_set_str("RecordFermentation1Humidity3", "");
    settings_set_str("RecordFermentation1TimeHour3", "");
    settings_set_str("RecordFermentation1TimeMin3", "");
    settings_set_str("RecordFermentation2Temp3", "");
    settings_set_str("RecordFermentation2Humidity3", "");
    settings_set_str("RecordFermentation2TimeHour3", "");
    settings_set_str("RecordFermentation2TimeMin3", "");
    settings_set_str("RecordStartMonth3", "");
    settings_set_str("RecordStartDay3", "");
    settings_set_str("RecordStartHour3", "");
    settings_set_str("RecordStartMin3", "");
    settings_set_str("RecordEndMonth3", "");
    settings_set_str("RecordEndDay3", "");
    settings_set_str("RecordEndHour3", "");
    settings_set_str("RecordEndMin3", "");
    settings_set_str("RecordFreezeTemp4", "");
    settings_set_str("RecordFreezeTimeHour4", "");
    settings_set_str("RecordFreezeTimeMin4", "");
    settings_set_str("RecordDeFreezeTemp4", "");
    settings_set_str("RecordDeFreezeTimeHour4", "");
    settings_set_str("RecordDeFreezeTimeMin4", "");
    settings_set_str("RecordFermentation1Temp4", "");
    settings_set_str("RecordFermentation1Humidity4", "");
    settings_set_str("RecordFermentation1TimeHour4", "");
    settings_set_str("RecordFermentation1TimeMin4", "");
    settings_set_str("RecordFermentation2Temp4", "");
    settings_set_str("RecordFermentation2Humidity4", "");
    settings_set_str("RecordFermentation2TimeHour4", "");
    settings_set_str("RecordFermentation2TimeMin4", "");
    settings_set_str("RecordStartMonth4", "");
    settings_set_str("RecordStartDay4", "");
    settings_set_str("RecordStartHour4", "");
    settings_set_str("RecordStartMin4", "");
    settings_set_str("RecordEndMonth4", "");
    settings_set_str("RecordEndDay4", "");
    settings_set_str("RecordEndHour4", "");
    settings_set_str("RecordEndMin4", "");
    settings_save_dirty();
    _apply_record_labels(bk_ui);  /* 화면 즉시 갱신 */
}

void settingmoderecord_backbt_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    if (lv_tick_elaps(last_click_time) < 250) return;
    last_click_time = lv_tick_get();
    hal_buzzer_beep();

    if (bk_ui->settingmode == NULL || !lv_obj_is_valid(bk_ui->settingmode))
        init_page_settingmode(bk_ui);
    lv_scr_load(bk_ui->settingmode);
}

static void _apply_record_labels(bk_lv_ui_t *bk_ui)
{
    int is_f = (strcmp(settings_get_str("Degree"), "\xc2\xb0""F") == 0);

    /* chartbox 헤더 이미지: C/F 모드에 따라 전환 */
    {
        const char *box_img = is_f
            ? "/images/setting_record_chart_box_f.png"
            : "/images/setting_record_chart_box.png";
        _img_set_src_timed(bk_ui->settingmoderecord_chartbox0, box_img);
        _img_set_src_timed(bk_ui->settingmoderecord_chartbox1, box_img);
        _img_set_src_timed(bk_ui->settingmoderecord_chartbox2, box_img);
        _img_set_src_timed(bk_ui->settingmoderecord_chartbox3, box_img);
        _img_set_src_timed(bk_ui->settingmoderecord_chartbox4, box_img);
    }

/* 비온도 필드: raw 값 그대로 표시 */
#define RL(obj, key) do { \
    const char *_v = settings_get_str(key); \
    lv_label_set_text(obj, (_v) ? _v : ""); \
} while(0)

/* 기록은 항상 섭씨(C)로 저장됨. F 모드이면 표시 시 C→F 변환(F = C×9/5 + 32).
 * 빈 슬롯("")은 변환 없이 그대로 표시. */
#define RT_BASIC(obj, key) do { \
    const char *_v = settings_get_str(key); \
    if (!_v || !_v[0]) { lv_label_set_text(obj, ""); break; } \
    if (is_f) { char _fb[16]; snprintf(_fb, sizeof(_fb), "%d", atoi(_v) * 9 / 5 + 32); \
                lv_label_set_text(obj, _fb); } \
    else       { lv_label_set_text(obj, _v); } \
} while(0)
#define RT_SCALE(obj, key) RT_BASIC(obj, key)

    /* ── 슬롯 0 (가장 최신) ── */
    {
        RT_BASIC(bk_ui->settingmoderecord_RecordFreezeTemp0,            "RecordFreezeTemp0");
        RL(bk_ui->settingmoderecord_RecordFreezeTimeHour0,              "RecordFreezeTimeHour0");
        RL(bk_ui->settingmoderecord_RecordFreezeTimeMin0,               "RecordFreezeTimeMin0");
        RT_BASIC(bk_ui->settingmoderecord_RecordDefreezeTemp0,          "RecordDeFreezeTemp0");
        RL(bk_ui->settingmoderecord_RecordDefreezeTimeHour0,            "RecordDeFreezeTimeHour0");
        RL(bk_ui->settingmoderecord_RecordDefreezeTimeMin0,             "RecordDeFreezeTimeMin0");
        RT_SCALE(bk_ui->settingmoderecord_RecordFermentation1Temp0,     "RecordFermentation1Temp0");
        RL(bk_ui->settingmoderecord_RecordFermentation1Humidity0,       "RecordFermentation1Humidity0");
        RL(bk_ui->settingmoderecord_RecordFermentation1TimeHour0,       "RecordFermentation1TimeHour0");
        RL(bk_ui->settingmoderecord_RecordFermentation1TimeMin0,        "RecordFermentation1TimeMin0");
        RT_SCALE(bk_ui->settingmoderecord_RecordFermentation2Temp0,     "RecordFermentation2Temp0");
        RL(bk_ui->settingmoderecord_RecordFermentation2Humidity0,       "RecordFermentation2Humidity0");
        RL(bk_ui->settingmoderecord_RecordFermentation2TimeHour0,       "RecordFermentation2TimeHour0");
        RL(bk_ui->settingmoderecord_RecordFermentation2TimeMin0,        "RecordFermentation2TimeMin0");
        RL(bk_ui->settingmoderecord_RecordStartMonth0,                  "RecordStartMonth0");
        RL(bk_ui->settingmoderecord_RecordStartDay0,                    "RecordStartDay0");
        RL(bk_ui->settingmoderecord_RecordStartHour0,                   "RecordStartHour0");
        RL(bk_ui->settingmoderecord_RecordStartMin0,                    "RecordStartMin0");
        RL(bk_ui->settingmoderecord_RecordEndMonth0,                    "RecordEndMonth0");
        RL(bk_ui->settingmoderecord_RecordEndDay0,                      "RecordEndDay0");
        RL(bk_ui->settingmoderecord_RecordEndHour0,                     "RecordEndHour0");
        RL(bk_ui->settingmoderecord_RecordEndMin0,                      "RecordEndMin0");
    }

    /* ── 슬롯 1 ── */
    {
        RT_BASIC(bk_ui->settingmoderecord_RecordFreezeTemp1,            "RecordFreezeTemp1");
        RL(bk_ui->settingmoderecord_RecordFreezeTimeHour1,              "RecordFreezeTimeHour1");
        RL(bk_ui->settingmoderecord_RecordFreezeTimeMin1,               "RecordFreezeTimeMin1");
        RT_BASIC(bk_ui->settingmoderecord_RecordDefreezeTemp1,          "RecordDeFreezeTemp1");
        RL(bk_ui->settingmoderecord_RecordDefreezeTimeHour1,            "RecordDeFreezeTimeHour1");
        RL(bk_ui->settingmoderecord_RecordDefreezeTimeMin1,             "RecordDeFreezeTimeMin1");
        RT_SCALE(bk_ui->settingmoderecord_RecordFermentation1Temp1,     "RecordFermentation1Temp1");
        RL(bk_ui->settingmoderecord_RecordFermentation1Humidity1,       "RecordFermentation1Humidity1");
        RL(bk_ui->settingmoderecord_RecordFermentation1TimeHour1,       "RecordFermentation1TimeHour1");
        RL(bk_ui->settingmoderecord_RecordFermentation1TimeMin1,        "RecordFermentation1TimeMin1");
        RT_SCALE(bk_ui->settingmoderecord_RecordFermentation2Temp1,     "RecordFermentation2Temp1");
        RL(bk_ui->settingmoderecord_RecordFermentation2Humidity1,       "RecordFermentation2Humidity1");
        RL(bk_ui->settingmoderecord_RecordFermentation2TimeHour1,       "RecordFermentation2TimeHour1");
        RL(bk_ui->settingmoderecord_RecordFermentation2TimeMin1,        "RecordFermentation2TimeMin1");
        RL(bk_ui->settingmoderecord_RecordStartMonth1,                  "RecordStartMonth1");
        RL(bk_ui->settingmoderecord_RecordStartDay1,                    "RecordStartDay1");
        RL(bk_ui->settingmoderecord_RecordStartHour1,                   "RecordStartHour1");
        RL(bk_ui->settingmoderecord_RecordStartMin1,                    "RecordStartMin1");
        RL(bk_ui->settingmoderecord_RecordEndMonth1,                    "RecordEndMonth1");
        RL(bk_ui->settingmoderecord_RecordEndDay1,                      "RecordEndDay1");
        RL(bk_ui->settingmoderecord_RecordEndHour1,                     "RecordEndHour1");
        RL(bk_ui->settingmoderecord_RecordEndMin1,                      "RecordEndMin1");
    }

    /* ── 슬롯 2 ── */
    {
        RT_BASIC(bk_ui->settingmoderecord_RecordFreezeTemp2,            "RecordFreezeTemp2");
        RL(bk_ui->settingmoderecord_RecordFreezeTimeHour2,              "RecordFreezeTimeHour2");
        RL(bk_ui->settingmoderecord_RecordFreezeTimeMin2,               "RecordFreezeTimeMin2");
        RT_BASIC(bk_ui->settingmoderecord_RecordDefreezeTemp2,          "RecordDeFreezeTemp2");
        RL(bk_ui->settingmoderecord_RecordDefreezeTimeHour2,            "RecordDeFreezeTimeHour2");
        RL(bk_ui->settingmoderecord_RecordDefreezeTimeMin2,             "RecordDeFreezeTimeMin2");
        RT_SCALE(bk_ui->settingmoderecord_RecordFermentation1Temp2,     "RecordFermentation1Temp2");
        RL(bk_ui->settingmoderecord_RecordFermentation1Humidity2,       "RecordFermentation1Humidity2");
        RL(bk_ui->settingmoderecord_RecordFermentation1TimeHour2,       "RecordFermentation1TimeHour2");
        RL(bk_ui->settingmoderecord_RecordFermentation1TimeMin2,        "RecordFermentation1TimeMin2");
        RT_SCALE(bk_ui->settingmoderecord_RecordFermentation2Temp2,     "RecordFermentation2Temp2");
        RL(bk_ui->settingmoderecord_RecordFermentation2Humidity2,       "RecordFermentation2Humidity2");
        RL(bk_ui->settingmoderecord_RecordFermentation2TimeHour2,       "RecordFermentation2TimeHour2");
        RL(bk_ui->settingmoderecord_RecordFermentation2TimeMin2,        "RecordFermentation2TimeMin2");
        RL(bk_ui->settingmoderecord_RecordStartMonth2,                  "RecordStartMonth2");
        RL(bk_ui->settingmoderecord_RecordStartDay2,                    "RecordStartDay2");
        RL(bk_ui->settingmoderecord_RecordStartHour2,                   "RecordStartHour2");
        RL(bk_ui->settingmoderecord_RecordStartMin2,                    "RecordStartMin2");
        RL(bk_ui->settingmoderecord_RecordEndMonth2,                    "RecordEndMonth2");
        RL(bk_ui->settingmoderecord_RecordEndDay2,                      "RecordEndDay2");
        RL(bk_ui->settingmoderecord_RecordEndHour2,                     "RecordEndHour2");
        RL(bk_ui->settingmoderecord_RecordEndMin2,                      "RecordEndMin2");
    }

    /* ── 슬롯 3 ── */
    {
        RT_BASIC(bk_ui->settingmoderecord_RecordFreezeTemp3,            "RecordFreezeTemp3");
        RL(bk_ui->settingmoderecord_RecordFreezeTimeHour3,              "RecordFreezeTimeHour3");
        RL(bk_ui->settingmoderecord_RecordFreezeTimeMin3,               "RecordFreezeTimeMin3");
        RT_BASIC(bk_ui->settingmoderecord_RecordDefreezeTemp3,          "RecordDeFreezeTemp3");
        RL(bk_ui->settingmoderecord_RecordDefreezeTimeHour3,            "RecordDeFreezeTimeHour3");
        RL(bk_ui->settingmoderecord_RecordDefreezeTimeMin3,             "RecordDeFreezeTimeMin3");
        RT_SCALE(bk_ui->settingmoderecord_RecordFermentation1Temp3,     "RecordFermentation1Temp3");
        RL(bk_ui->settingmoderecord_RecordFermentation1Humidity3,       "RecordFermentation1Humidity3");
        RL(bk_ui->settingmoderecord_RecordFermentation1TimeHour3,       "RecordFermentation1TimeHour3");
        RL(bk_ui->settingmoderecord_RecordFermentation1TimeMin3,        "RecordFermentation1TimeMin3");
        RT_SCALE(bk_ui->settingmoderecord_RecordFermentation2Temp3,     "RecordFermentation2Temp3");
        RL(bk_ui->settingmoderecord_RecordFermentation2Humidity3,       "RecordFermentation2Humidity3");
        RL(bk_ui->settingmoderecord_RecordFermentation2TimeHour3,       "RecordFermentation2TimeHour3");
        RL(bk_ui->settingmoderecord_RecordFermentation2TimeMin3,        "RecordFermentation2TimeMin3");
        RL(bk_ui->settingmoderecord_RecordStartMonth3,                  "RecordStartMonth3");
        RL(bk_ui->settingmoderecord_RecordStartDay3,                    "RecordStartDay3");
        RL(bk_ui->settingmoderecord_RecordStartHour3,                   "RecordStartHour3");
        RL(bk_ui->settingmoderecord_RecordStartMin3,                    "RecordStartMin3");
        RL(bk_ui->settingmoderecord_RecordEndMonth3,                    "RecordEndMonth3");
        RL(bk_ui->settingmoderecord_RecordEndDay3,                      "RecordEndDay3");
        RL(bk_ui->settingmoderecord_RecordEndHour3,                     "RecordEndHour3");
        RL(bk_ui->settingmoderecord_RecordEndMin3,                      "RecordEndMin3");
    }

    /* ── 슬롯 4 (가장 오래된) ── */
    {
        RT_BASIC(bk_ui->settingmoderecord_RecordFreezeTemp4,            "RecordFreezeTemp4");
        RL(bk_ui->settingmoderecord_RecordFreezeTimeHour4,              "RecordFreezeTimeHour4");
        RL(bk_ui->settingmoderecord_RecordFreezeTimeMin4,               "RecordFreezeTimeMin4");
        RT_BASIC(bk_ui->settingmoderecord_RecordDefreezeTemp4,          "RecordDeFreezeTemp4");
        RL(bk_ui->settingmoderecord_RecordDefreezeTimeHour4,            "RecordDeFreezeTimeHour4");
        RL(bk_ui->settingmoderecord_RecordDefreezeTimeMin4,             "RecordDeFreezeTimeMin4");
        RT_SCALE(bk_ui->settingmoderecord_RecordFermentation1Temp4,     "RecordFermentation1Temp4");
        RL(bk_ui->settingmoderecord_RecordFermentation1Humidity4,       "RecordFermentation1Humidity4");
        RL(bk_ui->settingmoderecord_RecordFermentation1TimeHour4,       "RecordFermentation1TimeHour4");
        RL(bk_ui->settingmoderecord_RecordFermentation1TimeMin4,        "RecordFermentation1TimeMin4");
        RT_SCALE(bk_ui->settingmoderecord_RecordFermentation2Temp4,     "RecordFermentation2Temp4");
        RL(bk_ui->settingmoderecord_RecordFermentation2Humidity4,       "RecordFermentation2Humidity4");
        RL(bk_ui->settingmoderecord_RecordFermentation2TimeHour4,       "RecordFermentation2TimeHour4");
        RL(bk_ui->settingmoderecord_RecordFermentation2TimeMin4,        "RecordFermentation2TimeMin4");
        RL(bk_ui->settingmoderecord_RecordStartMonth4,                  "RecordStartMonth4");
        RL(bk_ui->settingmoderecord_RecordStartDay4,                    "RecordStartDay4");
        RL(bk_ui->settingmoderecord_RecordStartHour4,                   "RecordStartHour4");
        RL(bk_ui->settingmoderecord_RecordStartMin4,                    "RecordStartMin4");
        RL(bk_ui->settingmoderecord_RecordEndMonth4,                    "RecordEndMonth4");
        RL(bk_ui->settingmoderecord_RecordEndDay4,                      "RecordEndDay4");
        RL(bk_ui->settingmoderecord_RecordEndHour4,                     "RecordEndHour4");
        RL(bk_ui->settingmoderecord_RecordEndMin4,                      "RecordEndMin4");
    }

#undef RL
#undef RT_BASIC
#undef RT_SCALE
}

void settingmoderecord_loaded_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    ui_title_anim(bk_ui->settingmoderecord_title);
}

void settingmoderecord_unload_start_event_cb(lv_event_t *e)
{
    (void)e;
}

void settingmoderecord_unloaded_event_cb(lv_event_t *e)
{
    (void)e;
}

void settingmoderecord_load_start_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    ui_lang_apply_settingmoderecord(bk_ui);
    _apply_record_labels(bk_ui);
}
