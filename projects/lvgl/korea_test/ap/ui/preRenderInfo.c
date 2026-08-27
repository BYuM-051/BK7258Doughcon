#include "preRenderer.h"
#include "preRenderInfo.h"

#define TAG "[preRenderInfo.c] "
// #define bk_printf(...) do {} while(0) // disable printf

// extern Rendering Page Init / Deinit functions
extern void init_page_main(bk_lv_ui_t *bk_ui);
extern void destroy_page_main(bk_lv_ui_t *bk_ui);
extern void init_page_automode(bk_lv_ui_t *bk_ui);
extern void destroy_page_automode(bk_lv_ui_t *bk_ui);
extern void init_page_automodestart(bk_lv_ui_t *bk_ui);
extern void destroy_page_automodestart(bk_lv_ui_t *bk_ui);
extern void init_page_automodeend(bk_lv_ui_t *bk_ui);
extern void destroy_page_automodeend(bk_lv_ui_t *bk_ui);
extern void init_page_manualmode(bk_lv_ui_t *bk_ui);
extern void destroy_page_manualmode(bk_lv_ui_t *bk_ui);
extern void init_page_manualmodestart(bk_lv_ui_t *bk_ui);
extern void destroy_page_manualmodestart(bk_lv_ui_t *bk_ui);
extern void init_page_autodrymode(bk_lv_ui_t *bk_ui);
extern void destroy_page_autodrymode(bk_lv_ui_t *bk_ui);
extern void init_page_memorymode(bk_lv_ui_t *bk_ui);
extern void destroy_page_memorymode(bk_lv_ui_t *bk_ui);
extern void init_page_settingmode(bk_lv_ui_t *bk_ui);
extern void destroy_page_settingmode(bk_lv_ui_t *bk_ui);
extern void init_page_settingmodedetailsetting(bk_lv_ui_t *bk_ui);
extern void destroy_page_settingmodedetailsetting(bk_lv_ui_t *bk_ui);
extern void init_page_settingmodedegree(bk_lv_ui_t *bk_ui);
extern void destroy_page_settingmodedegree(bk_lv_ui_t *bk_ui);
extern void init_page_settingmoderecord(bk_lv_ui_t *bk_ui);
extern void destroy_page_settingmoderecord(bk_lv_ui_t *bk_ui);
extern void init_page_settingmodetest(bk_lv_ui_t *bk_ui);
extern void destroy_page_settingmodetest(bk_lv_ui_t *bk_ui);
extern void init_page_settingmodetime(bk_lv_ui_t *bk_ui);
extern void destroy_page_settingmodetime(bk_lv_ui_t *bk_ui);
extern void init_page_settingmodelanguage(bk_lv_ui_t *bk_ui);
extern void destroy_page_settingmodelanguage(bk_lv_ui_t *bk_ui);
extern void init_page_settingmodemanual(bk_lv_ui_t *bk_ui);
extern void destroy_page_settingmodemanual(bk_lv_ui_t *bk_ui);
extern void init_page_settingmodedefrost(bk_lv_ui_t *bk_ui);
extern void destroy_page_settingmodedefrost(bk_lv_ui_t *bk_ui);
extern void init_page_detailsettingtemp(bk_lv_ui_t *bk_ui);
extern void destroy_page_detailsettingtemp(bk_lv_ui_t *bk_ui);
extern void init_page_detailsettinghumidity(bk_lv_ui_t *bk_ui);
extern void destroy_page_detailsettinghumidity(bk_lv_ui_t *bk_ui);
extern void init_page_detailsettingtime(bk_lv_ui_t *bk_ui);
extern void destroy_page_detailsettingtime(bk_lv_ui_t *bk_ui);
extern void init_page_detailsettingdamper(bk_lv_ui_t *bk_ui);
extern void destroy_page_detailsettingdamper(bk_lv_ui_t *bk_ui);
extern void init_page_detailsettingdefrost(bk_lv_ui_t *bk_ui);
extern void destroy_page_detailsettingdefrost(bk_lv_ui_t *bk_ui);
extern void init_page_neurosys(bk_lv_ui_t *bk_ui);
extern void destroy_page_neurosys(bk_lv_ui_t *bk_ui);

extern bk_lv_ui_t bk_lv_tool_ui;


/*
 * preRender target이 있는 일반 page.
 *
 * __VA_ARGS__에 target page ID를 넣으면
 * preRenderTargetCount는 자동 계산한다.
 */
#define PAGE_INFO(id, member, initFunc, destroyFunc, ...)                     \
    [id] =                                                                   \
    {                                                                        \
        .page = &bk_lv_tool_ui.member,                                       \
        .pageId = id,                                                        \
        .initBase = initFunc,                                                \
        .initStep = ui_page_build_step,                                      \
        .destroy = destroyFunc,                                              \
        .preRenderTargets = (const pageId_t[]){ __VA_ARGS__ },               \
        .preRenderTargetCount =                                              \
            (uint32_t)(sizeof((const pageId_t[]){ __VA_ARGS__ }) /           \
                       sizeof(pageId_t))                                     \
    }


/*
 * preRender target이 없는 page.
 */
#define PAGE_INFO_NO_TARGETS(id, member, initFunc, destroyFunc)               \
    [id] =                                                                   \
    {                                                                        \
        .page = &bk_lv_tool_ui.member,                                       \
        .pageId = id,                                                        \
        .initBase = initFunc,                                                \
        .initStep = ui_page_build_step,                                      \
        .destroy = destroyFunc,                                              \
        .preRenderTargets = NULL,                                            \
        .preRenderTargetCount = 0                                            \
    }


const preRendererPageInfo_t preRenderPageInfo[PAGE_COUNT] =
{
    PAGE_INFO(
        PAGE_MAIN,
        main,
        init_page_main,
        destroy_page_main,
        PAGE_AUTOMODE,
        PAGE_MANUALMODE,
        PAGE_AUTODRYMODE,
        PAGE_MEMORYMODE,
        PAGE_SETTINGMODE
    ),

    PAGE_INFO(
        PAGE_AUTOMODE,
        automode,
        init_page_automode,
        destroy_page_automode,
        PAGE_MAIN,
        PAGE_AUTOMODESTART,
        PAGE_MEMORYMODE
    ),

    PAGE_INFO(
        PAGE_AUTOMODESTART,
        automodestart,
        init_page_automodestart,
        destroy_page_automodestart,
        PAGE_AUTOMODEEND,
        PAGE_AUTOMODE
    ),

    PAGE_INFO(
        PAGE_AUTOMODEEND,
        automodeend,
        init_page_automodeend,
        destroy_page_automodeend,
        PAGE_MANUALMODESTART,
        PAGE_AUTOMODE
    ),

    PAGE_INFO(
        PAGE_MANUALMODE,
        manualmode,
        init_page_manualmode,
        destroy_page_manualmode,
        PAGE_MAIN,
        PAGE_MANUALMODESTART
    ),

    PAGE_INFO(
        PAGE_MANUALMODESTART,
        manualmodestart,
        init_page_manualmodestart,
        destroy_page_manualmodestart,
        PAGE_AUTOMODE,
        PAGE_MANUALMODE
    ),

    PAGE_INFO(
        PAGE_AUTODRYMODE,
        autodrymode,
        init_page_autodrymode,
        destroy_page_autodrymode,
        PAGE_MAIN
    ),

    PAGE_INFO(
        PAGE_MEMORYMODE,
        memorymode,
        init_page_memorymode,
        destroy_page_memorymode,
        PAGE_AUTOMODE,
        PAGE_MAIN
    ),

    PAGE_INFO(
        PAGE_SETTINGMODE,
        settingmode,
        init_page_settingmode,
        destroy_page_settingmode,
        PAGE_SETTINGMODEDETAILSETTING,
        PAGE_SETTINGMODEDEGREE,
        PAGE_SETTINGMODERECORD,
        PAGE_SETTINGMODETEST,
        PAGE_SETTINGMODETIME,
        PAGE_SETTINGMODELANGUAGE,
        PAGE_MAIN
    ),

    PAGE_INFO(
        PAGE_SETTINGMODEDETAILSETTING,
        settingmodedetailsetting,
        init_page_settingmodedetailsetting,
        destroy_page_settingmodedetailsetting,
        PAGE_SETTINGMODE,
        PAGE_DETAILSETTINGTEMP,
        PAGE_DETAILSETTINGHUMIDITY,
        PAGE_DETAILSETTINGTIME,
        PAGE_DETAILSETTINGDAMPER,
        PAGE_DETAILSETTINGDEFROST
    ),

    PAGE_INFO(
        PAGE_SETTINGMODEDEGREE,
        settingmodedegree,
        init_page_settingmodedegree,
        destroy_page_settingmodedegree,
        PAGE_SETTINGMODE
    ),

    PAGE_INFO(
        PAGE_SETTINGMODERECORD,
        settingmoderecord,
        init_page_settingmoderecord,
        destroy_page_settingmoderecord,
        PAGE_SETTINGMODE
    ),

    PAGE_INFO(
        PAGE_SETTINGMODETEST,
        settingmodetest,
        init_page_settingmodetest,
        destroy_page_settingmodetest,
        PAGE_SETTINGMODE
    ),

    PAGE_INFO(
        PAGE_SETTINGMODETIME,
        settingmodetime,
        init_page_settingmodetime,
        destroy_page_settingmodetime,
        PAGE_SETTINGMODE
    ),

    PAGE_INFO(
        PAGE_SETTINGMODELANGUAGE,
        settingmodelanguage,
        init_page_settingmodelanguage,
        destroy_page_settingmodelanguage,
        PAGE_SETTINGMODE
    ),

    PAGE_INFO(
        PAGE_SETTINGMODEMANUAL,
        settingmodemanual,
        init_page_settingmodemanual,
        destroy_page_settingmodemanual,
        PAGE_SETTINGMODE
    ),

    PAGE_INFO(
        PAGE_SETTINGMODEDEFROST,
        settingmodedefrost,
        init_page_settingmodedefrost,
        destroy_page_settingmodedefrost,
        PAGE_SETTINGMODE
    ),

    PAGE_INFO(
        PAGE_DETAILSETTINGTEMP,
        detailsettingtemp,
        init_page_detailsettingtemp,
        destroy_page_detailsettingtemp,
        PAGE_SETTINGMODEDETAILSETTING
    ),

    PAGE_INFO(
        PAGE_DETAILSETTINGHUMIDITY,
        detailsettinghumidity,
        init_page_detailsettinghumidity,
        destroy_page_detailsettinghumidity,
        PAGE_SETTINGMODEDETAILSETTING
    ),

    PAGE_INFO(
        PAGE_DETAILSETTINGTIME,
        detailsettingtime,
        init_page_detailsettingtime,
        destroy_page_detailsettingtime,
        PAGE_SETTINGMODEDETAILSETTING
    ),

    PAGE_INFO(
        PAGE_DETAILSETTINGDAMPER,
        detailsettingdamper,
        init_page_detailsettingdamper,
        destroy_page_detailsettingdamper,
        PAGE_SETTINGMODEDETAILSETTING
    ),

    PAGE_INFO(
        PAGE_DETAILSETTINGDEFROST,
        detailsettingdefrost,
        init_page_detailsettingdefrost,
        destroy_page_detailsettingdefrost,
        PAGE_SETTINGMODEDETAILSETTING
    ),

    PAGE_INFO_NO_TARGETS(
        PAGE_NEUROSYS,
        neurosys,
        init_page_neurosys,
        destroy_page_neurosys
    )
};


#undef PAGE_INFO
#undef PAGE_INFO_NO_TARGETS


pageLifecycleFunc_t getPageInitFunc(pageId_t pageId)
{
    bk_printf(TAG "[SCREEN] getPageInitFunc(%d)\n", pageId);

    if (pageId >= 0 && pageId < PAGE_COUNT)
    {
        bk_printf(
            TAG "[SCREEN] getPageInitFunc was %d for pageId %d\n",
            preRenderPageInfo[pageId].initBase != NULL,
            pageId
        );

        return preRenderPageInfo[pageId].initBase;
    }

    return NULL;
}


pageInitStepFunc_t getPageInitStepFunc(pageId_t pageId)
{
    if (pageId >= 0 && pageId < PAGE_COUNT)
    {
        return preRenderPageInfo[pageId].initStep;
    }

    return NULL;
}


pageLifecycleFunc_t getPageDeinitFunc(pageId_t pageId)
{
    bk_printf(TAG "[SCREEN] getPageDeinitFunc(%d)\n", pageId);

    if (pageId >= 0 && pageId < PAGE_COUNT)
    {
        bk_printf(
            TAG "[SCREEN] getPageDeinitFunc was %d for pageId %d\n",
            preRenderPageInfo[pageId].destroy != NULL,
            pageId
        );

        return preRenderPageInfo[pageId].destroy;
    }

    return NULL;
}