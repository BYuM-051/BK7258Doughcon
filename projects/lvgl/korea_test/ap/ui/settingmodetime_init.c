#include "lvgl.h"
#include "beken_ui.h"
#include "custom_func.h"
#include "event_runtime.h"
#include "ui_lang.h"
#include <stdio.h>
#include <string.h>

#include "ui_config.h"
#include "settings.h"

#define TAG "[settingmodetime_init.c] "
#include "pageManager.h"
// #define bk_printf(fmt, ...) do {if(0) bk_printf(fmt, ##__VA_ARGS__); } while(0) // disable printf

extern bk_lv_ui_t bk_lv_tool_ui;
extern lv_obj_t *preRenderRoot;
static uint32_t currentStep = RENDER_STEP_CREATE_PAGE;
static uint32_t currentImageStep = 0;
static bool stepInitMode = false;
extern void settingmodetime_backbt_event_cb(lv_event_t *e);
extern void settingmodetime_setting_time_setdatebt_event_cb(lv_event_t *e);
extern void settingmodetime_setting_time_settimebt_event_cb(lv_event_t *e);
extern void settingmodetime_load_start_event_cb(lv_event_t *e);
extern void settingmodetime_loaded_event_cb(lv_event_t *e);
extern void settingmodetime_unload_start_event_cb(lv_event_t *e);
extern void settingmodetime_unloaded_event_cb(lv_event_t *e);
extern void settingmodetime_ampm_bt_event_cb(lv_event_t *e);
extern void settingmodetime_field_tap_event_cb(lv_event_t *e);
extern void lv_digital_date_register(lv_obj_t *label);
extern void lv_digital_date_unregister(lv_obj_t *label);

/* 입력 라벨 생성 헬퍼 */
static lv_obj_t *_mk_lbl(lv_obj_t *parent, const char *text,
                           const lv_font_t *font, lv_color_t color, int x, int y)
{
    lv_obj_t *o = lv_label_create(parent);
    lv_label_set_text(o, text);
    lv_obj_set_style_text_font(o, font, 0);
    lv_obj_set_style_text_color(o, color, 0);
    lv_obj_set_style_bg_opa(o, 0, 0);
    lv_obj_set_pos(o, x, y);
    lv_obj_remove_flag(o, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(o, LV_OBJ_FLAG_HIDDEN);
    return o;
}

/* 입력 필드 라벨 (클릭 가능, 필드 직접 선택용) */
static lv_obj_t *_mk_field_lbl(lv_obj_t *parent, const char *text,
                                const lv_font_t *font, lv_color_t color,
                                int x, int y, int field_idx)
{
    lv_obj_t *o = lv_label_create(parent);
    lv_label_set_text(o, text);
    lv_obj_set_style_text_font(o, font, 0);
    lv_obj_set_style_text_color(o, color, 0);
    lv_obj_set_style_bg_opa(o, 0, 0);
    lv_obj_set_pos(o, x, y);
    lv_obj_add_flag(o, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_remove_flag(o, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(o, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_event_cb(o, settingmodetime_field_tap_event_cb,
                        LV_EVENT_ALL, (void *)(intptr_t)field_idx);
    return o;
}

/* 언더바 생성 헬퍼 */
static lv_obj_t *_mk_ub(lv_obj_t *parent, int x, int y, int w, int h, lv_color_t color)
{
    lv_obj_t *o = lv_obj_create(parent);
    lv_obj_set_pos(o, x, y);
    lv_obj_set_size(o, w, h);
    lv_obj_set_style_bg_color(o, color, 0);
    lv_obj_set_style_bg_opa(o, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(o, 0, 0);
    lv_obj_set_style_radius(o, 0, 0);
    lv_obj_set_style_pad_all(o, 0, 0);
    lv_obj_remove_flag(o, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(o, LV_OBJ_FLAG_HIDDEN);
    return o;
}

void destroy_page_settingmodetime(bk_lv_ui_t *bk_ui)
{
    if (bk_ui == NULL) return;
    if (bk_ui->settingmodetime != NULL) {
        lv_digital_date_unregister(bk_ui->settingmodetime_setting_time_setdate);
        lv_obj_del(bk_ui->settingmodetime);
        bk_ui->settingmodetime = NULL;
    }
    bk_ui->settingmodetime_year_txt     = NULL;
    bk_ui->settingmodetime_month_txt    = NULL;
    bk_ui->settingmodetime_day_txt      = NULL;
    bk_ui->settingmodetime_hour_txt     = NULL;
    bk_ui->settingmodetime_min_txt      = NULL;
    bk_ui->settingmodetime_sep1_txt     = NULL;
    bk_ui->settingmodetime_sep2_txt     = NULL;
    bk_ui->settingmodetime_tsep_txt     = NULL;
    bk_ui->settingmodetime_year_ub      = NULL;
    bk_ui->settingmodetime_month_ub     = NULL;
    bk_ui->settingmodetime_day_ub       = NULL;
    bk_ui->settingmodetime_hour_ub      = NULL;
    bk_ui->settingmodetime_min_ub       = NULL;
    bk_ui->settingmodetime_daterow_ub   = NULL;
    bk_ui->settingmodetime_timerow_ub   = NULL;
    bk_ui->settingmodetime_ampm_txt     = NULL;
    bk_ui->settingmodetime_ampm_bt      = NULL;
    bk_ui->settingmodetime_keypadbaseim = NULL;
    for (int i = 0; i < 12; i++) {
        bk_ui->settingmodetime_KeyPadBt[i] = NULL;
        bk_ui->settingmodetime_KeyPadIm[i] = NULL;
    }
    bk_ui->settingmodetime_keypadhide = NULL;
    bk_ui->settingmodetime_keypadhide_im = NULL;

    currentStep = RENDER_STEP_CREATE_PAGE;
    currentImageStep = 0;
    preRenderPageState[PAGE_SETTINGMODETIME].isRendered = false;

    const uint32_t imageCount = preRenderPageConfig[PAGE_SETTINGMODETIME].preRenderImageCount;
    for(uint32_t i = 0; i < imageCount; i++)
    {
        const preRenderImageInfo_t *imageInfo = &preRenderPageConfig[PAGE_SETTINGMODETIME].preRenderImageInfo[i];
        const char *languageSuffix = imageInfo->hasLanguageVariant ?
                                     (settings_get_int("LANGUAGE") == 1 ? "_china" :
                                      settings_get_int("LANGUAGE") == 2 ? "_english" : "") : "";
        const char *degreeSuffix = imageInfo->hasDegreeVariant &&
                                   strcmp(settings_get_str("Degree"), "\xc2\xb0""F") == 0 ? "_f" : "";
        const char *extension = imageInfo->fileExtension != NULL ? imageInfo->fileExtension : ".png";
        char imagePath[128] = {0};

        snprintf(imagePath, sizeof(imagePath), "%s%s%s%s",
                 imageInfo->imagePath, degreeSuffix, languageSuffix, extension);
        lv_image_cache_drop(imagePath);
    }
}

void init_page_settingmodetime(bk_lv_ui_t *bk_ui)
{
    if(stepInitMode && currentStep == RENDER_STEP_CREATE_CHILD)
    {
        goto create_children;
    }

    if (bk_ui->settingmodetime != NULL && lv_obj_is_valid(bk_ui->settingmodetime)) {
        destroy_page_settingmodetime(bk_ui);
    }

    ui_lang_reset_settingmodetime_cache();

#if UI_PRENDERING_ENABLE
    bk_ui->settingmodetime = lv_obj_create(preRenderRoot);
    lv_obj_remove_style_all(bk_ui->settingmodetime);
    lv_obj_set_size(bk_ui->settingmodetime, 1024, 600);
    lv_obj_set_pos(bk_ui->settingmodetime, 0, 0);
    lv_obj_set_style_radius(bk_ui->settingmodetime, 0, LV_PART_MAIN);
    lv_obj_set_scrollbar_mode(bk_ui->settingmodetime, LV_SCROLLBAR_MODE_OFF);
    // 원래 LV_EVENT_ALL로 등록되어 있었음
    lv_obj_add_event_cb(bk_ui->settingmodetime, settingmodetime_load_start_event_cb, UI_EVENT_PAGE_SHOW_START, NULL);
    lv_obj_add_event_cb(bk_ui->settingmodetime, settingmodetime_loaded_event_cb, UI_EVENT_PAGE_SHOWN,     NULL);
    lv_obj_add_event_cb(bk_ui->settingmodetime, settingmodetime_unload_start_event_cb, UI_EVENT_PAGE_HIDE_START, NULL);
    lv_obj_add_event_cb(bk_ui->settingmodetime, settingmodetime_unloaded_event_cb, UI_EVENT_PAGE_HIDDEN,     NULL);
#else
    bk_ui->settingmodetime = lv_obj_create(NULL);
    lv_obj_set_size(bk_ui->settingmodetime, 1024, 600);
    lv_obj_set_scrollbar_mode(bk_ui->settingmodetime, LV_SCROLLBAR_MODE_OFF);
    // 원래 LV_EVENT_ALL로 등록되어 있었음
    lv_obj_add_event_cb(bk_ui->settingmodetime, settingmodetime_load_start_event_cb, LV_EVENT_SCREEN_LOAD_START, NULL);
    lv_obj_add_event_cb(bk_ui->settingmodetime, settingmodetime_loaded_event_cb, LV_EVENT_SCREEN_LOADED,     NULL);
    lv_obj_add_event_cb(bk_ui->settingmodetime, settingmodetime_unload_start_event_cb, LV_EVENT_SCREEN_UNLOAD_START, NULL);
    lv_obj_add_event_cb(bk_ui->settingmodetime, settingmodetime_unloaded_event_cb, LV_EVENT_SCREEN_UNLOADED,     NULL);
#endif /* UI_PRENDERING_ENABLE */

    if(stepInitMode)
    {
        return;
    }

create_children:

    /* 배경 — bg.jpg 대신 단색(0xd9d9d9) */
    bk_ui->settingmodetime_bg = lv_image_create(bk_ui->settingmodetime);
    lv_obj_add_flag(bk_ui->settingmodetime_bg, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_style_bg_color(bk_ui->settingmodetime, lv_color_hex(0xd9d9d9), 0);
    lv_obj_set_style_bg_opa(bk_ui->settingmodetime, LV_OPA_COVER, 0);
    lv_obj_set_pos(bk_ui->settingmodetime_bg, 0, 0);

    /* 타이틀 */
    bk_ui->settingmodetime_title = lv_image_create(bk_ui->settingmodetime);
    _img_set_src_timed(bk_ui->settingmodetime_title, "/images/detail_time_title.png");
    lv_obj_set_pos(bk_ui->settingmodetime_title, 0, 10);
    lv_obj_set_size(bk_ui->settingmodetime_title, 380, 80);
    lv_image_set_inner_align(bk_ui->settingmodetime_title, LV_IMAGE_ALIGN_TOP_LEFT);

    /* 뒤로 버튼 */
    bk_ui->settingmodetime_backbt = lv_button_create(bk_ui->settingmodetime);
    lv_obj_add_flag(bk_ui->settingmodetime_backbt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->settingmodetime_backbt, settingmodetime_backbt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->settingmodetime_backbt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->settingmodetime_backbt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->settingmodetime_backbt, 0, 0);
    lv_obj_set_pos(bk_ui->settingmodetime_backbt, 825, 13);
    lv_obj_set_size(bk_ui->settingmodetime_backbt, 179, 74);

    bk_ui->settingmodetime_exitim = lv_image_create(bk_ui->settingmodetime);
    _img_set_src_timed(bk_ui->settingmodetime_exitim, "/images/exit_bt.png");
    lv_obj_set_pos(bk_ui->settingmodetime_exitim, 825, 13);
    lv_obj_set_size(bk_ui->settingmodetime_exitim, 179, 74);

    /* ── 날짜 행 (y=165~251) ─────────────────────────────────────────── */
    bk_ui->settingmodetime_imageview4 = lv_image_create(bk_ui->settingmodetime);
    _img_set_src_timed(bk_ui->settingmodetime_imageview4, "/images/setting_time_date.png");
    lv_obj_set_pos(bk_ui->settingmodetime_imageview4, 112, 165);
    lv_obj_set_size(bk_ui->settingmodetime_imageview4, 800, 86);

    /* 날짜 표시 라벨 — 통짜 문자열("2026.01.01") 표시용이었으나, 진입 시부터
     * 항상 필드 분리 표시(year_txt/month_txt/day_txt)로 통일하면서 더 이상
     * 화면에 보이지 않음. settings 값 유지 목적으로 객체/텍스트 갱신은 남겨두되
     * 항상 숨김 처리. */
    bk_ui->settingmodetime_setting_time_setdate = lv_label_create(bk_ui->settingmodetime);
    lv_digital_date_register(bk_ui->settingmodetime_setting_time_setdate);
    lv_obj_set_style_bg_opa(bk_ui->settingmodetime_setting_time_setdate, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmodetime_setting_time_setdate, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmodetime_setting_time_setdate, &lv_font_scdream_regular_42, 0);
    lv_obj_add_flag(bk_ui->settingmodetime_setting_time_setdate, LV_OBJ_FLAG_HIDDEN);
    /* year_txt(편집 라벨, underbar 있는 상태)와 정확히 같은 y로 맞춤 —
     * 기존엔 177(표시)과 178+6=184(편집) 사이 1px 차이가 있어 편집 진입 시
     * 언더바가 나타나며 텍스트가 살짝 내려가는 것처럼 보였음. */
    lv_obj_set_pos(bk_ui->settingmodetime_setting_time_setdate, 544, 178+6);

    bk_ui->settingmodetime_setting_time_setdatebt = lv_button_create(bk_ui->settingmodetime);
    lv_obj_add_flag(bk_ui->settingmodetime_setting_time_setdatebt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->settingmodetime_setting_time_setdatebt,
                        settingmodetime_setting_time_setdatebt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->settingmodetime_setting_time_setdatebt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->settingmodetime_setting_time_setdatebt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->settingmodetime_setting_time_setdatebt, 0, 0);
    lv_obj_set_pos(bk_ui->settingmodetime_setting_time_setdatebt, 112, 165);
    lv_obj_set_size(bk_ui->settingmodetime_setting_time_setdatebt, 800, 86);

    /* ── 날짜 입력 라벨 (행 내부, 기본 HIDDEN) ───────────────────────
     * 날짜 행 y=165~251(86px), font_42 높이≈50px, y=178~228 내에 배치.
     * "날짜설정" PNG 텍스트 우측(x≥430)에 YYYY . MM . DD 배치. */
    lv_color_t c_normal = lv_color_hex(0x3C3A3D);
    lv_color_t c_sep    = lv_color_hex(0x888888);

    /* display label(setting_time_setdate)과 동일 x=544 기준 배치.
     * year/month/day 는 클릭으로 해당 필드 직접 이동 가능. */
    bk_ui->settingmodetime_year_txt =
        _mk_field_lbl(bk_ui->settingmodetime, "2026", &lv_font_scdream_regular_42, c_normal, 544, 178+6+5, 1);
    bk_ui->settingmodetime_sep1_txt =
        _mk_lbl(bk_ui->settingmodetime, ".", &lv_font_scdream_regular_35, c_sep, 650, 186+6+5);
    bk_ui->settingmodetime_month_txt =
        _mk_field_lbl(bk_ui->settingmodetime, "01", &lv_font_scdream_regular_42, c_normal, 665, 178+6+5, 2);
    bk_ui->settingmodetime_sep2_txt =
        _mk_lbl(bk_ui->settingmodetime, ".", &lv_font_scdream_regular_35, c_sep, 720, 186+6+5);
    bk_ui->settingmodetime_day_txt =
        _mk_field_lbl(bk_ui->settingmodetime, "01", &lv_font_scdream_regular_42, c_normal, 735, 178+6+5, 3);

    /* 날짜 필드 언더바 (y=228+3) — 검정, 위치도 3px 아래로, 두께 2배(4→8), 추가 2px 아래로 */
    bk_ui->settingmodetime_year_ub  = _mk_ub(bk_ui->settingmodetime, 544, 230, 104, 8, lv_color_hex(0x000000));
    bk_ui->settingmodetime_month_ub = _mk_ub(bk_ui->settingmodetime, 665, 230,  52, 8, lv_color_hex(0x000000));
    bk_ui->settingmodetime_day_ub   = _mk_ub(bk_ui->settingmodetime, 735, 230,  52, 8, lv_color_hex(0x000000));

    /* ── 시간 행 (y=325~411) ─────────────────────────────────────────── */
    bk_ui->settingmodetime_imageview7 = lv_image_create(bk_ui->settingmodetime);
    _img_set_src_timed(bk_ui->settingmodetime_imageview7, "/images/setting_time_clock.png");
    lv_obj_set_pos(bk_ui->settingmodetime_imageview7, 112, 325);
    lv_obj_set_size(bk_ui->settingmodetime_imageview7, 800, 86);

    /* 시간 표시 라벨 (편집 중 숨겨짐) */
    bk_ui->settingmodetime_setting_time_settime = lv_label_create(bk_ui->settingmodetime);
    lv_label_set_text(bk_ui->settingmodetime_setting_time_settime, "12:00");
    lv_obj_set_style_bg_opa(bk_ui->settingmodetime_setting_time_settime, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmodetime_setting_time_settime, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmodetime_setting_time_settime, &lv_font_scdream_regular_42, 0);
    lv_obj_set_pos(bk_ui->settingmodetime_setting_time_settime, 544, 335+6);
    /* setting_time_setdate와 동일한 이유로 항상 숨김 — 진입 시부터 hour_txt/
     * min_txt/ampm_txt 필드 분리 표시로 통일 (더 이상 통짜 문자열 안 씀). */
    lv_obj_add_flag(bk_ui->settingmodetime_setting_time_settime, LV_OBJ_FLAG_HIDDEN);

    bk_ui->settingmodetime_setting_time_settimebt = lv_button_create(bk_ui->settingmodetime);
    lv_obj_add_flag(bk_ui->settingmodetime_setting_time_settimebt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->settingmodetime_setting_time_settimebt,
                        settingmodetime_setting_time_settimebt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->settingmodetime_setting_time_settimebt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->settingmodetime_setting_time_settimebt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->settingmodetime_setting_time_settimebt, 0, 0);
    lv_obj_set_pos(bk_ui->settingmodetime_setting_time_settimebt, 112, 325);
    lv_obj_set_size(bk_ui->settingmodetime_setting_time_settimebt, 800, 86);

    /* ── 시간 입력 라벨 (행 내부, 기본 HIDDEN) ───────────────────────
     * 시간 행 y=325~411(86px). [AM/PM] HH : MM 배치.
     * AM/PM 토글 버튼(투명)이 ampm_txt 위에 올려짐. */

    /* AM/PM 토글 버튼 (display label x=544 기준) */
    bk_ui->settingmodetime_ampm_bt = lv_button_create(bk_ui->settingmodetime);
    lv_obj_set_pos(bk_ui->settingmodetime_ampm_bt, 544, 332);
    lv_obj_set_size(bk_ui->settingmodetime_ampm_bt, 72, 42);
    lv_obj_set_style_bg_opa(bk_ui->settingmodetime_ampm_bt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->settingmodetime_ampm_bt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->settingmodetime_ampm_bt, 0, 0);
    lv_obj_add_event_cb(bk_ui->settingmodetime_ampm_bt,
                        settingmodetime_ampm_bt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_add_flag(bk_ui->settingmodetime_ampm_bt, LV_OBJ_FLAG_HIDDEN);

    /* AM/PM 텍스트 라벨 (검정색) */
    /* hour_txt(335+6=341)와 같은 y로 맞춰 시:분과 나란히 정렬 */
    bk_ui->settingmodetime_ampm_txt =
        _mk_lbl(bk_ui->settingmodetime, "AM", &lv_font_scdream_regular_42, c_normal, 544, 335+6+5);

    /* 시 입력 (클릭으로 직접 이동) */
    bk_ui->settingmodetime_hour_txt =
        _mk_field_lbl(bk_ui->settingmodetime, "00", &lv_font_scdream_regular_42, c_normal, 634, 335+6+5, 4);
    /* ":" 구분자 */
    bk_ui->settingmodetime_tsep_txt =
        _mk_lbl(bk_ui->settingmodetime, ":", &lv_font_scdream_regular_35, c_sep, 692, 343+6+5);
    /* 분 입력 (클릭으로 직접 이동) */
    bk_ui->settingmodetime_min_txt =
        _mk_field_lbl(bk_ui->settingmodetime, "00", &lv_font_scdream_regular_42, c_normal, 710, 335+6+5, 5);

    /* 시간 필드 언더바 (y=386+3) — 검정, 위치도 3px 아래로, 두께 2배(4→8), 추가 2px 아래로 */
    bk_ui->settingmodetime_hour_ub = _mk_ub(bk_ui->settingmodetime, 634, 388, 52, 8, lv_color_hex(0x000000));
    bk_ui->settingmodetime_min_ub  = _mk_ub(bk_ui->settingmodetime, 710, 388, 52, 8, lv_color_hex(0x000000));

    /* 필드 분리 표시(연/월/일, 시/분, AM-PM)를 진입 시부터 항상 보이게 함 —
     * _mk_lbl/_mk_field_lbl은 기본 HIDDEN으로 생성하므로 여기서 일괄 해제.
     * 언더바(_ub)는 실제로 편집 중인 필드에서만 보여야 하므로 그대로 HIDDEN 유지. */
    lv_obj_clear_flag(bk_ui->settingmodetime_year_txt,  LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(bk_ui->settingmodetime_sep1_txt,  LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(bk_ui->settingmodetime_month_txt, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(bk_ui->settingmodetime_sep2_txt,  LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(bk_ui->settingmodetime_day_txt,   LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(bk_ui->settingmodetime_hour_txt,  LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(bk_ui->settingmodetime_tsep_txt,  LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(bk_ui->settingmodetime_min_txt,   LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(bk_ui->settingmodetime_ampm_txt,  LV_OBJ_FLAG_HIDDEN);
    /* ampm_bt(탭 영역)는 텍스트와 달리 시간 편집 중에만 활성화 — 항상 켜두면
     * 편집 세션 밖에서 커밋 없이 hour_txt를 조용히 바꿔버릴 수 있음.
     * _open_time_edit()에서 켜고, 편집 종료 지점들에서 끈다. */

    /* ── 키패드: lazy 생성 (settingmodetime_cb.c _keypad_on_smt) ──── */
    bk_ui->settingmodetime_keypadbaseim = NULL;
    bk_ui->settingmodetime_keypadhide   = NULL;
    bk_ui->settingmodetime_keypadhide_im = NULL;
    for (int i = 0; i < 12; i++) {
        bk_ui->settingmodetime_KeyPadBt[i] = NULL;
        bk_ui->settingmodetime_KeyPadIm[i] = NULL;
    }
}

rendererFuncStatus_t init_page_settingmodetime_with_step(bk_lv_ui_t *bk_ui)
{
    static uint32_t renderStartTick = 0;

    if(preRenderPageState[PAGE_SETTINGMODETIME].isRendered)
    {
        return RENDERER_FUNC_DONE;
    }

    switch(currentStep)
    {
        case RENDER_STEP_CREATE_PAGE:
        {
            renderStartTick = lv_tick_get();
            bk_printf(TAG "[RENDER][SETTINGMODETIME] start tick=%lu\n", (unsigned long)renderStartTick);

            if(bk_ui == NULL)
            {
                return RENDERER_FUNC_FAILED;
            }

            stepInitMode = true;
            init_page_settingmodetime(bk_ui);
            stepInitMode = false;
            if(bk_ui->settingmodetime == NULL || !lv_obj_is_valid(bk_ui->settingmodetime))
            {
                bk_printf(TAG "[RENDER][SETTINGMODETIME] CREATE_PAGE failed\n");
                return RENDERER_FUNC_FAILED;
            }

#if UI_PRENDERING_ENABLE
            lv_obj_add_flag(bk_ui->settingmodetime, LV_OBJ_FLAG_HIDDEN);
#endif
            currentStep = RENDER_STEP_CREATE_CHILD;
            return RENDERER_FUNC_NOT_DONE;
        }

        case RENDER_STEP_CREATE_CHILD:
        {
            stepInitMode = true;
            init_page_settingmodetime(bk_ui);
            stepInitMode = false;
            currentStep = RENDER_STEP_CACHE_BACKGROUND;
            return RENDERER_FUNC_NOT_DONE;
        }

        case RENDER_STEP_CACHE_BACKGROUND:
        {
            if(preRenderPageConfig[PAGE_SETTINGMODETIME].backgroundImageAssetId != SHARED_IMAGE_NONE)
            {
                const sharedImageAssetId_t assetId =
                    preRenderPageConfig[PAGE_SETTINGMODETIME].backgroundImageAssetId;
                if(set_shared_image_asset(bk_ui->settingmodetime_bg, assetId) != RENDERER_FUNC_DONE)
                {
                    return RENDERER_FUNC_FAILED;
                }
            }

            currentStep = RENDER_STEP_CACHE_IMAGE;
            return RENDERER_FUNC_NOT_DONE;
        }

        case RENDER_STEP_CACHE_IMAGE:
        {
            const uint32_t imageCount = preRenderPageConfig[PAGE_SETTINGMODETIME].preRenderImageCount;
            if(currentImageStep < imageCount)
            {
                const preRenderImageInfo_t *imageInfo =
                    &preRenderPageConfig[PAGE_SETTINGMODETIME].preRenderImageInfo[currentImageStep];
                const char *languageSuffix = imageInfo->hasLanguageVariant ?
                                             (settings_get_int("LANGUAGE") == 1 ? "_china" :
                                              settings_get_int("LANGUAGE") == 2 ? "_english" : "") : "";
                const char *degreeSuffix = imageInfo->hasDegreeVariant &&
                                           strcmp(settings_get_str("Degree"), "\xc2\xb0""F") == 0 ? "_f" : "";
                const char *extension =
                    imageInfo->fileExtension != NULL ? imageInfo->fileExtension : ".png";
                char imagePath[128] = {0};
                uint32_t imageStartTick = lv_tick_get();

                snprintf(imagePath, sizeof(imagePath), "%s%s%s%s",
                         imageInfo->imagePath, degreeSuffix, languageSuffix, extension);

                lv_result_t result = lv_image_decoder_prewarm(imagePath);
                if(result != LV_RESULT_OK)
                {
                    bk_printf(TAG "[PREWARM][SETTINGMODETIME] image %lu/%lu failed: %s (%lu ms)\n",
                              (unsigned long)(currentImageStep + 1),
                              (unsigned long)imageCount,
                              imagePath,
                              (unsigned long)lv_tick_elaps(imageStartTick));
                    return RENDERER_FUNC_FAILED;
                }

                bk_printf(TAG "[PREWARM][SETTINGMODETIME] image %lu/%lu done: %s (%lu ms)\n",
                          (unsigned long)(currentImageStep + 1),
                          (unsigned long)imageCount,
                          imagePath,
                          (unsigned long)lv_tick_elaps(imageStartTick));
                currentImageStep++;
                return RENDERER_FUNC_NOT_DONE;
            }

            currentImageStep = 0;
            currentStep = RENDER_STEP_ATTACH_EVENT;
            return RENDERER_FUNC_NOT_DONE;
        }

        case RENDER_STEP_ATTACH_EVENT:
        {
            /* init_page_settingmodetime() also attaches the page and control callbacks. */
            currentStep = RENDER_STEP_DONE;
            return RENDERER_FUNC_NOT_DONE;
        }

        case RENDER_STEP_DONE:
        {
            bk_printf(TAG "[RENDER][SETTINGMODETIME] done total=%lu ms\n",
                      (unsigned long)lv_tick_elaps(renderStartTick));

            currentStep = RENDER_STEP_CREATE_PAGE;
            currentImageStep = 0;
            renderStartTick = 0;
            preRenderPageState[PAGE_SETTINGMODETIME].isRendered = true;
            return RENDERER_FUNC_DONE;
        }

        default:
        {
            bk_printf(TAG "[RENDER][SETTINGMODETIME] invalid step=%lu\n",
                      (unsigned long)currentStep);
            return RENDERER_FUNC_FAILED;
        }
    }
}
