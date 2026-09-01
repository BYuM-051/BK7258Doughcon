#include "lvgl.h"
#include "beken_ui.h"

#include "preRenderer.h"
#include "preRenderInfo.h"
#define TAG "[popuptime_init.c] "
// #define bk_printf(fmt, ...) do {if(0) printf(fmt, ##__VA_ARGS__); } while(0) // disable printf
/* popuptime 제거됨 — settingmodetime 인라인 키패드로 대체 */

void destroy_page_popuptime(bk_lv_ui_t *bk_ui)
{
    if (bk_ui == NULL) return;
    if (bk_ui->popuptime != NULL && lv_obj_is_valid(bk_ui->popuptime)) {
        lv_obj_del(bk_ui->popuptime);
    }
    bk_ui->popuptime = NULL;
    bk_ui->popuptime_settemp_setn1 = NULL;
    bk_ui->popuptime_settemp_setn2 = NULL;
    bk_ui->popuptime_settemp_setn3 = NULL;
}

void init_page_popuptime(bk_lv_ui_t *bk_ui)
{
    (void)bk_ui;
}

rendererFuncStatus_t init_page_popuptime_with_step(bk_lv_ui_t *bk_ui)
{
    return RENDERER_FUNC_FAILED;
}
