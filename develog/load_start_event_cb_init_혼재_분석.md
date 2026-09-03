# load_start_event_cb init 코드 혼재 분석

> 대상: `projects/lvgl/korea_test/ap/ui/*_cb.c`
> 기준: `*_load_start_event_cb` 본문에 init 성격 코드(위젯/레이어 생성, 이미지 src 세팅, 이미지 배열 빌드, 상태 초기화 등)가 섞여 있는지 여부
> 작성일: 2026-09-03

---

## 🔴 MIXED — init 코드 혼재 (정리 필요)

| 파일 | 함수 | 혼재 내용 |
|---|---|---|
| autodrymode_cb.c | `autodrymode_load_start_event_cb` | `_create_run_arc_adm()` arc 레이어 생성, `_img_ensure_src()` 이미지 src 세팅, `lv_obj_move_background()` 재배치 |
| automodeend_cb.c | `automodeend_load_start_event_cb` | `lv_timer_create()` 타이머 생성, `_img_set_src_timed()` 배경 이미지 세팅, 언어별 위치 조정 |
| detailsettingdamper_cb.c | `detailsettingdamper_load_start_event_cb` | `_build_img_arrays_if_changed()` 이미지 배열 빌드 |
| detailsettingdefrost_cb.c | `detailsettingdefrost_load_start_event_cb` | `_build_img_arrays()` 이미지 배열 빌드, 상태 변수 초기화 |
| detailsettinghumidity_cb.c | `detailsettinghumidity_load_start_event_cb` | `_build_img_arrays_if_changed()` 이미지 배열 빌드 |
| detailsettingtemp_cb.c | `detailsettingtemp_load_start_event_cb` | `_build_img_arrays_if_changed()`, `s_page = 0` 상태 초기화 |
| detailsettingtime_cb.c | `detailsettingtime_load_start_event_cb` | `_build_img_arrays_if_changed()`, `_hide_all_rollers()` 롤러 초기화 |
| manualmodestart_cb.c | `manualmodestart_load_start_event_cb` | `_img_ensure_src()` 이미지 세팅, `_keypad_off_manualmodestart()`, 배경색 스타일 설정 |
| settingmodetest_cb.c | `settingmodetest_load_start_event_cb` | `settingmodetest_apply_bg()` 캔버스 설정, `memset()` 상태 배열 초기화 |
| settingmodetime_cb.c | `settingmodetime_load_start_event_cb` | `_init_input_from_rtc()` 입력 필드 초기화, `_reset_all_field_colors()` 색상 초기화 |

## 🟢 CLEAN — 표시 시점 갱신만 (건드리지 않아도 됨)

| 파일 | 함수 | 하는 일 |
|---|---|---|
| automode_cb.c | `automode_load_start_event_cb` | 설정값 검증/온도 변환, 라벨 갱신, 완료시간 계산·리셋 |
| automodestart_cb.c | `automodestart_load_start_event_cb` | 이미지 숨김, 설정값 라벨 갱신 |
| main_cb.c | `main_load_start_event_cb` | `state->auto_mode = false` 상태 설정만 |
| memorymode_cb.c | `memorymode_load_start_event_cb` | `destroy_page_popupdelete()` 정리, `s_checking` 리셋 |
| settingmode_cb.c | `settingmode_load_start_event_cb` | `ui_lang_apply_settingmode()` 만 |
| settingmodedefrost_cb.c | `settingmodedefrost_load_start_event_cb` | `ui_lang_apply_settingmodedefrost()` 만 |
| settingmodedegree_cb.c | `settingmodedegree_load_start_event_cb` | 언어 적용 + `_update_images()` 표시 갱신 |
| settingmodedetailsetting_cb.c | `settingmodedetailsetting_load_start_event_cb` | `ui_lang_apply_settingmodedetailsetting()` 만 |
| settingmodelanguage_cb.c | `settingmodelanguage_load_start_event_cb` | 언어 적용 + `_update_language_ui()` |
| settingmodemanual_cb.c | `settingmodemanual_load_start_event_cb` | 언어 적용 + 위젯 숨김 플래그 |
| settingmoderecord_cb.c | `settingmoderecord_load_start_event_cb` | 언어 적용 + `_apply_record_labels()` |

## ⚪ EMPTY / 콜백 없음

| 파일 | 비고 |
|---|---|
| manualmode_cb.c | `manualmode_load_start_event_cb` 본문 비어있음 |
| neurosys_cb.c | 콜백 없음 |
| popupcalendar_cb.c | 콜백 없음 |
| popupcaution_cb.c | 콜백 없음 |
| popupconnectionerror_cb.c | 콜백 없음 |
| popupdelete_cb.c | 콜백 없음 |
| popuperror_cb.c | 콜백 없음 |
| popuppassword_cb.c | 콜백 없음 |
| popupreset_cb.c | 콜백 없음 |
| popuptime_cb.c | 콜백 없음 |
| timebar_cb.c | `timebar_load_event_cb` (다른 이름)만 존재 |

---

## 메모

- MIXED 중 detailsetting* 5개는 `_build_img_arrays_if_changed()` 패턴이 공통 → 동일 방식으로 init 쪽 이전 가능.
- 팝업류(`popup*_cb.c`)는 `load_start_event_cb` 자체가 없어 이번 작업 범위 밖.
