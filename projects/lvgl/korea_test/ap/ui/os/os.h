#pragma once
/* os/os.h — wrapper header.
 *
 * This file sits in ui/ which is on the -I search path before the SDK
 * include directory, so it intercepts every #include <os/os.h>.
 *
 * - Emulator build (HAL_USE_EMULATOR): supply minimal RTOS type stubs so
 *   simulator_stubs.c can compile without the full BK7258 SDK.
 * - Hardware build: forward to the real SDK header via #include_next.
 */

#ifdef HAL_USE_EMULATOR

#include <stdint.h>

typedef void *beken_thread_t;
typedef void *beken_thread_arg_t;
typedef void *beken_mutex_t;

void rtos_delay_milliseconds(uint32_t ms);
void rtos_init_mutex(beken_mutex_t *m);
void rtos_lock_mutex(beken_mutex_t *m);
void rtos_unlock_mutex(beken_mutex_t *m);
int  rtos_create_thread(beken_thread_t *thread, int priority, const char *name,
                        void (*fn)(beken_thread_arg_t), uint32_t stack,
                        beken_thread_arg_t arg);

#else /* hardware build — pull in the real SDK os/os.h */

#include_next <os/os.h>

#endif /* HAL_USE_EMULATOR */
