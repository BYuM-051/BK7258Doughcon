#ifndef __PRE_RENDER_INFO_H__
#define __PRE_RENDER_INFO_H__

#include <stdatomic.h>
#include "beken_ui.h"
#ifdef __cplusplus
extern "C" {
#endif

extern lv_event_code_t UI_EVENT_PAGE_SHOW_START;
extern lv_event_code_t UI_EVENT_PAGE_SHOWN;
extern lv_event_code_t UI_EVENT_PAGE_HIDE_START;
extern lv_event_code_t UI_EVENT_PAGE_HIDDEN;

typedef enum
{
    PAGE_MAIN = 0,

    PAGE_AUTOMODE,
    PAGE_AUTOMODESTART,
    PAGE_AUTOMODEEND,

    PAGE_MANUALMODE,
    PAGE_MANUALMODESTART,

    PAGE_AUTODRYMODE,
    PAGE_MEMORYMODE,

    PAGE_SETTINGMODE,
    PAGE_SETTINGMODEDETAILSETTING,
    PAGE_SETTINGMODEDEGREE,
    PAGE_SETTINGMODERECORD,
    PAGE_SETTINGMODETEST,
    PAGE_SETTINGMODETIME,
    PAGE_SETTINGMODELANGUAGE,
    PAGE_SETTINGMODEMANUAL,
    PAGE_SETTINGMODEDEFROST,

    PAGE_DETAILSETTINGTEMP,
    PAGE_DETAILSETTINGHUMIDITY,
    PAGE_DETAILSETTINGTIME,
    PAGE_DETAILSETTINGDAMPER,
    PAGE_DETAILSETTINGDEFROST,

    PAGE_NEUROSYS, // 이거 대체 뭐하는 페이지지

    PAGE_COUNT,
    PAGE_NONE = PAGE_COUNT
} pageId_t;
typedef enum
{
    RENDER_STEP_CREATE_PAGE = 0,
    RENDER_STEP_CREATE_CHILD,
    RENDER_STEP_CACHE_BACKGROUND,
    RENDER_STEP_CACHE_IMAGE,
    RENDER_STEP_ATTACH_EVENT,
    RENDER_STEP_DONE,
    RENDER_STEP_COUNT
} renderStep_t;
typedef enum
{
    RENDERER_FUNC_NOT_DONE = 0,
    RENDERER_FUNC_DONE,
    RENDERER_FUNC_FAILED
} rendererFuncStatus_t;
typedef enum
{
    SHARED_IMAGE_NONE = 0,
    SHARED_IMAGE_ASSET_BG_DOUGH,
    SHARED_IMAGE_ASSET_BG_AUTOMODESTART,
    SHARED_IMAGE_COUNT
} sharedImageAssetId_t;

typedef void (*pageLifecycleFunc_t)(bk_lv_ui_t *ui);
typedef rendererFuncStatus_t (*pageLifecycleFuncWithStep_t)(bk_lv_ui_t *ui);

typedef struct
{
    const char *imagePath;
    const bool hasLanguageVariant;
    const bool hasDegreeVariant;
    const char *fileExtension;
} preRenderImageInfo_t;
extern const preRenderImageInfo_t sharedImageAssetInfo[SHARED_IMAGE_COUNT];

typedef struct
{
    const preRenderImageInfo_t *imageInfo;
    lv_draw_buf_t *imageBuffer;
} preRendererImageState_t;

typedef struct
{
    pageId_t pageId;
    pageLifecycleFunc_t init_func;
    pageLifecycleFuncWithStep_t init_func_with_step;
    pageLifecycleFunc_t deinit_func;
    const pageId_t *preRenderTargetPages;
    const uint32_t preRenderTargetPageCount;
    const uint32_t preRenderImageCount;
    const preRenderImageInfo_t *preRenderImageInfo;
    const sharedImageAssetId_t backgroundImageAssetId;
} preRendererPageConfig_t;
typedef struct
{
    lv_obj_t **page;
    _Atomic bool isRendered;
    _Atomic uint32_t renderStep;
    const preRendererPageConfig_t *config;
} preRendererPageState_t;

extern const preRendererPageConfig_t preRenderPageConfig[PAGE_COUNT];
extern preRendererPageState_t preRenderPageState[PAGE_COUNT];
extern preRendererImageState_t sharedImageAssetState[SHARED_IMAGE_COUNT];

//extern Rendering Page Init / Deinit functions
extern bk_lv_ui_t bk_lv_tool_ui;

extern void init_page_main(bk_lv_ui_t *bk_ui);
extern rendererFuncStatus_t init_page_main_with_step(bk_lv_ui_t *bk_ui);
extern void destroy_page_main(bk_lv_ui_t *bk_ui);
extern void init_page_automode(bk_lv_ui_t *bk_ui);
extern rendererFuncStatus_t init_page_automode_with_step(bk_lv_ui_t *bk_ui);
extern void destroy_page_automode(bk_lv_ui_t *bk_ui);
extern void init_page_automodestart(bk_lv_ui_t *bk_ui);
extern rendererFuncStatus_t init_page_automodestart_with_step(bk_lv_ui_t *bk_ui);
extern void destroy_page_automodestart(bk_lv_ui_t *bk_ui);
extern void init_page_automodeend(bk_lv_ui_t *bk_ui);
extern rendererFuncStatus_t init_page_automodeend_with_step(bk_lv_ui_t *bk_ui);
extern void destroy_page_automodeend(bk_lv_ui_t *bk_ui);
extern void init_page_manualmode(bk_lv_ui_t *bk_ui);
extern rendererFuncStatus_t init_page_manualmode_with_step(bk_lv_ui_t *bk_ui);
extern void destroy_page_manualmode(bk_lv_ui_t *bk_ui);
extern void init_page_manualmodestart(bk_lv_ui_t *bk_ui);
extern rendererFuncStatus_t init_page_manualmodestart_with_step(bk_lv_ui_t *bk_ui);
extern void destroy_page_manualmodestart(bk_lv_ui_t *bk_ui);
extern void init_page_autodrymode(bk_lv_ui_t *bk_ui);
extern rendererFuncStatus_t init_page_autodrymode_with_step(bk_lv_ui_t *bk_ui);
extern void destroy_page_autodrymode(bk_lv_ui_t *bk_ui);
extern void init_page_memorymode(bk_lv_ui_t *bk_ui);
extern rendererFuncStatus_t init_page_memorymode_with_step(bk_lv_ui_t *bk_ui);
extern void destroy_page_memorymode(bk_lv_ui_t *bk_ui);
extern void init_page_settingmode(bk_lv_ui_t *bk_ui);
extern rendererFuncStatus_t init_page_settingmode_with_step(bk_lv_ui_t *bk_ui);
extern void destroy_page_settingmode(bk_lv_ui_t *bk_ui);
extern void init_page_settingmodedetailsetting(bk_lv_ui_t *bk_ui);
extern rendererFuncStatus_t init_page_settingmodedetailsetting_with_step(bk_lv_ui_t *bk_ui);
extern void destroy_page_settingmodedetailsetting(bk_lv_ui_t *bk_ui);
extern void init_page_settingmodedegree(bk_lv_ui_t *bk_ui);
extern rendererFuncStatus_t init_page_settingmodedegree_with_step(bk_lv_ui_t *bk_ui);
extern void destroy_page_settingmodedegree(bk_lv_ui_t *bk_ui);
extern void init_page_settingmoderecord(bk_lv_ui_t *bk_ui);
extern rendererFuncStatus_t init_page_settingmoderecord_with_step(bk_lv_ui_t *bk_ui);
extern void destroy_page_settingmoderecord(bk_lv_ui_t *bk_ui);
extern void init_page_settingmodetest(bk_lv_ui_t *bk_ui);
extern rendererFuncStatus_t init_page_settingmodetest_with_step(bk_lv_ui_t *bk_ui);
extern void destroy_page_settingmodetest(bk_lv_ui_t *bk_ui);
extern void init_page_settingmodetime(bk_lv_ui_t *bk_ui);
extern rendererFuncStatus_t init_page_settingmodetime_with_step(bk_lv_ui_t *bk_ui);
extern void destroy_page_settingmodetime(bk_lv_ui_t *bk_ui);
extern void init_page_settingmodelanguage(bk_lv_ui_t *bk_ui);
extern rendererFuncStatus_t init_page_settingmodelanguage_with_step(bk_lv_ui_t *bk_ui);
extern void destroy_page_settingmodelanguage(bk_lv_ui_t *bk_ui);
extern void init_page_settingmodemanual(bk_lv_ui_t *bk_ui);
extern rendererFuncStatus_t init_page_settingmodemanual_with_step(bk_lv_ui_t *bk_ui);
extern void destroy_page_settingmodemanual(bk_lv_ui_t *bk_ui);
extern void init_page_settingmodedefrost(bk_lv_ui_t *bk_ui);
extern rendererFuncStatus_t init_page_settingmodedefrost_with_step(bk_lv_ui_t *bk_ui);
extern void destroy_page_settingmodedefrost(bk_lv_ui_t *bk_ui);
extern void init_page_detailsettingtemp(bk_lv_ui_t *bk_ui);
extern rendererFuncStatus_t init_page_detailsettingtemp_with_step(bk_lv_ui_t *bk_ui);
extern void destroy_page_detailsettingtemp(bk_lv_ui_t *bk_ui);
extern void init_page_detailsettinghumidity(bk_lv_ui_t *bk_ui);
extern rendererFuncStatus_t init_page_detailsettinghumidity_with_step(bk_lv_ui_t *bk_ui);
extern void destroy_page_detailsettinghumidity(bk_lv_ui_t *bk_ui);
extern void init_page_detailsettingtime(bk_lv_ui_t *bk_ui);
extern rendererFuncStatus_t init_page_detailsettingtime_with_step(bk_lv_ui_t *bk_ui);
extern void destroy_page_detailsettingtime(bk_lv_ui_t *bk_ui);
extern void init_page_detailsettingdamper(bk_lv_ui_t *bk_ui);
extern rendererFuncStatus_t init_page_detailsettingdamper_with_step(bk_lv_ui_t *bk_ui);
extern void destroy_page_detailsettingdamper(bk_lv_ui_t *bk_ui);
extern void init_page_detailsettingdefrost(bk_lv_ui_t *bk_ui);
extern rendererFuncStatus_t init_page_detailsettingdefrost_with_step(bk_lv_ui_t *bk_ui);
extern void destroy_page_detailsettingdefrost(bk_lv_ui_t *bk_ui);
extern void init_page_neurosys(bk_lv_ui_t *bk_ui);
extern rendererFuncStatus_t init_page_neurosys_with_step(bk_lv_ui_t *bk_ui);
extern void destroy_page_neurosys(bk_lv_ui_t *bk_ui);
//=============================================

extern pageLifecycleFuncWithStep_t getPageInitFunc(pageId_t pageId);
extern pageLifecycleFunc_t getPageDeinitFunc(pageId_t pageId);

extern bool getImageFullPath(const char *basePath, bool hasLanguageVariant, bool hasDegreeVariant, const char *extension, char *imagePath, size_t imagePathSize);

extern rendererFuncStatus_t init_shared_image_asset(void);
extern rendererFuncStatus_t set_shared_image_asset(lv_obj_t *imageObj, sharedImageAssetId_t assetId);

extern bool isPageIdValid(pageId_t pageId);

#ifdef __cplusplus
}
#endif
#endif // __PRE_RENDER_INFO_H__
