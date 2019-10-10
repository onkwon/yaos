#ifndef __YAOS_POWER_H__
#define __YAOS_POWER_H__

#include <stdlib.h>

/** Sleep type */
typedef enum {
	/** Only the core stopped while all peripherals continue to run */
	SLEEP_NAP	= 1,
	/** The core and peripherals stop to run including external clock while
	 * still retaining the SRAM and register contents */
	SLEEP_DEEP	= 2,
	/** RTC and watchdog are the only ones continue to run. All the
	 * contents of SRAM or the registers are not preserved */
	SLEEP_BLACKOUT	= 3,
} sleep_t;

void sys_reboot(size_t msec);
void enter_sleep_mode(sleep_t sleeptype);

void hw_enter_sleep_nap(void);
void hw_sleep_on_exit(void);

#endif /* __YAOS_POWER_H__ */
