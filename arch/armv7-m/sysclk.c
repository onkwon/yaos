/*
 * "[...] Sincerity (comprising truth-to-experience, honesty towards the self,
 * and the capacity for human empathy and compassion) is a quality which
 * resides within the laguage of literature. It isn't a fact or an intention
 * behind the work [...]"
 *
 *             - An introduction to Literary and Cultural Theory, Peter Barry
 *
 *
 *                                                   o8o
 *                                                   `"'
 *     oooo    ooo  .oooo.    .ooooo.   .oooo.o     oooo   .ooooo.
 *      `88.  .8'  `P  )88b  d88' `88b d88(  "8     `888  d88' `88b
 *       `88..8'    .oP"888  888   888 `"Y88b.       888  888   888
 *        `888'    d8(  888  888   888 o.  )88b .o.  888  888   888
 *         .8'     `Y888""8o `Y8bod8P' 8""888P' Y8P o888o `Y8bod8P'
 *     .o..P'
 *     `Y8P'                   Kyunghwan Kwon <kwon@yaos.io>
 *
 *  Welcome aboard!
 */

#include "include/sysclk.h"
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
