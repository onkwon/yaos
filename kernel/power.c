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

#include <kernel/power.h>
#include <kernel/sched.h>
#include <kernel/systick.h>
#include <error.h>

int cpuload;
unsigned int cpu_idle, cpu_idle_stamp;

static inline void enter_sleep_mode_core()
{
#ifdef CONFIG_CPU_LOAD
	/* FIXME: make sure cpu load measuring to be done correctly
	 * yeah, it is too much to turn off the scheduler for the measuring.
	 * find out another way to do it. */
	run_scheduler(OFF);
#endif

	__enter_sleep_mode();

#ifdef CONFIG_CPU_LOAD
	assert(current == &init);

	cpu_idle += systick - cpu_idle_stamp;

	run_scheduler(ON);
#endif
}

void enter_sleep_mode(sleep_t sleeptype)
{
	switch (sleeptype) {
	case SLEEP_DEEP:
		__enter_stop_mode();
		break;
	case SLEEP_BLACKOUT:
		__enter_standby_mode();
		break;
	case SLEEP_NAP:
		enter_sleep_mode_core();
	default:
		break;
	}
}

#include <kernel/device.h>

void reboot()
{
	notice("Rebooting...");
	device_sync_all();
	dsb();
	__reboot();
	freeze();
}
