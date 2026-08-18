#include "lvgl.h"
#include "beken_ui.h"
#include "custom_func.h"
#include "event_runtime.h"
#include <stdio.h>
#include <string.h>

#include "beken_ui.h"
#include "settings.h"
#include "hardware_hal.h"
#include "ui_lang.h"

extern bk_lv_ui_t bk_lv_tool_ui;
extern int  memorymode_get_pending_delete_slot(void);
extern void destroy_page_popupdelete(bk_lv_ui_t *bk_ui);
extern void memorymode_refresh_display(bk_lv_ui_t *bk_ui);
extern void memorymode_clear_checking(void);

static uint32_t last_click_time = 0;

void popupdelete_yesbt_event_cb(lv_event_t *e);
void popupdelete_nobt_event_cb(lv_event_t *e);

void popupdelete_yesbt_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    if (lv_tick_elaps(last_click_time) < 250) return;
    last_click_time = lv_tick_get();
    hal_buzzer_beep();

    int slot = memorymode_get_pending_delete_slot();
    if (slot < 0 || slot > 11) {
        printf("[DEL] YES: invalid slot=%d, abort\n", slot);
        destroy_page_popupdelete(bk_ui);
        return;
    }
    printf("[DEL] YES: deleting slot=%d\n", slot);
    char k[40];
    const char *empty = "";
#define _MDEL(field) do { \
    snprintf(k, sizeof(k), field "%d", slot); \
    settings_set_str(k, empty); \
} while (0)
    _MDEL("MemoryDayPeriod");
    _MDEL("MemoryFreezeTemp");
    _MDEL("MemoryDefrostTemp");
    _MDEL("MemoryDefrostHour");
    _MDEL("MemoryDefrostMin");
    _MDEL("MemoryFermentation1Temp");
    _MDEL("MemoryFermentation1Humidity");
    _MDEL("MemoryFermentation1Hour");
    _MDEL("MemoryFermentation1Min");
    _MDEL("MemoryFermentation2Temp");
    _MDEL("MemoryFermentation2Humidity");
    _MDEL("MemoryFermentation2Hour");
    _MDEL("MemoryFermentation2Min");
    _MDEL("MemoryCompleteHour");
    _MDEL("MemoryCompleteMin");
#undef _MDEL

    settings_save_dirty();

    /* 삭제한 행이 checked(on) 상태로 남지 않도록 선택 해제 후 갱신 */
    memorymode_clear_checking();

    /* 오버레이 제거 후 memorymode 표시 갱신 — 화면 전환 없음 */
    destroy_page_popupdelete(bk_ui);
    memorymode_refresh_display(bk_ui);
}

void popupdelete_nobt_event_cb(lv_event_t *e)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    if (lv_tick_elaps(last_click_time) < 250) return;
    last_click_time = lv_tick_get();
    hal_buzzer_beep();

    destroy_page_popupdelete(bk_ui);
}
