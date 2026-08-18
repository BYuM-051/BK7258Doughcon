#ifndef UI_ANIMATIONS_H
#define UI_ANIMATIONS_H

#include "lvgl.h"
#include "ui_config.h"

/* Title slide-in from left on screen load.
 * Controlled by UI_TITLE_ANIM_ENABLE in ui_config.h */
#if UI_TITLE_ANIM_ENABLE
void ui_title_anim(lv_obj_t *title);
#else
static inline void ui_title_anim(lv_obj_t *title) { (void)title; }
#endif

/* Keypad panel slide-in (keypadbaseim: x = -width → 0, ease_out, 500ms) */
void ui_keypad_slide_on(lv_obj_t *base);

/* Keypad panel slide-out (keypadbaseim: x = 0 → -width, ease_in, 500ms, then HIDDEN) */
void ui_keypad_slide_off(lv_obj_t *base);

#endif /* UI_ANIMATIONS_H */
