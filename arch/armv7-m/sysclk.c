#include "sysclk.h"
#include <foundation.h>
#include <kernel/systick.h>

#define LIMIT		(100 * KHZ) /* 100KHz */

static unsigned int sysclk_period;

unsigned int get_sysclk_period()
{
	return sysclk_period;
}

int sysclk_init()
{
	unsigned int sysclk, period, hz;

	hz = HZ;
	sysclk = get_sysclk_freq();
	period = sysclk / hz - 1;

	if (period > SYSTICK_MAX) {
		hz = sysclk / (SYSTICK_MAX + 1) + 1;

		error("HZ must be higher than %d", sysclk / (SYSTICK_MAX + 1));
	} else if (!period || hz > LIMIT) {
		hz = LIMIT;

		error("can not exceed %dHz", LIMIT);
	}

	period = sysclk / hz - 1;
	sysfreq = sysclk / (period + 1);
	sysclk_period = period + 1;

	assert(sysfreq == hz);
	assert(period > 0);

	reset_sysclk();
	set_sysclk(period);

	return NVECTOR_SYSTICK;
}
