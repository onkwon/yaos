#include "kernel/systick.h"
#include "arch/hw_sysclk.h"
#include "kernel/lock.h"
#include "kernel/sched.h"
#include "syslog.h"
#include <stdint.h>
#include <assert.h>

unsigned long systick;
uint64_t systick64;

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
#if defined(CONFIG_SCHEDULER)
	resched();
#endif
}

unsigned long get_systick(void)
{
	return systick;
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
