#include "bk_private/bk_init.h"
#include <components/system.h>
#include <os/os.h>
#include <os/mem.h>
#include <components/shell_task.h>
#include <modules/pm.h>
#include <driver/pwr_clk.h>
#include "cli.h"
#include "components/media_types.h"
#include "driver/drv_tp.h"
#if CONFIG_LVGL
#include "lvgl.h"
#endif
#include "lv_vendor.h"
#include "ui/beken_ui.h"

#include "media_service.h"
#include "components/bk_display.h"
#include "driver/gpio.h"
#include "gpio_driver.h"
#include "driver/pwr_clk.h"
#define TAG "86box"

#define LOGI(...) BK_LOGI(TAG, ##__VA_ARGS__)
#define LOGW(...) BK_LOGW(TAG, ##__VA_ARGS__)
#define LOGE(...) BK_LOGE(TAG, ##__VA_ARGS__)
#define LOGD(...) BK_LOGD(TAG, ##__VA_ARGS__)
#define LOGV(...) BK_LOGV(TAG, ##__VA_ARGS__)

#define LCD_LDO_PIN         (GPIO_13)
extern void user_app_main(void);
extern void rtos_set_user_app_entry(beken_thread_function_t entry);
extern int bk_cli_init(void);
extern void beken_ui_init(void);
extern void bk_set_jtag_mode(uint32_t cpu_id, uint32_t group_id);
extern lv_vnd_config_t vendor_config;
extern const lcd_device_t lcd_device_st7701s;
extern const lcd_device_t lcd_device_h050iwv;
extern const lcd_device_t lcd_device_ge070jii2135;

bk_display_rgb_ctlr_config_t rgb_ctlr_config = {
    .lcd_device = &lcd_device_ge070jii2135,
    // .lcd_device = &lcd_device_h050iwv,
    .clk_pin = GPIO_0,
    .cs_pin = GPIO_12,
    .sda_pin = GPIO_1,
    .rst_pin = GPIO_2,    /* GPIO_6 is GT911 INT — use safe unused pin */
    
};

static avdk_err_t lcd_backlight_open(uint8_t bl_io)
{
    gpio_dev_unmap(bl_io);
    BK_LOG_ON_ERR(bk_gpio_enable_output(bl_io));
    BK_LOG_ON_ERR(bk_gpio_pull_up(bl_io));
    bk_gpio_set_output_high(bl_io);
    return AVDK_ERR_OK;
}

static avdk_err_t lcd_backlight_close(uint8_t bl_io)
{
    BK_LOG_ON_ERR(bk_gpio_pull_down(bl_io));
    bk_gpio_set_output_low(bl_io);
    return AVDK_ERR_OK;
}

bk_err_t lvgl_app_86box_init(void)
{
    bk_err_t ret;
    lv_vnd_config_t lv_vnd_config = {0};

    lv_vnd_config.width = rgb_ctlr_config.lcd_device->width;
    lv_vnd_config.height = rgb_ctlr_config.lcd_device->height;
    // lv_vnd_config.render_mode = RENDER_PARTIAL_MODE;

    lv_vnd_config.render_mode = RENDER_DIRECT_MODE;
    lv_vnd_config.rotation = ROTATE_NONE;

    for (int i = 0; i < CONFIG_LVGL_FRAME_BUFFER_NUM; i++) {
        lv_vnd_config.frame_buffer[i] = frame_buffer_display_malloc(lv_vnd_config.width * lv_vnd_config.height * sizeof(bk_color_t));
        if (lv_vnd_config.frame_buffer[i] == NULL) {
            LOGE("lv_frame_buffer[%d] malloc failed\r\n", i);
            return BK_FAIL;
        }
        /* 픽셀 버퍼 검정 초기화: 백라이트 ON 전 쓰레기값 방지 */
        os_memset(lv_vnd_config.frame_buffer[i]->frame, 0,
                  lv_vnd_config.width * lv_vnd_config.height * sizeof(bk_color_t));
    }
#if 0

    /* psram_malloc uses AP PSRAM heap (4.5MB at 0x602A3000).
     * os_malloc uses SRAM heap (160KB) — too small for 300KB draw buf.
     * DISPLAY slab (2.5MB) is exhausted after 2×1.2MB frame buffers.
     * os_free in lv_vendor_deinit handles PSRAM pointers via os_check_heap_range. */
    uint32_t draw_buf_size = lv_vnd_config.width * lv_vnd_config.height / 4 * sizeof(bk_color_t);
    lv_vnd_config.draw_buf_2_1 = psram_malloc(draw_buf_size);
    if (lv_vnd_config.draw_buf_2_1 == NULL) {
        LOGE("draw_buf_2_1 psram_malloc failed\r\n");
        return BK_FAIL;
    }
#endif
    bk_display_rgb_new(&lv_vnd_config.handle, &rgb_ctlr_config);
    ret = lv_vendor_init(&lv_vnd_config);
    if (ret != BK_OK) {
        LOGE("lv_vendor_init failed %d\r\n", ret);
        return ret;
    }
    bk_pm_module_vote_ctrl_external_ldo(GPIO_CTRL_LDO_MODULE_LCD, LCD_LDO_PIN, GPIO_OUTPUT_STATE_HIGH);
    bk_display_open(lv_vnd_config.handle);

#if (CONFIG_TP)
    drv_tp_open(lv_vnd_config.width, lv_vnd_config.height, TP_MIRROR_NONE);
    drv_tp_reg_touch_event(lv_vendor_tp_notify, NULL);  /* wake LVGL immediately on touch */
#endif

    lv_vendor_disp_lock();
#if 1
    beken_ui_init();

#else
    lv_obj_t  *jpg = lv_image_create(lv_screen_active());
    lv_img_set_src(jpg, "/images/bg.jpg");

    lv_obj_t  *png = lv_image_create(lv_screen_active());
    lv_obj_set_pos(png, 0, 100);
    lv_img_set_src(png, "/images/auto_mode_freeze_board_china.png");
#endif
    lv_vendor_disp_unlock();

    lv_vendor_start();

    /*  intro.jpg 첫 프레임 렌더링 대기 후 백라이트 ON — 깨진 화면 방지
    *   근데왜 이렇게 딜레이를 하드코딩으로 넣었지? lv_refr를 하든가. 애초에 위쪽에서 블락이 될텐데 왜 여기서 한번 더 쉬어가는거지    */
    rtos_delay_milliseconds(500);
    lcd_backlight_open(GPIO_9);

    return BK_OK;
}

bk_err_t lvgl_app_86box_deinit(void)
{
    lcd_backlight_close(GPIO_9);
    bk_display_close(vendor_config.handle);

#if (CONFIG_TP)
    drv_tp_close();
#endif

    lv_vendor_stop();

    bk_display_delete(vendor_config.handle);

    lv_vendor_deinit();
    bk_pm_module_vote_ctrl_external_ldo(GPIO_CTRL_LDO_MODULE_LCD, LCD_LDO_PIN, GPIO_OUTPUT_STATE_LOW);
    return BK_OK;
}

#define CMDS_COUNT  (sizeof(s_86box_commands) / sizeof(struct cli_command))

void cli_86box_cmd(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv)
{
    LOGD("%s %d\r\n", __func__, __LINE__);

    lvgl_app_86box_deinit();
}

static const struct cli_command s_86box_commands[] =
{
    {"86box", "86box", cli_86box_cmd},
};

int cli_86box_init(void)
{
    return cli_register_commands(s_86box_commands, CMDS_COUNT);
}

extern void hal_buzzer_start(int freq_hz, int duration_ms);

int main(void)
{
    lcd_backlight_close(GPIO_9);
    bk_init();
    hal_buzzer_start(2000, 200);   /* 부팅 beep: 2 kHz × 200 ms */
    media_service_init();

    bk_pm_module_vote_psram_ctrl(PM_POWER_PSRAM_MODULE_NAME_LVGL_CODE_RUN, PM_POWER_MODULE_STATE_ON);

    lvgl_app_86box_init();

    return 0;
}
