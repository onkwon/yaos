#include <kernel/power.h>
#include <kernel/sched.h>
#include <kernel/systick.h>
#include <error.h>

void enter_sleep_mode()
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

	extern unsigned int cpu_idle, cpu_idle_stamp;
	cpu_idle += systick_ms - cpu_idle_stamp;

	run_scheduler(ON);
#endif
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
