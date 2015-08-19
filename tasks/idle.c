#include <foundation.h>
#include <kernel/page.h>
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
	cleanup();

	struct task *task;

	while (1) {
		while (zombie) {
			task = (struct task *)zombie;
			zombie = task->addr;
			destroy(task);
		}

		/* if not TASK_RUNNING it is the best chance to
		 * enter sleep mode since the init task gets its turn
		 * from scheduler without changing the state to TASK_RUNNING
		 * when there is no task to schedule. */
		if (get_task_state(current)) {
#ifdef CONFIG_DEBUG
			/* check if there is any peripherals enabled that is
			 * actually doing nothing but only consuming power
			 * during in the power saving mode. Turn it off if so */
			extern void disp_sysinfo();
			disp_sysinfo();
#endif
			/* pre-dos(); */
			/* set a timer here to enter stop mode saving more
			 * power when sleeping longer
			 * add_timer(timeout, enter_stop_mode); */
			enter_sleep_mode();
			/* post-dos(); */
		}

		yield();
	}
}

/* Do not register idle task the same way of `REGISTER_TASK()` like usual
 * user tasks. This one becomes the init task by boot code. */
