/*
 * Copyright (c) 2025 BekenCorp. All rights reserved.
 * 
 * This software is proprietary and confidential. No part of this software may be
 * reproduced, distributed, or transmitted in any form or by any means, including
 * photocopying, recording, or other electronic or mechanical methods, without the
 * prior written permission of BekenCorp, except in the case of brief quotations
 * embodied in critical reviews and certain other noncommercial uses permitted
 * by copyright law.
 * 
 * For permission requests, write to BekenCorp at armino_support@bekencorp.com.

 * Author: Beken LVGL Designer Tool
*/
/*
 * @file: beken_ui.c
 * @brief: beken ui implementation file
 * This file contains the implementation of the Beken UI system.
 * Customers can modify the UI implementation in beken_ui.c without
 * touching the main application code.
 */

#include "beken_ui.h"
#include <stdio.h>
#include "main_activity.h"
#include "device_state.h"
#include "settings.h"
#include <os/os.h>
#include "lv_vendor.h"
#include "ui_lang.h"

#include "preRenderer.h"

#define TAG "[beken_ui.c] "
// #define bk_printf(fmt, ...) do {if(0) printf(fmt, ##__VA_ARGS__); } while(0) // disable printf

bk_lv_ui_t bk_lv_tool_ui = {0};
lv_obj_t *preRenderRoot = NULL; // renderer root 생명주기를 beken_ui.c에서 관리하도록 변경. preRenderer.c에서는 extern으로 선언만 한다.


/**
 * @brief Get the configured screen width
 * @return Screen width in pixels
 */
int beken_get_screen_width(void)
{
    return SCREEN_WIDTH;
}

/**
 * @brief Get the configured screen height
 * @return Screen height in pixels
 */
int beken_get_screen_height(void)
{
    return SCREEN_HEIGHT;
}

/**
 * @brief Initialize the Beken UI system
 */
extern void uart_comm_init(void);
extern void uart_comm_tick(void);
extern void settings_init(void);
extern void settings_load_from_flash(void);
extern void settings_flush(void);
extern void rtc_sync_init(void);
extern void rtc_sync_periodic_save(void);
extern void lvgl_vfs_init_flash_quick(void);
extern void lvgl_vfs_copy_and_remount_psram(void);

#ifndef HAL_USE_EMULATOR
static beken_thread_t s_uart_thread  = NULL;
static beken_thread_t s_flash_thread = NULL;

#if UI_BOOT_WARMUP_ENABLE
/* 부팅 중 automode만 한 번 렌더시켜, 최초 1회성 렌더 지연(~1초)을 부팅
 * 시간에 미리 치른다 — 사용자가 실제로 처음 들어갈 때는 이미 "두 번째
 * 진입"처럼 빠르게 느껴짐. introactivity는 일반 화면(scr_act)이라
 * lv_scr_load()를 하는 순간 가려지므로, intro.jpg를 lv_layer_top()
 * 오버레이로 하나 더 띄워 그 뒤에서 화면을 순회한다(password 팝업/timebar와
 * 동일한 오버레이 패턴) — 사용자 눈에는 계속 intro 화면이 떠 있는 것처럼
 * 보인다. 마지막에 main으로 전환하고 오버레이를 지우는 것으로 끝난다.
 *
 * 원래는 automode/memorymode/settingmode/settingmodetest 4개를 모두
 * warmup했으나, 4개 화면을 전부 destroy 없이 permanently 살려두는 바람에
 * 상시 PSRAM 점유가 늘어 picker/reset_popup류의 큰 ARGB8888 decode가
 * 실패하는 크래시 빈도가 늘어난 것으로 보여, 실사용 빈도가 가장 높은
 * automode 하나로 줄였다(절충안). memorymode/settingmode/settingmodetest는
 * 기존 화면별 prewarm(공유 캐시, evict 가능)만으로 커버한다. */
static void _boot_warmup_screens(bk_lv_ui_t *bk_ui)
{
#if 1 // warm-up 구현을 생명주기 관리자가 알아서 할 예정임 여기는 그런거 없다.
    //FIXME : NULL screen에서 Main으로 진입하는 첫 번째라서 확인을 좀 해봐야 할 듯.
    #define UI_EVENT_BOOT_WARMUP_EVENT UI_EVENT_PAGE_SHOWN
    // init_page_main(bk_ui);
    // lv_obj_send_event(bk_ui->main, UI_EVENT_BOOT_WARMUP_EVENT, NULL);
    // lv_obj_move_to_index(bk_ui->main, -1);
    bk_printf(TAG "[BOOT] entering main\n");
    ui_page_change(PAGE_MAIN);
    bk_printf(TAG "[BOOT] warmup main: %lu ms\n", (unsigned long)lv_tick_elaps(0));
#else
    uint32_t t0 = lv_tick_get();
    uint32_t ts;

    lv_obj_t *cover = lv_obj_create(lv_layer_top());
    lv_obj_set_size(cover, 1024, 600);
    lv_obj_set_pos(cover, 0, 0);
    lv_obj_set_scrollbar_mode(cover, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_border_width(cover, 0, 0);
    lv_obj_set_style_radius(cover, 0, 0);
    lv_obj_set_style_pad_all(cover, 0, 0);
    lv_obj_set_style_bg_opa(cover, LV_OPA_TRANSP, 0);

    lv_obj_t *cover_img = lv_image_create(cover);
    lv_obj_set_pos(cover_img, 0, 0);
    lv_obj_set_size(cover_img, LV_PCT(100), LV_PCT(100));
    lv_image_set_src(cover_img, "/images/intro.jpg");

    lv_obj_t *cover_lbl = lv_label_create(cover);
    lv_label_set_text(cover_lbl, "BOOTING...");
    lv_obj_set_style_text_color(cover_lbl, lv_color_white(), 0);
    lv_obj_align(cover_lbl, LV_ALIGN_BOTTOM_MID, 0, -50);

    lv_refr_now(NULL);  /* cover가 실제로 화면에 그려짐을 보장한 뒤 아래 화면을 순회 */

    ts = lv_tick_get();
    init_page_automode(bk_ui);
    lv_obj_move_to_index(bk_ui->automode, -1);
    // lv_scr_load(bk_ui->automode);
    // lv_obj_add_flag(bk_ui->automode, LV_OBJ_FLAG_HIDDEN); // hidden으로 하면 실제로 render가 안됨. 이거 이런식으로 하는게 아니라 screen을 하나 두고 child만 계속 바꿔끼는 식으로 해야겠는데?
    lv_refr_now(NULL);
    bk_printf(TAG "[BOOT] warmup automode: %lu ms\n", (unsigned long)lv_tick_elaps(ts));

    // NOTE : 이부분은 내가 한거 -BYuM 근데 어차피 얘 호출 안 할거임. preWarm 생명주기는 다른애가 관리할 예정
    // ts = lv_tick_get();
    // init_page_manualmode(bk_ui);
    // lv_obj_move_to_index(bk_ui->manualmode, -1);
    // lv_refr_now(NULL);
    // bk_printf(TAG "[BOOT] warmup manualmode: %lu ms\n", (unsigned long)lv_tick_elaps(ts));

    // ts = lv_tick_get();
    // init_page_autodrymode(bk_ui);
    // lv_obj_move_to_index(bk_ui->autodrymode, -1);
    // lv_refr_now(NULL);
    // bk_printf(TAG "[BOOT] warmup autodrymode: %lu ms\n", (unsigned long)lv_tick_elaps(ts));

    /* main으로 최종 전환 — automode 오브젝트는 destroy하지 않고 그대로 둠:
     * 실제 진입 시 재생성 없이 재사용된다. */
    lv_obj_move_to_index(bk_ui->main, -1);
    lv_scr_load(preRenderRoot);
    lv_refr_now(NULL);
    lv_obj_del(cover);
    lv_refr_now(NULL);

    bk_printf(TAG "[BOOT] warmup total: %lu ms\n", (unsigned long)lv_tick_elaps(t0));
#endif /* 1 // warm-up 구현을 생명주기 관리자가 알아서 할 예정임 여기는 그런거 없다. */
}
#endif /* UI_BOOT_WARMUP_ENABLE */

static void _uart_comm_task(beken_thread_arg_t arg)
{
    bk_printf(TAG "[BOOT] _uart_comm_task() started\n");
    (void)arg;
    uint32_t _boot_t;

    /* 크래시 로그를 코드 상태와 정확히 매칭하기 위한 빌드 마커.
     * PSRAM 조각화 크래시(psram_malloc_cm:722) 대응 진행 상황을 여기 반영:
     *   ams_bg=per-visit   : automodestart 배경 canvas 버퍼, 화면 진입마다 alloc,
     *                        이탈마다 free(원본 동작). "permanent"로 바꿔봤으나
     *                        디코드 실패 시 깨진 상태가 고정되어 되돌림.
     *   pw_popup=rgb565    : password_popup canvas RGB565(602KB), 모서리는
     *                        에셋에 배경색 미리 합성(알파 불필요) */
    bk_printf(TAG "[BUILD] ams_bg=per-visit pw_popup=rgb565 built=%s %s\n", __DATE__, __TIME__);

    /* Phase 1: 설정/RTC — flash VFS 마운트 상태에서 즉시 읽기 가능 */
    _boot_t = lv_tick_get();
    settings_init();
    bk_printf(TAG "[BOOT] settings_init: %lu ms\n", lv_tick_elaps(_boot_t));

    _boot_t = lv_tick_get();
    settings_load_from_flash();
    bk_printf(TAG "[BOOT] settings_load_from_flash: %lu ms\n", lv_tick_elaps(_boot_t));

    _boot_t = lv_tick_get();
    device_state_init();
    bk_printf(TAG "[BOOT] device_state_init: %lu ms\n", lv_tick_elaps(_boot_t));

    _boot_t = lv_tick_get();
    rtc_sync_init();
    bk_printf(TAG "[BOOT] rtc_sync_init: %lu ms\n", lv_tick_elaps(_boot_t));

#if UI_LFS_PSRAM_CACHE_ENABLE // follow the preprocessor directives and check ui_config.h
    /* Phase 2: flash→PSRAM 복사 + 재마운트 (~12s, LVGL lock 상태).
     * (한때 이 복사를 생략해 3.85MB를 아껴보려 했으나, 실측 결과 free PSRAM이
     * 전혀 늘지 않고 crash도 동일하게 재현됨 — LFS_PSRAM_ADDR 영역은 heap과
     * 별도로 정적 예약된 주소범위라 복사를 생략해도 힙에 반환되지 않는 것으로
     * 보임. 속도만 느려지므로 원복.) (나는 그렇게 생각 안합니다 :/) */
    lv_vendor_disp_lock();
    bk_printf(TAG "[BOOT] VFS copy flash->PSRAM start\n");
    _boot_t = lv_tick_get();
    lvgl_vfs_copy_and_remount_psram();
    bk_printf(TAG "[BOOT] VFS copy+remount: %lu ms\n", lv_tick_elaps(_boot_t));
    lv_vendor_disp_unlock();
#endif /*UI_LFS_PSRAM_CACHE_ENABLE*/

    /* Phase 3: UI 초기화 (PSRAM VFS 기반) */
    lv_vendor_disp_lock();
    _boot_t = lv_tick_get();
    bk_printf(TAG "[BOOT] ui_screen_event_init() start\n");
    ui_screen_event_init();
    bk_printf(TAG "[BOOT] ui_screen_event_init: %lu ms\n", lv_tick_elaps(_boot_t));

    preRenderRoot = lv_obj_create(NULL);
    lv_obj_set_scrollbar_mode(preRenderRoot, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_border_width(preRenderRoot, 0, 0);
    lv_obj_set_style_radius(preRenderRoot, 0, 0);
    lv_obj_set_style_pad_all(preRenderRoot, 0, 0);
    lv_obj_set_style_bg_opa(preRenderRoot, LV_OPA_TRANSP, 0); // 배경 투명

    main_activity_on_create();
    bk_printf(TAG "[BOOT] main_activity_on_create: %lu ms\n", lv_tick_elaps(_boot_t));

    _boot_t = lv_tick_get();
    init_page_timebar(&bk_lv_tool_ui);
    ui_lang_apply_timebar(&bk_lv_tool_ui);
    bk_printf(TAG "[BOOT] init_page_timebar: %lu ms\n", lv_tick_elaps(_boot_t));

    _boot_t = lv_tick_get();
    init_page_main(&bk_lv_tool_ui);
    bk_printf(TAG "[BOOT] init_page_main: %lu ms\n", lv_tick_elaps(_boot_t));

    if (!g_device_state.black_out_checking) {
#if UI_BOOT_WARMUP_ENABLE
        /* introactivity("BOOTING...")가 아직 떠 있는 동안 백그라운드처럼
         * automode/memorymode/settingmode를 순회 warmup한 뒤 main으로 전환.
         * _boot_warmup_screens()가 마지막에 lv_scr_load(main)까지 수행한다. */
        _boot_warmup_screens(&bk_lv_tool_ui);
#else
        lv_scr_load(bk_lv_tool_ui.main);
#endif
        bk_printf(TAG "[BOOT] main screen loaded\n");
    } else {
        // NOTE : blackout recovery screen 확인해야함
        bk_printf(TAG "[BOOT] blackout recovery screen active\n");
    }
    destroy_page_introactivity(&bk_lv_tool_ui);
    lv_vendor_disp_unlock();

    /* Phase 4: UART 루프 */
    uart_comm_init();
    while (1) {
        uart_comm_tick();
        rtos_delay_milliseconds(200);
    }
}

/* flash/RTC 유지보수 태스크 — UART 태스크와 분리해 GC(~98ms)가 UART 수신 타이밍에
 * 영향을 주지 않도록 한다. Android는 SharedPreferences가 별도 스레드에서 기록됨. */
static void _flash_maint_task(beken_thread_arg_t arg)
{
    (void)arg;
    while (1) {
        settings_flush();           /* EasyFlash GC + dirty write */
        rtc_sync_periodic_save();   /* 60초마다 epoch 저장 */
        rtos_delay_milliseconds(1000);
    }
}
#else /* HAL_USE_EMULATOR */
static void _emu_uart_tick(lv_timer_t *t) { (void)t; uart_comm_tick(); }
#endif

void beken_ui_init(void)
{
    bk_printf(TAG "[BOOT] beken_ui_init() called\n");
    /* VFS flash 직접 마운트 (즉시) — intro.jpg 읽기 가능 */
    lvgl_vfs_init_flash_quick();

    /* 부팅 화면 표시 */
    init_page_introactivity(&bk_lv_tool_ui);
    lv_scr_load(bk_lv_tool_ui.introactivity);
    bk_printf(TAG "[BOOT] boot screen displayed\n");

#ifdef HAL_USE_EMULATOR
    settings_init();
    settings_load_from_flash();
    device_state_init();
    rtc_sync_init();
    main_activity_on_create();
    init_page_main(&bk_lv_tool_ui);
    init_page_timebar(&bk_lv_tool_ui);
    ui_lang_apply_timebar(&bk_lv_tool_ui);
    if (!g_device_state.black_out_checking)
        lv_scr_load(bk_lv_tool_ui.main);
    uart_comm_init();
    lv_timer_create(_emu_uart_tick, 200, NULL);
#else
    rtos_create_thread(&s_uart_thread,  2, "uart_comm",
                       _uart_comm_task,   8192, NULL);
    rtos_create_thread(&s_flash_thread, 5, "flash_maint",
                       _flash_maint_task, 4096, NULL);
#endif
}
// void beken_ui_init(void)
// {
//     // settings_init();          // SettingData 초기화
//     // device_state_init();      // 전역 상태 복원
//     uart_comm_init();  
//     main_activity_on_create();
//     init_page_main(&bk_lv_tool_ui);
//     init_page_timebar(&bk_lv_tool_ui);
//    
//     /* 2. UART tick 타이머 (SerialComm 250ms 주기 역할) */
//     lv_timer_create(uart_comm_tick, 200, NULL);


//     // init_page_detailsettingtemp(&bk_lv_tool_ui);
//     // lv_screen_load(bk_lv_tool_ui.detailsettingtemp);
// }

// void beken_ui_init(void)
// {
//     /* 1. 런타임 초기화 (MainApplication.onCreate + init() 역할) */
//           // UART 열기 + handshake 준비
//     main_activity_on_create(); // 전원/언어/정전복구 → 첫 화면 로드

    
//     // ↓ 기존 코드 — main_activity_on_create 안에서 화면 로드하므로 제거 가능
//     // init_page_main(&bk_lv_tool_ui);
//     // lv_screen_load(bk_lv_tool_ui.main);
// }