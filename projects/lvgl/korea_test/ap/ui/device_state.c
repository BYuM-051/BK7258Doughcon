/* device_state.c */
#include <string.h>
#include "device_state.h"
#include "settings.h"

device_state_t g_device_state;

void device_state_init(void)
{
    memset(&g_device_state, 0, sizeof(g_device_state));

    /* Restore persisted state from settings */
    g_device_state.degree           = settings_get_int("Degree");
    g_device_state.language         = settings_get_int("LANGUAGE");
    g_device_state.mute             = (settings_get_int("Mute") == 1);
    g_device_state.operation        = (settings_get_int("saveChecking") == 1);
    g_device_state.current_op_mode  = settings_get_int("saveOperationTemp");
    g_device_state.day_period       = settings_get_int("saveDayPeriod");
    g_device_state.remain_hour      = settings_get_int("saveRemainHour");
    g_device_state.remain_min       = settings_get_int("saveRemainMin");
}
