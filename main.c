#include <kernel/task.h>
#include <kernel/page.h>
#include <kernel/module.h>
#include <asm/context.h>

static void load_user_task()
{
	extern int _user_task_list;
	struct task_t *p;
	unsigned int pri;

	for (p = (struct task_t *)&_user_task_list;
			get_task_state(p) == TASK_REGISTERED; p++) {
		if (p->addr == NULL)
			continue;

		/* share one kernel stack with all tasks to save memory */
		if (alloc_mm(p, init.mm.kernel, STACK_SHARE))
			continue;

		pri = get_task_pri(p);
		set_task_dressed(p, 0, p->addr);
		set_task_pri(p, pri);
		set_task_context(p);

		/* make it runnable, and add into runqueue */
		set_task_state(p, TASK_RUNNING);
		runqueue_add(p);
	}
}

static void cleanup()
{
	/* Clean up redundant code and data used during initialization */
	free_bootmem();
}

#include <time.h>
#include <kernel/syscall.h>

/* when no task in runqueue, this one takes place.
 * do some power saving */
static void idle()
{
	cleanup();
	yield();

	while (1) {
		printf("init()\n");
		printf("type %08x, state %08x, pri %08x\n",
				get_task_type(current),
				get_task_state(current),
				get_task_pri(current));
		printf("control %08x, sp %08x, msp %08x, psp %08x\n",
				GET_CON(), GET_SP(), GET_KSP(), GET_USP());
		msleep(500);
	}
}

#include <kernel/init.h>
#include <kernel/softirq.h>
#include <error.h>

static void __init init_task()
{
	/* stack must be allocated the first to make a task. and to build root
	 * relationship properly `current` must be set to `init`. */
	current = &init;

	if (alloc_mm(&init, NULL, STACK_SEPARATE)) {
		/* failed to make the init task. no way to go further */
		freeze();
	}

	set_task_dressed(&init, TASK_KERNEL, idle);
	set_task_context_hard(&init);
	set_task_pri(&init, LEAST_PRIORITY);
	set_task_state(&init, TASK_RUNNING);

	/* make it the sole */
	list_link_init(&init.children);
	list_link_init(&init.sibling);

	softirq_init(); /* must be called after init task built */
	load_user_task(); /* that are registered statically */

	/* a banner */
	printk("ibox %s %s\n", VERSION, MACHINE);

	/* switch from boot stack memory to new one */
	set_user_sp(init.mm.sp);
	set_kernel_sp(init.mm.kernel);

	/* everything ready now */
	sei();
	sys_schedule();

	/* it doesn't really reach up to this point. init task becomes
	 * idle task by scheduler as its context already set to idle */
	idle();
}

static void __init sys_init()
{
	extern char _init_func_list;
	unsigned int *p = (unsigned int *)&_init_func_list;

	while (*p)
		((void (*)())*p++)();
}

#include <driver/console.h>
#include <kernel/fs.h>

int __init main()
{
	sys_init();
	mm_init();
#ifdef CONFIG_FS
	fs_init();
#endif
#ifdef CONFIG_SYSCALL
	driver_init();
#endif
	systick_init();
	scheduler_init();

	init_task();

	return 0;
}
