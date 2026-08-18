
#include "lvgl.h"
#include "beken_ui.h"
#include "custom_func.h"
#include "event_runtime.h"
#include <time.h>
#include <stdio.h>
#include <string.h>

#include "beken_ui.h"
#include "settings.h"
#include "device_state.h"

extern bk_lv_ui_t bk_lv_tool_ui;
extern void timebar_timebar_error_checkbt_event_cb(lv_event_t *e);
extern void timebar_sound_checkbt_event_cb(lv_event_t *e);
extern void timebar_load_event_cb(lv_event_t *e);
extern void timebar_start_icon_timer(void);
extern void lv_digital_clock_register(lv_obj_t *label, int show_second, int use_ampm, int hour, int minute, int second);
extern void lv_digital_date_register(lv_obj_t *label);
extern void lv_digital_clock_unregister(lv_obj_t *label);
extern void lv_digital_date_unregister(lv_obj_t *label);

void destroy_page_timebar(bk_lv_ui_t *bk_ui)
{
    if (bk_ui == NULL) {
        return;
    }
    if (bk_ui->timebar != NULL) {
        lv_digital_clock_unregister(bk_ui->timebar_timebar_timetxt);
        lv_digital_date_unregister(bk_ui->timebar_timebar_datetxt);
        lv_obj_del(bk_ui->timebar);
        bk_ui->timebar = NULL;
    }
}

void init_page_timebar(bk_lv_ui_t * bk_ui) {
    if (bk_ui->timebar != NULL && lv_obj_is_valid(bk_ui->timebar)) {
        destroy_page_timebar(bk_ui);
    }

    // bk_ui->timebar = lv_obj_create(NULL);
    // lv_obj_set_size(bk_ui->timebar, 1024, 600);
    // lv_obj_set_scrollbar_mode(bk_ui->timebar, LV_SCROLLBAR_MODE_OFF);
    // lv_obj_add_event_cb(bk_ui->timebar, timebar_load_event_cb, LV_EVENT_SCREEN_LOAD_START, NULL);
    // 2. 디스플레이의 최상위 레이어(Top Layer) 가져오기
   // 2. 디스플레이의 최상위 레이어(Top Layer) 가져오기
    lv_obj_t * top_layer = lv_display_get_layer_top(lv_display_get_default());

    // 3. 최상위 레이어를 부모로 하여 타임바 컨테이너 생성
    bk_ui->timebar = lv_obj_create(top_layer);
    
    // 4. 하단 고정 좌표 및 크기 설정 (1024x600 해상도 기준)
    lv_obj_set_size(bk_ui->timebar, 1024, 60);
    // lv_obj_set_pos(bk_ui->timebar, 0, 0);
    lv_obj_set_pos(bk_ui->timebar, 0, 540); // Y축 540부터 시작하여 하단 60픽셀 차지

    // 5. 컨테이너 스타일 설정 (여백 및 테두리 제거로 이미지와 밀착)
    lv_obj_set_style_pad_all(bk_ui->timebar, 0, 0);
    lv_obj_set_style_border_width(bk_ui->timebar, 0, 0);
    lv_obj_set_style_bg_opa(bk_ui->timebar, LV_OPA_TRANSP, 0); // 배경 투명
    lv_obj_set_scrollbar_mode(bk_ui->timebar, LV_SCROLLBAR_MODE_OFF);

    // ImageView: timebar_bg
    bk_ui->timebar_timebar_bg = lv_image_create(bk_ui->timebar);
    _img_set_src_timed(bk_ui->timebar_timebar_bg, "/images/timebar_timebar.png");
    lv_obj_set_pos(bk_ui->timebar_timebar_bg, 0, 0);
    lv_obj_set_size(bk_ui->timebar_timebar_bg, 1024, 60);

    // ImageView: timebar_error_checkim
    bk_ui->timebar_timebar_error_checkim = lv_image_create(bk_ui->timebar);
    _img_set_src_deferred(bk_ui->timebar_timebar_error_checkim, "/images/timebar_error_on.png");
    lv_obj_add_flag(bk_ui->timebar_timebar_error_checkim, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_pos(bk_ui->timebar_timebar_error_checkim, 234, 0);
    lv_obj_set_size(bk_ui->timebar_timebar_error_checkim, 70, 60);

    // Button: timebar_error_checkbt
    bk_ui->timebar_timebar_error_checkbt = lv_button_create(bk_ui->timebar);
    lv_obj_add_flag(bk_ui->timebar_timebar_error_checkbt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->timebar_timebar_error_checkbt, timebar_timebar_error_checkbt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->timebar_timebar_error_checkbt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->timebar_timebar_error_checkbt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->timebar_timebar_error_checkbt, 0, 0);
    lv_obj_set_pos(bk_ui->timebar_timebar_error_checkbt, 234, 0);
    lv_obj_set_size(bk_ui->timebar_timebar_error_checkbt, 70, 60);

    // ImageView: comp_checkim
    bk_ui->timebar_comp_checkim = lv_image_create(bk_ui->timebar);
    _img_set_src_deferred(bk_ui->timebar_comp_checkim, "/images/timebar_comp_on.png");
    lv_obj_add_flag(bk_ui->timebar_comp_checkim, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_pos(bk_ui->timebar_comp_checkim, 306, 0);
    lv_obj_set_size(bk_ui->timebar_comp_checkim, 70, 60);

    // ImageView: watervalve_checkim
    bk_ui->timebar_watervalve_checkim = lv_image_create(bk_ui->timebar);
    _img_set_src_deferred(bk_ui->timebar_watervalve_checkim, "/images/timebar_watevalve_on.png");
    lv_obj_add_flag(bk_ui->timebar_watervalve_checkim, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_pos(bk_ui->timebar_watervalve_checkim, 378, 0);
    lv_obj_set_size(bk_ui->timebar_watervalve_checkim, 70, 60);

    // ImageView: roomfan_checkim
    bk_ui->timebar_roomfan_checkim = lv_image_create(bk_ui->timebar);
    _img_set_src_deferred(bk_ui->timebar_roomfan_checkim, "/images/timebar_roomfan_on.png");
    lv_obj_add_flag(bk_ui->timebar_roomfan_checkim, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_pos(bk_ui->timebar_roomfan_checkim, 450, 0);
    lv_obj_set_size(bk_ui->timebar_roomfan_checkim, 70, 60);

    // ImageView: damper_checkim
    bk_ui->timebar_damper_checkim = lv_image_create(bk_ui->timebar);
    _img_set_src_deferred(bk_ui->timebar_damper_checkim, "/images/timebar_damper_on.png");
    lv_obj_add_flag(bk_ui->timebar_damper_checkim, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_pos(bk_ui->timebar_damper_checkim, 522, 0);
    lv_obj_set_size(bk_ui->timebar_damper_checkim, 70, 60);

    // ImageView: humid_checkim
    bk_ui->timebar_humid_checkim = lv_image_create(bk_ui->timebar);
    _img_set_src_deferred(bk_ui->timebar_humid_checkim, "/images/timebar_humidityheater_on.png");
    lv_obj_add_flag(bk_ui->timebar_humid_checkim, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_pos(bk_ui->timebar_humid_checkim, 666, 0);
    lv_obj_set_size(bk_ui->timebar_humid_checkim, 70, 60);

    // ImageView: heat_checkim
    bk_ui->timebar_heat_checkim = lv_image_create(bk_ui->timebar);
    _img_set_src_deferred(bk_ui->timebar_heat_checkim, "/images/timebar_fireheater_on.png");
    lv_obj_add_flag(bk_ui->timebar_heat_checkim, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_pos(bk_ui->timebar_heat_checkim, 594, 0);
    lv_obj_set_size(bk_ui->timebar_heat_checkim, 70, 60);

    // ImageView: defrost_checkim
    bk_ui->timebar_defrost_checkim = lv_image_create(bk_ui->timebar);
    _img_set_src_deferred(bk_ui->timebar_defrost_checkim, "/images/timebar_defrostheater_on.png");
    lv_obj_add_flag(bk_ui->timebar_defrost_checkim, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_pos(bk_ui->timebar_defrost_checkim, 738, 0);
    lv_obj_set_size(bk_ui->timebar_defrost_checkim, 70, 60);

    // ImageView: sound_checkim
    bk_ui->timebar_sound_checkim = lv_image_create(bk_ui->timebar);
    _img_set_src_timed(bk_ui->timebar_sound_checkim, "/images/timebar_volume_off.png");
    lv_obj_set_pos(bk_ui->timebar_sound_checkim, 810, 0);
    lv_obj_set_size(bk_ui->timebar_sound_checkim, 60, 60);

    // Button: sound_checkbt
    bk_ui->timebar_sound_checkbt = lv_button_create(bk_ui->timebar);
    lv_obj_add_flag(bk_ui->timebar_sound_checkbt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->timebar_sound_checkbt, timebar_sound_checkbt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->timebar_sound_checkbt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->timebar_sound_checkbt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->timebar_sound_checkbt, 0, 0);
    lv_obj_set_pos(bk_ui->timebar_sound_checkbt, 810, 0);
    lv_obj_set_size(bk_ui->timebar_sound_checkbt, 60, 60);
     // TextView: timebar_datetxt
    bk_ui->timebar_timebar_datetxt = lv_label_create(bk_ui->timebar);
    lv_digital_date_register(bk_ui->timebar_timebar_datetxt);
    //  lv_label_set_text(bk_ui->timebar_timebar_datetxt, "12:12");
    lv_obj_set_style_bg_opa(bk_ui->timebar_timebar_datetxt, 0, 0);
    lv_obj_set_style_text_color(bk_ui->timebar_timebar_datetxt, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->timebar_timebar_datetxt, &lv_font_scdream_regular_22, 0);
    lv_obj_set_pos(bk_ui->timebar_timebar_datetxt, 874, 5);
    lv_obj_set_size(bk_ui->timebar_timebar_datetxt, 144, 26);
    lv_obj_set_style_text_align(bk_ui->timebar_timebar_datetxt, LV_TEXT_ALIGN_RIGHT, 0);


        // TextView: timebar_timetxt
    bk_ui->timebar_timebar_timetxt = lv_label_create(bk_ui->timebar);
    // lv_label_set_text(bk_ui->timebar_timebar_timetxt, "2026.04.23");
    lv_digital_clock_register(bk_ui->timebar_timebar_timetxt, 0, 0, 11, 25, 50);  
    lv_obj_set_style_bg_opa(bk_ui->timebar_timebar_timetxt, 0, 0);
    lv_obj_set_style_text_color(bk_ui->timebar_timebar_timetxt, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->timebar_timebar_timetxt, &lv_font_scdream_regular_22, 0);
    // lv_obj_set_pos(bk_ui->timebar_timebar_timetxt, 874, 33);
    lv_obj_set_size(bk_ui->timebar_timebar_timetxt, 144, 26);
        // 좌표(874, 33) 대신 우측 상단을 기준으로 정렬 (x는 오른쪽에서 6만큼 안으로, y는 위에서 33만큼 아래로)
    lv_obj_align(bk_ui->timebar_timebar_timetxt, LV_ALIGN_TOP_RIGHT, -6, 33);
    lv_obj_set_style_text_align(bk_ui->timebar_timebar_timetxt, LV_TEXT_ALIGN_RIGHT, 0);

    /* 아이콘 상태 갱신 타이머 시작 (1초 주기) */
    timebar_start_icon_timer();
}
