/* ui_lang.c — apply language/degree image selection to each screen.
 *
 * Naming convention (derived from build/images/ filenames):
 *   base[_f][_china|_english].png
 *   where:
 *     (no suffix)   = Korean  (LANGUAGE == 0)
 *     _china        = Chinese (LANGUAGE == 1)
 *     _english      = English (LANGUAGE == 2)
 *     _f            = °F variant (inserted before language suffix)
 *
 * Always uses .png extension because English variants only exist as .png.
 */
#include "lvgl.h"
#include "beken_ui.h"
#include "settings.h"
#include "device_state.h"
#include "ui_lang.h"
#include "custom_func.h"
#include <string.h>
#include <stdio.h>

#include "pageManager.h"
#define TAG "[ui_lang.c] "
// #define bk_printf(fmt, ...) do {if(0) printf(fmt, ##__VA_ARGS__); } while(0) // disable printf

#define _DEGREE_F_STR  "\xc2\xb0""F"

/* Build and apply a language+degree-aware image path.
 * is_f=0 suppresses the _f suffix regardless of degree setting. */
static void _img(lv_obj_t *obj, const char *base, const char *ext,
                 int lang, int is_f)
{
    if (!obj || !lv_obj_is_valid(obj)) return;
    char path[128];
    const char *fsuf = is_f    ? "_f"       : "";
    const char *lsuf = (lang == 1) ? "_china"
                     : (lang == 2) ? "_english"
                     :               "";
    snprintf(path, sizeof(path), "%s%s%s%s", base, fsuf, lsuf, ext);
    _img_set_src_timed(obj, path);
}

/* Read current language and degree state from settings. */
static void _get_state(int *lang, int *is_f)
{
    *lang = settings_get_int("LANGUAGE");
    *is_f = (strcmp(settings_get_str("Degree"), _DEGREE_F_STR) == 0) ? 1 : 0;
}

/* Shorthand macros — capture lang/is_f from the local scope. */
#define _L(obj, base, ext)   _img(obj, base, ext, lang, 0)
#define _LF(obj, base, ext)  _img(obj, base, ext, lang, is_f)

/* ── manualmode ─────────────────────────────────────────────────────── */
static int s_last_key_manualmode = -1;

void ui_lang_reset_manualmode_cache(void)
{
    s_last_key_manualmode = -1;
}

void ui_lang_apply_manualmode(bk_lv_ui_t *bk_ui)
{
    int lang, is_f;
    _get_state(&lang, &is_f);
    int _key_manualmode = lang * 10 + is_f;
    if (_key_manualmode == s_last_key_manualmode) return;
    s_last_key_manualmode = _key_manualmode;
    _L(bk_ui->manualmode_title,       "/images/manualmode_title", ".png");
    _L(bk_ui->manualmode_imageview3,  "/images/exit_bt",          ".png");
    _L(bk_ui->manualmode_imageview4,  "/images/manual_menu",      ".jpg");
}

/* ── manualmodestart ────────────────────────────────────────────────── */
// static int s_last_key_manualmodestart = -1;

// void ui_lang_reset_manualmodestart_cache(void)
// {
//     s_last_key_manualmodestart = -1;
// }

// void ui_lang_apply_manualmodestart(bk_lv_ui_t *bk_ui)
// {
//     int lang, is_f;
//     _get_state(&lang, &is_f);
//     device_state_t *state = &g_device_state;

//     /* manualmodestart_load_event_cb에서 black_out_checking일 때 _img_ensure_src()로
//      * blackout을 이미 로드한 직후 여기서 또 무조건 재설정해 이중 로드가 발생했다.
//      * 언어/단위/모드가 실제로 바뀐 경우 또는 화면이 새로 만들어진 직후에만 재적용. */
//     int key = lang * 100 + is_f * 10 + state->manual_current_mode;
//     if (key == s_last_key_manualmodestart) return;
//     s_last_key_manualmodestart = key;

//     _L(bk_ui->manualmodestart_title,    "/images/manualmode_title", ".png");
//     _L(bk_ui->manualmodestart_backim,   "/images/exit_bt",          ".png");
//     _L(bk_ui->manualmodestart_startim,  "/images/start_bt",         ".png");
//     /* 냉동(1)/해동(2)은 tempbox_zero, 발효(3)는 tempbox
//      * 파일명 규칙: tempbox[_f]_zero[_china|_english].png  (_f는 _zero 앞에 위치) */
//     if (state->manual_current_mode == 1 || state->manual_current_mode == 2) {
//         char _tbz[128];
//         snprintf(_tbz, sizeof(_tbz), "/images/tempbox%s_zero%s.png",
//                  is_f ? "_f" : "",
//                  (lang == 1) ? "_china" : (lang == 2) ? "_english" : "");
//         _img_set_src_timed(bk_ui->manualmodestart_tempbox, _tbz);
//     } else
//         _LF(bk_ui->manualmodestart_tempbox, "/images/tempbox",      ".png");
//     _L(bk_ui->manualmodestart_blackout, "/images/blackout",         ".png");
// #if 0
//     /* Mode-specific circle text and keypad background */
//     if (state->manual_current_mode == 1) {
//         _LF(bk_ui->manualmodestart_manual_txt_basic,
//             "/images/manual_freeze_circle_txt",        ".png");
//         _L(bk_ui->manualmodestart_keypadbaseim,
//             "/images/freeze_keypad",                    ".png");
//     } else if (state->manual_current_mode == 2) {
//         _LF(bk_ui->manualmodestart_manual_txt_basic,
//             "/images/manual_defrost_circle_txt",       ".png");
//         _L(bk_ui->manualmodestart_keypadbaseim,
//             "/images/defrost_keypad",                   ".png");
//     } else if (state->manual_current_mode == 3) {
//         _LF(bk_ui->manualmodestart_manual_txt_basic,
//             "/images/manual_fermentation2_circle_txt", ".png");
//         _L(bk_ui->manualmodestart_keypadbaseim,
//             "/images/fermentation_keypad",              ".png");
//     }else if (state->manual_current_mode == 4){
//          _LF(bk_ui->manualmodestart_manual_txt_basic,
//             "/images/manual_fermentation1_circle_txt", ".png");

//     }
//     #endif
//      if (state->manual_current_mode == 1) {
//         _LF(bk_ui->manualmodestart_manual_txt_basic,
//             "/images/manual_freeze_circle_txt",        ".png");
//         _L(bk_ui->manualmodestart_keypadbaseim,
//             "/images/keypadn",                    ".png");
//     } else if (state->manual_current_mode == 2) {
//         _LF(bk_ui->manualmodestart_manual_txt_basic,
//             "/images/manual_defrost_circle_txt",       ".png");
//         _L(bk_ui->manualmodestart_keypadbaseim,
//             "/images/keypadn",                   ".png");
//     } else if (state->manual_current_mode == 3) {
//         _LF(bk_ui->manualmodestart_manual_txt_basic,
//             "/images/manual_fermentation2_circle_txt", ".png");
//         _L(bk_ui->manualmodestart_keypadbaseim,
//             "/images/keypadn",              ".png");
//     }else if (state->manual_current_mode == 4){
//          _LF(bk_ui->manualmodestart_manual_txt_basic,
//             "/images/manual_fermentation1_circle_txt", ".png");

//     }
// }

/* ── automode ───────────────────────────────────────────────────────── */
// static int s_last_key_automode = -1;

// void ui_lang_reset_automode_cache(void)
// {
//     s_last_key_automode = -1;
// }

// void ui_lang_apply_automode(bk_lv_ui_t *bk_ui)
// {
//     int lang, is_f;
//     _get_state(&lang, &is_f);
//     /* automode 오브젝트가 살아있는 채로 재진입할 때마다(언어 변경 없이도) 매번
//      * 호출되어 board 이미지 4장 포함 11장을 재decode — lv_refr_now에서 1초 이상
//      * 소요. 언어/단위가 실제로 바뀐 경우 또는 init_page_automode가 오브젝트를
//      * 새로 만든 직후(ui_lang_reset_automode_cache 호출됨)에만 재적용한다. */
//     int key = lang * 10 + is_f;
//     if (key == s_last_key_automode) return;
//     s_last_key_automode = key;
//     _L(bk_ui->automode_title,                   "/images/automode_title",                ".png");
//     _L(bk_ui->automode_imageview3,              "/images/exit_bt",                       ".png");
//     _L(bk_ui->automode_imageview5,              "/images/start_bt",                      ".png");
//     _L(bk_ui->automode_imageview6,              "/images/auto_mode_start_box_time",      ".png");
//     _L(bk_ui->automode_imageview23,             "/images/load_bt",                       ".png");
//     _L(bk_ui->automode_imageview25,             "/images/save_bt",                       ".png");
//     _L(bk_ui->automode_imageview26,             "/images/auto_mode_freeze_board",        ".png");
//     _L(bk_ui->automode_imageview31,             "/images/auto_mode_defrost_board",       ".png");
//     _L(bk_ui->automode_imageview44,             "/images/auto_mode_fermentation1_board", ".png");
//     _L(bk_ui->automode_imageview60,             "/images/auto_mode_fermentation2_board", ".png");
//     _L(bk_ui->automode_AutoModeDefrostAutoTime, "/images/defrost_auto_time_box",         ".png");
//     _L(bk_ui->automode_keypadbaseim,            "/images/keypad",                        ".png");
// }

/* ── automodestart ──────────────────────────────────────────────────── */
static int s_last_key_automodestart = -1;

void ui_lang_reset_automodestart_cache(void)
{
    s_last_key_automodestart = -1;
}

void ui_lang_apply_automodestart(bk_lv_ui_t *bk_ui)
{
    int lang, is_f;
    _get_state(&lang, &is_f);
    device_state_t *state = &g_device_state;
    int _is_zero_mode = (state->current_op_mode == OP_MODE_FREEZE ||
                         state->current_op_mode == OP_MODE_DEFROST) ? 1 : 0;
    /* automodestart_load_event_cb에서 black_out_checking일 때 _img_ensure_src()로
     * blackout을 이미 로드한 직후 여기서 또 무조건 재설정해 이중 로드가 발생했다.
     * 언어/단위/모드가 실제로 바뀐 경우 또는 화면이 새로 만들어진 직후에만 재적용. */
    int key = (lang * 10 + is_f) * 2 + _is_zero_mode;
    if (key == s_last_key_automodestart) return;
    s_last_key_automodestart = key;
    /* auto_bg: automodestart_cb.c의 _ams_bg_load()에서 canvas 경유 로드 */
    _L(bk_ui->automodestart_title,          "/images/automode_title",            ".png");
    /* 냉동/해동 운전 중에는 tempbox_zero, 발효는 tempbox
     * 파일명 규칙: tempbox[_f]_zero[_china|_english].png  (_f는 _zero 앞에 위치) */
    if (_is_zero_mode) {
        char _tbz[128];
        snprintf(_tbz, sizeof(_tbz), "/images/tempbox%s_zero%s.png",
                 is_f ? "_f" : "",
                 (lang == 1) ? "_china" : (lang == 2) ? "_english" : "");
        _img_set_src_timed(bk_ui->automodestart_auto_tempbox, _tbz);
    } else
        _LF(bk_ui->automodestart_auto_tempbox, "/images/tempbox",      ".png");
    _L(bk_ui->automodestart_imageview12,    "/images/auto_mode_start_box_time", ".png");
    _L(bk_ui->automodestart_imageview8,     "/images/stop_bt",                  ".png");
    _L(bk_ui->automodestart_blackout,       "/images/blackout",                 ".png");

    /* 영어 모드: 발효 GIF 아이콘을 아래로 이동 (텍스트 레이아웃 차이 보정) */
    if (lang == 2) {
        lv_obj_set_pos(bk_ui->automodestart_fermentation1_gif,       522, 313-7-2);
        lv_obj_set_pos(bk_ui->automodestart_fermentation1_gif_basic, 522, 313-7-2);
        lv_obj_set_pos(bk_ui->automodestart_fermentation2_gif,       440, 313-7-2);
        lv_obj_set_pos(bk_ui->automodestart_fermentation2_gif_basic, 440, 313-7-2);
    } else {
        lv_obj_set_pos(bk_ui->automodestart_fermentation1_gif,       522, 272);
        lv_obj_set_pos(bk_ui->automodestart_fermentation1_gif_basic, 522, 272);
        lv_obj_set_pos(bk_ui->automodestart_fermentation2_gif,       440, 272);
        lv_obj_set_pos(bk_ui->automodestart_fermentation2_gif_basic, 440, 272);
    }
}

/* ── automodeend ────────────────────────────────────────────────────── */
static int s_last_key_automodeend = -1;

void ui_lang_reset_automodeend_cache(void)
{
    s_last_key_automodeend = -1;
}

void ui_lang_apply_automodeend(bk_lv_ui_t *bk_ui)
{
    int lang, is_f;
    _get_state(&lang, &is_f);
    int _key_automodeend = lang * 10 + is_f;
    if (_key_automodeend == s_last_key_automodeend) return;
    s_last_key_automodeend = _key_automodeend;
    _L(bk_ui->automodeend_auto_mode_end_bg, "/images/auto_mode_end_bgi", ".jpg");
    _L(bk_ui->automodeend_title,            "/images/automode_title",      ".png");
    _L(bk_ui->automodeend_imageview5,       "/images/stop_bt",             ".png");
    _L(bk_ui->automodeend_blackout,         "/images/blackout",            ".png");
}

/* ── autodrymode ────────────────────────────────────────────────────── */
static int s_last_key_autodrymode = -1;

void ui_lang_reset_autodrymode_cache(void)
{
    s_last_key_autodrymode = -1;
}

void ui_lang_apply_autodrymode(bk_lv_ui_t *bk_ui)
{
    int lang, is_f;
    _get_state(&lang, &is_f);
    int _key_autodrymode = lang * 10 + is_f;
    if (_key_autodrymode == s_last_key_autodrymode) return;
    s_last_key_autodrymode = _key_autodrymode;
    _L(bk_ui->autodrymode_title,              "/images/autodrymode_title",   ".png");
    _L(bk_ui->autodrymode_backim,             "/images/exit_bt",             ".png");
    _L(bk_ui->autodrymode_auto_dry_startim,   "/images/start_bt",            ".png");
    _LF(bk_ui->autodrymode_auto_dry_txt_basic,"/images/auto_dry_circle_txt", ".png");
    _LF(bk_ui->autodrymode_auto_tempbox,      "/images/tempbox",             ".png");
    _L(bk_ui->autodrymode_imageview16,        "/images/auto_dry_time_box",   ".png");
    _L(bk_ui->autodrymode_keypadbaseim,       "/images/keypad",              ".png");
    _L(bk_ui->autodrymode_blackout,           "/images/blackout",            ".png");
}

/* ── memorymode ─────────────────────────────────────────────────────── */
static int s_last_key_memorymode = -1;

void ui_lang_reset_memorymode_cache(void)
{
    s_last_key_memorymode = -1;
}

void ui_lang_apply_memorymode(bk_lv_ui_t *bk_ui)
{
    int lang, is_f;
    _get_state(&lang, &is_f);
    /* memorymode_load_event_cb의 SCREEN_LOADED에서 _img_ensure_src()로 이미
     * 로드한 title/title_line/memorybox0/ok/delete를 여기서 또 무조건
     * 재설정해 매번 이중 로드가 발생했다. 언어/단위가 실제로 바뀐 경우 또는
     * init_page_memorymode가 오브젝트를 새로 만든 직후에만 재적용한다. */
    int key = lang * 10 + is_f;
    if (key == s_last_key_memorymode) return;
    s_last_key_memorymode = key;
    _L(bk_ui->memorymode_title,       "/images/memorymode_title",  ".png");
    _L(bk_ui->memorymode_imageview3,  "/images/exit_bt",           ".png");
    _L(bk_ui->memorymode_imageview4,  "/images/memory_title_line", ".png");
    _LF(bk_ui->memorymode_memorybox0, "/images/memory_box",        ".png");
    _LF(bk_ui->memorymode_memorybox1, "/images/memory_box",        ".png");
    _LF(bk_ui->memorymode_memorybox2, "/images/memory_box",        ".png");
    _LF(bk_ui->memorymode_memorybox3, "/images/memory_box",        ".png");
    _L(bk_ui->memorymode_imageview92, "/images/ok",                ".png");
    _L(bk_ui->memorymode_deleteim,    "/images/delete",            ".png");
}

/* ── settingmode ────────────────────────────────────────────────────── */
static int s_last_key_settingmode = -1;

void ui_lang_reset_settingmode_cache(void)
{
    s_last_key_settingmode = -1;
}

void ui_lang_apply_settingmode(bk_lv_ui_t *bk_ui)
{
    int lang, is_f;
    _get_state(&lang, &is_f);
    int _key_settingmode = lang * 10 + is_f;
    if (_key_settingmode == s_last_key_settingmode) return;
    s_last_key_settingmode = _key_settingmode;
#if UI_SETTINGMODE_COMBINED_BG_ENABLE
    _L(bk_ui->settingmode_bg,          "/images/feature-setting",            ".jpg");
#else
    _L(bk_ui->settingmode_title,       "/images/setting_title",              ".png");
    _L(bk_ui->settingmode_imageview3,  "/images/setting_mode_detailsetting", ".png");
    _L(bk_ui->settingmode_imageview5,  "/images/setting_mode_degree",        ".png");
    _L(bk_ui->settingmode_imageview7,  "/images/setting_mode_record",        ".png");
    _L(bk_ui->settingmode_imageview9,  "/images/setting_mode_test",          ".png");
    _L(bk_ui->settingmode_imageview11, "/images/setting_mode_time",          ".png");
    _L(bk_ui->settingmode_imageview13, "/images/setting_mode_language",      ".png");
    _L(bk_ui->settingmode_imageview15, "/images/exit_bt",                    ".png");
#endif
}

/* ── settingmodedetailsetting ────────────────────────────────────────── */
static int s_last_key_settingmodedetailsetting = -1;

void ui_lang_reset_settingmodedetailsetting_cache(void)
{
    s_last_key_settingmodedetailsetting = -1;
}

void ui_lang_apply_settingmodedetailsetting(bk_lv_ui_t *bk_ui)
{
    int lang, is_f;
    _get_state(&lang, &is_f);
    int _key_settingmodedetailsetting = lang * 10 + is_f;
    if (_key_settingmodedetailsetting == s_last_key_settingmodedetailsetting) return;
    s_last_key_settingmodedetailsetting = _key_settingmodedetailsetting;
#if UI_SETTINGMODEDETAILSETTING_COMBINED_BG_ENABLE
    _L(bk_ui->settingmodedetailsetting_bg,          "/images/advancedsetting",    ".jpg");
#else
    _L(bk_ui->settingmodedetailsetting_title,       "/images/detail_title",       ".png");
    _L(bk_ui->settingmodedetailsetting_imageview3,  "/images/exit_bt",            ".png");
    _L(bk_ui->settingmodedetailsetting_imageview4,  "/images/detail_temp_bt",     ".png");
    _L(bk_ui->settingmodedetailsetting_imageview6,  "/images/detail_humidity_bt", ".png");
    _L(bk_ui->settingmodedetailsetting_imageview8,  "/images/detail_time_bt",     ".png");
    _L(bk_ui->settingmodedetailsetting_imageview10, "/images/detail_damper_bt",   ".png");
    _L(bk_ui->settingmodedetailsetting_imageview12, "/images/detail_defrost_bt",  ".png");
    _L(bk_ui->settingmodedetailsetting_imageview14, "/images/detail_reset_bt",    ".png");
#endif
}

/* ── settingmodedefrost ──────────────────────────────────────────────── */
static int s_last_key_settingmodedefrost = -1;

void ui_lang_reset_settingmodedefrost_cache(void)
{
    s_last_key_settingmodedefrost = -1;
}

void ui_lang_apply_settingmodedefrost(bk_lv_ui_t *bk_ui)
{
    int lang, is_f;
    _get_state(&lang, &is_f);
    int _key_settingmodedefrost = lang * 10 + is_f;
    if (_key_settingmodedefrost == s_last_key_settingmodedefrost) return;
    s_last_key_settingmodedefrost = _key_settingmodedefrost;
    _L(bk_ui->settingmodedefrost_title,      "/images/setting_title", ".png");
    _L(bk_ui->settingmodedefrost_imageview3, "/images/exit_bt",       ".png");
}

/* ── settingmodemanual ───────────────────────────────────────────────── */
static int s_last_key_settingmodemanual = -1;

void ui_lang_reset_settingmodemanual_cache(void)
{
    s_last_key_settingmodemanual = -1;
}

void ui_lang_apply_settingmodemanual(bk_lv_ui_t *bk_ui)
{
    int lang, is_f;
    _get_state(&lang, &is_f);
    int _key_settingmodemanual = lang * 10 + is_f;
    if (_key_settingmodemanual == s_last_key_settingmodemanual) return;
    s_last_key_settingmodemanual = _key_settingmodemanual;
    /* usermanual_title: no language variants */
    _L(bk_ui->settingmodemanual_imageview3, "/images/exit_bt", ".png");
}

/* ── settingmodetime ─────────────────────────────────────────────────── */
static int s_last_key_settingmodetime = -1;

void ui_lang_reset_settingmodetime_cache(void)
{
    s_last_key_settingmodetime = -1;
}

void ui_lang_apply_settingmodetime(bk_lv_ui_t *bk_ui)
{
    int lang, is_f;
    _get_state(&lang, &is_f);
    int _key_settingmodetime = lang * 10 + is_f;
    if (_key_settingmodetime == s_last_key_settingmodetime) return;
    s_last_key_settingmodetime = _key_settingmodetime;
    _L(bk_ui->settingmodetime_title,      "/images/detail_time_title",  ".png");
    _L(bk_ui->settingmodetime_exitim,     "/images/exit_bt",            ".png");
    _L(bk_ui->settingmodetime_imageview4, "/images/setting_time_date",  ".png");
    _L(bk_ui->settingmodetime_imageview7, "/images/setting_time_clock", ".png");
}

/* ── settingmodelanguage ─────────────────────────────────────────────── */
static int s_last_key_settingmodelanguage = -1;

void ui_lang_reset_settingmodelanguage_cache(void)
{
    s_last_key_settingmodelanguage = -1;
}

void ui_lang_apply_settingmodelanguage(bk_lv_ui_t *bk_ui)
{
    int lang, is_f;
    _get_state(&lang, &is_f);
    int _key_settingmodelanguage = lang * 10 + is_f;
    if (_key_settingmodelanguage == s_last_key_settingmodelanguage) return;
    s_last_key_settingmodelanguage = _key_settingmodelanguage;
    _L(bk_ui->settingmodelanguage_title,  "/images/language_title", ".png");
    _L(bk_ui->settingmodelanguage_exitim, "/images/exit_bt",        ".png");
    /* koreanim/chinaim/englishim: fixed language-selector icons, no lang variants */
}

/* ── settingmoderecord ───────────────────────────────────────────────── */
static int s_last_key_settingmoderecord = -1;

void ui_lang_reset_settingmoderecord_cache(void)
{
    s_last_key_settingmoderecord = -1;
}

void ui_lang_apply_settingmoderecord(bk_lv_ui_t *bk_ui)
{
    int lang, is_f;
    _get_state(&lang, &is_f);
    int _key_settingmoderecord = lang * 10 + is_f;
    if (_key_settingmoderecord == s_last_key_settingmoderecord) return;
    s_last_key_settingmoderecord = _key_settingmoderecord;
    _L(bk_ui->settingmoderecord_title,      "/images/record_title",               ".png");
    _L(bk_ui->settingmoderecord_imageview2, "/images/deleteall_bt",               ".png");
    _L(bk_ui->settingmoderecord_imageview5, "/images/exit_bt",                    ".png");
    _L(bk_ui->settingmoderecord_imageview6, "/images/setting_record_chart_title", ".png");
    /* chartbox0~4: no language variants, only Korean _f — skip */
}

/* ── settingmodetest ─────────────────────────────────────────────────── */
static int s_last_key_settingmodetest = -1;

void ui_lang_reset_settingmodetest_cache(void)
{
    s_last_key_settingmodetest = -1;
}

void ui_lang_apply_settingmodetest(bk_lv_ui_t *bk_ui)
{
    int lang, is_f;
    _get_state(&lang, &is_f);
    int _key_settingmodetest = lang * 10 + is_f;
    if (_key_settingmodetest == s_last_key_settingmodetest) return;
    s_last_key_settingmodetest = _key_settingmodetest;
    _L(bk_ui->settingmodetest_title,             "/images/test_title",           ".png");
    _L(bk_ui->settingmodetest_imageview3,        "/images/exit_bt",              ".png");
    _L(bk_ui->settingmodetest_imageview4,        "/images/testmode_box",         ".jpg");
}

/* ── settingmodedegree ───────────────────────────────────────────────── */
static int s_last_key_settingmodedegree = -1;

void ui_lang_reset_settingmodedegree_cache(void)
{
    s_last_key_settingmodedegree = -1;
}

void ui_lang_apply_settingmodedegree(bk_lv_ui_t *bk_ui)
{
    int lang, is_f;
    _get_state(&lang, &is_f);
    int _key_settingmodedegree = lang * 10 + is_f;
    if (_key_settingmodedegree == s_last_key_settingmodedegree) return;
    s_last_key_settingmodedegree = _key_settingmodedegree;
    _L(bk_ui->settingmodedegree_title,      "/images/symbol_title", ".png");
    _L(bk_ui->settingmodedegree_imageview3, "/images/exit_bt",      ".png");
}

/* ── detailsettingtemp ───────────────────────────────────────────────── */
static int s_last_key_detailsettingtemp = -1;

void ui_lang_reset_detailsettingtemp_cache(void)
{
    s_last_key_detailsettingtemp = -1;
}

void ui_lang_apply_detailsettingtemp(bk_lv_ui_t *bk_ui)
{
    int lang, is_f;
    _get_state(&lang, &is_f);
    int _key_detailsettingtemp = lang * 10 + is_f;
    if (_key_detailsettingtemp == s_last_key_detailsettingtemp) return;
    s_last_key_detailsettingtemp = _key_detailsettingtemp;
    _L(bk_ui->detailsettingtemp_title,       "/images/detail_temp_title", ".png");
    _L(bk_ui->detailsettingtemp_imageview3,  "/images/exit_bt",           ".png");
    _LF(bk_ui->detailsettingtemp_settingim1, "/images/detail_temp_2_off", ".png");
    _LF(bk_ui->detailsettingtemp_settingim2, "/images/detail_temp_3_off", ".png");
    _LF(bk_ui->detailsettingtemp_settingim3, "/images/detail_temp_4_off", ".png");
    _LF(bk_ui->detailsettingtemp_settingim4, "/images/detail_temp_5_off", ".png");
}

/* ── detailsettinghumidity ───────────────────────────────────────────── */
static int s_last_key_detailsettinghumidity = -1;

void ui_lang_reset_detailsettinghumidity_cache(void)
{
    s_last_key_detailsettinghumidity = -1;
}

void ui_lang_apply_detailsettinghumidity(bk_lv_ui_t *bk_ui)
{
    int lang, is_f;
    _get_state(&lang, &is_f);
    int _key_detailsettinghumidity = lang * 10 + is_f;
    if (_key_detailsettinghumidity == s_last_key_detailsettinghumidity) return;
    s_last_key_detailsettinghumidity = _key_detailsettinghumidity;
    _L(bk_ui->detailsettinghumidity_title,      "/images/detail_humidity_title", ".png");
    _L(bk_ui->detailsettinghumidity_imageview3,  "/images/exit_bt",              ".png");
    _L(bk_ui->detailsettinghumidity_settingim1,  "/images/detail_humidity_1_off",".png");
    _L(bk_ui->detailsettinghumidity_settingim2,  "/images/detail_humidity_2_off",".png");
    _L(bk_ui->detailsettinghumidity_settingim3,  "/images/detail_humidity_3_off",".png");
}

/* ── detailsettingdefrost ────────────────────────────────────────────── */
static int s_last_key_detailsettingdefrost = -1;

void ui_lang_reset_detailsettingdefrost_cache(void)
{
    s_last_key_detailsettingdefrost = -1;
}

void ui_lang_apply_detailsettingdefrost(bk_lv_ui_t *bk_ui)
{
    int lang, is_f;
    _get_state(&lang, &is_f);
    int _key_detailsettingdefrost = lang * 10 + is_f;
    if (_key_detailsettingdefrost == s_last_key_detailsettingdefrost) return;
    s_last_key_detailsettingdefrost = _key_detailsettingdefrost;
    _L(bk_ui->detailsettingdefrost_title,       "/images/detail_defrost_title", ".png");
    _L(bk_ui->detailsettingdefrost_imageview3,  "/images/exit_bt",              ".png");
    _L(bk_ui->detailsettingdefrost_settingim1,  "/images/detail_defrost_1_off", ".png");
    _LF(bk_ui->detailsettingdefrost_settingim2, "/images/detail_defrost_2_off", ".png");
    _L(bk_ui->detailsettingdefrost_settingim3,  "/images/detail_defrost_3_off", ".png");
}

/* ── detailsettingtime ───────────────────────────────────────────────── */
static int s_last_key_detailsettingtime = -1;

void ui_lang_reset_detailsettingtime_cache(void)
{
    s_last_key_detailsettingtime = -1;
}

void ui_lang_apply_detailsettingtime(bk_lv_ui_t *bk_ui)
{
    int lang, is_f;
    _get_state(&lang, &is_f);
    int _key_detailsettingtime = lang * 10 + is_f;
    if (_key_detailsettingtime == s_last_key_detailsettingtime) return;
    s_last_key_detailsettingtime = _key_detailsettingtime;
    _L(bk_ui->detailsettingtime_title,      "/images/detail_time_title",  ".png");
    _L(bk_ui->detailsettingtime_imageview3, "/images/exit_bt",            ".png");
    _L(bk_ui->detailsettingtime_settingim1, "/images/detail_time_1_off",  ".png");
    _L(bk_ui->detailsettingtime_settingim2, "/images/detail_time_2_off",  ".png");
    _L(bk_ui->detailsettingtime_settingim3, "/images/detail_time_3_off",  ".png");
    _L(bk_ui->detailsettingtime_settingim4, "/images/detail_time_4_off",  ".png");
    _L(bk_ui->detailsettingtime_settingim5, "/images/detail_time_5_off",  ".png");
}

/* ── detailsettingdamper ─────────────────────────────────────────────── */
static int s_last_key_detailsettingdamper = -1;

void ui_lang_reset_detailsettingdamper_cache(void)
{
    s_last_key_detailsettingdamper = -1;
}

void ui_lang_apply_detailsettingdamper(bk_lv_ui_t *bk_ui)
{
    int lang, is_f;
    _get_state(&lang, &is_f);
    int _key_detailsettingdamper = lang * 10 + is_f;
    if (_key_detailsettingdamper == s_last_key_detailsettingdamper) return;
    s_last_key_detailsettingdamper = _key_detailsettingdamper;
    _L(bk_ui->detailsettingdamper_title,      "/images/detail_damper_title", ".png");
    _L(bk_ui->detailsettingdamper_imageview3, "/images/exit_bt",             ".png");
    /* settingim1~4: china variant only — English falls back to Korean base */
    {
        char p[128];
        const char *csuf = (lang == 1) ? "_china" : "";
        snprintf(p, sizeof(p), "/images/detail_damper_1_off%s.png", csuf);
        _img_set_src_timed(bk_ui->detailsettingdamper_settingim1, p);
        snprintf(p, sizeof(p), "/images/detail_damper_2_off%s.png", csuf);
        _img_set_src_timed(bk_ui->detailsettingdamper_settingim2, p);
        snprintf(p, sizeof(p), "/images/detail_damper_3_off%s.png", csuf);
        _img_set_src_timed(bk_ui->detailsettingdamper_settingim3, p);
        snprintf(p, sizeof(p), "/images/detail_damper_4_off%s.png", csuf);
        _img_set_src_timed(bk_ui->detailsettingdamper_settingim4, p);
    }
}

/* ── timebar ────────────────────────────────────────────────────────── */
static int s_last_key_timebar = -1;

void ui_lang_reset_timebar_cache(void)
{
    s_last_key_timebar = -1;
}

void ui_lang_apply_timebar(bk_lv_ui_t *bk_ui)
{
    int lang, is_f;
    _get_state(&lang, &is_f);
    int _key_timebar = lang * 10 + is_f;
    if (_key_timebar == s_last_key_timebar) return;
    s_last_key_timebar = _key_timebar;
    _L(bk_ui->timebar_timebar_bg,         "/images/timebar_timebar",           ".png");
    _L(bk_ui->timebar_comp_checkim,       "/images/timebar_comp_on",           ".png");
    _L(bk_ui->timebar_watervalve_checkim, "/images/timebar_watevalve_on",      ".png");
    _L(bk_ui->timebar_roomfan_checkim,    "/images/timebar_roomfan_on",        ".png");
    _L(bk_ui->timebar_damper_checkim,     "/images/timebar_damper_on",         ".png");
    _L(bk_ui->timebar_humid_checkim,      "/images/timebar_humidityheater_on", ".png");
    _L(bk_ui->timebar_heat_checkim,       "/images/timebar_fireheater_on",     ".png");
    _L(bk_ui->timebar_defrost_checkim,    "/images/timebar_defrostheater_on",  ".png");
    /* timebar_error_on_english.png 없음 → 영어만 기본(한국어) 이미지로 폴백.
     * 이전엔 이 이유로 중국어까지 통째로 스킵해서 중국어 모드에서도 기본
     * 이미지가 나오는 버그가 있었음 — 중국어는 정상 적용. */
    _img_set_src_timed(bk_ui->timebar_timebar_error_checkim,
                        (lang == 1) ? "/images/timebar_error_on_china.png"
                                    : "/images/timebar_error_on.png");
}

/* ── main ───────────────────────────────────────────────────────────── */
// static int s_last_key_main = -1;

// void ui_lang_reset_main_cache(void)
// {
//     s_last_key_main = -1;
// }

// void ui_lang_apply_main(bk_lv_ui_t *bk_ui)
// {
//     int lang, is_f;
//     _get_state(&lang, &is_f);
//     int _key_main = lang * 10 + is_f;
//     if (_key_main == s_last_key_main) return;
//     s_last_key_main = _key_main;
// #if UI_MAIN_COMBINED_BG_ENABLE
//     _L(bk_ui->main_bg,         "/images/main",        ".jpg");
// #else
//     _L(bk_ui->main_imageview2,  "/images/automode",    ".png");
//     _L(bk_ui->main_imageview4,  "/images/manualmode",  ".png");
//     _L(bk_ui->main_imageview6,  "/images/autodrymode", ".png");
//     _L(bk_ui->main_imageview8,  "/images/memorymode",  ".png");
//     _L(bk_ui->main_imageview10, "/images/settingmode", ".png");
// #endif
// }

/* ── popupcaution ───────────────────────────────────────────────────── */
static int s_last_key_popupcaution = -1;

void ui_lang_reset_popupcaution_cache(void)
{
    s_last_key_popupcaution = -1;
}

void ui_lang_apply_popupcaution(bk_lv_ui_t *bk_ui)
{
    int lang, is_f;
    _get_state(&lang, &is_f);
    int _key_popupcaution = lang * 10 + is_f;
    if (_key_popupcaution == s_last_key_popupcaution) return;
    s_last_key_popupcaution = _key_popupcaution;
#if UI_POPUP_DIALOG_JPG_ENABLE
    _L(bk_ui->popupcaution_imageview1, "/images/caution_popup", ".jpg");
#else
    _L(bk_ui->popupcaution_imageview1, "/images/caution_popup", ".png");
#endif
}

/* ── popupreset ─────────────────────────────────────────────────────── */
static int s_last_key_popupreset = -1;

void ui_lang_reset_popupreset_cache(void)
{
    s_last_key_popupreset = -1;
}

void ui_lang_apply_popupreset(bk_lv_ui_t *bk_ui)
{
    int lang, is_f;
    _get_state(&lang, &is_f);
    int _key_popupreset = lang * 10 + is_f;
    if (_key_popupreset == s_last_key_popupreset) return;
    s_last_key_popupreset = _key_popupreset;
#if UI_POPUP_DIALOG_JPG_ENABLE
    _L(bk_ui->popupreset_imageview1, "/images/reset_popup", ".jpg");
#else
    _L(bk_ui->popupreset_imageview1, "/images/reset_popup", ".png");
#endif
}

/* ── popupdelete ────────────────────────────────────────────────────── */
static int s_last_key_popupdelete = -1;

void ui_lang_reset_popupdelete_cache(void)
{
    s_last_key_popupdelete = -1;
}

void ui_lang_apply_popupdelete(bk_lv_ui_t *bk_ui)
{
    int lang, is_f;
    _get_state(&lang, &is_f);
    int _key_popupdelete = lang * 10 + is_f;
    if (_key_popupdelete == s_last_key_popupdelete) return;
    s_last_key_popupdelete = _key_popupdelete;
#if UI_POPUP_DIALOG_JPG_ENABLE
    _L(bk_ui->popupdelete_imageview1, "/images/delete_popup", ".jpg");
#else
    _L(bk_ui->popupdelete_imageview1, "/images/delete_popup", ".png");
#endif
}

/* ── popupconnectionerror ───────────────────────────────────────────── */
static int s_last_key_popupconnectionerror = -1;

void ui_lang_reset_popupconnectionerror_cache(void)
{
    s_last_key_popupconnectionerror = -1;
}

void ui_lang_apply_popupconnectionerror(bk_lv_ui_t *bk_ui)
{
    int lang, is_f;
    _get_state(&lang, &is_f);
    int _key_popupconnectionerror = lang * 10 + is_f;
    if (_key_popupconnectionerror == s_last_key_popupconnectionerror) return;
    s_last_key_popupconnectionerror = _key_popupconnectionerror;
    _L(bk_ui->popupconnectionerror_popup, "/images/popup_connection_error", ".png");
}

/* ── popuperror ─────────────────────────────────────────────────────── */
static int s_last_key_popuperror = -1;

void ui_lang_reset_popuperror_cache(void)
{
    s_last_key_popuperror = -1;
}

void ui_lang_apply_popuperror(bk_lv_ui_t *bk_ui)
{
    int lang, is_f;
    _get_state(&lang, &is_f);
    int _key_popuperror = lang * 10 + is_f;
    if (_key_popuperror == s_last_key_popuperror) return;
    s_last_key_popuperror = _key_popuperror;
    const char *csuf = (lang == 1) ? "_china" : (lang == 2) ? "_english" : "";
    char p[128];

    /* box_error: china variant only — English falls back to Korean */
    snprintf(p, sizeof(p), "/images/box_error%s.png", (lang == 1) ? "_china" : "");
    _img_set_src_timed(bk_ui->popuperror_imageview2, p);

#define _EL(obj, name) \
    snprintf(p, sizeof(p), "/images/%s%s.png", (name), csuf); \
    _img_set_src_timed(obj, p)

    _EL(bk_ui->popuperror_e1,  "e1_off");
    _EL(bk_ui->popuperror_e2,  "e2_off");
    _EL(bk_ui->popuperror_e3,  "e3_off");
    _EL(bk_ui->popuperror_e4,  "e4_off");
    _EL(bk_ui->popuperror_e5,  "e5_off");
    _EL(bk_ui->popuperror_e6,  "e6_off");
    _EL(bk_ui->popuperror_e7,  "e7_off");
    _EL(bk_ui->popuperror_e8,  "e8_off");
    _EL(bk_ui->popuperror_e9,  "e9_off");
    _EL(bk_ui->popuperror_e10, "e10_off");
#undef _EL
}

/* ── picker (language-aware, called from detailsetting init/cb) ──── */
void ui_lang_apply_picker(lv_obj_t *obj, int num)
{
    int lang = settings_get_int("LANGUAGE");
    const char *lsuf = (lang == 1) ? "_china" : (lang == 2) ? "_english" : "";
    char path[128];
    snprintf(path, sizeof(path), "/images/picker_%d%s.png", num, lsuf);
    bk_printf(TAG "[PICKER] lang=%d -> %s\n", lang, path);
#if UI_CACHE_DROP_LOW_MEM_ENABLE
    /* picker_N.png(376x376 ARGB8888 ~552KB, alpha 있는 PNG)가 want=565508 B로
     * 반복 크래시한 이력이 있는 decode 지점 — 화면 전환 시점 체크(uart_comm.c)와
     * 이 개별 호출 사이의 갭을 줄이기 위해 decode 직전에도 한 번 더 체크. */
    ui_cache_drop_if_low_mem();
#endif
    _img_set_src_timed(obj, path);
}

/* ── ON/OFF value display (language-aware, text vs 开/关 font) ───── */
void ui_lang_set_onoff_display(lv_obj_t *label, const char *value)
{
    if (!label || !lv_obj_is_valid(label)) return;
    int lang = settings_get_int("LANGUAGE");
    bool is_on = value && (strcmp(value, "ON") == 0);
    if (lang == 1) {
        lv_obj_set_style_text_font(label, &lv_font_onoff_cn_32, 0);
        /* "开"(U+5F00) / "关"(U+5173) */
        lv_label_set_text(label, is_on ? "\xe5\xbc\x80" : "\xe5\x85\xb3");
    } else {
        lv_obj_set_style_text_font(label, &lv_font_scdream_regular_32, 0);
        /* value를 그대로 쓰지 않고 캐노니컬 "ON"/"OFF"로 정규화한다 — 다른 경로에서
         * 이미 开/关 같은 비정규 문자열이 저장돼 있어도 ASCII 전용 폰트로 그 값을
         * 그대로 그리면 네모(tofu)로 보이는 것을 방지한다. */
        lv_label_set_text(label, is_on ? "ON" : "OFF");
    }
}

/* ── next_bt page indicator (language-aware) ─────────────────────── */
void ui_lang_apply_next_bt(lv_obj_t *obj, int num)
{
    int lang = settings_get_int("LANGUAGE");
    const char *lsuf = (lang == 1) ? "_china" : (lang == 2) ? "_english" : "";
    char path[128];
    snprintf(path, sizeof(path), "/images/next_bt_%d%s.png", num, lsuf);
    _img_set_src_timed(obj, path);
}

/* ── popuppassword ──────────────────────────────────────────────────── */
static int s_last_key_popuppassword = -1;

void ui_lang_reset_popuppassword_cache(void)
{
    s_last_key_popuppassword = -1;
}

void ui_lang_apply_popuppassword(bk_lv_ui_t *bk_ui)
{
    int lang, is_f;
    _get_state(&lang, &is_f);
    int _key_popuppassword = lang * 10 + is_f;
    if (_key_popuppassword == s_last_key_popuppassword) return;
    s_last_key_popuppassword = _key_popuppassword;
#if UI_POPUPPASSWORD_COMBINED_BG_ENABLE
    /* dim된 settingmode 배경 + password 카드를 미리 합성해둔 이미지 1장 —
     * settingmode 전체를 매번 재렌더하던 비용을 없애기 위한 것 (ui_config.h 참고) */
    _L(bk_ui->popuppassword_imageview1,    "/images/feature-setting_password", ".jpg");
#else
    _L(bk_ui->popuppassword_imageview1,    "/images/password_popup", ".jpg");
#endif
    _L(bk_ui->popuppassword_pop_cautionim, "/images/popup_caution",  ".png");
}

/* ── apply_all ───────────────────────────────────────────────────────
 * Apply language/degree images to every already-created screen at once.
 * Screens not yet created (NULL or invalid) are silently skipped.
 * NOTE: This decodes images on every cached screen and is intentionally slow.
 * Prefer ui_lang_invalidate_cached_screens() for runtime language/degree changes. */
void ui_lang_apply_all(bk_lv_ui_t *bk_ui)
{
#define _A(screen, fn) \
    do { if (bk_ui->screen && lv_obj_is_valid(bk_ui->screen)) fn(bk_ui); } while(0)

    _A(timebar,                  ui_lang_apply_timebar);
    // _A(main,                     ui_lang_apply_main);
    _A(manualmode,               ui_lang_apply_manualmode);
    _A(manualmodestart,          ui_lang_apply_manualmodestart);
    // _A(automode,                 ui_lang_apply_automode);
    _A(automodestart,            ui_lang_apply_automodestart);
    _A(automodeend,              ui_lang_apply_automodeend);
    _A(autodrymode,              ui_lang_apply_autodrymode);
    _A(memorymode,               ui_lang_apply_memorymode);
    _A(settingmode,              ui_lang_apply_settingmode);
    _A(settingmodedetailsetting, ui_lang_apply_settingmodedetailsetting);
    _A(settingmodedefrost,       ui_lang_apply_settingmodedefrost);
    _A(settingmodemanual,        ui_lang_apply_settingmodemanual);
    _A(settingmodetime,          ui_lang_apply_settingmodetime);
    _A(settingmodelanguage,      ui_lang_apply_settingmodelanguage);
    _A(settingmoderecord,        ui_lang_apply_settingmoderecord);
    _A(settingmodetest,          ui_lang_apply_settingmodetest);
    _A(settingmodedegree,        ui_lang_apply_settingmodedegree);
    _A(detailsettingtemp,        ui_lang_apply_detailsettingtemp);
    _A(detailsettinghumidity,    ui_lang_apply_detailsettinghumidity);
    _A(detailsettingdefrost,     ui_lang_apply_detailsettingdefrost);
    _A(detailsettingtime,        ui_lang_apply_detailsettingtime);
    _A(detailsettingdamper,      ui_lang_apply_detailsettingdamper);
    _A(popupcaution,             ui_lang_apply_popupcaution);
    _A(popupreset,               ui_lang_apply_popupreset);
    _A(popupdelete,              ui_lang_apply_popupdelete);
    _A(popupconnectionerror,     ui_lang_apply_popupconnectionerror);
    _A(popuperror,               ui_lang_apply_popuperror);
    _A(popuppassword,            ui_lang_apply_popuppassword);

#undef _A
}

/* ── invalidate_cached_screens ───────────────────────────────────────
 * Fast alternative to ui_lang_apply_all() for runtime language/degree changes.
 *
 * Instead of re-decoding images on every cached screen (slow), destroy all
 * cached screens except the currently active one.  The next time each screen
 * is visited, init_page_* re-creates it from scratch — which already calls
 * ui_lang_apply_* with the new settings, so images are always correct.
 *
 * timebar is persistent and always visible: update it directly instead. */
extern void automode_ams_prewarm_reset(void);
extern void automode_mm_prewarm_cancel(void);
extern void automodestart_lang_invalidate(bk_lv_ui_t *bk_ui);

void ui_lang_invalidate_cached_screens(bk_lv_ui_t *bk_ui)
{
#if UI_PRENDERING_ENABLE
    lv_obj_t *current_page = ui_get_current_page();
    lv_obj_t *active = current_page;
#else
    lv_obj_t *active = lv_scr_act();
#endif /* UI_PRENDERING_ENABLE */
    /* lv_obj_del은 SCREEN_UNLOAD_START를 우회하므로 prewarm 타이머와
     * automodestart 내부 타이머를 여기서 명시적으로 취소 */
    automode_ams_prewarm_reset();
    automode_mm_prewarm_cancel();
    if (bk_ui->automodestart && lv_obj_is_valid(bk_ui->automodestart) &&
        (lv_obj_t *)bk_ui->automodestart != active)
        automodestart_lang_invalidate(bk_ui);

    /* timebar lives on lv_layer_top(), not a screen — update in-place */
    if (bk_ui->timebar && lv_obj_is_valid(bk_ui->timebar))
        ui_lang_apply_timebar(bk_ui);

/* Destroy a cached screen only when it is not the currently displayed one.
 * destroy_page_* sets bk_ui->field to NULL, so init_page_* is forced on next visit. */
#define _D(screen, fn) \
    do { \
        if (bk_ui->screen && lv_obj_is_valid(bk_ui->screen) && \
            (lv_obj_t *)bk_ui->screen != active) \
            fn(bk_ui); \
    } while(0)

    _D(main,                     destroy_page_main);
    _D(manualmode,               destroy_page_manualmode);
    _D(manualmodestart,          destroy_page_manualmodestart);
    _D(automode,                 destroy_page_automode);
    _D(automodestart,            destroy_page_automodestart);
    _D(automodeend,              destroy_page_automodeend);
    _D(autodrymode,              destroy_page_autodrymode);
    _D(memorymode,               destroy_page_memorymode);
    _D(settingmode,              destroy_page_settingmode);
    _D(settingmodedetailsetting, destroy_page_settingmodedetailsetting);
    _D(settingmodedefrost,       destroy_page_settingmodedefrost);
    _D(settingmodemanual,        destroy_page_settingmodemanual);
    _D(settingmodetime,          destroy_page_settingmodetime);
    _D(settingmodelanguage,      destroy_page_settingmodelanguage);
    _D(settingmoderecord,        destroy_page_settingmoderecord);
    _D(settingmodetest,          destroy_page_settingmodetest);
    _D(settingmodedegree,        destroy_page_settingmodedegree);
    _D(detailsettingtemp,        destroy_page_detailsettingtemp);
    _D(detailsettinghumidity,    destroy_page_detailsettinghumidity);
    _D(detailsettingdefrost,     destroy_page_detailsettingdefrost);
    _D(detailsettingtime,        destroy_page_detailsettingtime);
    _D(detailsettingdamper,      destroy_page_detailsettingdamper);
    _D(popupcaution,             destroy_page_popupcaution);
    _D(popupreset,               destroy_page_popupreset);
    _D(popupdelete,              destroy_page_popupdelete);
    _D(popupconnectionerror,     destroy_page_popupconnectionerror);
    _D(popuperror,               destroy_page_popuperror);
    _D(popuppassword,            destroy_page_popuppassword);

#undef _D
}
