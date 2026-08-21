#include "preRenderer.h"
#include "ui_config.h"

#define TAG "[preRenderer.c] "

extern bk_lv_ui_t bk_lv_tool_ui;

void preRendererTask(void *arg);
void preRendererWrapper(lv_event_t *e);

#if UI_PRENDERING_ENABLE
void preRendererTask(void *arg)
{
    bk_lv_ui_t *bk_ui = &bk_lv_tool_ui;
    lv_obj_t *currentPage = (lv_obj_t *)arg;

    bk_printf(TAG "pre-rendering start\n");

    if(currentPage == bk_ui->automode)
    {
        bk_printf(TAG "IM AUTO MODE!\n");
    }
    else if(currentPage == bk_ui->autodrymode)
    {
        bk_printf(TAG "IM AUTO DRY MODE!\n");
    }
    else if(currentPage == bk_ui->manualmode)
    {
        bk_printf(TAG "IM MANUAL MODE!\n");
    }
    else
    {
        bk_printf(TAG "no pre-rendering for this page\n");
    }
    bk_printf(TAG "pre-rendering done\n");

    vTaskDelete(NULL);
}

void preRendererWrapper(lv_event_t *e)
{
    lv_obj_t *currentPage = (lv_obj_t *)lv_event_get_user_data(e);
    // Create a new task for pre-rendering
    if (xTaskCreate(preRendererTask, "preRendererTask", 2048, (void *)currentPage, 5, NULL) != pdPASS) {
        bk_printf(TAG "Failed to create preRendererTask\n");
    }
}

#endif /* UI_PRENDERING_ENABLE */