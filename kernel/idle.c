#include "kernel/task.h"
#include "kernel/systick.h"
#include "kernel/syscall.h"
#include "kernel/timer.h"
#include "kernel/power.h"
#include "syslog.h"

static void cleanup(void)
{
}

#if defined(CONFIG_SLEEP_LONG)
static unsigned long update_sleep_period(void)
{
	int32_t ticks_remained;
	unsigned long clks_to_go, period, this_period;

	ticks_remained = timer_nearest();

	period = get_systick_clk_period();
	this_period = get_systick_clk_this_period();
	clks_to_go = ticks_remained * period - this_period;
	clks_to_go = max((long)clks_to_go, (long)period); // in case of shorter than minimum
	clks_to_go = min(clks_to_go, HW_SYSCLK_RESOLUTION);

	if (clks_to_go != this_period) {
#if 0
		debug("sleep period change from %lu(%lu) to %lu(%lu)",
				this_period, this_period / period,
				clks_to_go, clks_to_go / period);
#endif
		update_systick_period(clks_to_go);
	}

	return min(clks_to_go / period, (unsigned long)ticks_remained);
}
#else /* !CONFIG_SLEEP_LONG */
#define update_sleep_period()
#endif

void idle_task(void)
{
	cleanup();

	set_task_pri(current, TASK_PRIORITY_LOWEST);
	set_task_state(current, TASK_STOPPED); // no more go into runqueue
	yield();

	unsigned long cpu_total, cpu_load, cpu_idle, cpu_idle_stamp;

	cpu_total = cpu_load = cpu_idle = 0;
	cpu_idle_stamp = SYSTICK_INITIAL;

	while (1) {
		free_zombie();

#if defined(CONFIG_CPU_LOAD)
		unsigned long stamp = get_systick();
		if (stamp > cpu_idle_stamp)
			cpu_total += stamp - cpu_idle_stamp;
		else
			cpu_total += (unsigned long)-1 - cpu_idle_stamp + stamp;
		cpu_idle_stamp = stamp;

		if (TICKS_TO_SEC(cpu_total)) { // every second
			cpu_load = 100UL - cpu_idle * 100 / cpu_total;
			debug("CPU load %lu%% %lu %lu", cpu_load, cpu_idle, cpu_total);
			cpu_total = cpu_idle = 0;
		}
#endif

		/* TODO: check if there is any peripherals enabled that is
		 * actually doing nothing but only consuming power. turn it off
		 * if so. */
		//disp_sysinfo();

		//debug("%lx: %lu", get_systick(), (uint32_t)current->sum_exec_runtime);

		/* TODO: pre-dos(): flushing must be performed or/and you have
		 * to wait until ongoing process to be done before entering
		 * power saving mode.
		 *
		 * e.g. while (!gbi(*(reg_t)USART1, 6)); // for transmission
		 * complete or data(even system) can be corrupted. */

		/* having a running timer means still have a job to be done
		 * when the timer expires. otherwise free to enter stop mode
		 * which disable most peripherals off even the system clock. */
#if defined(CONFIG_SLEEP_DEEP)
		if (timer_nearest() == TIMER_EMPTY) {
			enter_sleep_mode(SLEEP_DEEP);
		} else {
#else
		{
#endif
			cpu_idle += update_sleep_period();
			enter_sleep_mode(SLEEP_NAP);
		}

		/* TODO: post-dos() */
	}
}
