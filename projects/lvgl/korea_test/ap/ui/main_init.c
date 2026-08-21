#include "lvgl.h"
#include "beken_ui.h"
#include "custom_func.h"
#include "event_runtime.h"
#include "ui_lang.h"
#include <stdio.h>
#include <string.h>

extern bk_lv_ui_t bk_lv_tool_ui;
extern lv_obj_t *preRenderRoot;

extern void lv_digital_clock_register(lv_obj_t *label, int show_second, int use_ampm, int hour, int minute, int second);
extern void lv_digital_date_register(lv_obj_t *label);
extern void lv_digital_clock_unregister(lv_obj_t *label);
extern void lv_digital_date_unregister(lv_obj_t *label);
extern void main_automode_event_cb(lv_event_t *e);
extern void main_manualmode_event_cb(lv_event_t *e);
extern void main_autodrymode_event_cb(lv_event_t *e);
extern void main_memorymode_event_cb(lv_event_t *e);
extern void main_settingmode_event_cb(lv_event_t *e);
extern void main_load_event_cb(lv_event_t *e);

// void destroy_page_main(bk_lv_ui_t *bk_ui)
// {
//     if (bk_ui == NULL) {
//         return;
//     }
//     if (bk_ui->main != NULL) {
//         lv_obj_del(bk_ui->main);
//         bk_ui->main = NULL;
//     }
// }
void destroy_page_main(bk_lv_ui_t *bk_ui) {
    if (bk_ui->main != NULL) {
        lv_digital_clock_unregister(bk_ui->main_dclock_1);
        lv_digital_date_unregister(bk_ui->main_timebar_date);
        lv_obj_del(bk_ui->main);
        bk_ui->main = NULL;
    }
}

void init_page_main(bk_lv_ui_t * bk_ui) {
    if (bk_ui->main != NULL && lv_obj_is_valid(bk_ui->main)) {
        destroy_page_main(bk_ui);
    }

    // bk_ui->main = lv_obj_create(NULL);
    // lv_obj_set_size(bk_ui->main, 1024, 600);
    // lv_obj_set_scrollbar_mode(bk_ui->main, LV_SCROLLBAR_MODE_OFF);
    // lv_obj_set_style_bg_color(bk_ui->main, lv_color_hex(0xD5D5D5), 0);
    // lv_obj_set_style_bg_opa(bk_ui->main, LV_OPA_COVER, 0);
    // lv_obj_add_event_cb(bk_ui->main, main_load_event_cb, LV_EVENT_SCREEN_LOAD_START, NULL);
    /* 느낌 쎄한데?
     * 이거 이렇게 재사용 안 하고 마구 root에서 새로 만들면, 기존에 오브젝트들 garbage collection 안 될것 같은데.
     * 그래서 main_init.c에서 init_page_main() 호출할 때, 기존에 main 오브젝트 있으면 destroy_page_main() 호출해서 지우고 새로 만들도록 함.
     * 근데 이거 destroy_page_main() 호출하면, main 오브젝트 안에 있는 digital clock/date 오브젝트도 같이 지워지는데, 이거는 destroy_page_main() 호출 전에
     * digital clock/date 오브젝트 unregister() 해줘야 함. 안 그러면 unregister() 안 된 오브젝트가 남아서, 나중에 다시 init_page_main() 호출하면
     * digital clock/date 오브젝트가 새로 만들어지는데, 기존에 unregister() 안 된 오브젝트가 남아있어서, digital clock/date 오브젝트가 2개가 되어버림. 
     * 그래서 destroy_page_main() 호출 전에 digital clock/date 오브젝트 unregister() 해주도록 함.
     * 2024-06-19: 근데 이거 unregister() 해주고 destroy_page_main() 호출하면, digital clock/date 오브젝트가 unregister() 되면서, digital clock/date 오브젝트가 화면에서 사라짐. 그래서 destroy_page_main() 호출 전에 unregister() 해주면 안 되고, destroy_page_main() 호출 후에 unregister() 해주도록 함
     */
    ui_lang_reset_main_cache();
    bk_ui->main = lv_obj_create(preRenderRoot);
    lv_obj_set_size(bk_ui->main, 1024, 600);
    lv_obj_set_scrollbar_mode(bk_ui->main, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_event_cb(bk_ui->main, main_load_event_cb, LV_EVENT_SCREEN_LOAD_START,   NULL);
    lv_obj_add_event_cb(bk_ui->main, main_load_event_cb, LV_EVENT_SCREEN_LOADED,       NULL);
    lv_obj_add_event_cb(bk_ui->main, main_load_event_cb, LV_EVENT_SCREEN_UNLOAD_START, NULL);
    bk_ui->main_bg = lv_image_create(bk_ui->main);
#if !UI_MAIN_COMBINED_BG_ENABLE
    _bg_set(bk_ui->main_bg);
#endif
    lv_obj_set_pos(bk_ui->main_bg, 0, 0);



    // Button: automode
    bk_ui->main_automode = lv_button_create(bk_ui->main);
    lv_obj_add_flag(bk_ui->main_automode, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->main_automode, main_automode_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->main_automode, 0, 0);
    lv_obj_set_style_border_width(bk_ui->main_automode, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->main_automode, 0, 0);
    lv_obj_set_pos(bk_ui->main_automode, 33, 108);
    lv_obj_set_size(bk_ui->main_automode, 288, 158);

    // ImageView: imageview2
#if !UI_MAIN_COMBINED_BG_ENABLE
    bk_ui->main_imageview2 = lv_image_create(bk_ui->main);
    lv_obj_set_pos(bk_ui->main_imageview2, 33, 108);
    lv_obj_set_size(bk_ui->main_imageview2, 288, 158);
#endif

    // Button: manualmode
    bk_ui->main_manualmode = lv_button_create(bk_ui->main);
    lv_obj_add_flag(bk_ui->main_manualmode, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->main_manualmode, main_manualmode_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->main_manualmode, 0, 0);
    lv_obj_set_style_border_width(bk_ui->main_manualmode, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->main_manualmode, 0, 0);
    lv_obj_set_pos(bk_ui->main_manualmode, 368, 108);
    lv_obj_set_size(bk_ui->main_manualmode, 288, 158);

    // ImageView: imageview4
#if !UI_MAIN_COMBINED_BG_ENABLE
    bk_ui->main_imageview4 = lv_image_create(bk_ui->main);
    lv_obj_set_pos(bk_ui->main_imageview4, 368, 108);
    lv_obj_set_size(bk_ui->main_imageview4, 288, 158);
#endif

    // Button: autodrymode
    bk_ui->main_autodrymode = lv_button_create(bk_ui->main);
    lv_obj_add_flag(bk_ui->main_autodrymode, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->main_autodrymode, main_autodrymode_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->main_autodrymode, 0, 0);
    lv_obj_set_style_border_width(bk_ui->main_autodrymode, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->main_autodrymode, 0, 0);
    lv_obj_set_pos(bk_ui->main_autodrymode, 703, 108);
    lv_obj_set_size(bk_ui->main_autodrymode, 288, 158);

    // ImageView: imageview6
#if !UI_MAIN_COMBINED_BG_ENABLE
    bk_ui->main_imageview6 = lv_image_create(bk_ui->main);
    lv_obj_set_pos(bk_ui->main_imageview6, 703, 108);
    lv_obj_set_size(bk_ui->main_imageview6, 288, 158);
#endif

    // Button: memorymode
    bk_ui->main_memorymode = lv_button_create(bk_ui->main);
    lv_obj_add_flag(bk_ui->main_memorymode, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->main_memorymode, main_memorymode_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->main_memorymode, 0, 0);
    lv_obj_set_style_border_width(bk_ui->main_memorymode, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->main_memorymode, 0, 0);
    lv_obj_set_pos(bk_ui->main_memorymode, 200, 323);
    lv_obj_set_size(bk_ui->main_memorymode, 288, 158);

    // ImageView: imageview8
#if !UI_MAIN_COMBINED_BG_ENABLE
    bk_ui->main_imageview8 = lv_image_create(bk_ui->main);
    lv_obj_set_pos(bk_ui->main_imageview8, 200, 323);
    lv_obj_set_size(bk_ui->main_imageview8, 288, 158);
#endif

    // Button: settingmode
    bk_ui->main_settingmode = lv_button_create(bk_ui->main);
    lv_obj_add_flag(bk_ui->main_settingmode, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->main_settingmode, main_settingmode_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->main_settingmode, 0, 0);
    lv_obj_set_style_border_width(bk_ui->main_settingmode, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->main_settingmode, 0, 0);
    lv_obj_set_pos(bk_ui->main_settingmode, 536, 323);
    lv_obj_set_size(bk_ui->main_settingmode, 288, 158);

    // ImageView: imageview10
#if !UI_MAIN_COMBINED_BG_ENABLE
    bk_ui->main_imageview10 = lv_image_create(bk_ui->main);
    lv_obj_set_pos(bk_ui->main_imageview10, 536, 323);
    lv_obj_set_size(bk_ui->main_imageview10, 288, 158);
#endif

    // // TextView: main_timebar_date  (날짜 YYYY.MM.DD)
    // bk_ui->main_timebar_date = lv_label_create(bk_ui->main);
    // lv_label_set_text(bk_ui->main_timebar_date, "");
    // lv_label_set_long_mode(bk_ui->main_timebar_date, LV_LABEL_LONG_MODE_WRAP);
    // lv_obj_set_x(bk_ui->main_timebar_date, 881);
    // lv_obj_set_y(bk_ui->main_timebar_date, 544);
    // lv_obj_set_width(bk_ui->main_timebar_date, 140);
    // lv_obj_set_height(bk_ui->main_timebar_date, 25);
    // lv_obj_set_style_text_color(bk_ui->main_timebar_date, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_set_style_text_opa(bk_ui->main_timebar_date, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_set_style_text_font(bk_ui->main_timebar_date, &lv_font_scdream_regular_22, LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_set_style_text_align(bk_ui->main_timebar_date, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_digital_date_register(bk_ui->main_timebar_date);

    ui_lang_apply_main(bk_ui);

    // // TextView: main_dclock_1  (시간 HH:MM)
    // bk_ui->main_dclock_1 = lv_label_create(bk_ui->main);
    // lv_label_set_text(bk_ui->main_dclock_1, "");
    // lv_digital_clock_register(bk_ui->main_dclock_1, 0, 0, 11, 25, 50);
    // lv_obj_set_x(bk_ui->main_dclock_1, 891);
    // lv_obj_set_y(bk_ui->main_dclock_1, 570);
    // lv_obj_set_width(bk_ui->main_dclock_1, 114);
    // lv_obj_set_height(bk_ui->main_dclock_1, 30);
    // lv_obj_set_style_text_color(bk_ui->main_dclock_1, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_set_style_text_opa(bk_ui->main_dclock_1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_set_style_text_font(bk_ui->main_dclock_1, &lv_font_scdream_regular_22, LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_set_style_text_align(bk_ui->main_dclock_1, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);

}
