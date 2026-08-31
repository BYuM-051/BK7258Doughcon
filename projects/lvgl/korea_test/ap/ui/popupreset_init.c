#include "lvgl.h"
#include "beken_ui.h"
#include "custom_func.h"
#include "event_runtime.h"
#include <stdio.h>
#include <string.h>

#include "ui_lang.h"
#include "settings.h"

#include "preRenderer.h"
#include "preRenderInfo.h"
#define TAG "[popupreset_init.c] "
#define bk_printf(fmt, ...) do {if(0) printf(fmt, ##__VA_ARGS__); } while(0) // disable printf

extern bk_lv_ui_t bk_lv_tool_ui;
extern void popupreset_yesbt_event_cb(lv_event_t *e);
extern void popupreset_nobt_event_cb(lv_event_t *e);
extern void popupreset_load_event_cb(lv_event_t *e);

/* reset_popup 전용 canvas 버퍼.
 * UI_POPUP_DIALOG_JPG_ENABLE=1: 알파를 에셋에 미리 합성한 .jpg(RGB565,
 *   364KB)를 씀 — 알파가 필요 없어져 버퍼가 절반으로 줄어듦.
 * UI_POPUP_DIALOG_JPG_ENABLE=0: 알파 있는 원본 .png(ARGB8888, 728KB) 사용.
 * 어느 쪽이든 **이 프로젝트의 PSRAM 할당자(psram_malloc_cm)가 실패 시 NULL
 * 대신 즉시 assert로 하드 크래시**하므로, 버퍼는 bg_canvas_buf_alloc()과
 * 동일한 패턴으로 **부팅 극초반(main_activity_on_create, 단편화 전)에 딱
 * 1회만 malloc**하고 이후 절대 free하지 않는다. canvas 위젯(오브젝트) 자체는
 * 화면 진입/이탈에 맞춰 만들었다 지운다(가볍다, 버퍼 재사용). */
#if UI_POPUP_DIALOG_JPG_ENABLE
#define _RP_BPP  16
#define _RP_FMT  LV_COLOR_FORMAT_RGB565
#define _RP_EXT  ".jpg"
/* jpg는 투명 여백을 잘라내고 배경색으로 합성한 498x328 에셋(원본 520x350에서
 * 상하좌우 ~11px 투명 마진을 trim) — 화면 배치 좌표는 그만큼 +11,+11 보정 */
#define _RP_W    498
#define _RP_H    328
#else
#define _RP_BPP  32
#define _RP_FMT  LV_COLOR_FORMAT_ARGB8888
#define _RP_EXT  ".png"
#define _RP_W    520
#define _RP_H    350
#endif

static void     *s_rp_canvas_buf = NULL;
static lv_obj_t *s_rp_canvas     = NULL;
static int       s_rp_buf_lang   = -1;

/* main_activity_on_create()에서 1회 호출 — 버퍼만 선점(단순 lv_malloc,
 * LVGL task/canvas 생성 불필요). bg_canvas_buf_alloc()과 동일 패턴. */
void popupreset_canvas_buf_alloc(void)
{
    if (s_rp_canvas_buf) return;
    uint32_t buf_sz = LV_CANVAS_BUF_SIZE(_RP_W, _RP_H, _RP_BPP, LV_DRAW_BUF_ALIGN);
    s_rp_canvas_buf = lv_malloc(buf_sz);
    if (s_rp_canvas_buf)
        bk_printf(TAG "[POPUP] reset_popup buf alloc ok (%lu B)\n", (unsigned long)buf_sz);
    else
        bk_printf(TAG "[POPUP] reset_popup buf alloc FAILED\n");
}

void popupreset_canvas_free(void)
{
    /* 버퍼는 절대 free하지 않음(위 설명 참고) — canvas 위젯만 정리.
     * UI_CANVAS_BUF_PERMANENT_ENABLE=0이면 예전처럼 화면 이탈 시 버퍼도 반납. */
    if (s_rp_canvas && lv_obj_is_valid(s_rp_canvas)) {
        lv_obj_del(s_rp_canvas);
        s_rp_canvas = NULL;
    }
#if !UI_CANVAS_BUF_PERMANENT_ENABLE
    lv_free(s_rp_canvas_buf);
    s_rp_canvas_buf = NULL;
#endif
    s_rp_buf_lang = -1;
}

void popupreset_bg_preload(void)
{
    int lang = settings_get_int("LANGUAGE");
    if (s_rp_buf_lang == lang && s_rp_canvas) return;  /* 이미 decode됨 */

    const char *lsuf = (lang == 1) ? "_china" : (lang == 2) ? "_english" : "";
    char path[128];
    snprintf(path, sizeof(path), "/images/reset_popup%s" _RP_EXT, lsuf);

    if (!s_rp_canvas_buf) popupreset_canvas_buf_alloc();  /* 안전망 — 정상적으론 부팅 시 이미 완료 */
    if (!s_rp_canvas_buf) return;  /* 그래도 실패면 팝업 열릴 때 원본 decode로 fallback */

    uint32_t buf_sz = LV_CANVAS_BUF_SIZE(_RP_W, _RP_H, _RP_BPP, LV_DRAW_BUF_ALIGN);
    memset(s_rp_canvas_buf, 0, buf_sz);

    if (s_rp_canvas && lv_obj_is_valid(s_rp_canvas)) lv_obj_del(s_rp_canvas);
    s_rp_canvas = lv_canvas_create(lv_layer_top());
    lv_canvas_set_buffer(s_rp_canvas, s_rp_canvas_buf, _RP_W, _RP_H, _RP_FMT);
    lv_obj_add_flag(s_rp_canvas, LV_OBJ_FLAG_HIDDEN);

    lv_layer_t layer;
    lv_canvas_init_layer(s_rp_canvas, &layer);
    lv_draw_image_dsc_t img_dsc;
    lv_draw_image_dsc_init(&img_dsc);
    img_dsc.src = path;
    lv_area_t area = {0, 0, _RP_W - 1, _RP_H - 1};
    lv_draw_image(&layer, &img_dsc, &area);
    lv_canvas_finish_layer(s_rp_canvas, &layer);

    s_rp_buf_lang = lang;
    bk_printf(TAG "[PERF] reset_popup preloaded: %s\n", path);
}

void destroy_page_popupreset(bk_lv_ui_t *bk_ui)
{
    if (bk_ui == NULL) {
        return;
    }
    if (bk_ui->popupreset != NULL) {
        lv_obj_del(bk_ui->popupreset);
        bk_ui->popupreset = NULL;
    }
}

void init_page_popupreset(bk_lv_ui_t * bk_ui) {
    if (bk_ui->popupreset != NULL && lv_obj_is_valid(bk_ui->popupreset)) {
        destroy_page_popupreset(bk_ui);
    }

    /* lv_layer_top()에 오버레이 — 배경 화면 유지하면서 팝업만 표시 */
    ui_lang_reset_popupreset_cache();
    bk_ui->popupreset = lv_obj_create(lv_layer_top());
    lv_obj_set_size(bk_ui->popupreset, 1024, 600);
    lv_obj_set_pos(bk_ui->popupreset, 0, 0);
#if POPUP_OPAQUE_BG
    lv_obj_set_style_bg_color(bk_ui->popupreset, lv_color_hex(0x1C1C1C), 0);
    lv_obj_set_style_bg_opa(bk_ui->popupreset, LV_OPA_COVER, 0);
#else
    lv_obj_set_style_bg_color(bk_ui->popupreset, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(bk_ui->popupreset, LV_OPA_50, 0);
#endif
    lv_obj_set_style_border_width(bk_ui->popupreset, 0, 0);
    lv_obj_set_style_pad_all(bk_ui->popupreset, 0, 0);
    lv_obj_set_scrollbar_mode(bk_ui->popupreset, LV_SCROLLBAR_MODE_OFF);

    // ImageView: imageview1 (reset_popup.png — yes/no 버튼 포함 다이얼로그 이미지)
    bk_ui->popupreset_imageview1 = lv_image_create(bk_ui->popupreset);
#if UI_POPUP_DIALOG_JPG_ENABLE
    /* jpg 에셋이 원본 대비 상하좌우 11px씩 trim되어 있으므로 배치 좌표를
     * +11,+11 보정해 화면상 위치가 기존 png(239,82)와 동일하게 유지되게 함 */
    lv_obj_set_pos(bk_ui->popupreset_imageview1, 239 + 11, 82 + 11);
#else
    lv_obj_set_pos(bk_ui->popupreset_imageview1, 239, 82);
#endif
    lv_obj_set_size(bk_ui->popupreset_imageview1, LV_SIZE_CONTENT, LV_SIZE_CONTENT);

    // Button: yesbt
    bk_ui->popupreset_yesbt = lv_button_create(bk_ui->popupreset);
    lv_obj_add_flag(bk_ui->popupreset_yesbt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->popupreset_yesbt, popupreset_yesbt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->popupreset_yesbt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->popupreset_yesbt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->popupreset_yesbt, 0, 0);
    lv_obj_set_pos(bk_ui->popupreset_yesbt, 496, 348);
    lv_obj_set_size(bk_ui->popupreset_yesbt, 255, 75);

    // Button: nobt
    bk_ui->popupreset_nobt = lv_button_create(bk_ui->popupreset);
    lv_obj_add_flag(bk_ui->popupreset_nobt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bk_ui->popupreset_nobt, popupreset_nobt_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(bk_ui->popupreset_nobt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->popupreset_nobt, 0, 0);
    lv_obj_set_style_shadow_width(bk_ui->popupreset_nobt, 0, 0);
    lv_obj_set_pos(bk_ui->popupreset_nobt, 244, 348);
    lv_obj_set_size(bk_ui->popupreset_nobt, 255, 75);

    ui_lang_apply_popupreset(bk_ui);  /* fallback: PNG 경로 설정 (canvas 없을 때) */
    if (s_rp_canvas && lv_obj_is_valid(s_rp_canvas))
        lv_image_set_src(bk_ui->popupreset_imageview1, lv_canvas_get_image(s_rp_canvas));
}

rendererFuncStatus_t init_page_popupreset_with_step(bk_lv_ui_t *bk_ui)
{
    return RENDERER_FUNC_FAILED;
}
