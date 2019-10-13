#ifndef __YAOS_SYSTICK_H__
#define __YAOS_SYSTICK_H__

/**
 * kerenl runs on systick and the systick gets counted on sysclk. sysclk comes
 * from core clock.
 */

#include "types.h"

#define SYSTICK_HZ			1000U
#define SYSTICK_INITIAL			0xFFFFEC77 /* makes 32bit overflow in
						      5sec at 1Khz for the
						      system validation */

#if !defined(HW_SYSCLK_MAX_KHZ)
#define HW_SYSCLK_MAXFREQ_KHZ		(100U)
#endif
#if !defined(HW_SYSCLK_RESOLUTION)
#define HW_SYSCLK_RESOLUTION		((1UL << 24) - 1) /* 24-bit timer */
#endif

#define SYSCLK_MAX			(HW_SYSCLK_MAXFREQ_KHZ * KHZ)

#define SEC_TO_TICKS(sec)		((sec) * SYSTICK_HZ)
#define TICKS_TO_SEC(ticks)		((ticks) / SYSTICK_HZ)
#define MSEC_TO_TICKS(msec)		(SEC_TO_TICKS(msec) / 1000)
#define TICKS_TO_MSEC(ticks)		(TICKS_TO_SEC((ticks) * 1000))

#define time_after(goal, chasing)	((int)(goal)    - (int)(chasing) < 0)
#define time_before(goal, chasing)	((int)(chasing) - (int)(goal)    < 0)

unsigned long systick_init(unsigned long hz);
void systick_start(void);
void systick_stop(void);
unsigned long get_systick(void);
uint64_t get_systick64_core(void);

unsigned long get_systick_clk(void);
unsigned long get_systick_clk_period(void);
unsigned long get_systick_clk_this_period(void);
unsigned long systick_to_clks(unsigned long ticks);
unsigned long systick_clk_to_ticks(unsigned long clks);
void update_systick_period(unsigned long period);
void run_systick_periodic(void);

void set_timeout(unsigned long *goal, unsigned long msec);
bool is_timedout(unsigned long goal);
void mdelay(unsigned long msec);

#endif /* __YAOS_SYSTICK_H__ */
