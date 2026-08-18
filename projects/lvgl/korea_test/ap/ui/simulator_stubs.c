/* simulator_stubs.c — RTOS stubs for PC simulator build (HAL_USE_EMULATOR).
 * rtos_create_thread is NOT implemented here because beken_ui.c uses LVGL
 * timer instead of a real thread when HAL_USE_EMULATOR is defined.
 */
#ifdef HAL_USE_EMULATOR

#include "os/os.h"
#include <stdint.h>
#include <stddef.h>

#ifdef _WIN32
#  include <windows.h>
static void _sim_sleep_ms(uint32_t ms) { Sleep(ms); }
#else
#  include <unistd.h>
static void _sim_sleep_ms(uint32_t ms) { usleep((useconds_t)ms * 1000u); }
#endif

void rtos_delay_milliseconds(uint32_t ms) { _sim_sleep_ms(ms); }

void rtos_init_mutex(beken_mutex_t *m)
{
    if (m) *m = (void *)(uintptr_t)1;  /* mark as initialised; no real lock needed */
}

void rtos_lock_mutex(beken_mutex_t *m)   { (void)m; }
void rtos_unlock_mutex(beken_mutex_t *m) { (void)m; }

int rtos_create_thread(beken_thread_t *thread, int priority, const char *name,
                       void (*fn)(beken_thread_arg_t), uint32_t stack,
                       beken_thread_arg_t arg)
{
    /* Not reached in simulator (beken_ui uses LVGL timer instead).
     * Defined to satisfy linker if referenced from non-emulator code. */
    (void)thread; (void)priority; (void)name;
    (void)fn; (void)stack; (void)arg;
    return -1;
}

#endif /* HAL_USE_EMULATOR */
