#include "lvgl.h"
#include "beken_ui.h"
#include "custom_func.h"
#include "event_runtime.h"
#include <stdio.h>
#include <string.h>

#include "ui_lang.h"
#include "settings.h"

#define TAG "[popuppassword_init.c] "
#define bk_printf(fmt, ...) do {if(0) printf(fmt, ##__VA_ARGS__); } while(0) // disable printf

extern bk_lv_ui_t bk_lv_tool_ui;
extern void popuppassword_pop_keypad1_event_cb(lv_event_t *e);
extern void popuppassword_pop_keypad2_event_cb(lv_event_t *e);
extern void popuppassword_pop_keypad3_event_cb(lv_event_t *e);
extern void popuppassword_pop_keypad4_event_cb(lv_event_t *e);
extern void popuppassword_pop_keypad5_event_cb(lv_event_t *e);
extern void popuppassword_pop_keypad6_event_cb(lv_event_t *e);
extern void popuppassword_pop_keypad7_event_cb(lv_event_t *e);
extern void popuppassword_pop_keypad8_event_cb(lv_event_t *e);
extern void popuppassword_pop_keypad9_event_cb(lv_event_t *e);
extern void popuppassword_pop_keypadbackspace_event_cb(lv_event_t *e);
extern void popuppassword_pop_keypad0_event_cb(lv_event_t *e);
extern void popuppassword_pop_keypadalldel_event_cb(lv_event_t *e);  
extern void popuppassword_pop_dismiss_event_cb(lv_event_t *e);
extern void popuppassword_pop_enter_event_cb(lv_event_t *e);
extern void lv_digital_clock_pause(void);
extern void lv_digital_clock_resume(void);

/* password_popup.png canvas 버퍼 — LVGL 캐시 외부, lv_image_cache_drop 영향 없음.
 * settingmode 진입 시 alloc, 이탈 시 free (자동운전/testmode_box/reset_popup처럼
 * 영구 고정도 시도해봤으나, 실측 결과 4개 버퍼(2.77MB)만 고정해도 free가
 * 4.19MB→1.25MB까지 떨어지며 공유 이미지 캐시 쪽의 평범한 아이콘 decode(245KB)가
 * 단편화로 실패하는 새 크래시를 유발함(teraterm 로그로 확인) — 이 버퍼는 실제
 * 크래시가 확인된 적이 없어 영구 고정의 이득보다 다른 곳 압박 비용이 더 크다고
 * 판단, 원래 방식(진입마다 alloc, 이탈마다 free)으로 되돌림. */
static void     *s_pw_canvas_buf = NULL;   /* settingmode 진입 시 alloc, 이탈 시 free */
static lv_obj_t *s_pw_canvas     = NULL;   /* lv_layer_top() HIDDEN 자식, 소유자 역할 */
static int       s_pw_buf_lang   = -1;     /* decode된 언어 캐시 키 */

/* 팝업 오픈 시 숨긴 active screen 포인터 — close 시 복원용 */
static lv_obj_t *s_hidden_screen = NULL;

#if UI_POPUPPASSWORD_COMBINED_BG_ENABLE
/* init_page_popuppassword()가 lv_scr_load()로 팝업을 활성화하는 그 순간에만 1 —
 * 이 순간 settingmode의 SCREEN_UNLOAD_START가 동기적으로 발생하는데, 그게
 * "사용자가 settingmode를 완전히 떠남"이 아니라 "팝업을 여는 중"임을
 * settingmode_cb.c가 구분할 수 있도록 하는 가드. */
static int s_pw_screen_opening = 0;
int popuppassword_is_screen_opening(void) { return s_pw_screen_opening; }
#endif

void destroy_page_popuppassword(bk_lv_ui_t *bk_ui)
{
    if (bk_ui == NULL) {
        return;
    }
    if (bk_ui->popuppassword != NULL) {
#if UI_POPUPPASSWORD_COMBINED_BG_ENABLE
        /* 독립 화면으로 열었으므로, 삭제 전에 act_scr을 안전한 화면(settingmode)
         * 으로 되돌려야 함 — 그대로 삭제하면 act_scr이 dangling pointer가 됨.
         * 호출자(성공 시 _pw_validate, 취소 시 dismiss)가 바로 이어서 다른
         * 화면으로 lv_scr_load하면, 이 settingmode 전환은 실제 렌더 전에
         * 곧바로 대체되어 화면에는 보이지 않는다. */
        if (lv_scr_act() == bk_ui->popuppassword &&
            bk_ui->settingmode && lv_obj_is_valid(bk_ui->settingmode)) {
            lv_scr_load(bk_ui->settingmode);
        }
#endif
        lv_obj_del(bk_ui->popuppassword);
        bk_ui->popuppassword = NULL;
    }
    /* active screen 복원 — 팝업이 덮고 있던 화면을 다시 표시 */
#if UI_POPUPPASSWORD_HIDE_ACTIVE_SCREEN_ENABLE
    if (s_hidden_screen && lv_obj_is_valid(s_hidden_screen)) {
        lv_obj_clear_flag(s_hidden_screen, LV_OBJ_FLAG_HIDDEN);
    }
    s_hidden_screen = NULL;
#endif
    lv_digital_clock_resume();
}

/* settingmode SCREEN_UNLOAD_START에서 호출 — canvas + buf 반납 (약 602 KB) */
void popuppassword_canvas_free(void)
{
#if UI_POPUPPASSWORD_COMBINED_BG_ENABLE
    /* combined-bg 모드에서는 imageview1이 파일 기반 결합 이미지를 직접 쓰므로
     * 이 canvas 자체를 만들지 않음 — 호출은 유지하되 아무 것도 하지 않음. */
    return;
#endif
    if (s_pw_canvas && lv_obj_is_valid(s_pw_canvas)) {
        lv_obj_del(s_pw_canvas);
        s_pw_canvas = NULL;
    }
    lv_free(s_pw_canvas_buf);
    s_pw_canvas_buf = NULL;
    s_pw_buf_lang   = -1;
}

/* password_popup.png 1회 decode → canvas 버퍼
 * settingmode SCREEN_LOADED에서 호출 → 사용자가 버튼 누르기 전에 decode 완료
 * lv_image_cache_drop(NULL)과 무관: canvas buf는 LVGL 캐시 외부
 * combined-bg 모드에서는 imageview1이 결합 이미지 파일을 직접 로드하므로 no-op */
void popuppassword_bg_preload(void)
{
#if UI_POPUPPASSWORD_COMBINED_BG_ENABLE
    return;
#endif
    int lang = settings_get_int("LANGUAGE");
    if (s_pw_buf_lang == lang && s_pw_canvas) return;  /* 이미 decode됨 */

    const char *lsuf = (lang == 1) ? "_china" : (lang == 2) ? "_english" : "";
    char path[128];
    snprintf(path, sizeof(path), "/images/password_popup%s.jpg",lsuf);

    /* RGB565(알파 없음, 828×372×2 ≈ 602 KB) 사용. 죽은 여백 테두리를 에셋에서
     * 잘라내(850x392 → 828x372) 필요 없는 픽셀만큼 더 가벼워짐.
     * password_popup*.png는 더 이상 투명 영역이 없음 — 둥근 모서리 바깥쪽을
     * 팝업 배경 실측색(bg.jpg를 60% 검정 오버레이로 dim했을 때의 결과, #5B5853)으로
     * 이미지 자체에 미리 칠해뒀다(에셋 수정, 코드 아님). 그래서 알파 채널이나
     * ARGB8888 같은 별도 처리 없이도 모서리가 배경과 자연스럽게 섞인다.
     * (한때 이 문제를 ARGB8888(1302 KB)로 해결했으나 canvas가 settingmode 진입/
     * 이탈마다 재할당되는 구조라 요청 크기가 커지며 PSRAM 단편화 crash가 늘었음
     * → 에셋을 고쳐 알파 자체를 없애는 이 방식이 메모리도 가장 가볍고 안전함) */
    uint32_t buf_sz = LV_CANVAS_BUF_SIZE(828, 372, 16, LV_DRAW_BUF_ALIGN);
    if (!s_pw_canvas_buf) {
        s_pw_canvas_buf = lv_malloc(buf_sz);
    }
    if (!s_pw_canvas_buf) return;  /* 힙 부족 — 팝업 열릴 때 원본 PNG decode로 fallback */

    /* decode 전 버퍼 초기화: 이미지가 완전 불투명이라 전체가 덮어써지지만,
     * lv_malloc은 초기화하지 않으므로 안전하게 0으로 초기화해둠. */
    memset(s_pw_canvas_buf, 0, buf_sz);

    /* 이전 canvas 객체 제거 후 재생성 (lang 변경 또는 첫 호출) */
    if (s_pw_canvas && lv_obj_is_valid(s_pw_canvas)) lv_obj_del(s_pw_canvas);
    s_pw_canvas = lv_canvas_create(lv_layer_top());
    lv_canvas_set_buffer(s_pw_canvas, s_pw_canvas_buf, 828, 372, LV_COLOR_FORMAT_RGB565);
    lv_obj_add_flag(s_pw_canvas, LV_OBJ_FLAG_HIDDEN);

    lv_layer_t layer;
    lv_canvas_init_layer(s_pw_canvas, &layer);
    lv_draw_image_dsc_t img_dsc;
    lv_draw_image_dsc_init(&img_dsc);
    img_dsc.src = path;
    lv_area_t area = {0, 0, 827, 371};
    lv_draw_image(&layer, &img_dsc, &area);
    lv_canvas_finish_layer(s_pw_canvas, &layer);

    s_pw_buf_lang = lang;
    bk_printf(TAG "[POPUP] password_popup preloaded: %s\n", path);
}

/* 버튼 dirty area 완전 차단:
 * state-specific selector(PRESSED/FOCUSED)를 추가하면 LVGL이 상태 전환마다
 * lv_obj_invalidate()를 호출한다. LV_PART_MAIN(=DEFAULT) 단독 사용 시
 * 어떤 state로 전환해도 "style 변화 없음"으로 판단 → invalidate 스킵. */
static void _btn_make_transp(lv_obj_t *btn)
{
    static lv_style_t s_transp;
    static bool s_init = false;
    if (!s_init) {
        s_init = true;
        lv_style_init(&s_transp);
        lv_style_set_bg_opa(&s_transp, 0);
        lv_style_set_border_width(&s_transp, 0);
        lv_style_set_shadow_width(&s_transp, 0);
        lv_style_set_outline_width(&s_transp, 0);
    }
    lv_obj_remove_style_all(btn);
    lv_obj_add_style(btn, &s_transp, LV_PART_MAIN);
}

void init_page_popuppassword(bk_lv_ui_t * bk_ui) {
    if (bk_ui->popuppassword != NULL && lv_obj_is_valid(bk_ui->popuppassword)) {
        destroy_page_popuppassword(bk_ui);
    }
    /* imageview1/pop_cautionim이 매번 새로 생성되므로, 다른 팝업들과 동일하게
     * 언어 캐시를 리셋해 ui_lang_apply_popuppassword()가 항상 src를 다시 설정하게 함
     * (canvas preload 실패 시의 fallback 경로가 캐시 때문에 스킵되는 것을 방지). */
    ui_lang_reset_popuppassword_cache();
    /* active screen을 HIDDEN 처리:
     * lv_layer_top() 자식은 LV_OPA_COVER와 무관하게 LVGL이 active screen을 먼저
     * 렌더한 뒤 layer_top을 합성한다. HIDDEN 처리하면 active screen 렌더가 완전히
     * 스킵되어 ~9μs/px → ~1μs/px (9배 단축). 팝업 닫힐 때 destroy에서 복원한다.
     * 주의: 과거 이 방식에서 몇 번 입력 후 MemFault 크래시 이력 있음 — A/B 테스트용. */
#if UI_POPUPPASSWORD_HIDE_ACTIVE_SCREEN_ENABLE
    s_hidden_screen = lv_scr_act();
    lv_obj_add_flag(s_hidden_screen, LV_OBJ_FLAG_HIDDEN);
    /* 숨긴 화면의 시계 라벨을 전역 1초 타이머가 계속 갱신하며 invalidate를
     * 유발하는 것이 크래시 원인으로 의심됨 (basic_callback.c의 popuptime
     * 배경 깨짐 버그와 동일 클래스) — 팝업이 떠 있는 동안 타이머 업데이트 자체를 끔. */
    lv_digital_clock_pause();
#endif
    bk_printf(TAG "pop up pasword");

#if UI_POPUPPASSWORD_COMBINED_BG_ENABLE
    /* settingmode의 자식으로 붙이는 방식은 실측 결과 오히려 더 느려짐 —
     * settingmode 자신의 렌더 트리에 팝업 서브트리까지 얹혀서 매 redraw마다
     * 같이 평가되는 비용이 추가된 것으로 보임. 대신 독립된 화면(lv_scr_load)
     * 으로 만든다 — 이게 automode/settingmode 등 다른 모든 화면이 이미
     * 쓰고 있는, LVGL이 완전히 지원하는 정상 전환 방식이라 settingmode는
     * 더 이상 act_scr가 아니게 되어 렌더 대상에서 아예 빠진다.
     * imageview1이 dim된 settingmode+카드를 미리 합성한 불투명 이미지라
     * bg_opa 반투명 누적 어두워짐 버그(주석 참고)도 발생하지 않는다. */
    bk_ui->popuppassword = lv_obj_create(NULL);
    lv_obj_set_size(bk_ui->popuppassword, 1024, 600);
    lv_obj_set_pos(bk_ui->popuppassword, 0, 0);
    lv_obj_set_scrollbar_mode(bk_ui->popuppassword, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_opa(bk_ui->popuppassword, LV_OPA_TRANSP, 0);  /* imageview1이 전체를 불투명하게 덮음 */
    lv_obj_set_style_border_width(bk_ui->popuppassword, 0, 0);
    lv_obj_set_style_pad_all(bk_ui->popuppassword, 0, 0);

    // ImageView: imageview1 — dim(settingmode)+카드 합성 이미지, 화면 전체를 덮음
    bk_ui->popuppassword_imageview1 = lv_image_create(bk_ui->popuppassword);
    lv_obj_set_pos(bk_ui->popuppassword_imageview1, 0, 0);
    lv_obj_set_size(bk_ui->popuppassword_imageview1, 1024, 600);
#else
    /* lv_layer_top() 자식으로 생성 — active screen이 유지되므로 bg 피드백 루프 없음
     * (popuptime과 동일 구조: lv_obj_create(NULL)+lv_scr_load 시 bg_opa 블렌딩이
     *  프레임버퍼를 직접 읽어 매 dirty area마다 누적 darkening 발생) */
    bk_ui->popuppassword = lv_obj_create(lv_layer_top());
    lv_obj_set_size(bk_ui->popuppassword, 1024, 600);
    lv_obj_set_pos(bk_ui->popuppassword, 0, 0);
    lv_obj_set_scrollbar_mode(bk_ui->popuppassword, LV_SCROLLBAR_MODE_OFF);
        // lv_obj_add_flag(bk_ui->popuppassword, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_style_bg_color(bk_ui->popuppassword, lv_color_black(), 0);
    lv_obj_set_style_border_width(bk_ui->popuppassword, 0, 0);  /* 기본 테마 흰색 테두리 제거 */
    lv_obj_set_style_radius(bk_ui->popuppassword, 0, 0);        /* 기본 테마 라운드 코너 제거 — 구석이 둥글게 보이던 문제 */
    // lv_obj_set_style_bg_opa(bk_ui->popuppassword, LV_OPA_50, 0);

#if UI_POPUPPASSWORD_SOLID_BG_ENABLE
    /* 불투명 단색 — 반투명 블렌딩 비용 자체가 없어짐 (keypad 속도 비교용) */
    lv_obj_set_style_bg_color(bk_ui->popuppassword, lv_color_hex(UI_POPUPPASSWORD_SOLID_BG_COLOR_HEX), 0);
    lv_obj_set_style_bg_opa(bk_ui->popuppassword, LV_OPA_COVER, 0);
#elif POPUP_OPAQUE_BG
    lv_obj_set_style_bg_color(bk_ui->popuppassword, lv_color_hex(0x1C1C1C), 0);
    lv_obj_set_style_bg_opa(bk_ui->popuppassword, LV_OPA_COVER, 0);
#else
    /* 100% 투명 — 뒤의 settingmode가 그대로 비쳐 보임(dim 없음) */
    lv_obj_set_style_bg_opa(bk_ui->popuppassword, LV_OPA_60, 0);
#endif
    lv_obj_set_style_pad_all(bk_ui->popuppassword, 0, 0);


    // ImageView: imageview1
    bk_ui->popuppassword_imageview1 = lv_image_create(bk_ui->popuppassword);
    // _img_set_src_timed(bk_ui->popuppassword_imageview1, "/images/password_popup.jpg");

    /* password_popup*.png의 죽은 여백 테두리(좌우 11px, 상하 10px)를 잘라내
     * 850x392 → 828x372로 줄이면서, 카드가 화면에서 보이는 위치는 그대로 유지되도록
     * 잘려나간 만큼 위치를 안쪽으로 이동 (90+11, 110+10). 키패드 버튼들은 화면
     * 절대좌표 기준이라 카드 내용 위치가 그대로면 별도 조정 불필요. */
    lv_obj_set_pos(bk_ui->popuppassword_imageview1, 101, 120);
    lv_obj_set_size(bk_ui->popuppassword_imageview1, 828, 372);
#endif
    
    // EditText: pop_edt — container + inner label for masked password display
    bk_ui->popuppassword_pop_edt = lv_obj_create(bk_ui->popuppassword);
    lv_obj_set_style_bg_opa(bk_ui->popuppassword_pop_edt, 0, 0);
    lv_obj_set_style_border_width(bk_ui->popuppassword_pop_edt, 0, 0);
    lv_obj_set_style_pad_all(bk_ui->popuppassword_pop_edt, 0, 0);
    lv_obj_set_scrollbar_mode(bk_ui->popuppassword_pop_edt, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_pos(bk_ui->popuppassword_pop_edt, 216, 310);
    lv_obj_set_size(bk_ui->popuppassword_pop_edt, 300, 40);

    {
        lv_obj_t *edt_lbl = lv_label_create(bk_ui->popuppassword_pop_edt);
        lv_label_set_text(edt_lbl, "");
        lv_obj_set_style_text_color(edt_lbl, lv_color_black(), 0);
        // lv_obj_set_style_text_font(edt_lbl, &lv_font_montserrat_36, 0);
        lv_obj_set_style_text_font(edt_lbl, &lv_font_scdream_regular_35, 0);

        lv_obj_set_style_text_align(edt_lbl, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_size(edt_lbl, 300, 40);
        lv_obj_set_pos(edt_lbl, 0, 0);
    }


    // // Unsupported widget: EditText id=pop_edt
    // bk_ui->popuppassword_pop_edt = lv_obj_create(bk_ui->popuppassword);
    // lv_obj_set_style_bg_opa(bk_ui->popuppassword_pop_edt, 0, 0);
    // lv_obj_set_pos(bk_ui->popuppassword_pop_edt, 216, 310);
    // lv_obj_set_size(bk_ui->popuppassword_pop_edt, 300, 40);

    // ImageView: pop_cautionim
    bk_ui->popuppassword_pop_cautionim = lv_image_create(bk_ui->popuppassword);
    _img_set_src_timed(bk_ui->popuppassword_pop_cautionim, "/images/popup_caution.png");
    lv_obj_add_flag(bk_ui->popuppassword_pop_cautionim, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_pos(bk_ui->popuppassword_pop_cautionim, 128, 363);
    /* popup_caution*.png 원본 해상도는 420x35 — 기존 370x35 박스가 원본보다 좁아
     * 영문판("Please check your password.") 우측이 잘려 보였음. 원본 크기로 맞춤. */
    lv_obj_set_size(bk_ui->popuppassword_pop_cautionim, 420, 35);

    // ImageView: pop_keypad1im
    bk_ui->popuppassword_pop_keypad1im = lv_image_create(bk_ui->popuppassword);
    _img_set_src_timed(bk_ui->popuppassword_pop_keypad1im, "/images/pop_keypad1.png");
    lv_obj_add_flag(bk_ui->popuppassword_pop_keypad1im, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_pos(bk_ui->popuppassword_pop_keypad1im, 603, 134);
    lv_obj_set_size(bk_ui->popuppassword_pop_keypad1im, 100, 80);

    // Button: pop_keypad1
    bk_ui->popuppassword_pop_keypad1 = lv_button_create(bk_ui->popuppassword);
    lv_obj_add_event_cb(bk_ui->popuppassword_pop_keypad1, popuppassword_pop_keypad1_event_cb, LV_EVENT_ALL, NULL);
    _btn_make_transp(bk_ui->popuppassword_pop_keypad1);
    lv_obj_set_pos(bk_ui->popuppassword_pop_keypad1, 603, 134);
    lv_obj_set_size(bk_ui->popuppassword_pop_keypad1, 100, 80);

    // ImageView: pop_keypad2im
    bk_ui->popuppassword_pop_keypad2im = lv_image_create(bk_ui->popuppassword);
    _img_set_src_timed(bk_ui->popuppassword_pop_keypad2im, "/images/pop_keypad2.png");
    lv_obj_add_flag(bk_ui->popuppassword_pop_keypad2im, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_pos(bk_ui->popuppassword_pop_keypad2im, 709, 134);
    lv_obj_set_size(bk_ui->popuppassword_pop_keypad2im, 100, 80);

    // Button: pop_keypad2
    bk_ui->popuppassword_pop_keypad2 = lv_button_create(bk_ui->popuppassword);
    lv_obj_add_event_cb(bk_ui->popuppassword_pop_keypad2, popuppassword_pop_keypad2_event_cb, LV_EVENT_ALL, NULL);
    _btn_make_transp(bk_ui->popuppassword_pop_keypad2);
    lv_obj_set_pos(bk_ui->popuppassword_pop_keypad2, 709, 134);
    lv_obj_set_size(bk_ui->popuppassword_pop_keypad2, 100, 80);

    // ImageView: pop_keypad3im
    bk_ui->popuppassword_pop_keypad3im = lv_image_create(bk_ui->popuppassword);
    _img_set_src_timed(bk_ui->popuppassword_pop_keypad3im, "/images/pop_keypad3.png");
    lv_obj_add_flag(bk_ui->popuppassword_pop_keypad3im, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_pos(bk_ui->popuppassword_pop_keypad3im, 815, 134);
    lv_obj_set_size(bk_ui->popuppassword_pop_keypad3im, 100, 80);

    // Button: pop_keypad3
    bk_ui->popuppassword_pop_keypad3 = lv_button_create(bk_ui->popuppassword);
    lv_obj_add_event_cb(bk_ui->popuppassword_pop_keypad3, popuppassword_pop_keypad3_event_cb, LV_EVENT_ALL, NULL);
    _btn_make_transp(bk_ui->popuppassword_pop_keypad3);
    lv_obj_set_pos(bk_ui->popuppassword_pop_keypad3, 815, 134);
    lv_obj_set_size(bk_ui->popuppassword_pop_keypad3, 100, 80);

    // ImageView: pop_keypad4im
    bk_ui->popuppassword_pop_keypad4im = lv_image_create(bk_ui->popuppassword);
    _img_set_src_timed(bk_ui->popuppassword_pop_keypad4im, "/images/pop_keypad4.png");
    lv_obj_add_flag(bk_ui->popuppassword_pop_keypad4im, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_pos(bk_ui->popuppassword_pop_keypad4im, 603, 222);
    lv_obj_set_size(bk_ui->popuppassword_pop_keypad4im, 100, 80);

    // Button: pop_keypad4
    bk_ui->popuppassword_pop_keypad4 = lv_button_create(bk_ui->popuppassword);
    lv_obj_add_event_cb(bk_ui->popuppassword_pop_keypad4, popuppassword_pop_keypad4_event_cb, LV_EVENT_ALL, NULL);
    _btn_make_transp(bk_ui->popuppassword_pop_keypad4);
    lv_obj_set_pos(bk_ui->popuppassword_pop_keypad4, 603, 222);
    lv_obj_set_size(bk_ui->popuppassword_pop_keypad4, 100, 80);

    // ImageView: pop_keypad5im
    bk_ui->popuppassword_pop_keypad5im = lv_image_create(bk_ui->popuppassword);
    _img_set_src_timed(bk_ui->popuppassword_pop_keypad5im, "/images/pop_keypad5.png");
    lv_obj_add_flag(bk_ui->popuppassword_pop_keypad5im, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_pos(bk_ui->popuppassword_pop_keypad5im, 709, 222);
    lv_obj_set_size(bk_ui->popuppassword_pop_keypad5im, 100, 80);

    // Button: pop_keypad5
    bk_ui->popuppassword_pop_keypad5 = lv_button_create(bk_ui->popuppassword);
    lv_obj_add_event_cb(bk_ui->popuppassword_pop_keypad5, popuppassword_pop_keypad5_event_cb, LV_EVENT_ALL, NULL);
    _btn_make_transp(bk_ui->popuppassword_pop_keypad5);
    lv_obj_set_pos(bk_ui->popuppassword_pop_keypad5, 709, 222);
    lv_obj_set_size(bk_ui->popuppassword_pop_keypad5, 100, 80);

    // ImageView: pop_keypad6im
    bk_ui->popuppassword_pop_keypad6im = lv_image_create(bk_ui->popuppassword);
    _img_set_src_timed(bk_ui->popuppassword_pop_keypad6im, "/images/pop_keypad6.png");
    lv_obj_add_flag(bk_ui->popuppassword_pop_keypad6im, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_pos(bk_ui->popuppassword_pop_keypad6im, 815, 222);
    lv_obj_set_size(bk_ui->popuppassword_pop_keypad6im, 100, 80);

    // Button: pop_keypad6
    bk_ui->popuppassword_pop_keypad6 = lv_button_create(bk_ui->popuppassword);
    lv_obj_add_event_cb(bk_ui->popuppassword_pop_keypad6, popuppassword_pop_keypad6_event_cb, LV_EVENT_ALL, NULL);
    _btn_make_transp(bk_ui->popuppassword_pop_keypad6);
    lv_obj_set_pos(bk_ui->popuppassword_pop_keypad6, 815, 222);
    lv_obj_set_size(bk_ui->popuppassword_pop_keypad6, 100, 80);

    // ImageView: pop_keypad7im
    bk_ui->popuppassword_pop_keypad7im = lv_image_create(bk_ui->popuppassword);
    _img_set_src_timed(bk_ui->popuppassword_pop_keypad7im, "/images/pop_keypad7.png");
    lv_obj_add_flag(bk_ui->popuppassword_pop_keypad7im, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_pos(bk_ui->popuppassword_pop_keypad7im, 603, 310);
    lv_obj_set_size(bk_ui->popuppassword_pop_keypad7im, 100, 80);

    // Button: pop_keypad7
    bk_ui->popuppassword_pop_keypad7 = lv_button_create(bk_ui->popuppassword);
    lv_obj_add_event_cb(bk_ui->popuppassword_pop_keypad7, popuppassword_pop_keypad7_event_cb, LV_EVENT_ALL, NULL);
    _btn_make_transp(bk_ui->popuppassword_pop_keypad7);
    lv_obj_set_pos(bk_ui->popuppassword_pop_keypad7, 603, 310);
    lv_obj_set_size(bk_ui->popuppassword_pop_keypad7, 100, 80);

    // ImageView: pop_keypad8im
    bk_ui->popuppassword_pop_keypad8im = lv_image_create(bk_ui->popuppassword);
    _img_set_src_timed(bk_ui->popuppassword_pop_keypad8im, "/images/pop_keypad8.png");
    lv_obj_add_flag(bk_ui->popuppassword_pop_keypad8im, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_pos(bk_ui->popuppassword_pop_keypad8im, 709, 310);
    lv_obj_set_size(bk_ui->popuppassword_pop_keypad8im, 100, 80);

    // Button: pop_keypad8
    bk_ui->popuppassword_pop_keypad8 = lv_button_create(bk_ui->popuppassword);
    lv_obj_add_event_cb(bk_ui->popuppassword_pop_keypad8, popuppassword_pop_keypad8_event_cb, LV_EVENT_ALL, NULL);
    _btn_make_transp(bk_ui->popuppassword_pop_keypad8);
    lv_obj_set_pos(bk_ui->popuppassword_pop_keypad8, 709, 310);
    lv_obj_set_size(bk_ui->popuppassword_pop_keypad8, 100, 80);

    // ImageView: pop_keypad9im
    bk_ui->popuppassword_pop_keypad9im = lv_image_create(bk_ui->popuppassword);
    _img_set_src_timed(bk_ui->popuppassword_pop_keypad9im, "/images/pop_keypad9.png");
    lv_obj_add_flag(bk_ui->popuppassword_pop_keypad9im, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_pos(bk_ui->popuppassword_pop_keypad9im, 815, 310);
    lv_obj_set_size(bk_ui->popuppassword_pop_keypad9im, 100, 80);

    // Button: pop_keypad9
    bk_ui->popuppassword_pop_keypad9 = lv_button_create(bk_ui->popuppassword);
    lv_obj_add_event_cb(bk_ui->popuppassword_pop_keypad9, popuppassword_pop_keypad9_event_cb, LV_EVENT_ALL, NULL);
    _btn_make_transp(bk_ui->popuppassword_pop_keypad9);
    lv_obj_set_pos(bk_ui->popuppassword_pop_keypad9, 815, 310);
    lv_obj_set_size(bk_ui->popuppassword_pop_keypad9, 100, 80);

    // ImageView: pop_keypadbackspaceim
    bk_ui->popuppassword_pop_keypadbackspaceim = lv_image_create(bk_ui->popuppassword);
    _img_set_src_timed(bk_ui->popuppassword_pop_keypadbackspaceim, "/images/pop_keypadback.png");
    lv_obj_add_flag(bk_ui->popuppassword_pop_keypadbackspaceim, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_pos(bk_ui->popuppassword_pop_keypadbackspaceim, 815, 398);
    lv_obj_set_size(bk_ui->popuppassword_pop_keypadbackspaceim, 100, 80);

    // Button: pop_keypadbackspace
    bk_ui->popuppassword_pop_keypadbackspace = lv_button_create(bk_ui->popuppassword);
    lv_obj_add_event_cb(bk_ui->popuppassword_pop_keypadbackspace, popuppassword_pop_keypadbackspace_event_cb, LV_EVENT_ALL, NULL);
    _btn_make_transp(bk_ui->popuppassword_pop_keypadbackspace);
    lv_obj_set_pos(bk_ui->popuppassword_pop_keypadbackspace, 815, 398);
    lv_obj_set_size(bk_ui->popuppassword_pop_keypadbackspace, 100, 80);

    // ImageView: pop_keypad0im
    bk_ui->popuppassword_pop_keypad0im = lv_image_create(bk_ui->popuppassword);
    _img_set_src_timed(bk_ui->popuppassword_pop_keypad0im, "/images/pop_keypad0.png");
    lv_obj_add_flag(bk_ui->popuppassword_pop_keypad0im, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_pos(bk_ui->popuppassword_pop_keypad0im, 709, 398);
    lv_obj_set_size(bk_ui->popuppassword_pop_keypad0im, 100, 80);

    // Button: pop_keypad0
    bk_ui->popuppassword_pop_keypad0 = lv_button_create(bk_ui->popuppassword);
    lv_obj_add_event_cb(bk_ui->popuppassword_pop_keypad0, popuppassword_pop_keypad0_event_cb, LV_EVENT_ALL, NULL);
    _btn_make_transp(bk_ui->popuppassword_pop_keypad0);
    lv_obj_set_pos(bk_ui->popuppassword_pop_keypad0, 709, 398);
    lv_obj_set_size(bk_ui->popuppassword_pop_keypad0, 100, 80);

    // ImageView: pop_keypadalldelim
    bk_ui->popuppassword_pop_keypadalldelim = lv_image_create(bk_ui->popuppassword);
    _img_set_src_timed(bk_ui->popuppassword_pop_keypadalldelim, "/images/pop_keypadalldel.png");
    lv_obj_add_flag(bk_ui->popuppassword_pop_keypadalldelim, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_pos(bk_ui->popuppassword_pop_keypadalldelim, 603, 398);
    lv_obj_set_size(bk_ui->popuppassword_pop_keypadalldelim, 100, 80);

    // Button: pop_keypadalldel
    bk_ui->popuppassword_pop_keypadalldel = lv_button_create(bk_ui->popuppassword);
    lv_obj_add_event_cb(bk_ui->popuppassword_pop_keypadalldel, popuppassword_pop_keypadalldel_event_cb, LV_EVENT_ALL, NULL);
    _btn_make_transp(bk_ui->popuppassword_pop_keypadalldel);
    lv_obj_set_pos(bk_ui->popuppassword_pop_keypadalldel, 603, 398);
    lv_obj_set_size(bk_ui->popuppassword_pop_keypadalldel, 100, 80);

    // Button: pop_dismiss
    bk_ui->popuppassword_pop_dismiss = lv_button_create(bk_ui->popuppassword);
    lv_obj_add_event_cb(bk_ui->popuppassword_pop_dismiss, popuppassword_pop_dismiss_event_cb, LV_EVENT_ALL, NULL);
    _btn_make_transp(bk_ui->popuppassword_pop_dismiss);
    lv_obj_set_pos(bk_ui->popuppassword_pop_dismiss, 100, 410);
    lv_obj_set_size(bk_ui->popuppassword_pop_dismiss, 245, 70);

    // Button: pop_enter
    bk_ui->popuppassword_pop_enter = lv_button_create(bk_ui->popuppassword);
    lv_obj_add_event_cb(bk_ui->popuppassword_pop_enter, popuppassword_pop_enter_event_cb, LV_EVENT_ALL, NULL);
    _btn_make_transp(bk_ui->popuppassword_pop_enter);
    lv_obj_set_pos(bk_ui->popuppassword_pop_enter, 346, 410);
    lv_obj_set_size(bk_ui->popuppassword_pop_enter, 245, 70);

    ui_lang_apply_popuppassword(bk_ui);  /* fallback: PNG 경로 설정 (canvas 없을 때) */
#if !UI_POPUPPASSWORD_COMBINED_BG_ENABLE
    /* canvas 버퍼가 준비됐으면 raw 픽셀로 덮어씀 → decode 없이 즉시 렌더 */
    if (s_pw_canvas && lv_obj_is_valid(s_pw_canvas))
        lv_image_set_src(bk_ui->popuppassword_imageview1,
                         lv_canvas_get_image(s_pw_canvas));
    /* timebar(index 0)보다 아래 z-order — timebar가 항상 위에 표시됨 */
    lv_obj_move_to_index(bk_ui->popuppassword, 0);
#endif
#if UI_POPUPPASSWORD_COMBINED_BG_ENABLE
    /* 독립 화면으로 전환 — timebar는 lv_layer_top()이라 어떤 화면이 active여도
     * 항상 그 위에 별도로 합성되므로 z-order 조정 불필요.
     * s_pw_screen_opening 가드: 이 호출이 settingmode의 SCREEN_UNLOAD_START를
     * 동기적으로 발생시키는데, 그게 "팝업을 여는 중"임을 알려서 settingmode_cb.c가
     * destroy_page_popuppassword()를 오발동하지 않게 한다. */
    s_pw_screen_opening = 1;
    lv_scr_load(bk_ui->popuppassword);
    s_pw_screen_opening = 0;
#endif
}



// void init_keypad_group(bk_lv_ui_t *bk_ui) {
//     static lv_style_t style_transp;
//     lv_style_init(&style_transp);
//     lv_style_set_bg_opa(&style_transp, LV_OPA_TRANSP);
//     lv_style_set_border_width(&style_transp, 0);
//     lv_style_set_shadow_width(&style_transp, 0);

//     // const int x_start = 20, y_start = 453, x_step = 72;
//     const int x_start = 600; // 키패드 시작 X 좌표
//     const int y_start = 124; // 키패드 시작 Y 좌표
//     const int x_gap = 106;   // 가로 간격 (버튼 너비 100 + 여백 10)
//     const int y_gap = 88;
//     const char *keypad_names[] = {"1","2","3","4","5","6","7","8","9","ce","0","back"};
//     char path_buf[64];

//     for (int i = 0; i < 12; i++) {
//         int col = i % 3; // 0, 1, 2
//         int row = i / 3; // 0, 1, 2, 3
        
//         int x_pos = x_start + (col * x_gap);
//         int y_pos = y_start + (row * y_gap);
//          // ImageView: pop_keypad1im
//         bk_ui->popuppassword_pop_keypadim[i] = lv_image_create(bk_ui->popuppassword);
//         snprintf(path_buf, sizeof(path_buf), "/images/keypad%s.png", keypad_names[i]);

//         _img_set_src_timed(bk_ui->popuppassword_pop_keypadim[i],path_buf);
//         lv_obj_add_flag(bk_ui->popuppassword_pop_keypadim[i], LV_OBJ_FLAG_HIDDEN);
//         // lv_obj_set_pos(bk_ui->popuppassword_pop_keypadim[i], 600, 124);
//         lv_obj_set_pos(bk_ui->popuppassword_pop_keypadim[i], x_pos, y_pos);


//         lv_obj_set_size(bk_ui->popuppassword_pop_keypadim[i], 100, 80);

//         // Button: pop_keypad1
//         bk_ui->popuppassword_pop_keypadBt[i] = lv_button_create(bk_ui->popuppassword);
//         lv_obj_add_flag(bk_ui->popuppassword_pop_keypadBt[i], LV_OBJ_FLAG_CLICKABLE);
//         lv_obj_add_event_cb(bk_ui->popuppassword_pop_keypadBt[i], popuppassword_pop_keypad_event_cb, LV_EVENT_ALL, NULL);
//         lv_obj_set_style_bg_opa(bk_ui->popuppassword_pop_keypadBt[i], 0, 0);
//         lv_obj_set_style_border_width(bk_ui->popuppassword_pop_keypadBt[i], 0, 0);
//         lv_obj_set_style_shadow_width(bk_ui->popuppassword_pop_keypadBt[i], 0, 0);
//         // lv_obj_set_pos(bk_ui->popuppassword_pop_keypadBt[i], 600, 124);
//         lv_obj_set_size(bk_ui->popuppassword_pop_keypadBt[i], 100, 80);
//         lv_obj_set_pos(bk_ui->popuppassword_pop_keypadBt[i], x_pos, y_pos);

//     }
        
// }
