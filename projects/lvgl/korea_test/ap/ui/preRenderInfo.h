#ifndef __PRE_RENDER_INFO_H__
#define __PRE_RENDER_INFO_H__

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
typedef struct
{
    lv_obj_t **page;
    pageId_t pageId;
    pageLifecycleFunc_t init_func;
    pageLifecycleFunc_t deinit_func;
} preRendererPageInfo_t;

extern const preRendererPageInfo_t preRenderPageInfo[PAGE_COUNT];

pageLifecycleFunc_t getPageInitFunc(pageId_t pageId);
pageLifecycleFunc_t getPageDeinitFunc(pageId_t pageId);

#ifdef __cplusplus
}
#endif
#endif // __PRE_RENDER_INFO_H__