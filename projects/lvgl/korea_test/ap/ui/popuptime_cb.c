#include "lvgl.h"
#include "beken_ui.h"

/* popuptime 제거됨 — settingmodetime 인라인 키패드로 대체.
 * 아래 stub 함수는 다른 파일의 extern 선언과 링크 호환을 위해 유지. */

void popuptime_setup_rollers(bk_lv_ui_t *bk_ui, int mode)
{
    (void)bk_ui;
    (void)mode;
}

void popuptime_set_return_screen(lv_obj_t *scr)
{
    (void)scr;
}

void popuptime_timepopup_yesbt_event_cb(lv_event_t *e)
{
    (void)e;
}

void popuptime_timepopup_nobt_event_cb(lv_event_t *e)
{
    (void)e;
}
