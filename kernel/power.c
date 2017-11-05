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
