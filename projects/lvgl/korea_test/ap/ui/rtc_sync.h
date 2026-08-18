#pragma once
#include <stdbool.h>

/* Call once from beken_ui_init() after settings_init().
 * Restores the last saved epoch from EasyFlash so the clock is
 * immediately plausible even before the MCU syncs. */
void rtc_sync_init(void);

/* Returns true if the current system time is >= 2020-01-01.
 * Used to decide whether MCU sync is still needed. */
bool rtc_is_valid(void);

/* Called from uart_comm.c when the MCU provides its current time.
 * yr2 = 2-digit year (e.g. 25 for 2025), mo = 1..12.
 * Validates, applies settimeofday(), and saves to EasyFlash. */
void rtc_sync_from_mcu(int yr2, int mo, int day, int hr, int mn, int sc);

/* Call from uart_comm_tick() (or a timer) every ~60 seconds.
 * Saves current epoch to EasyFlash so next power-on starts close to
 * the correct time even if MCU sync hasn't happened yet. */
void rtc_sync_periodic_save(void);

/* Call immediately after hal_rtc_set() when the user manually sets the clock.
 * Saves the new epoch to EasyFlash right away so a reboot within the next
 * 60 seconds does not revert the time. */
void rtc_sync_save_now(void);
