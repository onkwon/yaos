#include <foundation.h>
#include <kernel/init.h>
#include <kernel/task.h>
#include <kernel/sched.h>

static void __init load_user_task()
{
	extern char _user_task_list;
	struct task *p;
	unsigned int pri;

	for (p = (struct task *)&_user_task_list; *(unsigned int *)p; p++) {
		if (p->addr == NULL)
			continue;

		/* share the init kernel stack to save memory */
		if (alloc_mm(p, &init, STACK_SHARED))
			continue;

		pri = get_task_pri(p);
		set_task_dressed(p, p->flags | STACK_SHARED, p->addr);
		set_task_pri(p, pri);
		set_task_context(p, wrapper);

		/* make it runnable, and add into runqueue */
		set_task_state(p, TASK_RUNNING);
		runqueue_add_core(p);
	}
}

static int __init make_init_task()
{
	extern void idle(); /* becomes init task */

	/* stack must be allocated first. and to build root relationship
	 * properly `current` must be set to `init`. */
	current = &init;

	if (alloc_mm(&init, NULL, 0))
		return -ERR_ALLOC;

	set_task_dressed(&init, TASK_STATIC | TASK_KERNEL, idle);
	set_task_context_hard(&init, wrapper);
	set_task_state(&init, TASK_RUNNING);

	/* make it the sole */
	links_init(&init.children);
	links_init(&init.sibling);

	return 0;
}

#include <fs/fs.h>
#include <kernel/device.h>

static int __init console_init()
{
	extern int sys_open_core(char *filename, int mode, void *opt);

	stdin = stdout = stderr =
		sys_open_core(DEVFS_ROOT CONSOLE, O_RDWR, (void *)115200);

	return 0;
}

static void __init sys_init()
{
	extern char _init_func_list;
	unsigned int *p = (unsigned int *)&_init_func_list;

	while (*p)
		((void (*)())*p++)();
}

#include <kernel/page.h>
#include <kernel/softirq.h>
#include <kernel/timer.h>
#include <kernel/systick.h>
#include <asm/power.h>

int __init kernel_init()
{
	/* keep the calling order below because of dependencies */
	sys_init();
	mm_init();
	fs_init();
	driver_init();
	device_init();
	systick_init();
	scheduler_init();
	console_init();

	make_init_task();
	load_user_task(); /* that are registered statically */

	softirq_init();
#ifdef CONFIG_TIMER
	timer_init();
#endif

#ifndef DEFSTR
#define DEFMKSTR(x)	#x
#define DEFSTR(x)	DEFMKSTR(x)
#endif
	/* a banner */
	notice("\n\nyaos %s %s\n"
	       "Running at %dHz\n"
	       "Reset source : %x",
	       DEFSTR(VERSION), DEFSTR(MACHINE),
	       get_hclk(),
	       __read_reset_source());

	debug("&current 0x%08x\n"
	      "User stack size : %d, Heap size : %d",
	      &current,
	      USER_STACK_SIZE, HEAP_SIZE);

	/* switch from boot stack memory to new one */
	set_user_sp(init.mm.sp);
	set_kernel_sp(init.mm.kernel.sp);

	/* everything ready now */
#ifndef ARMv7A
	sei();
#endif
	resched();

	/* it doesn't really reach up to this point. init task becomes idle as
	 * its context is already set to idle */
	__context_restore(current);
	__ret_from_exc(0);
	freeze();

	return 0;
}
