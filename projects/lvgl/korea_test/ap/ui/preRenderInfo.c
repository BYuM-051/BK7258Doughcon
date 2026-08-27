#include "preRenderer.h"
#include "preRenderInfo.h"

#define TAG "[preRenderInfo.c] "
// #define bk_printf(fmt, ...) do {if(0) printf(fmt, ##__VA_ARGS__); } while(0) // disable printf

//extern Rendering Page Init / Deinit functions
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
//=============================================

extern bk_lv_ui_t bk_lv_tool_ui;

const preRendererPageInfo_t preRenderPageInfo[PAGE_COUNT] =
{
    [PAGE_MAIN] = {&bk_lv_tool_ui.main, PAGE_MAIN, init_page_main, destroy_page_main},
    [PAGE_AUTOMODE] = {&bk_lv_tool_ui.automode, PAGE_AUTOMODE, init_page_automode, destroy_page_automode},
    [PAGE_AUTOMODESTART] = {&bk_lv_tool_ui.automodestart, PAGE_AUTOMODESTART, init_page_automodestart, destroy_page_automodestart},
    [PAGE_AUTOMODEEND] = {&bk_lv_tool_ui.automodeend, PAGE_AUTOMODEEND, init_page_automodeend, destroy_page_automodeend},
    [PAGE_MANUALMODE] = {&bk_lv_tool_ui.manualmode, PAGE_MANUALMODE, init_page_manualmode, destroy_page_manualmode},
    [PAGE_MANUALMODESTART] = {&bk_lv_tool_ui.manualmodestart, PAGE_MANUALMODESTART, init_page_manualmodestart, destroy_page_manualmodestart},
    [PAGE_AUTODRYMODE] = {&bk_lv_tool_ui.autodrymode, PAGE_AUTODRYMODE, init_page_autodrymode, destroy_page_autodrymode},
    [PAGE_MEMORYMODE] = {&bk_lv_tool_ui.memorymode, PAGE_MEMORYMODE, init_page_memorymode, destroy_page_memorymode},
    [PAGE_SETTINGMODE] = {&bk_lv_tool_ui.settingmode, PAGE_SETTINGMODE, init_page_settingmode, destroy_page_settingmode},
    [PAGE_SETTINGMODEDETAILSETTING] = {&bk_lv_tool_ui.settingmodedetailsetting, PAGE_SETTINGMODEDETAILSETTING, init_page_settingmodedetailsetting, destroy_page_settingmodedetailsetting},
    [PAGE_SETTINGMODEDEGREE] = {&bk_lv_tool_ui.settingmodedegree, PAGE_SETTINGMODEDEGREE, init_page_settingmodedegree, destroy_page_settingmodedegree},
    [PAGE_SETTINGMODERECORD] = {&bk_lv_tool_ui.settingmoderecord, PAGE_SETTINGMODERECORD, init_page_settingmoderecord, destroy_page_settingmoderecord},
    [PAGE_SETTINGMODETEST] = {&bk_lv_tool_ui.settingmodetest, PAGE_SETTINGMODETEST, init_page_settingmodetest, destroy_page_settingmodetest},
    [PAGE_SETTINGMODETIME] = {&bk_lv_tool_ui.settingmodetime, PAGE_SETTINGMODETIME, init_page_settingmodetime, destroy_page_settingmodetime},
    [PAGE_SETTINGMODELANGUAGE] = {&bk_lv_tool_ui.settingmodelanguage, PAGE_SETTINGMODELANGUAGE, init_page_settingmodelanguage, destroy_page_settingmodelanguage},
    [PAGE_SETTINGMODEMANUAL] = {&bk_lv_tool_ui.settingmodemanual, PAGE_SETTINGMODEMANUAL, init_page_settingmodemanual, destroy_page_settingmodemanual},
    [PAGE_SETTINGMODEDEFROST] = {&bk_lv_tool_ui.settingmodedefrost, PAGE_SETTINGMODEDEFROST, init_page_settingmodedefrost, destroy_page_settingmodedefrost},
    [PAGE_DETAILSETTINGTEMP] = {&bk_lv_tool_ui.detailsettingtemp, PAGE_DETAILSETTINGTEMP, init_page_detailsettingtemp, destroy_page_detailsettingtemp},
    [PAGE_DETAILSETTINGHUMIDITY] = {&bk_lv_tool_ui.detailsettinghumidity, PAGE_DETAILSETTINGHUMIDITY, init_page_detailsettinghumidity, destroy_page_detailsettinghumidity},
    [PAGE_DETAILSETTINGTIME] = {&bk_lv_tool_ui.detailsettingtime, PAGE_DETAILSETTINGTIME, init_page_detailsettingtime, destroy_page_detailsettingtime},
    [PAGE_DETAILSETTINGDAMPER] = {&bk_lv_tool_ui.detailsettingdamper, PAGE_DETAILSETTINGDAMPER, init_page_detailsettingdamper, destroy_page_detailsettingdamper},
    [PAGE_DETAILSETTINGDEFROST] = {&bk_lv_tool_ui.detailsettingdefrost, PAGE_DETAILSETTINGDEFROST, init_page_detailsettingdefrost, destroy_page_detailsettingdefrost},
    [PAGE_NEUROSYS] = {&bk_lv_tool_ui.neurosys, PAGE_NEUROSYS, init_page_neurosys, destroy_page_neurosys}
};

pageLifecycleFunc_t getPageInitFunc(pageId_t pageId)
{
    bk_printf(TAG "[SCREEN] getPageInitFunc(%d)\n", pageId);
    if(pageId >= 0 && pageId < PAGE_COUNT)
    {
        bk_printf(TAG "[SCREEN] getPageInitFunc was %d for pageId %d\n", preRenderPageInfo[pageId].init_func != NULL, pageId);
        return preRenderPageInfo[pageId].init_func;
    }
    return NULL;
}

pageLifecycleFunc_t getPageDeinitFunc(pageId_t pageId)
{
    bk_printf(TAG "[SCREEN] getPageDeinitFunc(%d)\n", pageId);
    if(pageId >= 0 && pageId < PAGE_COUNT)
    {
        bk_printf(TAG "[SCREEN] getPageDeinitFunc was %d for pageId %d\n", preRenderPageInfo[pageId].deinit_func != NULL, pageId);
        return preRenderPageInfo[pageId].deinit_func;
    }
    return NULL;
}
