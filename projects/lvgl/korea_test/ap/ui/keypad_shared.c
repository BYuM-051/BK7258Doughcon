/* keypad_shared.c — Shared keypad logic (digit input → settings write) */
#include <string.h>
#include <stdio.h>
#include "keypad_shared.h"
#include "device_state.h"
#include "settings.h"
#include "hardware_hal.h"

/* Map edit target → settings key (mirrors _EDIT_TARGET_MAP in java_parser.py) */
static const struct { int target; const char *key; } s_edit_map[] = {
    { EDIT_FREEZE_TEMP,       "CurrentSaveFreezeTemp"            },
    { EDIT_DEFROST_TEMP,      "CurrentSaveDefreezeTemp"          },
    { EDIT_DEFROST_HUMIDITY,  "CurrentSaveDefreezeHumidity"      },
    { EDIT_DEFROST_TIME_HOUR, "CurrentSaveDefreezeTimeHour"      },
    { EDIT_DEFROST_TIME_MIN,  "CurrentSaveDefreezeTimeMin"       },
    { EDIT_FERM1_TEMP,        "CurrentSaveFermentation1Temp"     },
    { EDIT_FERM1_HUMIDITY,    "CurrentSaveFermentation1Humidity" },
    { EDIT_FERM1_TIME_HOUR,   "CurrentSaveFermentation1TimeHour" },
    { EDIT_FERM1_TIME_MIN,    "CurrentSaveFermentation1TimeMin"  },
    { EDIT_FERM2_TEMP,        "CurrentSaveFermentation2Temp"     },
    { EDIT_FERM2_HUMIDITY,    "CurrentSaveFermentation2Humidity" },
    { EDIT_FERM2_TIME_HOUR,   "CurrentSaveFermentation2TimeHour" },
    { EDIT_FERM2_TIME_MIN,    "CurrentSaveFermentation2TimeMin"  },
    { EDIT_COMPLETE_YEAR,     "originCompleteYear"               },
    { EDIT_COMPLETE_MONTH,    "originCompleteMonth"              },
    { EDIT_COMPLETE_DAY,      "originCompleteDay"                },
    { EDIT_COMPLETE_HOUR,     "originCompleteHour"               },
    { EDIT_COMPLETE_MIN,      "originCompleteMin"                },
};
#define EDIT_MAP_SIZE  (sizeof(s_edit_map) / sizeof(s_edit_map[0]))

static const char *_key_for_target(int target)
{
    for (size_t i = 0; i < EDIT_MAP_SIZE; i++)
        if (s_edit_map[i].target == target) return s_edit_map[i].key;
    return NULL;
}

void keypad_digit(char digit)
{
    device_state_t *st = &g_device_state;
    if (!st->edit_mode) return;
    size_t len = strlen(st->edit_buffer);
    if (len < sizeof(st->edit_buffer) - 1) {
        st->edit_buffer[len]     = digit;
        st->edit_buffer[len + 1] = '\0';
        hal_buzzer_beep();
    }
}

void keypad_backspace(void)
{
    device_state_t *st = &g_device_state;
    if (!st->edit_mode) return;
    size_t len = strlen(st->edit_buffer);
    if (len > 0) {
        st->edit_buffer[len - 1] = '\0';
        hal_buzzer_beep();
    }
}

void keypad_confirm(void)
{
    device_state_t *st = &g_device_state;
    if (!st->edit_mode) return;

    const char *key = _key_for_target(st->edit_target);
    if (key && st->edit_buffer[0] != '\0')
        settings_set_str(key, st->edit_buffer);

    st->edit_mode   = false;
    st->edit_target = EDIT_NONE;
    memset(st->edit_buffer, 0, sizeof(st->edit_buffer));
    hal_buzzer_beep();
}

void keypad_cancel(void)
{
    device_state_t *st = &g_device_state;
    st->edit_mode   = false;
    st->edit_target = EDIT_NONE;
    memset(st->edit_buffer, 0, sizeof(st->edit_buffer));
    hal_buzzer_beep();
}

bool keypad_is_active(void)
{
    return g_device_state.edit_mode;
}
