#include "sysclk.h"
#include <foundation.h>
#include <kernel/systick.h>

#define LIMIT		100000 /* 100KHz, 10us */

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

		error("%dHz required at the minimum", LIMIT);
	}

	period = sysclk / hz - 1;
	sysfreq = sysclk / (period + 1);

	assert(sysfreq == hz);
	assert(period > 0);

	reset_sysclk();
	set_sysclk(period);

	return IRQ_SYSTICK;
}
