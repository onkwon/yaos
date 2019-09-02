#ifndef __YAOS_SYSTICK_H__
#define __YAOS_SYSTICK_H__

/**
 * kerenl runs on systick and the systick gets counted on sysclk. sysclk comes
 * from core clock.
 */

#include "types.h"

#define SYSTICK_HZ			1000U

#if !defined(HW_SYSCLK_MAX_KHZ)
#define HW_SYSCLK_MAXFREQ_KHZ		(100U)
#endif
#if !defined(HW_SYSCLK_RESOLUTION)
#define HW_SYSCLK_RESOLUTION		((1UL << 24) - 1) /* 24-bit timer */
#endif

#define SYSCLK_MAX			(HW_SYSCLK_MAXFREQ_KHZ * KHZ)

#define TICKS_TO_MSEC(ticks)		(1000 * ticks / SYSTICK_HZ)
#define MSEC_TO_TICKS(msec)		(msec * SYSTICK_HZ / 1000)

unsigned long systick_init(unsigned long hz);
void systick_start(void);
unsigned long get_systick(void);

unsigned long get_systick_clk(void);
unsigned long get_systick_clk_period(void);
unsigned long systick_to_clks(unsigned long ticks);
unsigned long systick_clk_to_ticks(unsigned long clks);

#endif /* __YAOS_SYSTICK_H__ */
