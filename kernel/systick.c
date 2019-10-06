#include "kernel/systick.h"
#include "arch/hw_sysclk.h"
#include "kernel/lock.h"
#include "kernel/sched.h"
#include "kernel/timer.h"
#include "syslog.h"

#include <stdint.h>
#include <assert.h>

unsigned long systick;
uint64_t systick64 = 0xFFFFEC77; /* makes 32bit overflow in 5sec at 1Khz for
				    the system validation */

static unsigned long systick_clk, systick_clk_period;

/* Lock can be removed keeping it to be called only from `ISR_systick()`. */
static inline void update_tick(unsigned long delta)
{
	systick64 += delta;
}

void __attribute__((used)) ISR_systick(void)
{
#ifdef CONFIG_SLEEP_LONG
#else
#endif
	update_tick(1);
	timer_run();
#if defined(CONFIG_SCHEDULER)
	resched();
#endif
}

unsigned long get_systick(void)
{
	return systick;
}

/** return 64-bit system ticks
 * call only in a user task or in system call handler. calling it in higher
 * priority interrupt would cause problem somthine like time jump to way back
 * past or future. when priority interrupt comes in systick update would loose
 * its operation atomicity */
uint64_t get_systick64_core(void)
{
	return systick64;
}

unsigned long get_systick_clk(void)
{
	return systick_clk;
}

unsigned long get_systick_clk_period(void)
{
	return systick_clk_period;
}

unsigned long systick_clk_to_ticks(unsigned long clks)
{
	return clks / systick_clk_period;
}

unsigned long systick_to_clks(unsigned long ticks)
{
	return ticks * systick_clk_period;
}

unsigned long systick_init(unsigned long hz)
{
	unsigned long clk, period;

	clk = hw_sysclk_freq_get();
	period = clk / hz - 1;

	if (period > HW_SYSCLK_RESOLUTION) {
		hz = clk / (HW_SYSCLK_RESOLUTION + 1) + 1;

		warn("HZ must be higher than %lu",
				clk / (HW_SYSCLK_RESOLUTION + 1));
	} else if (!period || hz > SYSCLK_MAX) {
		hz = SYSCLK_MAX;

		warn("can not exceed %luHz", SYSCLK_MAX);
	}

	period = clk / hz - 1;
	systick_clk = clk / (period + 1);
	systick_clk_period = period + 1;

	assert(systick_clk == hz);
	assert(period > 0);

	hw_sysclk_reset();
	hw_sysclk_period_set(period);

	return get_systick_clk();
}

void systick_start(void)
{
	hw_sysclk_run();
}

void set_timeout(unsigned long *goal, unsigned long msec)
{
	*goal = get_systick() + MSEC_TO_TICKS(msec) - 1;
}

bool is_timedout(unsigned long goal)
{
	if (time_after(goal, get_systick()))
		return true;

	return false;
}

void mdelay(unsigned long msec)
{
	unsigned long tout;
	set_timeout(&tout, msec);
	while (!is_timedout(tout));
}
