#include "preRenderer.h"
#include "preRenderInfo.h"

#define TAG "[preRenderInfo.c] "
#define bk_printf(fmt, ...) do {if(0) bk_printf(fmt, ##__VA_ARGS__); } while(0) // disable printf

//extern Rendering Page Init / Deinit functions
extern bk_lv_ui_t bk_lv_tool_ui;

extern void init_page_main(bk_lv_ui_t *bk_ui);
extern void init_page_main_with_step(bk_lv_ui_t *bk_ui);
extern void destroy_page_main(bk_lv_ui_t *bk_ui);
extern void init_page_automode(bk_lv_ui_t *bk_ui);
extern void init_page_automode_with_step(bk_lv_ui_t *bk_ui);
extern void destroy_page_automode(bk_lv_ui_t *bk_ui);
extern void init_page_automodestart(bk_lv_ui_t *bk_ui);
extern void init_page_automodestart_with_step(bk_lv_ui_t *bk_ui);
extern void destroy_page_automodestart(bk_lv_ui_t *bk_ui);
extern void init_page_automodeend(bk_lv_ui_t *bk_ui);
extern void init_page_automodeend_with_step(bk_lv_ui_t *bk_ui);
extern void destroy_page_automodeend(bk_lv_ui_t *bk_ui);
extern void init_page_manualmode(bk_lv_ui_t *bk_ui);
extern void init_page_manualmode_with_step(bk_lv_ui_t *bk_ui);
extern void destroy_page_manualmode(bk_lv_ui_t *bk_ui);
extern void init_page_manualmodestart(bk_lv_ui_t *bk_ui);
extern void init_page_manualmodestart_with_step(bk_lv_ui_t *bk_ui);
extern void destroy_page_manualmodestart(bk_lv_ui_t *bk_ui);
extern void init_page_autodrymode(bk_lv_ui_t *bk_ui);
extern void init_page_autodrymode_with_step(bk_lv_ui_t *bk_ui);
extern void destroy_page_autodrymode(bk_lv_ui_t *bk_ui);
extern void init_page_memorymode(bk_lv_ui_t *bk_ui);
extern void init_page_memorymode_with_step(bk_lv_ui_t *bk_ui);
extern void destroy_page_memorymode(bk_lv_ui_t *bk_ui);
extern void init_page_settingmode(bk_lv_ui_t *bk_ui);
extern void init_page_settingmode_with_step(bk_lv_ui_t *bk_ui);
extern void destroy_page_settingmode(bk_lv_ui_t *bk_ui);
extern void init_page_settingmodedetailsetting(bk_lv_ui_t *bk_ui);
extern void init_page_settingmodedetailsetting_with_step(bk_lv_ui_t *bk_ui);
extern void destroy_page_settingmodedetailsetting(bk_lv_ui_t *bk_ui);
extern void init_page_settingmodedegree(bk_lv_ui_t *bk_ui);
extern void init_page_settingmodedegree_with_step(bk_lv_ui_t *bk_ui);
extern void destroy_page_settingmodedegree(bk_lv_ui_t *bk_ui);
extern void init_page_settingmoderecord(bk_lv_ui_t *bk_ui);
extern void init_page_settingmoderecord_with_step(bk_lv_ui_t *bk_ui);
extern void destroy_page_settingmoderecord(bk_lv_ui_t *bk_ui);
extern void init_page_settingmodetest(bk_lv_ui_t *bk_ui);
extern void init_page_settingmodetest_with_step(bk_lv_ui_t *bk_ui);
extern void destroy_page_settingmodetest(bk_lv_ui_t *bk_ui);
extern void init_page_settingmodetime(bk_lv_ui_t *bk_ui);
extern void init_page_settingmodetime_with_step(bk_lv_ui_t *bk_ui);
extern void destroy_page_settingmodetime(bk_lv_ui_t *bk_ui);
extern void init_page_settingmodelanguage(bk_lv_ui_t *bk_ui);
extern void init_page_settingmodelanguage_with_step(bk_lv_ui_t *bk_ui);
extern void destroy_page_settingmodelanguage(bk_lv_ui_t *bk_ui);
extern void init_page_settingmodemanual(bk_lv_ui_t *bk_ui);
extern void init_page_settingmodemanual_with_step(bk_lv_ui_t *bk_ui);
extern void destroy_page_settingmodemanual(bk_lv_ui_t *bk_ui);
extern void init_page_settingmodedefrost(bk_lv_ui_t *bk_ui);
extern void init_page_settingmodedefrost_with_step(bk_lv_ui_t *bk_ui);
extern void destroy_page_settingmodedefrost(bk_lv_ui_t *bk_ui);
extern void init_page_detailsettingtemp(bk_lv_ui_t *bk_ui);
extern void init_page_detailsettingtemp_with_step(bk_lv_ui_t *bk_ui);
extern void destroy_page_detailsettingtemp(bk_lv_ui_t *bk_ui);
extern void init_page_detailsettinghumidity(bk_lv_ui_t *bk_ui);
extern void init_page_detailsettinghumidity_with_step(bk_lv_ui_t *bk_ui);
extern void destroy_page_detailsettinghumidity(bk_lv_ui_t *bk_ui);
extern void init_page_detailsettingtime(bk_lv_ui_t *bk_ui);
extern void init_page_detailsettingtime_with_step(bk_lv_ui_t *bk_ui);
extern void destroy_page_detailsettingtime(bk_lv_ui_t *bk_ui);
extern void init_page_detailsettingdamper(bk_lv_ui_t *bk_ui);
extern void init_page_detailsettingdamper_with_step(bk_lv_ui_t *bk_ui);
extern void destroy_page_detailsettingdamper(bk_lv_ui_t *bk_ui);
extern void init_page_detailsettingdefrost(bk_lv_ui_t *bk_ui);
extern void init_page_detailsettingdefrost_with_step(bk_lv_ui_t *bk_ui);
extern void destroy_page_detailsettingdefrost(bk_lv_ui_t *bk_ui);
extern void init_page_neurosys(bk_lv_ui_t *bk_ui);
extern void destroy_page_neurosys(bk_lv_ui_t *bk_ui);
//=============================================
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
        .preRenderTargetPageCount = 5
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
        .preRenderTargetPageCount = 3
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
        .preRenderTargetPageCount = 2
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
        .preRenderTargetPageCount = 2
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
        .preRenderTargetPageCount = 2
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
        .preRenderTargetPageCount = 2
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
        .preRenderTargetPageCount = 1
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
        .preRenderTargetPageCount = 2
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
        .preRenderTargetPageCount = 7
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
        .preRenderTargetPageCount = 6
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
        .preRenderTargetPageCount = 1
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
        .preRenderTargetPageCount = 1
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
        .preRenderTargetPageCount = 1
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
        .preRenderTargetPageCount = 1
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
        .preRenderTargetPageCount = 1
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
        .preRenderTargetPageCount = 1
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
        .preRenderTargetPageCount = 1
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
        .preRenderTargetPageCount = 1
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
        .preRenderTargetPageCount = 1
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
        .preRenderTargetPageCount = 1
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
        .preRenderTargetPageCount = 1
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
        .preRenderTargetPageCount = 1
    },

    [PAGE_NEUROSYS] =
    {
        .pageId = PAGE_NEUROSYS,
        .init_func = init_page_neurosys,
        .init_func_with_step = NULL,
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

pageLifecycleFunc_t getPageInitFunc(pageId_t pageId)
{
    bk_printf(TAG "[SCREEN] getPageInitFunc(%d)\n", pageId);
    if(pageId >= 0 && pageId < PAGE_COUNT)
    {
        bk_printf(TAG "[SCREEN] getPageInitFunc was %d for pageId %d\n", preRenderPageConfig[pageId].init_func != NULL, pageId);
        return preRenderPageConfig[pageId].init_func;
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
