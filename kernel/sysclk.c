#include "kernel/sysclk.h"
#include "arch/hw_sysclk.h"
#include "kernel/lock.h"
#include "kernel/sched.h"
#include "syslog.h"
#include <stdint.h>
#include <assert.h>

unsigned long __attribute__((section(".data"))) systick;
uint64_t __attribute__((section(".data"))) systick64;

static unsigned long _sysclk, _sysclk_period;

/* Lock can be removed keeping it to be called only from `ISR_systick()`. */
static inline void update_tick(unsigned long delta)
{
	systick64 += delta;
}

void ISR_systick(void)
{
#ifdef CONFIG_SLEEP_LONG
#else
#endif
	update_tick(1);
	resched();
}

unsigned long sysclk_get(void)
{
	return _sysclk;
}

unsigned long sysclk_get_period(void)
{
	return _sysclk_period;
}

unsigned long sysclk_to_ticks(unsigned long clks)
{
	return clks / _sysclk_period;
}

unsigned long systick_to_clks(unsigned long ticks)
{
	return ticks * _sysclk_period;
}

unsigned long sysclk_init(unsigned long hz)
{
	unsigned long sysclk, period;

	sysclk = hw_sysclk_get_freq();
	period = sysclk / hz - 1;

	if (period > HW_SYSCLK_RESOLUTION) {
		hz = sysclk / (HW_SYSCLK_RESOLUTION + 1) + 1;

		syslog("HZ must be higher than %lu",
				sysclk / (HW_SYSCLK_RESOLUTION + 1));
	} else if (!period || hz > SYSCLK_MAX) {
		hz = SYSCLK_MAX;

		syslog("can not exceed %luHz", SYSCLK_MAX);
	}

	period = sysclk / hz - 1;
	_sysclk = sysclk / (period + 1);
	_sysclk_period = period + 1;

	assert(_sysclk == hz);
	assert(period > 0);

	hw_sysclk_reset();
	hw_sysclk_set_period(period);

	hw_sysclk_run();

	return sysclk_get();
}
