#ifndef __PRE_RENDER_INFO_H__
#define __PRE_RENDER_INFO_H__

#include <stdatomic.h>
#ifdef __cplusplus
extern "C" {
#endif

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

typedef void (*pageLifecycleFunc_t)(bk_lv_ui_t *ui);
typedef void (*pageLifecycleFuncWithStage_t)(bk_lv_ui_t *ui);
typedef struct
{
    pageId_t pageId;
    pageLifecycleFunc_t init_func;
    pageLifecycleFuncWithStage_t init_func_with_stage;
    pageLifecycleFunc_t deinit_func;
    const pageId_t *preRenderTargets;
    const uint32_t preRenderTargetCount;
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

pageLifecycleFunc_t getPageInitFunc(pageId_t pageId);
pageLifecycleFunc_t getPageDeinitFunc(pageId_t pageId);

#ifdef __cplusplus
}
#endif
#endif // __PRE_RENDER_INFO_H__