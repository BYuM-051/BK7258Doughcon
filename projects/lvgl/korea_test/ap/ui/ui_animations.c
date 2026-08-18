/* ui_animations.c — LVGL screen/keypad animation helpers */
#include "ui_animations.h"

static void _anim_x_cb(void *obj, int32_t v)
{
    lv_obj_set_x((lv_obj_t *)obj, v);
}

#if UI_TITLE_ANIM_ENABLE
void ui_title_anim(lv_obj_t *title)
{
    if (title == NULL) return;
    lv_coord_t target_x = lv_obj_get_x(title);
    lv_coord_t w        = lv_obj_get_width(title);

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, title);
    lv_anim_set_exec_cb(&a, _anim_x_cb);
    lv_anim_set_values(&a, target_x - (w > 0 ? w : 320), target_x);
    lv_anim_set_time(&a, 300);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_out);
    lv_anim_start(&a);
}
#endif /* UI_TITLE_ANIM_ENABLE */

#if UI_KEYPAD_ANIM_ENABLE
static void _anim_hide_cb(lv_anim_t *a)
{
    lv_obj_add_flag((lv_obj_t *)lv_anim_get_user_data(a), LV_OBJ_FLAG_HIDDEN);
}

void ui_keypad_slide_on(lv_obj_t *base)
{
    if (base == NULL) return;
    lv_coord_t w = lv_obj_get_width(base);
    lv_coord_t off = -(w > 0 ? w : 1024);

    lv_obj_set_x(base, off);
    lv_obj_clear_flag(base, LV_OBJ_FLAG_HIDDEN);

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, base);
    lv_anim_set_exec_cb(&a, _anim_x_cb);
    lv_anim_set_values(&a, off, 0);
    lv_anim_set_time(&a, 600);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_out);
    lv_anim_start(&a);
}

void ui_keypad_slide_off(lv_obj_t *base)
{
    if (base == NULL) return;
    lv_coord_t w = lv_obj_get_width(base);
    lv_coord_t off = -(w > 0 ? w : 1024);

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, base);
    lv_anim_set_exec_cb(&a, _anim_x_cb);
    lv_anim_set_values(&a, 0, off);
    lv_anim_set_time(&a, 600);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_in);
    lv_anim_set_user_data(&a, base);
    lv_anim_set_completed_cb(&a, _anim_hide_cb);
    lv_anim_start(&a);
}
#else
void ui_keypad_slide_on(lv_obj_t *base)
{
    if (base == NULL) return;
    /* 즉시 표시 (UI_KEYPAD_ANIM_ENABLE=0) */
    lv_obj_set_x(base, 0);
    lv_obj_clear_flag(base, LV_OBJ_FLAG_HIDDEN);
}

void ui_keypad_slide_off(lv_obj_t *base)
{
    if (base == NULL) return;
    /* 즉시 숨김. 다음 표시를 위해 위치도 원상복구. (UI_KEYPAD_ANIM_ENABLE=0) */
    lv_obj_set_x(base, 0);
    lv_obj_add_flag(base, LV_OBJ_FLAG_HIDDEN);
}
#endif /* UI_KEYPAD_ANIM_ENABLE */
