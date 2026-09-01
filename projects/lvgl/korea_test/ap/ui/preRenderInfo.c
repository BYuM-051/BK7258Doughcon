#include "preRenderer.h"
#include "preRenderInfo.h"
#include "settings.h"
#include <stdio.h>
#include <string.h>

#define TAG "[preRenderInfo.c] "
// #define bk_printf(fmt, ...) do {if(0) bk_printf(fmt, ##__VA_ARGS__); } while(0) // disable printf

#define ARRAY_COUNT(array) (sizeof(array) / sizeof((array)[0]))

const preRenderImageInfo_t sharedImageAssetInfo[SHARED_IMAGE_COUNT] =
{
    [SHARED_IMAGE_ASSET_BG_DOUGH] =
    {
        .imagePath = "/images/bg",
        .hasLanguageVariant = false,
        .hasDegreeVariant = false,
        .fileExtension = ".jpg"
    },

    [SHARED_IMAGE_ASSET_BG_AUTOMODESTART] =
    {
        .imagePath = "/images/auto_mode_start_bgi",
        .hasLanguageVariant = true,
        .hasDegreeVariant = true,
        .fileExtension = ".jpg"
    }
};
preRendererImageState_t sharedImageAssetState[SHARED_IMAGE_COUNT] = {0};

static const preRenderImageInfo_t MainPreRenderImages[] =
{
    { "/images/automode", true, false, ".png" },
    { "/images/manualmode", true, false, ".png" },
    { "/images/autodrymode", true, false, ".png" },
    { "/images/memorymode", true, false, ".png" },
    { "/images/settingmode", true, false, ".png" }
};
static const preRenderImageInfo_t AutomodePreRenderImages[] =
{
    { "/images/automode_title", true, false, ".png" },
    { "/images/exit_bt", true, false, ".png" },
    { "/images/start_bt", true, false, ".png" },
    { "/images/auto_mode_start_box_time", true, false, ".png" },
    { "/images/load_bt", true, false, ".png" },
    { "/images/save_bt", true, false, ".png" },
    { "/images/auto_mode_freeze_board", true, false, ".png" },
    { "/images/auto_mode_defrost_board", true, false, ".png" },
    { "/images/auto_mode_fermentation1_board", true, false, ".png" },
    { "/images/auto_mode_fermentation2_board", true, false, ".png" },
    { "/images/defrost_auto_time_box", true, false, ".png" },
    { "/images/keypad", true, false, ".png" }
};
static const preRenderImageInfo_t SettingmodePreRenderImages[] =
{
#if UI_SETTINGMODE_COMBINED_BG_ENABLE
    { "/images/feature-setting", true, false, ".jpg" }
#else
    { "/images/setting_title", true, false, ".png" },
    { "/images/setting_mode_detailsetting", true, false, ".png" },
    { "/images/setting_mode_degree", true, false, ".png" },
    { "/images/setting_mode_record", true, false, ".png" },
    { "/images/setting_mode_test", true, false, ".png" },
    { "/images/setting_mode_time", true, false, ".png" },
    { "/images/setting_mode_language", true, false, ".png" },
    { "/images/exit_bt", true, false, ".png" }
#endif
};
static const preRenderImageInfo_t ManualmodePreRenderImages[] =
{
    { "/images/manualmode_title", true, false, ".png" },
    { "/images/exit_bt", true, false, ".png" },
    { "/images/manual_menu", true, false, ".png" }
};
static const preRenderImageInfo_t MemorymodePreRenderImages[] =
{
    { "/images/memorymode_title", true, false, ".png" },
    { "/images/exit_bt", true, false, ".png" },
    { "/images/memory_title_line", true, false, ".png" },
    { "/images/memory_box", true, false, ".png" },
    { "/images/memory_left", false, false, ".png" },
    { "/images/memory_right", false, false, ".png" },
    { "/images/ok", true, false, ".png" },
    { "/images/delete", true, false, ".png" }
};
static const preRenderImageInfo_t AutodrymodePreRenderImages[] =
{
    { "/images/autodrymode_title", true, false, ".png" },
    { "/images/exit_bt", true, false, ".png" },
    { "/images/auto_dry_circle_basic", false, false, ".png" },
    { "/images/auto_dry_circle_txt", true, true, ".png" },
    { "/images/auto_dry_time_box", true, false, ".png" },
    { "/images/tempbox", true, true, ".png" },
    { "/images/start_bt", true, false, ".png" },
    { "/images/blackout", true, false, ".png" },
    { "/images/keypad", true, false, ".png" }
};
static const preRenderImageInfo_t AutomodestartPreRenderImages[] =
{
    { "/images/automode_title", true, false, ".png" },
    { "/images/stop_bt", true, false, ".png" },
    { "/images/tempbox", true, true, ".png" },
    { "/images/auto_mode_start_box_time", true, false, ".png" },
    { "/images/freeze_gif", false, false, ".png" },
    { "/images/defrost_gif", false, false, ".png" },
    { "/images/fermentation1_gif", false, false, ".png" },
    { "/images/fermentation2_gif", false, false, ".png" },
    { "/images/blackout", true, false, ".png" }
};
static const preRenderImageInfo_t AutomodeendPreRenderImages[] =
{
    { "/images/auto_mode_end_bgi", true, false, ".jpg" },
    { "/images/auto_mode_end_bgi1", true, false, ".jpg" },
    { "/images/automode_title", true, false, ".png" },
    { "/images/stop_bt", true, false, ".png" },
    { "/images/blackout", true, false, ".png" }
};
static const preRenderImageInfo_t ManualmodestartPreRenderImages[] =
{
    { "/images/manualmode_title", true, false, ".png" },
    { "/images/exit_bt", true, false, ".png" },
    { "/images/start_bt", true, false, ".png" },
    { "/images/stop_bt", true, false, ".png" },
    { "/images/tempbox", true, true, ".png" },
    { "/images/manual_freeze_circle_basic", false, false, ".png" },
    { "/images/manual_defrost_circle_basic", false, false, ".png" },
    { "/images/manual_fermentation2_circle_basic", false, false, ".png" },
    { "/images/manual_freeze_circle_txt", true, true, ".png" },
    { "/images/manual_defrost_circle_txt", true, true, ".png" },
    { "/images/manual_fermentation1_circle_txt", true, true, ".png" },
    { "/images/manual_fermentation2_circle_txt", true, true, ".png" },
    { "/images/manual_freeze_gif", false, false, ".png" },
    { "/images/manual_defrost_gif", false, false, ".png" },
    { "/images/manual_fermentation1_gif", false, false, ".png" },
    { "/images/manual_fermentation2_gif", false, false, ".png" },
    { "/images/blackout", true, false, ".png" }
};
static const preRenderImageInfo_t SettingmodedetailsettingPreRenderImages[] =
{
#if UI_SETTINGMODEDETAILSETTING_COMBINED_BG_ENABLE
    { "/images/advancedsetting", true, false, ".jpg" }
#else
    { "/images/detail_title", true, false, ".png" },
    { "/images/exit_bt", true, false, ".png" },
    { "/images/detail_temp_bt", true, false, ".png" },
    { "/images/detail_humidity_bt", true, false, ".png" },
    { "/images/detail_time_bt", true, false, ".png" },
    { "/images/detail_damper_bt", true, false, ".png" },
    { "/images/detail_defrost_bt", true, false, ".png" },
    { "/images/detail_reset_bt", true, false, ".png" }
#endif
};
static const preRenderImageInfo_t SettingmodedegreePreRenderImages[] =
{
    { "/images/symbol_title", true, false, ".png" },
    { "/images/exit_bt", true, false, ".png" },
    { "/images/degree_bt_c_off", false, false, ".png" },
    { "/images/degree_bt_c_on", false, false, ".png" },
    { "/images/degree_bt_f_off", false, false, ".png" },
    { "/images/degree_bt_f_on", false, false, ".png" }
};
static const preRenderImageInfo_t SettingmoderecordPreRenderImages[] =
{
    { "/images/record_title", true, false, ".png" },
    { "/images/deleteall_bt", true, false, ".png" },
    { "/images/exit_bt", true, false, ".png" },
    { "/images/setting_record_chart_title", true, false, ".png" },
    { "/images/setting_record_chart_box", false, true, ".png" }
};
static const preRenderImageInfo_t SettingmodetestPreRenderImages[] =
{
    { "/images/test_title", true, false, ".png" },
    { "/images/exit_bt", true, false, ".png" },
    { "/images/testmode_box", true, false, ".jpg" },
    { "/images/test_comp_off", true, false, ".png" },
    { "/images/test_roomfan_off", true, false, ".png" },
    { "/images/test_fireheater_off", true, false, ".png" },
    { "/images/test_humidityheater_off", true, false, ".png" },
    { "/images/test_water_off", true, false, ".png" },
    { "/images/test_defrost_off", true, false, ".png" },
    { "/images/test_led_off", true, false, ".png" },
    { "/images/test_cabinetheater_off", true, false, ".png" },
    { "/images/test_damper_off", true, false, ".png" },
    { "/images/test_error_check_im", false, false, ".png" }
};
static const preRenderImageInfo_t SettingmodetimePreRenderImages[] =
{
    { "/images/detail_time_title", true, false, ".png" },
    { "/images/exit_bt", true, false, ".png" },
    { "/images/setting_time_date", true, false, ".png" },
    { "/images/setting_time_clock", true, false, ".png" }
};
static const preRenderImageInfo_t SettingmodelanguagePreRenderImages[] =
{
    { "/images/language_title", true, false, ".png" },
    { "/images/exit_bt", true, false, ".png" },
    { "/images/language_korean_off", false, false, ".png" },
    { "/images/language_korean_on", false, false, ".png" },
    { "/images/language_china_off", false, false, ".png" },
    { "/images/language_china_on", false, false, ".png" },
    { "/images/language_english_off", false, false, ".png" },
    { "/images/language_english_on", false, false, ".png" }
};
static const preRenderImageInfo_t SettingmodemanualPreRenderImages[] =
{
    { "/images/exit_bt", true, false, ".png" }
};
static const preRenderImageInfo_t SettingmodedefrostPreRenderImages[] =
{
    { "/images/setting_title", true, false, ".png" },
    { "/images/exit_bt", true, false, ".png" }
};
static const preRenderImageInfo_t DetailsettingtempPreRenderImages[] =
{
    { "/images/detail_temp_title", true, false, ".png" },
    { "/images/exit_bt", true, false, ".png" },
    { "/images/detail_temp_2_off", true, true, ".png" },
    { "/images/detail_temp_3_off", true, true, ".png" },
    { "/images/detail_temp_4_off", true, true, ".png" },
    { "/images/detail_temp_5_off", true, true, ".png" },
    { "/images/left_bt", false, false, ".png" },
    { "/images/right_bt", false, false, ".png" }
};
static const preRenderImageInfo_t DetailsettinghumidityPreRenderImages[] =
{
    { "/images/detail_humidity_title", true, false, ".png" },
    { "/images/exit_bt", true, false, ".png" },
    { "/images/detail_humidity_1_off", true, false, ".png" },
    { "/images/detail_humidity_2_off", true, false, ".png" },
    { "/images/detail_humidity_3_off", true, false, ".png" }
};
static const preRenderImageInfo_t DetailsettingtimePreRenderImages[] =
{
    { "/images/detail_time_title", true, false, ".png" },
    { "/images/exit_bt", true, false, ".png" },
    { "/images/detail_time_1_off", true, false, ".png" },
    { "/images/detail_time_2_off", true, false, ".png" },
    { "/images/detail_time_3_off", true, false, ".png" },
    { "/images/detail_time_4_off", true, false, ".png" },
    { "/images/detail_time_5_off", true, false, ".png" }
};
static const preRenderImageInfo_t DetailsettingdamperPreRenderImages[] =
{
    { "/images/detail_damper_title", true, false, ".png" },
    { "/images/exit_bt", true, false, ".png" }
};
static const preRenderImageInfo_t DetailsettingdefrostPreRenderImages[] =
{
    { "/images/detail_defrost_title", true, false, ".png" },
    { "/images/exit_bt", true, false, ".png" },
    { "/images/detail_defrost_1_off", true, false, ".png" },
    { "/images/detail_defrost_2_off", true, true, ".png" },
    { "/images/detail_defrost_3_off", true, false, ".png" }
};
// NOTE : preRenderTarget에 적어둔 순서대로 preRendering이 진행, 자주 쓰이는 페이지를 앞쪽에 배치하는 것이 좋음. (ex: PAGE_MAIN, PAGE_AUTOMODE, PAGE_MANUALMODE 등)
const preRendererPageConfig_t preRenderPageConfig[PAGE_COUNT] =
{
    [PAGE_MAIN] =
    {
        .pageId = PAGE_MAIN,
        .init_func = init_page_main,
        .init_func_with_step = init_page_main_with_step,
        .deinit_func = destroy_page_main,
        .preRenderTargetPages = (const pageId_t[])
        {
            PAGE_AUTOMODE,
            PAGE_MANUALMODE,
            PAGE_AUTODRYMODE,
            PAGE_MEMORYMODE,
            PAGE_SETTINGMODE
        },
        .preRenderTargetPageCount = 5,
        .backgroundImageAssetId = SHARED_IMAGE_ASSET_BG_DOUGH,
        .preRenderImageCount = ARRAY_COUNT(MainPreRenderImages),
        .preRenderImageInfo = MainPreRenderImages
    },

    [PAGE_AUTOMODE] =
    {
        .pageId = PAGE_AUTOMODE,
        .init_func = init_page_automode,
        .init_func_with_step = init_page_automode_with_step,
        .deinit_func = destroy_page_automode,
        .preRenderTargetPages = (const pageId_t[])
        {
            PAGE_MAIN,
            PAGE_AUTOMODESTART,
            PAGE_MEMORYMODE
        },
        .preRenderTargetPageCount = 3,
        .backgroundImageAssetId = SHARED_IMAGE_NONE,
        .preRenderImageCount = ARRAY_COUNT(AutomodePreRenderImages),
        .preRenderImageInfo = AutomodePreRenderImages
    },

    [PAGE_AUTOMODESTART] =
    {
        .pageId = PAGE_AUTOMODESTART,
        .init_func = init_page_automodestart,
        .init_func_with_step = init_page_automodestart_with_step,
        .deinit_func = destroy_page_automodestart,
        .preRenderTargetPages = (const pageId_t[])
        {
            PAGE_AUTOMODEEND,
            PAGE_AUTOMODE
        },
        .preRenderTargetPageCount = 2,
        .backgroundImageAssetId = SHARED_IMAGE_ASSET_BG_AUTOMODESTART,
        .preRenderImageCount = ARRAY_COUNT(AutomodestartPreRenderImages),
        .preRenderImageInfo = AutomodestartPreRenderImages
    },

    [PAGE_AUTOMODEEND] =
    {
        .pageId = PAGE_AUTOMODEEND,
        .init_func = init_page_automodeend,
        .init_func_with_step = init_page_automodeend_with_step,
        .deinit_func = destroy_page_automodeend,
        .preRenderTargetPages = (const pageId_t[])
        {
            PAGE_MANUALMODESTART,
            PAGE_AUTOMODE
        },
        .preRenderTargetPageCount = 2,
        .preRenderImageCount = ARRAY_COUNT(AutomodeendPreRenderImages),
        .preRenderImageInfo = AutomodeendPreRenderImages
    },

    [PAGE_MANUALMODE] =
    {
        .pageId = PAGE_MANUALMODE,
        .init_func = init_page_manualmode,
        .init_func_with_step = init_page_manualmode_with_step,
        .deinit_func = destroy_page_manualmode,
        .preRenderTargetPages = (const pageId_t[])
        {
            PAGE_MAIN,
            PAGE_MANUALMODESTART
        },
        .preRenderTargetPageCount = 2,
        .preRenderImageCount = ARRAY_COUNT(ManualmodePreRenderImages),
        .preRenderImageInfo = ManualmodePreRenderImages
    },

    [PAGE_MANUALMODESTART] =
    {
        .pageId = PAGE_MANUALMODESTART,
        .init_func = init_page_manualmodestart,
        .init_func_with_step = init_page_manualmodestart_with_step,
        .deinit_func = destroy_page_manualmodestart,
        .preRenderTargetPages = (const pageId_t[])
        {
            PAGE_AUTOMODE,
            PAGE_MANUALMODE
        },
        .preRenderTargetPageCount = 2,
        .preRenderImageCount = ARRAY_COUNT(ManualmodestartPreRenderImages),
        .preRenderImageInfo = ManualmodestartPreRenderImages
    },

    [PAGE_AUTODRYMODE] =
    {
        .pageId = PAGE_AUTODRYMODE,
        .init_func = init_page_autodrymode,
        .init_func_with_step = init_page_autodrymode_with_step,
        .deinit_func = destroy_page_autodrymode,
        .preRenderTargetPages = (const pageId_t[])
        {
            PAGE_MAIN
        },
        .preRenderTargetPageCount = 1,
        .preRenderImageCount = ARRAY_COUNT(AutodrymodePreRenderImages),
        .preRenderImageInfo = AutodrymodePreRenderImages
    },

    [PAGE_MEMORYMODE] =
    {
        .pageId = PAGE_MEMORYMODE,
        .init_func = init_page_memorymode,
        .init_func_with_step = init_page_memorymode_with_step,
        .deinit_func = destroy_page_memorymode,
        .preRenderTargetPages = (const pageId_t[])
        {
            PAGE_AUTOMODE,
            PAGE_MAIN
        },
        .preRenderTargetPageCount = 2,
        .preRenderImageCount = ARRAY_COUNT(MemorymodePreRenderImages),
        .preRenderImageInfo = MemorymodePreRenderImages
    },

    [PAGE_SETTINGMODE] =
    {
        .pageId = PAGE_SETTINGMODE,
        .init_func = init_page_settingmode,
        .init_func_with_step = init_page_settingmode_with_step,
        .deinit_func = destroy_page_settingmode,
        .preRenderTargetPages = (const pageId_t[])
        {
            PAGE_SETTINGMODEDETAILSETTING,
            PAGE_SETTINGMODEDEGREE,
            PAGE_SETTINGMODERECORD,
            PAGE_SETTINGMODETEST,
            PAGE_SETTINGMODETIME,
            PAGE_SETTINGMODELANGUAGE,
            PAGE_MAIN
        },
        .preRenderTargetPageCount = 7,
#if !UI_SETTINGMODE_COMBINED_BG_ENABLE
        .backgroundImageAssetId = SHARED_IMAGE_ASSET_BG_DOUGH,
#endif
        .preRenderImageCount = ARRAY_COUNT(SettingmodePreRenderImages),
        .preRenderImageInfo = SettingmodePreRenderImages
    },

    [PAGE_SETTINGMODEDETAILSETTING] =
    {
        .pageId = PAGE_SETTINGMODEDETAILSETTING,
        .init_func = init_page_settingmodedetailsetting,
        .init_func_with_step = init_page_settingmodedetailsetting_with_step,
        .deinit_func = destroy_page_settingmodedetailsetting,
        .preRenderTargetPages = (const pageId_t[])
        {
            PAGE_SETTINGMODE,
            PAGE_DETAILSETTINGTEMP,
            PAGE_DETAILSETTINGHUMIDITY,
            PAGE_DETAILSETTINGTIME,
            PAGE_DETAILSETTINGDAMPER,
            PAGE_DETAILSETTINGDEFROST
        },
        .preRenderTargetPageCount = 6,
#if !UI_SETTINGMODEDETAILSETTING_COMBINED_BG_ENABLE
        .backgroundImageAssetId = SHARED_IMAGE_ASSET_BG_DOUGH,
#endif
        .preRenderImageCount = ARRAY_COUNT(SettingmodedetailsettingPreRenderImages),
        .preRenderImageInfo = SettingmodedetailsettingPreRenderImages
    },

    [PAGE_SETTINGMODEDEGREE] =
    {
        .pageId = PAGE_SETTINGMODEDEGREE,
        .init_func = init_page_settingmodedegree,
        .init_func_with_step = init_page_settingmodedegree_with_step,
        .deinit_func = destroy_page_settingmodedegree,
        .preRenderTargetPages = (const pageId_t[])
        {
            PAGE_SETTINGMODE
        },
        .preRenderTargetPageCount = 1,
        .preRenderImageCount = ARRAY_COUNT(SettingmodedegreePreRenderImages),
        .preRenderImageInfo = SettingmodedegreePreRenderImages
    },

    [PAGE_SETTINGMODERECORD] =
    {
        .pageId = PAGE_SETTINGMODERECORD,
        .init_func = init_page_settingmoderecord,
        .init_func_with_step = init_page_settingmoderecord_with_step,
        .deinit_func = destroy_page_settingmoderecord,
        .preRenderTargetPages = (const pageId_t[])
        {
            PAGE_SETTINGMODE
        },
        .preRenderTargetPageCount = 1,
        .preRenderImageCount = ARRAY_COUNT(SettingmoderecordPreRenderImages),
        .preRenderImageInfo = SettingmoderecordPreRenderImages
    },

    [PAGE_SETTINGMODETEST] =
    {
        .pageId = PAGE_SETTINGMODETEST,
        .init_func = init_page_settingmodetest,
        .init_func_with_step = init_page_settingmodetest_with_step,
        .deinit_func = destroy_page_settingmodetest,
        .preRenderTargetPages = (const pageId_t[])
        {
            PAGE_SETTINGMODE
        },
        .preRenderTargetPageCount = 1,
        .preRenderImageCount = ARRAY_COUNT(SettingmodetestPreRenderImages),
        .preRenderImageInfo = SettingmodetestPreRenderImages
    },

    [PAGE_SETTINGMODETIME] =
    {
        .pageId = PAGE_SETTINGMODETIME,
        .init_func = init_page_settingmodetime,
        .init_func_with_step = init_page_settingmodetime_with_step,
        .deinit_func = destroy_page_settingmodetime,
        .preRenderTargetPages = (const pageId_t[])
        {
            PAGE_SETTINGMODE
        },
        .preRenderTargetPageCount = 1,
        .preRenderImageCount = ARRAY_COUNT(SettingmodetimePreRenderImages),
        .preRenderImageInfo = SettingmodetimePreRenderImages
    },

    [PAGE_SETTINGMODELANGUAGE] =
    {
        .pageId = PAGE_SETTINGMODELANGUAGE,
        .init_func = init_page_settingmodelanguage,
        .init_func_with_step = init_page_settingmodelanguage_with_step,
        .deinit_func = destroy_page_settingmodelanguage,
        .preRenderTargetPages = (const pageId_t[])
        {
            PAGE_SETTINGMODE
        },
        .preRenderTargetPageCount = 1,
        .preRenderImageCount = ARRAY_COUNT(SettingmodelanguagePreRenderImages),
        .preRenderImageInfo = SettingmodelanguagePreRenderImages
    },

    [PAGE_SETTINGMODEMANUAL] =
    {
        .pageId = PAGE_SETTINGMODEMANUAL,
        .init_func = init_page_settingmodemanual,
        .init_func_with_step = init_page_settingmodemanual_with_step,
        .deinit_func = destroy_page_settingmodemanual,
        .preRenderTargetPages = (const pageId_t[])
        {
            PAGE_SETTINGMODE
        },
        .preRenderTargetPageCount = 1,
        .backgroundImageAssetId = SHARED_IMAGE_ASSET_BG_DOUGH,
        .preRenderImageCount = ARRAY_COUNT(SettingmodemanualPreRenderImages),
        .preRenderImageInfo = SettingmodemanualPreRenderImages
    },

    [PAGE_SETTINGMODEDEFROST] =
    {
        .pageId = PAGE_SETTINGMODEDEFROST,
        .init_func = init_page_settingmodedefrost,
        .init_func_with_step = init_page_settingmodedefrost_with_step,
        .deinit_func = destroy_page_settingmodedefrost,
        .preRenderTargetPages = (const pageId_t[])
        {
            PAGE_SETTINGMODE
        },
        .preRenderTargetPageCount = 1,
        .backgroundImageAssetId = SHARED_IMAGE_ASSET_BG_DOUGH,
        .preRenderImageCount = ARRAY_COUNT(SettingmodedefrostPreRenderImages),
        .preRenderImageInfo = SettingmodedefrostPreRenderImages
    },

    [PAGE_DETAILSETTINGTEMP] =
    {
        .pageId = PAGE_DETAILSETTINGTEMP,
        .init_func = init_page_detailsettingtemp,
        .init_func_with_step = init_page_detailsettingtemp_with_step,
        .deinit_func = destroy_page_detailsettingtemp,
        .preRenderTargetPages = (const pageId_t[])
        {
            PAGE_SETTINGMODEDETAILSETTING
        },
        .preRenderTargetPageCount = 1,
        .preRenderImageCount = ARRAY_COUNT(DetailsettingtempPreRenderImages),
        .preRenderImageInfo = DetailsettingtempPreRenderImages
    },

    [PAGE_DETAILSETTINGHUMIDITY] =
    {
        .pageId = PAGE_DETAILSETTINGHUMIDITY,
        .init_func = init_page_detailsettinghumidity,
        .init_func_with_step = init_page_detailsettinghumidity_with_step,
        .deinit_func = destroy_page_detailsettinghumidity,
        .preRenderTargetPages = (const pageId_t[])
        {
            PAGE_SETTINGMODEDETAILSETTING
        },
        .preRenderTargetPageCount = 1,
        .preRenderImageCount = ARRAY_COUNT(DetailsettinghumidityPreRenderImages),
        .preRenderImageInfo = DetailsettinghumidityPreRenderImages
    },

    [PAGE_DETAILSETTINGTIME] =
    {
        .pageId = PAGE_DETAILSETTINGTIME,
        .init_func = init_page_detailsettingtime,
        .init_func_with_step = init_page_detailsettingtime_with_step,
        .deinit_func = destroy_page_detailsettingtime,
        .preRenderTargetPages = (const pageId_t[])
        {
            PAGE_SETTINGMODEDETAILSETTING
        },
        .preRenderTargetPageCount = 1,
        .preRenderImageCount = ARRAY_COUNT(DetailsettingtimePreRenderImages),
        .preRenderImageInfo = DetailsettingtimePreRenderImages
    },

    [PAGE_DETAILSETTINGDAMPER] =
    {
        .pageId = PAGE_DETAILSETTINGDAMPER,
        .init_func = init_page_detailsettingdamper,
        .init_func_with_step = init_page_detailsettingdamper_with_step,
        .deinit_func = destroy_page_detailsettingdamper,
        .preRenderTargetPages = (const pageId_t[])
        {
            PAGE_SETTINGMODEDETAILSETTING
        },
        .preRenderTargetPageCount = 1,
        .preRenderImageCount = ARRAY_COUNT(DetailsettingdamperPreRenderImages),
        .preRenderImageInfo = DetailsettingdamperPreRenderImages
    },

    [PAGE_DETAILSETTINGDEFROST] =
    {
        .pageId = PAGE_DETAILSETTINGDEFROST,
        .init_func = init_page_detailsettingdefrost,
        .init_func_with_step = init_page_detailsettingdefrost_with_step,
        .deinit_func = destroy_page_detailsettingdefrost,
        .preRenderTargetPages = (const pageId_t[])
        {
            PAGE_SETTINGMODEDETAILSETTING
        },
        .preRenderTargetPageCount = 1,
        .preRenderImageCount = ARRAY_COUNT(DetailsettingdefrostPreRenderImages),
        .preRenderImageInfo = DetailsettingdefrostPreRenderImages
    },

    [PAGE_NEUROSYS] =
    {
        .pageId = PAGE_NEUROSYS,
        .init_func = init_page_neurosys,
        .init_func_with_step = init_page_neurosys_with_step,
        .deinit_func = destroy_page_neurosys,
        .preRenderTargetPages = NULL,
        .preRenderTargetPageCount = 0
    }
};

preRendererPageState_t preRenderPageState[PAGE_COUNT] =
{
    [PAGE_MAIN] =
    {
        .page = &bk_lv_tool_ui.main,
        .config = &preRenderPageConfig[PAGE_MAIN],
    },

    [PAGE_AUTOMODE] =
    {
        .page = &bk_lv_tool_ui.automode,
        .config = &preRenderPageConfig[PAGE_AUTOMODE],
    },

    [PAGE_AUTOMODESTART] =
    {
        .page = &bk_lv_tool_ui.automodestart,
        .config = &preRenderPageConfig[PAGE_AUTOMODESTART],
    },

    [PAGE_AUTOMODEEND] =
    {
        .page = &bk_lv_tool_ui.automodeend,
        .config = &preRenderPageConfig[PAGE_AUTOMODEEND],
    },

    [PAGE_MANUALMODE] =
    {
        .page = &bk_lv_tool_ui.manualmode,
        .config = &preRenderPageConfig[PAGE_MANUALMODE],
    },

    [PAGE_MANUALMODESTART] =
    {
        .page = &bk_lv_tool_ui.manualmodestart,
        .config = &preRenderPageConfig[PAGE_MANUALMODESTART],
    },

    [PAGE_AUTODRYMODE] =
    {
        .page = &bk_lv_tool_ui.autodrymode,
        .config = &preRenderPageConfig[PAGE_AUTODRYMODE],
    },

    [PAGE_MEMORYMODE] =
    {
        .page = &bk_lv_tool_ui.memorymode,
        .config = &preRenderPageConfig[PAGE_MEMORYMODE],
    },

    [PAGE_SETTINGMODE] =
    {
        .page = &bk_lv_tool_ui.settingmode,
        .config = &preRenderPageConfig[PAGE_SETTINGMODE],
    },

    [PAGE_SETTINGMODEDETAILSETTING] =
    {
        .page = &bk_lv_tool_ui.settingmodedetailsetting,
        .config = &preRenderPageConfig[PAGE_SETTINGMODEDETAILSETTING],
    },

    [PAGE_SETTINGMODEDEGREE] =
    {
        .page = &bk_lv_tool_ui.settingmodedegree,
        .config = &preRenderPageConfig[PAGE_SETTINGMODEDEGREE],
    },

    [PAGE_SETTINGMODERECORD] =
    {
        .page = &bk_lv_tool_ui.settingmoderecord,
        .config = &preRenderPageConfig[PAGE_SETTINGMODERECORD],
    },

    [PAGE_SETTINGMODETEST] =
    {
        .page = &bk_lv_tool_ui.settingmodetest,
        .config = &preRenderPageConfig[PAGE_SETTINGMODETEST],
    },

    [PAGE_SETTINGMODETIME] =
    {
        .page = &bk_lv_tool_ui.settingmodetime,
        .config = &preRenderPageConfig[PAGE_SETTINGMODETIME],
    },

    [PAGE_SETTINGMODELANGUAGE] =
    {
        .page = &bk_lv_tool_ui.settingmodelanguage,
        .config = &preRenderPageConfig[PAGE_SETTINGMODELANGUAGE],
    },

    [PAGE_SETTINGMODEMANUAL] =
    {
        .page = &bk_lv_tool_ui.settingmodemanual,
        .config = &preRenderPageConfig[PAGE_SETTINGMODEMANUAL],
    },

    [PAGE_SETTINGMODEDEFROST] =
    {
        .page = &bk_lv_tool_ui.settingmodedefrost,
        .config = &preRenderPageConfig[PAGE_SETTINGMODEDEFROST],
    },

    [PAGE_DETAILSETTINGTEMP] =
    {
        .page = &bk_lv_tool_ui.detailsettingtemp,
        .config = &preRenderPageConfig[PAGE_DETAILSETTINGTEMP],
    },

    [PAGE_DETAILSETTINGHUMIDITY] =
    {
        .page = &bk_lv_tool_ui.detailsettinghumidity,
        .config = &preRenderPageConfig[PAGE_DETAILSETTINGHUMIDITY],
    },

    [PAGE_DETAILSETTINGTIME] =
    {
        .page = &bk_lv_tool_ui.detailsettingtime,
        .config = &preRenderPageConfig[PAGE_DETAILSETTINGTIME],
    },

    [PAGE_DETAILSETTINGDAMPER] =
    {
        .page = &bk_lv_tool_ui.detailsettingdamper,
        .config = &preRenderPageConfig[PAGE_DETAILSETTINGDAMPER],
    },

    [PAGE_DETAILSETTINGDEFROST] =
    {
        .page = &bk_lv_tool_ui.detailsettingdefrost,
        .config = &preRenderPageConfig[PAGE_DETAILSETTINGDEFROST],
    },

    [PAGE_NEUROSYS] =
    {
        .page = &bk_lv_tool_ui.neurosys,
        .config = &preRenderPageConfig[PAGE_NEUROSYS],
    },
};

pageLifecycleFuncWithStep_t getPageInitFunc(pageId_t pageId)
{
    bk_printf(TAG "[SCREEN] getPageInitFunc(%d)\n", pageId);
    if(pageId >= 0 && pageId < PAGE_COUNT)
    {
        bk_printf(TAG "[SCREEN] getPageInitFunc was %d for pageId %d\n", preRenderPageConfig[pageId].init_func != NULL, pageId);
        return preRenderPageConfig[pageId].init_func_with_step;
    }
    return NULL;
}

pageLifecycleFunc_t getPageDeinitFunc(pageId_t pageId)
{
    bk_printf(TAG "[SCREEN] getPageDeinitFunc(%d)\n", pageId);
    if(pageId >= 0 && pageId < PAGE_COUNT)
    {
        bk_printf(TAG "[SCREEN] getPageDeinitFunc was %d for pageId %d\n", preRenderPageConfig[pageId].deinit_func != NULL, pageId);
        return preRenderPageConfig[pageId].deinit_func;
    }
    return NULL;
}

rendererFuncStatus_t init_shared_image_asset()
{
    for(int i = 0 ; i < SHARED_IMAGE_COUNT ; i++)
    {
        if(i == SHARED_IMAGE_NONE)
        {
            continue;
        }
        sharedImageAssetState[i].imageInfo = &sharedImageAssetInfo[i];
        sharedImageAssetState[i].imageBuffer = NULL;
    }
    for(int i = 0 ; i < SHARED_IMAGE_COUNT ; i++)
    {
        if(i == SHARED_IMAGE_NONE)
        {
            continue;
        }
        bool hasLanguageVariant = sharedImageAssetInfo[i].hasLanguageVariant;
        bool hasDegreeVariant = sharedImageAssetInfo[i].hasDegreeVariant;
        const char *extension = sharedImageAssetInfo[i].fileExtension != NULL ?
                                sharedImageAssetInfo[i].fileExtension : ".png";
        const char *degreeSuffix = hasDegreeVariant &&
                                   strcmp(settings_get_str("Degree"), "\xc2\xb0""F") == 0 ? "_f" : "";
        char variantFilePath[128];
        lv_draw_buf_t *imageBuffer;
        if(hasLanguageVariant)
        {
            const char *languageSuffix = settings_get_int("LANGUAGE") == 1 ? "_china" :
                                         settings_get_int("LANGUAGE") == 2 ? "_english" : "";
            snprintf(variantFilePath, sizeof(variantFilePath), "%s%s%s%s",
                     sharedImageAssetInfo[i].imagePath, degreeSuffix, languageSuffix, extension);
            bk_printf(TAG "[SHARED_IMAGE] init_shared_image_asset: Loading image for assetId %d with language variant: [%s]\n", i, variantFilePath);
        }
        else
        {
            snprintf(variantFilePath, sizeof(variantFilePath), "%s%s%s",
                     sharedImageAssetInfo[i].imagePath, degreeSuffix, extension);
            bk_printf(TAG "[SHARED_IMAGE] init_shared_image_asset: Loading image for assetId %d without language variant: [%s]\n", i, variantFilePath);
        }

        imageBuffer = lv_image_decoder_prewarm_to_buffer(variantFilePath);
        if(!imageBuffer)
        {
            bk_printf(TAG "[SHARED_IMAGE] init_shared_image_asset: Failed to load image for assetId %d from path: %s\n", i, variantFilePath);
            goto failed;
        }
        sharedImageAssetState[i].imageBuffer = imageBuffer;
        bk_printf(TAG "[SHARED_IMAGE] init_shared_image_asset: Successfully loaded image for assetId %d from path: %s\n", i, variantFilePath);
    }
    return RENDERER_FUNC_DONE;

    failed:
    return RENDERER_FUNC_FAILED;
}

rendererFuncStatus_t set_shared_image_asset(lv_obj_t *obj, sharedImageAssetId_t assetId)
{
    if(obj == NULL || !lv_obj_is_valid(obj))
    {
        bk_printf(TAG "[SHARED_IMAGE] set_shared_image_asset: Invalid image object\n");
        return RENDERER_FUNC_FAILED;
    }
    if(assetId <= SHARED_IMAGE_NONE || assetId >= SHARED_IMAGE_COUNT)
    {
        bk_printf(TAG "[SHARED_IMAGE] set_shared_image_asset: Invalid assetId %d\n", assetId);
        return RENDERER_FUNC_FAILED;
    }

    preRendererImageState_t *assetState = &sharedImageAssetState[assetId];
    if(!assetState->imageBuffer)
    {
        bk_printf(TAG "[SHARED_IMAGE] set_shared_image_asset: No image buffer for assetId %d\n", assetId);
        return RENDERER_FUNC_FAILED;
    }

    lv_image_set_src(obj, assetState->imageBuffer);
    bk_printf(TAG "[SHARED_IMAGE] set_shared_image_asset: Set image for assetId %d on object %p\n", assetId, obj);

    return RENDERER_FUNC_DONE;
}

rendererFuncStatus_t refresh_shared_image_asset()
{
    for(int i = SHARED_IMAGE_NONE + 1; i < SHARED_IMAGE_COUNT; i++)
    {
        bool hasLanguageVariant = sharedImageAssetState[i].imageInfo->hasLanguageVariant;
        if(hasLanguageVariant)
        {
            // TODO : reload image for the current language variant
            bk_printf(TAG "[SHARED_IMAGE] refresh_shared_image_asset: Reloaded image for assetId %d with language variant\n", i);
        }
    }
    return RENDERER_FUNC_DONE;
}
