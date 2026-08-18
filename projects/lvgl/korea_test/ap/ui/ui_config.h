#ifndef UI_CONFIG_H
#define UI_CONFIG_H

/* 1: 화면 로드 시 타이틀 슬라이드-인 애니메이션 활성화
 * 0: 애니메이션 없이 즉시 표시 (CPU 절약) */
#define UI_TITLE_ANIM_ENABLE    0

/* 1: _img_set_src_timed 사용 — [IMGTIME] 로그 출력 + 에뮬레이터 딜레이 포함
 * 0: lv_image_set_src 직접 호출 — 래퍼 오버헤드 없음 */
#define UI_IMG_TIMED_ENABLE     0

/* 1: bg.jpg 대신 단색으로 배경 채움 — JPEG decode/캐시 없음, 메모리 절약
 * 0: /images/bg.jpg JPEG 이미지 사용 */
#define UI_BG_SOLID_COLOR       0

/* UI_BG_SOLID_COLOR=1 일 때 배경 색상 (0xRRGGBB) */
#define UI_BG_COLOR_HEX         0xF0EDE8

/* UI_BG_SOLID_COLOR=0 일 때만 의미 있음.
 * 1: bg.jpg를 부팅 시 1회 JPEG 디코딩해서 영구 canvas 버퍼(1024x540 RGB565,
 *    ~1.08MB)로 보관 후 재사용 — 화면 진입마다 재디코딩 없음, 대신 그 1.08MB를
 *    앱 수명 내내 점유(힙 여유 감소).
 * 0: 화면 진입마다 /images/bg.jpg를 매번 디코딩 — 평소 힙 여유는 늘지만
 *    해당 화면 진입 시점마다 디코딩 비용(및 큰 단일 할당 필요)이 반복됨.
 * 속도 vs 메모리 트레이드오프 비교용 컴파일 옵션. */
#define UI_BG_CANVAS_PRELOAD_ENABLE   0

/* 1: 키패드 표시/숨김 시 600ms 슬라이드 인/아웃 애니메이션 사용
 * 0: 애니메이션 없이 즉시 표시/숨김 (CPU 절약) */
#define UI_KEYPAD_ANIM_ENABLE   0

/* 1: main 화면의 bg.jpg + 아이콘 5장(automode/manualmode/autodrymode/
 *    memorymode/settingmode)을 디자이너가 미리 합성한 /images/main.jpg
 *    (+_china/_english) 1장으로 대체 — 개별 아이콘 imageview 생성/decode 없음.
 * 0: 기존처럼 bg.jpg + 개별 아이콘 5장 사용.
 * 주의: vfs_file1/images/의 개별 아이콘 원본은 combinednimus/ 로 이동되어
 *       있으므로 0으로 되돌리려면 해당 파일들을 다시 옮겨와야 함. */
#define UI_MAIN_COMBINED_BG_ENABLE   1

/* 1: settingmode(기능설정) 화면의 bg.jpg + 타이틀 + 아이콘 6장 + 나가기 버튼을
 *    /images/feature-setting.jpg(+_china/_english) 1장으로 대체.
 * 0: 기존처럼 bg.jpg + 개별 이미지 8장 사용.
 * 주의: UI_MAIN_COMBINED_BG_ENABLE과 동일 — 0으로 되돌리려면 combinednimus/
 *       에 옮겨둔 개별 아이콘을 vfs_file1/images/로 복원해야 함. */
#define UI_SETTINGMODE_COMBINED_BG_ENABLE   1

/* 1: settingmodedetailsetting(기본설정) 화면의 bg.jpg + 타이틀 + 아이콘 6장 +
 *    나가기 버튼을 /images/advancedsetting.jpg(+_china/_english) 1장으로 대체.
 * 0: 기존처럼 bg.jpg + 개별 이미지 8장 사용.
 * 주의: 위 두 옵션과 동일한 복원 절차 필요. */
#define UI_SETTINGMODEDETAILSETTING_COMBINED_BG_ENABLE   1

/* 1: password 팝업 키패드 12장(pop_keypad0~9, back, alldel)을 하나의 canvas에
 *    1회만 decode해 스프라이트시트처럼 재사용 (버튼별로 lv_image_set_offset()
 *    으로 자기 영역만 크롭 표시) — 개별 PNG를 매 팝업 오픈마다 다시 로드하던
 *    문제를 해결. 원인은 LVGL 이미지 캐시가 전체 화면 공통 4MB LRU 풀
 *    (LV_CACHE_DEF_SIZE)이라 다른 화면(특히 배경 JPG) decode에 밀려 12장의
 *    캐시 엔트리가 다음 팝업 오픈 전에 evict되기 때문 — canvas는 그 캐시
 *    바깥의 수동 관리 버퍼라 evict 자체가 불가능해짐.
 * 0: 기존처럼 버튼 12장을 개별 PNG로 매 팝업 오픈마다 로드. */
#define UI_POPUPPASSWORD_KEYPAD_COMBINED_ENABLE 0

/* 전체 prewarm(백그라운드 미리 decode) 마스터 스위치.
 * 대상 5곳: settingmode 아이콘(main_cb.c), automodestart(automode_cb.c),
 * memorymode(automode_cb.c), popuppassword(settingmode_cb.c),
 * reset_popup(settingmodedetailsetting_cb.c).
 * 1: 이전 화면에 머무는 동안 다음 화면 이미지를 미리 decode — 전환이 빠르고,
 *    popuppassword/reset_popup은 이게 꺼지면 클릭 시점 psram_malloc이
 *    fragmentation으로 실패할 수 있어 사실상 crash 방지 목적도 겸함.
 * 0: prewarm 없이 실제 진입 시점에 decode — 전환이 느려지고 위 두 화면은
 *    fragmentation 심할 때 크래시 위험이 커짐. 원인 격리/비교용 스위치. */
#define UI_PREWARM_ENABLE   1

/* password 팝업 배경을 반투명(LV_OPA_60 검정, 기존)이 아니라 불투명 단색으로
 * 칠할지 결정 — keypad 반응속도 비교용 스위치.
 * lv_layer_top() 자식 + 반투명 구조는 원래 이 반투명 자체 때문에 필요했던 것
 * (불투명이었으면 popuptime에서 겪은 "누적 darkening" 버그 자체가 안 생겨서
 * lv_scr_load 방식도 문제없었을 것). 반투명 블렌딩은 문서화된 것처럼 그 아래
 * active screen(settingmode)까지 매번 재렌더해야 해서 ~9배 비용 — 이 스위치로
 * 그 비용이 실제 keypad 반응 속도에 영향 있는지 A/B 비교.
 * 1: 0xD9D9D9(밝은 회색) 불투명 배경 — 블렌딩 비용 없음, 대신 뒤의 기능설정
 *    화면이 비쳐 보이지 않고 완전히 가려짐.
 * 0: 기존처럼 검정 60% 반투명 — 기능설정 화면이 어둡게 비쳐 보임. */
#define UI_POPUPPASSWORD_SOLID_BG_ENABLE      0
#define UI_POPUPPASSWORD_SOLID_BG_COLOR_HEX   0xD9D9D9

/* 1: password 팝업을 lv_layer_top() 오버레이(반투명/불투명 fill + 별도 카드
 *    이미지)가 아니라, settingmode(기능설정) 화면을 dim한 배경과 password
 *    카드를 미리 합성해둔 완전 불투명 이미지 1장(/images/feature-setting_password.jpg
 *    +_china/_english)으로 대체 — settingmode의 "평범한 자식 객체"로 붙인다
 *    (lv_scr_act()를 숨기거나 lv_scr_load로 화면을 바꾸는 게 아님).
 *    이러면 LVGL의 표준 occlusion 최적화(완전 불투명 객체가 덮은 아래쪽은
 *    다시 안 그림)가 자동으로 적용되어, 팝업이 열려 있는 동안 매 redraw마다
 *    그 밑의 settingmode 전체를 재렌더하던 비용(~9배, keypad 반응 지연의
 *    실제 원인이었음)이 사라진다. lv_scr_act() HIDDEN 처리(크래시 발생,
 *    MemFault) 및 lv_scr_load 방식(settingmode SCREEN_UNLOAD_START 오발동
 *    위험)보다 안전한 방식.
 * 0: 기존처럼 lv_layer_top() 오버레이 + 개별 카드 이미지 사용. */
#define UI_POPUPPASSWORD_COMBINED_BG_ENABLE   0  //787ms 로 늦어짐

/* 1: 키패드 12장(pop_keypad0~9, back, alldel)을 팝업 오픈 시 한꺼번에 로드하지
 *    않고, 각 버튼을 처음 누르는 순간(PRESSED)에 그 버튼 이미지 1장만 개별
 *    디코드 — 팝업을 여는 순간의 지연(12장 decode 비용)은 없어지지만, 각 키를
 *    "처음" 누를 때 그 1장만큼의 디코드 비용이 그 키 입력에 실림(이후 재사용).
 * 0: 기존처럼 popuppassword_load_event_cb(SCREEN_LOAD_START)에서 12장을
 *    한 번에 전부 로드 — 팝업 열릴 때 한 번만 비용을 치르고, 이후 모든 키
 *    입력은 이미 로드된 이미지라 디코드 비용 없음. */
#define UI_POPUPPASSWORD_KEYPAD_LAZY_PRESS_ENABLE   0

/* 1: 키패드 버튼을 누르는 동안 pop_keypadNim 하이라이트 이미지를 켬(PRESSED
 *    시 clear_flag(HIDDEN), RELEASED/PRESS_LOST 시 add_flag(HIDDEN)).
 *    RENDER_DIRECT_MODE에서는 이 flag 토글 하나하나가 풀-프레임버퍼 flush를
 *    유발할 수 있어(lv_disp_flush_for_direct_mode가 dirty area 무관하게 항상
 *    전체 flush) press당 추가 렌더 비용이 실측됨("무지 느려짐").
 * 0: 하이라이트 기능 자체를 끔 — _keypad_im_track()이 아무 것도 하지 않고
 *    바로 리턴, PRESSED/RELEASED에서 어떤 invalidate도 발생하지 않음.
 *    버튼을 눌러도 시각적 ON 표시는 없지만(디버그 로그는 그대로 남음),
 *    그 비용 자체가 없어져 키패드 반응이 가장 빠름. */
#define UI_POPUPPASSWORD_KEYPAD_HIGHLIGHT_ENABLE   1   //787ms/487ms 대비 255ms — 부분 flush 없이는 하이라이트가 순손해

/* 1: password 팝업이 열려있는 동안 active screen(settingmode)을
 *    lv_obj_add_flag(..., LV_OBJ_FLAG_HIDDEN)으로 숨김.
 *    lv_layer_top() 자식은 LV_OPA_COVER와 무관하게 LVGL이 active screen을
 *    먼저 렌더한 뒤 layer_top을 합성하므로, active screen 렌더 자체를 스킵하면
 *    ~9μs/px → ~1μs/px(9배 단축)의 극적인 효과가 실측된 바 있음. 팝업 닫힐 때
 *    destroy_page_popuppassword()에서 다시 보이게 복원.
 *    **주의(해결됨)**: addr2line으로 크래시 지점을 lv_obj.c:288의
 *    lv_obj_mark_layout_as_dirty(lv_obj_get_parent(obj)) 호출로 특정 —
 *    obj가 화면 자체라 parent가 NULL이라 NULL->layout_inv 역참조로 MemFault
 *    (fault addr 0x2a) 발생. lv_obj.c에 NULL 체크(lv_obj_style.c:303과 동일한
 *    방어패턴) 추가로 수정 완료.
 * 0: 기존처럼 active screen을 숨기지 않음 — 크래시 위험 없음. */
#define UI_POPUPPASSWORD_HIDE_ACTIVE_SCREEN_ENABLE   1

/* ── 고장진단(settingmodetest) 진입 속도 개선 3종 세트 ────────────────────
 * memorymode에 이미 적용된 것과 동일한 패턴을 이식. */

/* 1: settingmode→고장진단 재진입 시 화면을 destroy+재생성하지 않고 재사용
 *    (memorymode와 동일한 keep-alive). 오브젝트 수십 개 재생성 비용 제거.
 * 0: 기존처럼 진입마다 destroy 후 새로 생성. */
#define UI_SETTINGMODETEST_KEEPALIVE_ENABLE   1

/* 1: title/exit_bt/testmode_box.jpg를 오브젝트 생성 시점(_img_set_src_timed)과
 *    SCREEN_LOAD_START의 ui_lang_apply_settingmodetest() 두 곳에서 매번 중복
 *    디코드하던 것을 제거 — ui_lang_apply 한 곳만 소스로 삼음(언어 캐시 키가
 *    안 바뀌면 스킵되므로 keep-alive와 결합 시 재진입 비용이 사실상 0).
 * 0: 기존처럼 생성 시점에도 매번 즉시 디코드(중복 비용 유지, 비교용). */
#define UI_SETTINGMODETEST_DEFERRED_LOAD_ENABLE   1

/* 1: testmode_box.jpg(984x433, 가장 무거운 단일 이미지)를 settingmode(기능설정)
 *    화면에 머무는 동안 전용 canvas 버퍼(RGB565, ~832KB)에 미리 decode —
 *    popuppassword의 canvas prewarm과 동일한 원리. 공유 LVGL 이미지 캐시
 *    (LV_CACHE_DEF_SIZE) 밖의 별도 버퍼라, 과거 testmode 공유캐시 prewarm이
 *    popuppassword prewarm과 캐시를 두고 경쟁해 LRU eviction을 일으켰던 문제
 *    (settingmode_cb.c의 s_tm_bg_* 관련 주석 참고)를 원천적으로 피함.
 * 0: prewarm 없이 고장진단 진입 시점에 파일에서 직접 decode. */
#define UI_SETTINGMODETEST_PREWARM_ENABLE   1

/* 1: 부팅 완료 후(main 화면 표시 직전) automode/memorymode/settingmode를
 *    "로딩중" 오버레이(lv_layer_top(), 불투명) 뒤에서 한 번씩 순서대로
 *    lv_scr_load() + lv_refr_now()로 실제 렌더링시켜 미리 "몸풀기"를 시킨다.
 *    실측 결과 이 세 화면 모두 부팅 후 첫 진입 1회만 ~1초급 렌더 지연이 있고
 *    이후로는 계속 ~150ms로 빨라짐(이미지 decode는 prewarm으로 이미 해결된
 *    상태에서도 재현 — 렌더 파이프라인의 최초 1회 버퍼 할당 등으로 추정).
 *    이 1회성 비용을 오버레이 뒤에서 부팅 중에 미리 치러서, 사용자가 처음
 *    실제로 그 화면들에 들어갈 때는 이미 "두 번째 진입"인 것처럼 빠르게 함.
 *    자동설정/메모리모드/기능설정 오브젝트는 warmup 후에도 destroy하지 않고
 *    살려둔 채 main으로 복귀 — 이후 실제 진입 시 재생성 없이 그대로 재사용됨.
 * 0: 워밍업 없이 기존처럼 부팅 후 바로 main 표시 (각 화면 첫 진입이 느림). */
#define UI_BOOT_WARMUP_ENABLE   0

/* 1: password_popup.jpg canvas(~602KB)를 settingmode를 나갈 때 free하지 않고
 *    계속 유지 — settingmode 1차/2차 진입이 매번 똑같이 느렸던 원인(매번
 *    JPEG 재decode)을 없애 재진입을 빠르게 함.
 *    **주의**: canvas 메모리도 결국 공유 이미지 캐시(LV_CACHE_DEF_SIZE)와
 *    동일한 PSRAM heap에서 온다. 이 옵션+testmode_box 영구 보관을 합치면
 *    ~1.4MB가 상시 고정되는데, 실측 여유 PSRAM이 2.1~3.9MB 수준이라 다른
 *    곳(reset_popup 등)의 malloc 실패 위험을 키울 수 있어 0으로 되돌림.
 * 0: 기존처럼 나갈 때마다 free, 들어올 때마다 재decode
 *    (메모리 절약 — 현재 기본값, testmode_box도 동일하게 이 turn에서 되돌림). */
#define UI_POPUPPASSWORD_CANVAS_PERSIST_ENABLE   0

/* 1: reset_popup/delete_popup/caution_popup (알파 있는 RGBA PNG, 520x350,
 *    ARGB8888 decode 시 728KB — 단편화 시 malloc 실패로 크래시 이력 있음)를
 *    알파를 에셋에 미리 합성한 .jpg로 대체 — RGB565면 되므로 필요 버퍼가
 *    728KB → 364KB로 절반이 되어 단편화에 훨씬 강해진다.
 *    delete_popup/caution_popup은 항상 memorymode(단색 배경 0xD9D9D9) 위에서만
 *    반투명 50% 검정으로 덮이므로 배경색이 고정(#6C6C6C로 측정/계산)이라
 *    안전하게 합성 가능. reset_popup은 settingmodedetailsetting의 결합
 *    배경(advancedsetting.jpg) 위라 완전히 균일하진 않지만, 팝업 카드가 놓이는
 *    영역이 실측상 거의 단색(#6D6B68)이라 마찬가지로 안전하게 합성함.
 * 0: 기존처럼 알파 있는 .png 그대로 사용(reset_popup은 canvas+ARGB8888 유지). */
#define UI_POPUP_DIALOG_JPG_ENABLE   1

/* 1: reset_popup / automodestart bg / testmode_box 캔버스 버퍼 3개(합계 ~2.18MB)를
 *    main_activity_on_create()에서 부팅 극초반 1회만 malloc하고 이후 절대 free하지
 *    않음 — 각각 실제 크래시 이력이 로그로 확인된 버퍼들(reset_popup: 7/15부터
 *    반복, automodestart: "시작" 버튼 클릭 시 스택트레이스로 확인, testmode_box:
 *    settingmode↔settingmodetest 전환 시 확인). 화면 진입마다 큰 버퍼를 새로
 *    malloc하다가 그 타이밍에 하필 단편화가 심해 실패 → psram_malloc_cm이 NULL
 *    대신 즉시 assert 크래시하는 근본 원인을 없앤다.
 *    **주의**: 이 2.18MB가 앱 수명 내내 고정되므로, 공유 이미지 캐시
 *    (LV_CACHE_DEF_SIZE, 4MB)나 다른 동적 할당(예: lodepng의 realloc 임시 버퍼)이
 *    쓸 수 있는 여유가 그만큼 줄어든다 — 실제로 이 3개를 켠 채 password_popup/
 *    manualmodestart ferm2까지 추가로 고정했을 때(합계 3.82MB) free가
 *    4.19MB→1.25MB까지 떨어지며 평범한 아이콘 decode가 실패하는 크래시가
 *    새로 발생한 적 있음 — 그래서 그 두 개는 이후 free-per-visit으로 되돌림.
 * 0: 3개 모두 예전 방식(화면 진입 시 malloc, 이탈 시 free)으로 되돌림 — 평소
 *    힙 여유는 늘지만(2.18MB 반환), 이 3개 각각의 원래 크래시가 다시 재현될 수
 *    있음. 비교/실측용. */
#define UI_CANVAS_BUF_PERMANENT_ENABLE   1

/* 1: PSRAM 여유(rtos_get_psram_free_heap_size())가 임계값 아래로 떨어지면
 *    화면 전환 시점(uart_comm_tick, 매우 자주 호출됨)마다 체크해서 공유 이미지
 *    캐시(LV_CACHE_DEF_SIZE)를 즉시 lv_image_cache_drop(NULL)로 통째로 비움 —
 *    main_cb.c의 3분 주기 방식(main 화면 복귀 시에만, 시간 기반)과 달리 이건
 *    "실제로 위험할 때만, 어느 화면에서든" 반응하는 방식.
 *    쿨다운(UI_CACHE_DROP_LOW_MEM_COOLDOWN_MS) 없이 매 tick마다 임계값 미만이면
 *    계속 드롭하면, free가 임계값 근처에서 오르내릴 때 매번 캐시가 통째로
 *    비워져 화면마다 재decode가 반복되는 "드롭 연타"로 오히려 버벅일 수 있어
 *    쿨다운으로 최소 재발동 간격을 둔다. 그래도 근본 원인(단편화)을 없애는 건
 *    아니라서 크래시를 완전히 보장하진 못함 — 여유를 늘려 확률을 낮추는 완화책.
 * 0: 이 저메모리 감지 기능 없음(main_cb.c의 3분 주기 정리만 동작). */
#define UI_CACHE_DROP_LOW_MEM_ENABLE          1
#define UI_CACHE_DROP_LOW_MEM_THRESHOLD_BYTES (2 * 1024 * 1024)
#define UI_CACHE_DROP_LOW_MEM_COOLDOWN_MS     (20 * 1000)

#endif /* UI_CONFIG_H */
