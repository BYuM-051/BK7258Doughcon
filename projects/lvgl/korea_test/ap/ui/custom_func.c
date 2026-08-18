#include "custom_func.h"
#include "beken_ui.h"
#include "event_runtime.h"
#include <os/os.h>
#include <stdio.h>
#include <string.h>

/* _img_set_src_timed / _img_set_src_deferred / _img_ensure_src 는
 * ui/custom/custom_func.c 에 정의됨 — 여기서 재정의하면 링커 중복 오류 발생.
 * 이 파일은 custom/ 디렉터리에 없는 새 함수만 추가한다. */

#if UI_CACHE_DROP_LOW_MEM_ENABLE
/* ── PSRAM 저메모리 시 공유 이미지 캐시 강제 정리(공용 헬퍼) ──────────────
 * uart_comm.c(화면 전환 시마다)에서만 체크하면, 그 체크 시점과 실제 위험한
 * malloc 시점(예: picker_1/2.png decode) 사이에 갭이 생겨 놓칠 수 있다
 * (want=565508 B picker 크래시가 화면 전환 직후가 아니라 그 화면 안에서
 * 다음 picker 페이지로 넘길 때 발생한 사례 있음). 이 헬퍼를 위험한 개별
 * decode 호출 직전에도 추가로 불러, 체크 지점을 늘려 갭을 줄인다.
 * 쿨다운은 전역 static 하나를 모든 호출자가 공유 — 여러 지점에서 동시에
 * 호출돼도 캐시 드롭이 20초 내에 중복 발동하지 않게 함. */
static uint32_t s_last_low_mem_drop_tick = 0;
static bool     s_low_mem_drop_done_once = false;

void ui_cache_drop_if_low_mem(void)
{
    uint32_t free_now = (uint32_t)rtos_get_psram_free_heap_size();
    if (free_now >= UI_CACHE_DROP_LOW_MEM_THRESHOLD_BYTES) return;
    if (s_low_mem_drop_done_once &&
        lv_tick_elaps(s_last_low_mem_drop_tick) < UI_CACHE_DROP_LOW_MEM_COOLDOWN_MS) {
        return;
    }
    lv_image_cache_drop(NULL);
    s_last_low_mem_drop_tick = lv_tick_get();
    s_low_mem_drop_done_once = true;
    printf("[MEM] shared image cache dropped (low mem: free=%u B < %u B)\n",
           (unsigned)free_now, (unsigned)UI_CACHE_DROP_LOW_MEM_THRESHOLD_BYTES);
}
#endif /* UI_CACHE_DROP_LOW_MEM_ENABLE */

#if !UI_BG_SOLID_COLOR && UI_BG_CANVAS_PRELOAD_ENABLE
/* ── bg.jpg 공유 canvas 영구 버퍼 ───────────────────────────────────────
 * 여러 화면이 동일한 bg.jpg 사용 → 버퍼 1개 · decode 1회 · 모든 화면 재사용
 * lv_layer_top() HIDDEN canvas → 화면 전환과 무관하게 포인터 항상 유효
 * main_activity_on_create 초기 호출 → 힙 파편화 전 연속 블록 확보 */
static void     *s_bg_canvas_buf = NULL;
static lv_obj_t *s_bg_canvas     = NULL;

/* 버퍼만 선점 — main_activity_on_create에서 호출 (LVGL task 시작 전 단순 malloc)
 * canvas 생성 + JPEG decode는 bg_canvas_preload()에서 지연 실행 */
void bg_canvas_buf_alloc(void)
{
    if (s_bg_canvas_buf) return;
    uint32_t buf_sz = LV_CANVAS_BUF_SIZE(1024, 540, 16, LV_DRAW_BUF_ALIGN);
    s_bg_canvas_buf = lv_malloc(buf_sz);
    if (s_bg_canvas_buf)
        printf("[BG] buf alloc ok (%lu B)\n", (unsigned long)buf_sz);
    else
        printf("[BG] buf alloc failed\n");
}

/* canvas 생성 + JPEG decode — 첫 _bg_set 시점(LVGL task 실행 중)에 호출됨.
 * idempotent: 이미 완료된 경우 즉시 반환.
 * PSRAM < 1.3 MB 여유 시 decode skip → _bg_set fallback → JPEG 직접 로드. */
void bg_canvas_preload(void)
{
    if (s_bg_canvas && lv_obj_is_valid(s_bg_canvas)) return;

    /* buf_alloc이 main_activity_on_create에서 안 됐을 경우 fallback */
    if (!s_bg_canvas_buf) {
        uint32_t buf_sz = LV_CANVAS_BUF_SIZE(1024, 540, 16, LV_DRAW_BUF_ALIGN);
        s_bg_canvas_buf = lv_malloc(buf_sz);
    }
    if (!s_bg_canvas_buf) {
        printf("[BG] buf unavailable\n");
        return;
    }

    /* JPEG SW decode에 ~1.06 MB 임시 버퍼 필요 — PSRAM 부족 시 assert 방지 */
    size_t free_psram = rtos_get_psram_free_heap_size();
    printf("[BG] psram free = %u B\n", (unsigned)free_psram);
    if (free_psram < 1300 * 1024) {
        printf("[BG] psram low, skip canvas decode (fallback to JPEG)\n");
        return;
    }

    s_bg_canvas = lv_canvas_create(lv_layer_top());
    lv_canvas_set_buffer(s_bg_canvas, s_bg_canvas_buf, 1024, 540, LV_COLOR_FORMAT_RGB565);
    lv_obj_add_flag(s_bg_canvas, LV_OBJ_FLAG_HIDDEN);

    lv_layer_t layer;
    lv_canvas_init_layer(s_bg_canvas, &layer);
    lv_draw_image_dsc_t img_dsc;
    lv_draw_image_dsc_init(&img_dsc);
    img_dsc.src = "/images/bg.jpg";
    lv_area_t area = {0, 0, 1023, 539};
    lv_draw_image(&layer, &img_dsc, &area);
    lv_canvas_finish_layer(s_bg_canvas, &layer);
    printf("[BG] canvas ok\n");
}

const lv_image_dsc_t *bg_canvas_get_dsc(void)
{
    if (!s_bg_canvas || !lv_obj_is_valid(s_bg_canvas)) return NULL;
    return lv_canvas_get_image(s_bg_canvas);
}
#endif /* !UI_BG_SOLID_COLOR && UI_BG_CANVAS_PRELOAD_ENABLE */

