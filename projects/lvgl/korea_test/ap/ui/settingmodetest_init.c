#include "lvgl.h"
#include "beken_ui.h"
#include "custom_func.h"
#include "event_runtime.h"
#include "ui_lang.h"
#include "settings.h"
#include <stdio.h>
#include <string.h>

#include "ui_config.h"
#include "preRenderer.h"

#define TAG "[settingmodetest_init.c] "
#define bk_printf(fmt, ...) do {if(0) printf(fmt, ##__VA_ARGS__); } while(0) // disable printf

extern bk_lv_ui_t bk_lv_tool_ui;
extern lv_obj_t *preRenderRoot;
extern void settingmodetest_backbt_event_cb(lv_event_t *e);
extern void settingmodetest_compbt_event_cb(lv_event_t *e);
extern void settingmodetest_roomfanbt_event_cb(lv_event_t *e);
extern void settingmodetest_fireheaterbt_event_cb(lv_event_t *e);
extern void settingmodetest_humidityheaterbt_event_cb(lv_event_t *e);
extern void settingmodetest_watervalvebt_event_cb(lv_event_t *e);
extern void settingmodetest_defrostheaterbt_event_cb(lv_event_t *e);
extern void settingmodetest_ledbt_event_cb(lv_event_t *e);
extern void settingmodetest_cabinetheaterbt_event_cb(lv_event_t *e);
extern void settingmodetest_damperbt_event_cb(lv_event_t *e);
extern void settingmodetest_test_error_check_bt_event_cb(lv_event_t *e);
extern void settingmodetest_load_start_event_cb(lv_event_t *e);
extern void settingmodetest_loaded_event_cb(lv_event_t *e);
extern void settingmodetest_unload_start_event_cb(lv_event_t *e);
extern void settingmodetest_unloaded_event_cb(lv_event_t *e);

/* testmode_box.jpg 전용 canvas 버퍼 — 공유 LVGL 이미지 캐시(LV_CACHE_DEF_SIZE)
 * 밖에 있어 popuppassword 등 다른 prewarm과 캐시 자리를 두고 경쟁하지 않음
 * (과거 공유캐시 기반 testmode prewarm이 LRU eviction을 일으켰던 문제 회피,
 * settingmode_cb.c의 s_tm_bg_* 관련 주석 참고). settingmode 화면에 머무는
 * 동안 settingmode_cb.c에서 preload/free 호출. */
static void     *s_tmbox_canvas_buf = NULL;
static lv_obj_t *s_tmbox_canvas     = NULL;
static int       s_tmbox_buf_lang   = -1;

/* s_tmbox_canvas_buf는 main_activity_on_create()에서 부팅 극초반(단편화 전)
 * 1회만 malloc되고 이후 절대 free되지 않는다 — 예전엔 settingmodetest 화면을
 * 나갈 때마다 free하고 들어올 때마다 재malloc해서, psram_malloc_cm이 실패 시
 * NULL 대신 즉시 assert 크래시하는 이 프로젝트 특성상 단편화가 쌓이면
 * (want=852148 B) 하드크래시로 이어졌다(automodestart/reset_popup과 동일 원인,
 * teraterm719final.log에서 실측 확인). */
void settingmodetest_canvas_buf_alloc(void)
{
    if (s_tmbox_canvas_buf) return;
    uint32_t buf_sz = LV_CANVAS_BUF_SIZE(984, 433, 16, LV_DRAW_BUF_ALIGN);
    s_tmbox_canvas_buf = lv_malloc(buf_sz);
    if (s_tmbox_canvas_buf)
        bk_printf(TAG "[PERF] testmode_box buf alloc ok (%lu B)\n", (unsigned long)buf_sz);
    else
        bk_printf(TAG "[PERF] testmode_box buf alloc FAILED\n");
}

void settingmodetest_canvas_free(void)
{
#if !UI_SETTINGMODETEST_PREWARM_ENABLE
    return;
#endif
    /* 버퍼는 절대 free하지 않음(위 설명 참고) — canvas 위젯만 정리.
     * UI_CANVAS_BUF_PERMANENT_ENABLE=0이면 예전처럼 화면 이탈 시 버퍼도 반납. */
    if (s_tmbox_canvas && lv_obj_is_valid(s_tmbox_canvas)) {
        lv_obj_del(s_tmbox_canvas);
        s_tmbox_canvas = NULL;
    }
#if !UI_CANVAS_BUF_PERMANENT_ENABLE
    lv_free(s_tmbox_canvas_buf);
    s_tmbox_canvas_buf = NULL;
#endif
    s_tmbox_buf_lang   = -1;
}

void settingmodetest_bg_preload(void)
{
#if !UI_SETTINGMODETEST_PREWARM_ENABLE
    return;
#endif
    int lang = settings_get_int("LANGUAGE");
    if (s_tmbox_buf_lang == lang && s_tmbox_canvas) return;  /* 이미 decode됨 */

    const char *lsuf = (lang == 1) ? "_china" : (lang == 2) ? "_english" : "";
    char path[128];
    snprintf(path, sizeof(path), "/images/testmode_box%s.jpg", lsuf);

    if (!s_tmbox_canvas_buf) settingmodetest_canvas_buf_alloc();  /* 안전망 */
    if (!s_tmbox_canvas_buf) return;  /* 힙 부족 — 진입 시 원본 JPG decode로 fallback */

    uint32_t buf_sz = LV_CANVAS_BUF_SIZE(984, 433, 16, LV_DRAW_BUF_ALIGN);

    memset(s_tmbox_canvas_buf, 0, buf_sz);

    if (s_tmbox_canvas && lv_obj_is_valid(s_tmbox_canvas)) lv_obj_del(s_tmbox_canvas);
    s_tmbox_canvas = lv_canvas_create(lv_layer_top());
    lv_canvas_set_buffer(s_tmbox_canvas, s_tmbox_canvas_buf, 984, 433, LV_COLOR_FORMAT_RGB565);
    lv_obj_add_flag(s_tmbox_canvas, LV_OBJ_FLAG_HIDDEN);

    lv_layer_t layer;
    lv_canvas_init_layer(s_tmbox_canvas, &layer);
    lv_draw_image_dsc_t img_dsc;
    lv_draw_image_dsc_init(&img_dsc);
    img_dsc.src = path;
    lv_area_t area = {0, 0, 983, 432};
    lv_draw_image(&layer, &img_dsc, &area);
    lv_canvas_finish_layer(s_tmbox_canvas, &layer);

    s_tmbox_buf_lang = lang;
    bk_printf(TAG "[PERF] testmode_box preloaded: %s\n", path);
}

/* settingmodetest는 keep-alive라 init_page_settingmodetest()의 1회성 src
 * 설정만으로는 부족하다 — canvas가 settingmodetest 이탈 시 free됐다가 재진입
 * 시 새 버퍼로 재decode되면(주소가 바뀌므로) imageview4가 예전 canvas를 계속
 * 참조하는 use-after-free가 된다. 그래서 매 진입(SCREEN_LOAD_START)마다 이
 * 함수를 호출해 canvas를 보장하고 src를 다시 맞춘다. */
void settingmodetest_apply_bg(bk_lv_ui_t *bk_ui)
{
    settingmodetest_bg_preload();
    if (s_tmbox_canvas && lv_obj_is_valid(s_tmbox_canvas) &&
        bk_ui->settingmodetest_imageview4 && lv_obj_is_valid(bk_ui->settingmodetest_imageview4)) {
        lv_image_set_src(bk_ui->settingmodetest_imageview4, lv_canvas_get_image(s_tmbox_canvas));
    }
}

void destroy_page_settingmodetest(bk_lv_ui_t *bk_ui)
{
    if (bk_ui == NULL) {
        return;
    }
    if (bk_ui->settingmodetest != NULL) {
        lv_obj_del(bk_ui->settingmodetest);
        bk_ui->settingmodetest = NULL;
    }
    /* imageview4가 canvas를 src로 참조 중일 수 있으므로, 그 참조를 들고 있는
     * settingmodetest 오브젝트가 실제로 파괴될 때만 canvas를 해제한다.
     * (settingmode SCREEN_UNLOAD_START에서 해제하면 keep-alive로 살아있는
     * imageview4가 이미 해제된 메모리를 계속 참조하는 use-after-free 발생). */
    settingmodetest_canvas_free();
}

void init_page_settingmodetest(bk_lv_ui_t * bk_ui) {
#if UI_SETTINGMODETEST_KEEPALIVE_ENABLE
    if (bk_ui->settingmodetest != NULL && lv_obj_is_valid(bk_ui->settingmodetest)) {
        return;  /* keep-alive: reuse existing screen, load_event_cb handles refresh */
    }
#else
    if (bk_ui->settingmodetest != NULL && lv_obj_is_valid(bk_ui->settingmodetest)) {
        destroy_page_settingmodetest(bk_ui);
    }
#endif

    ui_lang_reset_settingmodetest_cache();

#if UI_PRENDERING_ENABLE
    bk_ui->settingmodetest = lv_obj_create(preRenderRoot);
    lv_obj_remove_style_all(bk_ui->settingmodetest);
    lv_obj_set_size(bk_ui->settingmodetest, 1024, 600);
    lv_obj_set_pos(bk_ui->settingmodetest, 0, 0);
    lv_obj_set_style_radius(bk_ui->settingmodetest, 0, LV_PART_MAIN);
    lv_obj_set_scrollbar_mode(bk_ui->settingmodetest, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_event_cb(bk_ui->settingmodetest, settingmodetest_load_start_event_cb, UI_EVENT_PAGE_SHOW_START,   NULL);
    lv_obj_add_event_cb(bk_ui->settingmodetest, settingmodetest_unload_start_event_cb, UI_EVENT_PAGE_HIDE_START, NULL);
    lv_obj_add_event_cb(bk_ui->settingmodetest, settingmodetest_loaded_event_cb, UI_EVENT_PAGE_SHOWN,       NULL);
    lv_obj_add_event_cb(bk_ui->settingmodetest, settingmodetest_unloaded_event_cb, UI_EVENT_PAGE_HIDDEN,     NULL);
#else
    bk_ui->settingmodetest = lv_obj_create(NULL);
    lv_obj_set_size(bk_ui->settingmodetest, 1024, 600);
    lv_obj_set_scrollbar_mode(bk_ui->settingmodetest, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_event_cb(bk_ui->settingmodetest, settingmodetest_load_start_event_cb, LV_EVENT_SCREEN_LOAD_START,   NULL);
    lv_obj_add_event_cb(bk_ui->settingmodetest, settingmodetest_unload_start_event_cb, LV_EVENT_SCREEN_UNLOAD_START, NULL);
    lv_obj_add_event_cb(bk_ui->settingmodetest, settingmodetest_loaded_event_cb, LV_EVENT_SCREEN_LOADED,       NULL);
    lv_obj_add_event_cb(bk_ui->settingmodetest, settingmodetest_unloaded_event_cb, LV_EVENT_SCREEN_UNLOADED,     NULL);
#endif /* UI_PRENDERING_ENABLE */
    /* 배경 — bg.jpg 대신 단색(0xd9d9d9) */
    bk_ui->settingmodetest_bg = lv_image_create(bk_ui->settingmodetest);
    lv_obj_add_flag(bk_ui->settingmodetest_bg, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_style_bg_color(bk_ui->settingmodetest, lv_color_hex(0xd9d9d9), 0);
    lv_obj_set_style_bg_opa(bk_ui->settingmodetest, LV_OPA_COVER, 0);
    lv_obj_set_pos(bk_ui->settingmodetest_bg, 0, 0);

    // ImageView: title
    bk_ui->settingmodetest_title = lv_image_create(bk_ui->settingmodetest);
#if !UI_SETTINGMODETEST_DEFERRED_LOAD_ENABLE
    _img_set_src_timed(bk_ui->settingmodetest_title, "/images/test_title.png");
#endif
    lv_obj_set_pos(bk_ui->settingmodetest_title, 0, 10);
    lv_obj_set_size(bk_ui->settingmodetest_title, 380, 80);
    lv_image_set_inner_align(bk_ui->settingmodetest_title, LV_IMAGE_ALIGN_TOP_LEFT);

    // Button: backbt
    bk_ui->settingmodetest_backbt = lv_button_create(bk_ui->settingmodetest);
    lv_obj_add_flag(bk_ui->settingmodetest_backbt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->settingmodetest_backbt, settingmodetest_backbt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->settingmodetest_backbt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->settingmodetest_backbt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->settingmodetest_backbt, 0, 0);
    lv_obj_set_pos(bk_ui->settingmodetest_backbt, 825, 13);
    lv_obj_set_size(bk_ui->settingmodetest_backbt, 179, 74);

    // ImageView: imageview3
    bk_ui->settingmodetest_imageview3 = lv_image_create(bk_ui->settingmodetest);
#if !UI_SETTINGMODETEST_DEFERRED_LOAD_ENABLE
    _img_set_src_timed(bk_ui->settingmodetest_imageview3, "/images/exit_bt.png");
#endif
    lv_obj_set_pos(bk_ui->settingmodetest_imageview3, 825, 13);
    lv_obj_set_size(bk_ui->settingmodetest_imageview3, 179, 74);

    // ImageView: imageview4
    bk_ui->settingmodetest_imageview4 = lv_image_create(bk_ui->settingmodetest);
#if !UI_SETTINGMODETEST_DEFERRED_LOAD_ENABLE
    _img_set_src_timed(bk_ui->settingmodetest_imageview4, "/images/testmode_box.jpg");
#endif
    lv_obj_set_pos(bk_ui->settingmodetest_imageview4, 20, 100);
    lv_obj_set_size(bk_ui->settingmodetest_imageview4, 984, 433);

    // Button: compbt
    bk_ui->settingmodetest_compbt = lv_button_create(bk_ui->settingmodetest);
    lv_obj_add_flag(bk_ui->settingmodetest_compbt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->settingmodetest_compbt, settingmodetest_compbt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->settingmodetest_compbt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->settingmodetest_compbt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->settingmodetest_compbt, 0, 0);
    lv_obj_set_pos(bk_ui->settingmodetest_compbt, 44, 132);
    lv_obj_set_size(bk_ui->settingmodetest_compbt, 308, 60);

    // ImageView: compim
    bk_ui->settingmodetest_compim = lv_image_create(bk_ui->settingmodetest);
    lv_obj_set_pos(bk_ui->settingmodetest_compim, 44, 132);
    lv_obj_set_size(bk_ui->settingmodetest_compim, 308, 60);

    // Button: roomfanbt
    bk_ui->settingmodetest_roomfanbt = lv_button_create(bk_ui->settingmodetest);
    lv_obj_add_flag(bk_ui->settingmodetest_roomfanbt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->settingmodetest_roomfanbt, settingmodetest_roomfanbt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->settingmodetest_roomfanbt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->settingmodetest_roomfanbt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->settingmodetest_roomfanbt, 0, 0);
    lv_obj_set_pos(bk_ui->settingmodetest_roomfanbt, 358, 132);
    lv_obj_set_size(bk_ui->settingmodetest_roomfanbt, 308, 60);

    // ImageView: roomfanim
    bk_ui->settingmodetest_roomfanim = lv_image_create(bk_ui->settingmodetest);
    lv_obj_set_pos(bk_ui->settingmodetest_roomfanim, 358, 132);
    lv_obj_set_size(bk_ui->settingmodetest_roomfanim, 308, 60);

    // Button: fireheaterbt
    bk_ui->settingmodetest_fireheaterbt = lv_button_create(bk_ui->settingmodetest);
    lv_obj_add_flag(bk_ui->settingmodetest_fireheaterbt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->settingmodetest_fireheaterbt, settingmodetest_fireheaterbt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->settingmodetest_fireheaterbt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->settingmodetest_fireheaterbt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->settingmodetest_fireheaterbt, 0, 0);
    lv_obj_set_pos(bk_ui->settingmodetest_fireheaterbt, 672, 132);
    lv_obj_set_size(bk_ui->settingmodetest_fireheaterbt, 308, 60);

    // ImageView: fireheaterim
    bk_ui->settingmodetest_fireheaterim = lv_image_create(bk_ui->settingmodetest);
    lv_obj_set_pos(bk_ui->settingmodetest_fireheaterim, 672, 132);
    lv_obj_set_size(bk_ui->settingmodetest_fireheaterim, 308, 60);

    // Button: humidityheaterbt
    bk_ui->settingmodetest_humidityheaterbt = lv_button_create(bk_ui->settingmodetest);
    lv_obj_add_flag(bk_ui->settingmodetest_humidityheaterbt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->settingmodetest_humidityheaterbt, settingmodetest_humidityheaterbt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->settingmodetest_humidityheaterbt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->settingmodetest_humidityheaterbt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->settingmodetest_humidityheaterbt, 0, 0);
    lv_obj_set_pos(bk_ui->settingmodetest_humidityheaterbt, 44, 199);
    lv_obj_set_size(bk_ui->settingmodetest_humidityheaterbt, 308, 60);

    // ImageView: humidityheaterim
    bk_ui->settingmodetest_humidityheaterim = lv_image_create(bk_ui->settingmodetest);
    lv_obj_set_pos(bk_ui->settingmodetest_humidityheaterim, 44, 199);
    lv_obj_set_size(bk_ui->settingmodetest_humidityheaterim, 308, 60);

    // Button: watervalvebt
    bk_ui->settingmodetest_watervalvebt = lv_button_create(bk_ui->settingmodetest);
    lv_obj_add_flag(bk_ui->settingmodetest_watervalvebt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->settingmodetest_watervalvebt, settingmodetest_watervalvebt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->settingmodetest_watervalvebt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->settingmodetest_watervalvebt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->settingmodetest_watervalvebt, 0, 0);
    lv_obj_set_pos(bk_ui->settingmodetest_watervalvebt, 358, 199);
    lv_obj_set_size(bk_ui->settingmodetest_watervalvebt, 308, 60);

    // ImageView: watervalveim
    bk_ui->settingmodetest_watervalveim = lv_image_create(bk_ui->settingmodetest);
    lv_obj_set_pos(bk_ui->settingmodetest_watervalveim, 358, 199);
    lv_obj_set_size(bk_ui->settingmodetest_watervalveim, 308, 60);

    // Button: defrostheaterbt
    bk_ui->settingmodetest_defrostheaterbt = lv_button_create(bk_ui->settingmodetest);
    lv_obj_add_flag(bk_ui->settingmodetest_defrostheaterbt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->settingmodetest_defrostheaterbt, settingmodetest_defrostheaterbt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->settingmodetest_defrostheaterbt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->settingmodetest_defrostheaterbt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->settingmodetest_defrostheaterbt, 0, 0);
    lv_obj_set_pos(bk_ui->settingmodetest_defrostheaterbt, 672, 199);
    lv_obj_set_size(bk_ui->settingmodetest_defrostheaterbt, 308, 60);

    // ImageView: defrostheaterim
    bk_ui->settingmodetest_defrostheaterim = lv_image_create(bk_ui->settingmodetest);
    lv_obj_set_pos(bk_ui->settingmodetest_defrostheaterim, 672, 199);
    lv_obj_set_size(bk_ui->settingmodetest_defrostheaterim, 308, 60);

    // Button: ledbt
    bk_ui->settingmodetest_ledbt = lv_button_create(bk_ui->settingmodetest);
    lv_obj_add_flag(bk_ui->settingmodetest_ledbt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->settingmodetest_ledbt, settingmodetest_ledbt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->settingmodetest_ledbt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->settingmodetest_ledbt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->settingmodetest_ledbt, 0, 0);
    lv_obj_set_pos(bk_ui->settingmodetest_ledbt, 44, 266);
    lv_obj_set_size(bk_ui->settingmodetest_ledbt, 308, 60);

    // ImageView: ledim
    bk_ui->settingmodetest_ledim = lv_image_create(bk_ui->settingmodetest);
    lv_obj_set_pos(bk_ui->settingmodetest_ledim, 44, 266);
    lv_obj_set_size(bk_ui->settingmodetest_ledim, 308, 60);

    // Button: cabinetheaterbt
    bk_ui->settingmodetest_cabinetheaterbt = lv_button_create(bk_ui->settingmodetest);
    lv_obj_add_flag(bk_ui->settingmodetest_cabinetheaterbt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->settingmodetest_cabinetheaterbt, settingmodetest_cabinetheaterbt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->settingmodetest_cabinetheaterbt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->settingmodetest_cabinetheaterbt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->settingmodetest_cabinetheaterbt, 0, 0);
    lv_obj_set_pos(bk_ui->settingmodetest_cabinetheaterbt, 358, 266);
    lv_obj_set_size(bk_ui->settingmodetest_cabinetheaterbt, 308, 60);

    // ImageView: cabinetheaterim
    bk_ui->settingmodetest_cabinetheaterim = lv_image_create(bk_ui->settingmodetest);
    lv_obj_set_pos(bk_ui->settingmodetest_cabinetheaterim, 358, 266);
    lv_obj_set_size(bk_ui->settingmodetest_cabinetheaterim, 308, 60);

    // Button: damperbt
    bk_ui->settingmodetest_damperbt = lv_button_create(bk_ui->settingmodetest);
    lv_obj_add_flag(bk_ui->settingmodetest_damperbt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->settingmodetest_damperbt, settingmodetest_damperbt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->settingmodetest_damperbt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->settingmodetest_damperbt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->settingmodetest_damperbt, 0, 0);
    lv_obj_set_pos(bk_ui->settingmodetest_damperbt, 672, 266);
    lv_obj_set_size(bk_ui->settingmodetest_damperbt, 308, 60);

    // ImageView: damperim
    bk_ui->settingmodetest_damperim = lv_image_create(bk_ui->settingmodetest);
    lv_obj_set_pos(bk_ui->settingmodetest_damperim, 672, 266);
    lv_obj_set_size(bk_ui->settingmodetest_damperim, 308, 60);

    // TextView: TestFreezeTemp
    bk_ui->settingmodetest_TestFreezeTemp = lv_label_create(bk_ui->settingmodetest);
    lv_label_set_text(bk_ui->settingmodetest_TestFreezeTemp, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmodetest_TestFreezeTemp, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmodetest_TestFreezeTemp, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmodetest_TestFreezeTemp, &lv_font_scdream_regular_22, 0);
    lv_obj_set_style_text_align(bk_ui->settingmodetest_TestFreezeTemp, LV_TEXT_ALIGN_LEFT, 0);
    lv_obj_set_pos(bk_ui->settingmodetest_TestFreezeTemp, 179-12, 468+6+2);
    lv_obj_set_size(bk_ui->settingmodetest_TestFreezeTemp, 63, 53);

    // TextView: TestComp
    bk_ui->settingmodetest_TestComp = lv_label_create(bk_ui->settingmodetest);
    lv_label_set_text(bk_ui->settingmodetest_TestComp, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmodetest_TestComp, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmodetest_TestComp, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmodetest_TestComp, &lv_font_scdream_regular_22, 0);
    lv_obj_set_style_text_align(bk_ui->settingmodetest_TestComp, LV_TEXT_ALIGN_LEFT, 0);
    lv_obj_set_pos(bk_ui->settingmodetest_TestComp, 176-12, 360+6+2);
    lv_obj_set_size(bk_ui->settingmodetest_TestComp, 63, 53);

    // TextView: TestHumidity
    bk_ui->settingmodetest_TestHumidity = lv_label_create(bk_ui->settingmodetest);
    lv_label_set_text(bk_ui->settingmodetest_TestHumidity, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmodetest_TestHumidity, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmodetest_TestHumidity, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmodetest_TestHumidity, &lv_font_scdream_regular_22, 0);
    lv_obj_set_style_text_align(bk_ui->settingmodetest_TestHumidity, LV_TEXT_ALIGN_LEFT, 0);
    lv_obj_set_pos(bk_ui->settingmodetest_TestHumidity, 755-12, 468+6+2);
    lv_obj_set_size(bk_ui->settingmodetest_TestHumidity, 63, 53);

    // TextView: TestDamper
    bk_ui->settingmodetest_TestDamper = lv_label_create(bk_ui->settingmodetest);
    lv_label_set_text(bk_ui->settingmodetest_TestDamper, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmodetest_TestDamper, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmodetest_TestDamper, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmodetest_TestDamper, &lv_font_scdream_regular_22, 0);
    lv_obj_set_style_text_align(bk_ui->settingmodetest_TestDamper, LV_TEXT_ALIGN_LEFT, 0);
    lv_obj_set_pos(bk_ui->settingmodetest_TestDamper, 368-12, 360+6+2);
    lv_obj_set_size(bk_ui->settingmodetest_TestDamper, 63, 53);

    // TextView: TestDcLed
    bk_ui->settingmodetest_TestDcLed = lv_label_create(bk_ui->settingmodetest);
    lv_label_set_text(bk_ui->settingmodetest_TestDcLed, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmodetest_TestDcLed, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmodetest_TestDcLed, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmodetest_TestDcLed, &lv_font_scdream_regular_22, 0);
    lv_obj_set_style_text_align(bk_ui->settingmodetest_TestDcLed, LV_TEXT_ALIGN_LEFT, 0);
    lv_obj_set_pos(bk_ui->settingmodetest_TestDcLed, 560-12, 360+6+2);
    lv_obj_set_size(bk_ui->settingmodetest_TestDcLed, 63, 53);

    // TextView: TestWaterValve
    bk_ui->settingmodetest_TestWaterValve = lv_label_create(bk_ui->settingmodetest);
    lv_label_set_text(bk_ui->settingmodetest_TestWaterValve, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmodetest_TestWaterValve, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmodetest_TestWaterValve, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmodetest_TestWaterValve, &lv_font_scdream_regular_22, 0);
    lv_obj_set_style_text_align(bk_ui->settingmodetest_TestWaterValve, LV_TEXT_ALIGN_LEFT, 0);
    lv_obj_set_pos(bk_ui->settingmodetest_TestWaterValve, 752-12, 360+6+2);
    lv_obj_set_size(bk_ui->settingmodetest_TestWaterValve, 63, 53);

    // TextView: TestDefrostHeater
    bk_ui->settingmodetest_TestDefrostHeater = lv_label_create(bk_ui->settingmodetest);
    lv_label_set_text(bk_ui->settingmodetest_TestDefrostHeater, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmodetest_TestDefrostHeater, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmodetest_TestDefrostHeater, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmodetest_TestDefrostHeater, &lv_font_scdream_regular_22, 0);
    lv_obj_set_style_text_align(bk_ui->settingmodetest_TestDefrostHeater, LV_TEXT_ALIGN_LEFT, 0);
    lv_obj_set_pos(bk_ui->settingmodetest_TestDefrostHeater, 176-12, 414+6+2);
    lv_obj_set_size(bk_ui->settingmodetest_TestDefrostHeater, 63, 53);

    // TextView: TestFireHeater
    bk_ui->settingmodetest_TestFireHeater = lv_label_create(bk_ui->settingmodetest);
    lv_label_set_text(bk_ui->settingmodetest_TestFireHeater, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmodetest_TestFireHeater, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmodetest_TestFireHeater, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmodetest_TestFireHeater, &lv_font_scdream_regular_22, 0);
    lv_obj_set_style_text_align(bk_ui->settingmodetest_TestFireHeater, LV_TEXT_ALIGN_LEFT, 0);
    lv_obj_set_pos(bk_ui->settingmodetest_TestFireHeater, 752-12, 414+6+2);
    lv_obj_set_size(bk_ui->settingmodetest_TestFireHeater, 63, 53);

    // TextView: TestHumidityHeater
    bk_ui->settingmodetest_TestHumidityHeater = lv_label_create(bk_ui->settingmodetest);
    lv_label_set_text(bk_ui->settingmodetest_TestHumidityHeater, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmodetest_TestHumidityHeater, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmodetest_TestHumidityHeater, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmodetest_TestHumidityHeater, &lv_font_scdream_regular_22, 0);
    lv_obj_set_style_text_align(bk_ui->settingmodetest_TestHumidityHeater, LV_TEXT_ALIGN_LEFT, 0);
    lv_obj_set_pos(bk_ui->settingmodetest_TestHumidityHeater, 368-12, 414+6+2);
    lv_obj_set_size(bk_ui->settingmodetest_TestHumidityHeater, 63, 53);

    // TextView: TestCabinetHeater
    bk_ui->settingmodetest_TestCabinetHeater = lv_label_create(bk_ui->settingmodetest);
    lv_label_set_text(bk_ui->settingmodetest_TestCabinetHeater, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmodetest_TestCabinetHeater, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmodetest_TestCabinetHeater, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmodetest_TestCabinetHeater, &lv_font_scdream_regular_22, 0);
    lv_obj_set_style_text_align(bk_ui->settingmodetest_TestCabinetHeater, LV_TEXT_ALIGN_LEFT, 0);
    lv_obj_set_pos(bk_ui->settingmodetest_TestCabinetHeater, 560-12, 414+6+2);
    lv_obj_set_size(bk_ui->settingmodetest_TestCabinetHeater, 63, 53);

    // TextView: TestRoomFan
    bk_ui->settingmodetest_TestRoomFan = lv_label_create(bk_ui->settingmodetest);
    lv_label_set_text(bk_ui->settingmodetest_TestRoomFan, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmodetest_TestRoomFan, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmodetest_TestRoomFan, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmodetest_TestRoomFan, &lv_font_scdream_regular_22, 0);
    lv_obj_set_style_text_align(bk_ui->settingmodetest_TestRoomFan, LV_TEXT_ALIGN_LEFT, 0);
    lv_obj_set_pos(bk_ui->settingmodetest_TestRoomFan, 944-12, 360+6+2);
    lv_obj_set_size(bk_ui->settingmodetest_TestRoomFan, 63, 53);

    // TextView: TestDefrostSensorTemp
    bk_ui->settingmodetest_TestDefrostSensorTemp = lv_label_create(bk_ui->settingmodetest);
    lv_label_set_text(bk_ui->settingmodetest_TestDefrostSensorTemp, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmodetest_TestDefrostSensorTemp, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmodetest_TestDefrostSensorTemp, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmodetest_TestDefrostSensorTemp, &lv_font_scdream_regular_22, 0);
    lv_obj_set_style_text_align(bk_ui->settingmodetest_TestDefrostSensorTemp, LV_TEXT_ALIGN_LEFT, 0);
    lv_obj_set_pos(bk_ui->settingmodetest_TestDefrostSensorTemp, 371-12, 468+6+2);
    lv_obj_set_size(bk_ui->settingmodetest_TestDefrostSensorTemp, 63, 53);

    // TextView: TestRTSensorTemp
    bk_ui->settingmodetest_TestRTSensorTemp = lv_label_create(bk_ui->settingmodetest);
    lv_label_set_text(bk_ui->settingmodetest_TestRTSensorTemp, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmodetest_TestRTSensorTemp, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmodetest_TestRTSensorTemp, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmodetest_TestRTSensorTemp, &lv_font_scdream_regular_22, 0);
    lv_obj_set_style_text_align(bk_ui->settingmodetest_TestRTSensorTemp, LV_TEXT_ALIGN_LEFT, 0);
    lv_obj_set_pos(bk_ui->settingmodetest_TestRTSensorTemp, 563-12, 468+6+2);
    lv_obj_set_size(bk_ui->settingmodetest_TestRTSensorTemp, 63, 53);

    // TextView: TestErrorCode
    bk_ui->settingmodetest_TestErrorCode = lv_label_create(bk_ui->settingmodetest);
    lv_label_set_text(bk_ui->settingmodetest_TestErrorCode, "");
    lv_obj_set_style_bg_opa(bk_ui->settingmodetest_TestErrorCode, 0, 0);
    lv_obj_set_style_text_color(bk_ui->settingmodetest_TestErrorCode, lv_color_hex(0x3C3A3D), 0);
    lv_obj_set_style_text_font(bk_ui->settingmodetest_TestErrorCode, &lv_font_scdream_regular_22, 0);
    lv_obj_set_style_text_align(bk_ui->settingmodetest_TestErrorCode, LV_TEXT_ALIGN_LEFT, 0);
    lv_obj_set_pos(bk_ui->settingmodetest_TestErrorCode, 947-12, 468+6+2);
    lv_obj_set_size(bk_ui->settingmodetest_TestErrorCode, 63, 53);

    // Button: test_error_check_bt — 기능은 timebar_error_checkbt와 동일(에러 발생시에만 표시,
    // 클릭 시 popuperror_toggle)
    bk_ui->settingmodetest_test_error_check_bt = lv_button_create(bk_ui->settingmodetest);
    lv_obj_add_flag(bk_ui->settingmodetest_test_error_check_bt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->settingmodetest_test_error_check_bt, settingmodetest_test_error_check_bt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->settingmodetest_test_error_check_bt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->settingmodetest_test_error_check_bt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->settingmodetest_test_error_check_bt, 0, 0);
    lv_obj_set_pos(bk_ui->settingmodetest_test_error_check_bt, 545, 13);
    lv_obj_set_size(bk_ui->settingmodetest_test_error_check_bt, 249, 74);
    lv_obj_add_flag(bk_ui->settingmodetest_test_error_check_bt, LV_OBJ_FLAG_HIDDEN);

    // ImageView: test_error_check_im (lazy-loaded on first show) — 원본 249x74
    bk_ui->settingmodetest_test_error_check_im = lv_image_create(bk_ui->settingmodetest);
    lv_obj_add_flag(bk_ui->settingmodetest_test_error_check_im, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_pos(bk_ui->settingmodetest_test_error_check_im, 545, 13);
    lv_obj_set_size(bk_ui->settingmodetest_test_error_check_im, 249, 74);
    _img_set_src_deferred(bk_ui->settingmodetest_test_error_check_im, "/images/test_error_check_im.png");

#if UI_SETTINGMODETEST_DEFERRED_LOAD_ENABLE
    ui_lang_apply_settingmodetest(bk_ui);  /* fallback: 파일 경로 설정 (canvas 없을 때) */
#if UI_SETTINGMODETEST_PREWARM_ENABLE
    /* canvas 버퍼가 준비됐으면 raw 픽셀로 덮어씀 → decode 없이 즉시 렌더.
     * keep-alive라 이 오브젝트 생성은 처음 한 번뿐이지만, canvas는 매 진입마다
     * settingmodetest_apply_bg()(settingmodetest_cb.c의 SCREEN_LOAD_START)가
     * 다시 보장/재적용하므로 canvas가 나갔다 들어왔다 free/재decode돼도 안전하다. */
    settingmodetest_apply_bg(bk_ui);
#endif
#endif
}

void init_page_settingmodetest_with_step(bk_lv_ui_t *bk_ui)
{
    return;
}
