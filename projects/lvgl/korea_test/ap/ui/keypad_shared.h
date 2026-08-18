/* keypad_shared.h — Shared numeric keypad input handling.
 * Called by every page that has a keypad overlay.
 */
#ifndef __KEYPAD_SHARED_H__
#define __KEYPAD_SHARED_H__

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Append digit '0'-'9' to the active edit buffer. No-op if not in edit mode. */
void keypad_digit(char digit);

/* Remove last character from edit buffer. */
void keypad_backspace(void);

/* Confirm edit: write buffer to settings, update the target label, exit edit mode. */
void keypad_confirm(void);

/* Cancel edit: discard buffer, exit edit mode. */
void keypad_cancel(void);

/* Returns true while an edit is in progress. */
bool keypad_is_active(void);

#ifdef __cplusplus
}
#endif
#endif /* __KEYPAD_SHARED_H__ */
