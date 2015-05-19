#include <foundation.h>
#include <stdlib.h>
#include <kernel/sched.h>
#include <syscall.h>

static void load_user_task()
{
	extern int _user_task_list;
	struct task_t *p = (struct task_t *)&_user_task_list;

	while (p->state) {
		/* sanity check */
		if (!p->addr) continue;
		if (!alloc_stack(p)) continue;

		/* share one kernel stack with all tasks to save memory */
		p->stack.kernel = init.stack.kernel;

		/* make relationship */
		list_add(&p->sibling, &init.children);

		set_task_context(p);
		set_task_state(p, TASK_RUNNING);

		/* initial state of every tasks are runnable,
		 * add into runqueue */
		runqueue_add(p);

		DEBUG(("%s: state %08x, addr %08x, "
			"sp %08x brk %08x heap %08x size %d kernel %08x",
				is_task_realtime(p)? "REALTIME" : "NORMAL  ",
				p->state, p->addr,
				p->stack.sp, p->stack.brk, p->stack.heap,
				p->stack.size,
				get_kernel_stack(p->stack.kernel)));

		p++;
	}
}

static void cleanup()
{
	/* Clean up redundant code and data, used during initializing */
	free_bootmem();
}

/* when no task in runqueue, this one takes place.
 * do some power saving */
static void idle()
{
	cleanup();

	reset_task_state(current, TASK_RUNNING);
	schedule();

	while (1) {
		printf("init()\n");
		printf("control %08x, sp %08x, msp %08x, psp %08x\n",
				GET_CON(), GET_SP(), GET_KSP(), GET_USP());
		msleep(500);
	}
}

#include <asm/init.h>

static void __init init_task()
{
	TASK_INIT(init, idle);
	set_task_priority(&init, LEAST_PRIORITY);
	set_task_state(&init, TASK_KERNEL | TASK_RUNNING);
	set_task_sp(init.stack.sp);
	set_kernel_sp(init.stack.kernel);

	load_user_task();

	/* everything ready now */
	printk("ibox %s %s\n", VERSION, MACHINE);

	sei();
	sys_schedule();

	/* it doesn't really reach up to this point. init task becomes
	 * idle task by scheduler as its context already set to idle */
	idle();
}

static void sys_init()
{
	extern char _init_func_list;
	unsigned long *p = (unsigned long *)&_init_func_list;

	while (*p)
		((void (*)())*p++)();
}

#include <kernel/page.h>
#include <driver/console.h>

int main()
{
	sys_init();
	mm_init();
#ifdef CONFIG_SYSCALL
	driver_init();
#else
	console_open(O_RDWR | O_NONBLOCK);
#endif
	systick_init();
	scheduler_init();

	init_task();

	return 0;
}
