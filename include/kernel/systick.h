#ifndef __YAOS_SYSTICK_H__
#define __YAOS_SYSTICK_H__

/**
 * kerenl runs on systick and the systick gets counted on sysclk. sysclk comes
 * from core clock.
 */

#include "types.h"

#if !defined(HW_SYSCLK_MAX_KHZ)
#define HW_SYSCLK_MAXFREQ_KHZ		(100U)
#endif
#if !defined(HW_SYSCLK_RESOLUTION)
#define HW_SYSCLK_RESOLUTION		((1UL << 24) - 1) /* 24-bit timer */
#endif

#define SYSCLK_MAX			(HW_SYSCLK_MAXFREQ_KHZ * KHZ)

void systick_start(void);
unsigned long systick_init(unsigned long hz);
unsigned long sysclk_to_ticks(unsigned long clks);
unsigned long systick_to_clks(unsigned long ticks);
unsigned long sysclk_get(void);
unsigned long sysclk_get_period(void);

#endif /* __YAOS_SYSTICK_H__ */
