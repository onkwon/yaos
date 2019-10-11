#include "kernel/systick.h"
#include "arch/hw_sysclk.h"
#include "kernel/lock.h"
#include "kernel/sched.h"
#include "kernel/timer.h"
#include "syslog.h"

#include <stdint.h>
#include <assert.h>

unsigned long systick;
uint64_t systick64 = SYSTICK_INITIAL;

static unsigned long systick_clk,
		     systick_clk_period,
		     systick_clk_this_period;

/* Lock can be removed keeping it to be called only from `ISR_systick()`. */
static inline void update_tick(unsigned long delta)
{
	systick64 += delta;
}

void __attribute__((used)) ISR_systick(void)
{
	unsigned long ticks = 1;
#ifdef CONFIG_SLEEP_LONG
	static unsigned long clks;

	clks += systick_clk_this_period;
	systick_clk_this_period = hw_sysclk_period_get();

	if (clks < systick_clk_period)
		return;

	/* TODO: udiv instruction takes 2-12 cycles. replace it with bit
	 * operation. what about making period as a constant reducing the
	 * number of memory accesses? */
	ticks = clks / systick_clk_period;
	clks -= systick_clk_period * ticks;
#endif
	update_tick(ticks);
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

unsigned long get_systick_clk_this_period(void)
{
	return systick_clk_this_period;
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

	period = clk / hz;
	systick_clk = clk / period;
	systick_clk_period = systick_clk_this_period = period;

	assert(systick_clk == hz);
	assert(period > 1);

	hw_sysclk_reset();
	hw_sysclk_period_set(period);

	return get_systick_clk();
}

void systick_start(void)
{
	hw_sysclk_run();
}

void systick_stop(void)
{
	hw_sysclk_stop();
}

void run_systick_periodic(void)
{
	hw_sysclk_period_set(systick_clk_period);
}

void update_systick_period(unsigned long period)
{
	hw_sysclk_period_set(period);
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
