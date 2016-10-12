#include <foundation.h>
#include <kernel/page.h>
#include <kernel/timer.h>
#include <asm/power.h>

static void cleanup()
{
	/* Clean up redundant code and data used during initialization */
	free_bootmem();
}

/* when no task in runqueue, this one takes place.
 * do some power savings */
void idle()
{
	unsigned int irqflag;

	cleanup();

	while (1) {
		kill_zombie();

		/* if not TASK_RUNNING it is the best chance to enter power
		 * saving mode since the init task gets its turn from scheduler
		 * without changing the state to TASK_RUNNING when there is no
		 * task to schedule. */
		if (get_task_state(current)) {
#ifdef CONFIG_DEBUG
			/* check if there is any peripherals enabled that is
			 * actually doing nothing but only consuming power.
			 * turn it off if so */
			extern void disp_sysinfo();
			disp_sysinfo();
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

			/* having a timer running means still have a job to be
			 * done when the timer expires or free to enter stop
			 * mode which disable most peripherals off even the
			 * system clock. */
			if (get_timer_nr())
				enter_sleep_mode();
			else
				enter_stop_mode();

			/* post-dos(); */
		}

		spin_lock_irqsave(&current->lock, irqflag);
		set_task_pri(current, LOWEST_PRIORITY);
		spin_unlock_irqrestore(&current->lock, irqflag);
		yield();
	}
}

/* Do not register idle task the same way of `REGISTER_TASK()` like usual
 * user tasks. This one becomes the init task by boot code. */
