/* ui_lang.h — per-screen language/degree image application
 * Call the matching function from each screen's load_event_cb after
 * any mode-specific image assignments, before ui_title_anim().
 */
#ifndef UI_LANG_H
#define UI_LANG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "beken_ui.h"

void ui_lang_apply_manualmode(bk_lv_ui_t *bk_ui);
void ui_lang_reset_manualmode_cache(void);
void ui_lang_apply_manualmodestart(bk_lv_ui_t *bk_ui);
/* manualmodestart 화면 오브젝트가 destroy+재생성될 때 init_page_manualmodestart()에서
 * 호출 — 캐시가 "이미 적용됨"으로 착각해 새 빈 이미지를 안 채우는 것을 방지. */
void ui_lang_reset_manualmodestart_cache(void);
void ui_lang_apply_automode(bk_lv_ui_t *bk_ui);
/* automode 화면 오브젝트가 destroy+재생성될 때 init_page_automode()에서 호출 —
 * 캐시가 "이미 적용됨"으로 착각해 새로 생성된 빈 이미지를 안 채우는 것을 방지. */
void ui_lang_reset_automode_cache(void);
void ui_lang_apply_automodestart(bk_lv_ui_t *bk_ui);
/* automodestart 화면 오브젝트가 destroy+재생성될 때 init_page_automodestart()에서
 * 호출 — 캐시가 "이미 적용됨"으로 착각해 새 빈 이미지를 안 채우는 것을 방지. */
void ui_lang_reset_automodestart_cache(void);
void ui_lang_apply_automodeend(bk_lv_ui_t *bk_ui);
void ui_lang_reset_automodeend_cache(void);
void ui_lang_apply_autodrymode(bk_lv_ui_t *bk_ui);
void ui_lang_reset_autodrymode_cache(void);
void ui_lang_apply_memorymode(bk_lv_ui_t *bk_ui);
/* memorymode 화면 오브젝트가 destroy+재생성될 때 init_page_memorymode()에서 호출 —
 * 캐시가 "이미 적용됨"으로 착각해 새로 생성된 빈 이미지를 안 채우는 것을 방지. */
void ui_lang_reset_memorymode_cache(void);
void ui_lang_apply_settingmode(bk_lv_ui_t *bk_ui);
void ui_lang_reset_settingmode_cache(void);
void ui_lang_apply_settingmodedetailsetting(bk_lv_ui_t *bk_ui);
void ui_lang_reset_settingmodedetailsetting_cache(void);
void ui_lang_apply_settingmodedefrost(bk_lv_ui_t *bk_ui);
void ui_lang_reset_settingmodedefrost_cache(void);
void ui_lang_apply_settingmodemanual(bk_lv_ui_t *bk_ui);
void ui_lang_reset_settingmodemanual_cache(void);
void ui_lang_apply_settingmodetime(bk_lv_ui_t *bk_ui);
void ui_lang_reset_settingmodetime_cache(void);
void ui_lang_apply_settingmodelanguage(bk_lv_ui_t *bk_ui);
void ui_lang_reset_settingmodelanguage_cache(void);
void ui_lang_apply_settingmoderecord(bk_lv_ui_t *bk_ui);
void ui_lang_reset_settingmoderecord_cache(void);
void ui_lang_apply_settingmodetest(bk_lv_ui_t *bk_ui);
void ui_lang_reset_settingmodetest_cache(void);
void ui_lang_apply_settingmodedegree(bk_lv_ui_t *bk_ui);
void ui_lang_reset_settingmodedegree_cache(void);
void ui_lang_apply_detailsettingtemp(bk_lv_ui_t *bk_ui);
void ui_lang_reset_detailsettingtemp_cache(void);
void ui_lang_apply_detailsettinghumidity(bk_lv_ui_t *bk_ui);
void ui_lang_reset_detailsettinghumidity_cache(void);
void ui_lang_apply_detailsettingdefrost(bk_lv_ui_t *bk_ui);
void ui_lang_reset_detailsettingdefrost_cache(void);
void ui_lang_apply_detailsettingtime(bk_lv_ui_t *bk_ui);
void ui_lang_reset_detailsettingtime_cache(void);
void ui_lang_apply_detailsettingdamper(bk_lv_ui_t *bk_ui);
void ui_lang_reset_detailsettingdamper_cache(void);
void ui_lang_apply_timebar(bk_lv_ui_t *bk_ui);
void ui_lang_reset_timebar_cache(void);  /* timebar는 destroy 안 되므로 실제 호출처는 없음 */
void ui_lang_apply_main(bk_lv_ui_t *bk_ui);
void ui_lang_reset_main_cache(void);
void ui_lang_apply_popupcaution(bk_lv_ui_t *bk_ui);
void ui_lang_reset_popupcaution_cache(void);
void ui_lang_apply_popupreset(bk_lv_ui_t *bk_ui);
void ui_lang_reset_popupreset_cache(void);
void ui_lang_apply_popupdelete(bk_lv_ui_t *bk_ui);
void ui_lang_reset_popupdelete_cache(void);
void ui_lang_apply_popupconnectionerror(bk_lv_ui_t *bk_ui);
void ui_lang_reset_popupconnectionerror_cache(void);
void ui_lang_apply_popuppassword(bk_lv_ui_t *bk_ui);
void ui_lang_reset_popuppassword_cache(void);
void ui_lang_apply_popuperror(bk_lv_ui_t *bk_ui);
void ui_lang_reset_popuperror_cache(void);
void ui_lang_apply_picker(lv_obj_t *obj, int num);
void ui_lang_apply_next_bt(lv_obj_t *obj, int num);

/* ON/OFF 값 표시 — 중국어(LANGUAGE==1)면 label의 폰트를 lv_font_onoff_cn_32로
 * 바꿔 开/关 텍스트로 표시, 그 외 언어는 기존 폰트로 value("ON"/"OFF")를 그대로
 * 표시한다. 이미지 오버레이 없이 같은 label 위젯의 폰트만 교체하므로 이미 적용된
 * 색상(_update_txt_colors 등)이 별도 처리 없이 그대로 유지된다. */
void ui_lang_set_onoff_display(lv_obj_t *label, const char *value);

/* Apply language/degree to every currently-created screen in one call.
 * Screens not yet created (NULL / invalid) are silently skipped. */
void ui_lang_apply_all(bk_lv_ui_t *bk_ui);

/* Fast alternative for runtime language/degree changes.
 * Destroys all cached screens except the currently displayed one so that
 * init_page_* re-creates them with correct settings on the next visit.
 * Timebar (always visible) is updated in-place. Zero image decodes at call time. */
void ui_lang_invalidate_cached_screens(bk_lv_ui_t *bk_ui);

#ifdef __cplusplus
}
#endif

#endif /* UI_LANG_H */
