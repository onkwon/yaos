#include <foundation.h>
#include <kernel/page.h>
#include <kernel/timer.h>
#include <kernel/power.h>
#include <kernel/systick.h>

static void cleanup()
{
	/* Clean up redundant code and data used during initialization */
	free_bootmem();
}

/* NOTE: FIXME: get sysclk periodic whenever woken by any other interrupts than
 * systick interrupt
 *
 * It is okay when woken up by systick interrupt because it is already set as
 * periodic in the last update. But the problem arises when it is woken up by
 * any other interrupts. If an interrupt occurs and tasks take place from then,
 * each task would have time slice as much as the current interval which can be
 * up to get_raw_sysclk_max(). So run_sysclk_periodic() should be placed in
 * every interrupt handler. maybe I think implement the common interrupt
 * handler to put it in only a place. */
static inline void update_sleep_period()
{
#ifdef CONFIG_SLEEP_LONG
	struct ktimer *timer;
	int ticks_left, clks2go;

	timer = get_timer_nearest();

	if (timer) {
		ticks_left = timer->expires - systick;
		clks2go = ticks_to_clks(ticks_left) - get_curr_interval();

		if (clks2go <= (int)get_sysclk_period()) { /* expires on the next */
			run_sysclk_periodic();
			return;
		}

		if (clks2go > get_raw_sysclk_max())
			clks2go = get_raw_sysclk_max();

		set_sleep_interval(clks2go);
	}
#endif
}

/* when no task in runqueue, this one takes place.
 * do some power savings */
void idle()
{
	extern unsigned int cpu_idle, cpu_idle_stamp;
	unsigned int tout = 0;
	int cpu_total = 0;

	cleanup();

	while (1) {
		kill_zombie();

#ifdef CONFIG_CPU_LOAD
		cpu_total += systick - cpu_idle_stamp;
		cpu_idle_stamp = systick;
		if (ticks_to_sec(cpu_total)) { /* every second */
			cpuload = 100 - cpu_idle * 100 / cpu_total;

			cpu_total = 0;
			cpu_idle = 0;
		}
#endif

		/* if not TASK_RUNNING it is the best chance to enter power
		 * saving mode since the init task gets its turn from scheduler
		 * without changing the state to TASK_RUNNING when there is no
		 * task to schedule. */
		if (get_task_state(current)) {
#ifdef CONFIG_DEBUG
			/* check if there is any peripherals enabled that is
			 * actually doing nothing but only consuming power.
			 * turn it off if so */
			if (is_timeout(tout)) {
				extern void disp_sysinfo();
				disp_sysinfo();
				set_timeout(&tout, msec_to_ticks(10000));

				extern unsigned int get_gpio_state(int port);
				printk("PORTA %x\n", get_gpio_state(0));
				printk("PORTB %x\n", get_gpio_state(1));
				printk("PORTC %x\n", get_gpio_state(2));
				printk("PORTD %x\n", get_gpio_state(3));
			}
#endif
			/* pre-dos();
			 *
			 * flushing must be performed or/and you have to wait
			 * until ongoing process to be done before entering
			 * power saving mode.
			 *
			 * e.g. while (!gbi(*(volatile unsigned int *)USART1,
			 * 6)); for transmission complete or data(even system)
			 * can be corrupted. */

			update_sleep_period();

			/* FIXME: idle task's state can be changed at any time
			 * since the job of destroying zombie tasks is
			 * delegated to idle task. */
			if (!get_task_state(current))
				continue;

			/* having a timer running means still have a job to be
			 * done when the timer expires or free to enter stop
			 * mode which disable most peripherals off even the
			 * system clock. */
			if (get_timer_nr()) {
				enter_sleep_mode();
#ifdef CONFIG_SLEEP_DEEP
			} else {
				unsigned long long stamp = get_systick64();
				notice("[%08x%08x] entering stop mode",
						(unsigned int)(stamp >> 32),
						(unsigned int)stamp);

				enter_stop_mode();
#endif
			}

			/* post-dos(); */
		}

		set_task_pri(current, LOWEST_PRIORITY);
		yield();
	}
}

/* Do not register idle task the same way of `REGISTER_TASK()` like usual
 * user tasks. This one becomes the init task by boot code. */
