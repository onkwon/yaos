#include <foundation.h>
#include <stdlib.h>
#include <kernel/sched.h>
#include <syscall.h>

static unsigned long *alloc_user_stack(struct task_t *p)
{
	if ( (p->stack_size <= 0) ||
			!(p->stack = (unsigned long *)kmalloc(p->stack_size)) )
		return NULL;

	/* make its stack pointer to point out the highest memory address */
	p->sp = p->stack + ((p->stack_size / sizeof(long)) - 1);

	return p->sp;
}

static void load_user_task()
{
	extern int _user_task_list;
	struct task_t *p = (struct task_t *)&_user_task_list;

	while (p->state) {
		/* sanity check */
		if (!p->addr) continue;
		if (!alloc_user_stack(p)) continue;

		init_task_context(p);
		set_task_state(p, TASK_RUNNING);

		/* initial state of all tasks are runnable, add into runqueue */
		runqueue_add(p);

		DEBUG(("%s: state %08x, sp %08x, addr %08x",
				is_task_realtime(p)? "REALTIME" : "NORMAL  ",
				p->state, p->sp, p->addr));

		p++;
	}
}

static void cleanup()
{
	/* Clean up redundant code and data, used during initializing */
	free_bootmem();
}

struct task_t init;

/* when no task in runqueue, this init_task takes place.
 * do some power saving */
static void init_task()
{
	/* set init_task() into struct task_t init */
	init.stack_size = DEFAULT_STACK_SIZE;
	init.addr       = init_task;
	init.se         = (struct sched_entity){ 0, 0, 0 };
	LIST_LINK_INIT(&init.rq);
	alloc_user_stack(&init);
	/* init task shares its stack with kernel. */
	SET_KSP(init.sp);
	SET_USP(init.sp);
	set_task_priority(&init, LEAST_PRIORITY);
	set_task_state(&init, TASK_KERNEL);

	cleanup();

	printf("ibox %s %s\n", VERSION, MACHINE);

	current = &init;
	/* ensure that scheduler gets activated after everything ready. */
	schedule_on();

	while (1) {
		printf("init()\n");
		printf("control %08x, sp %08x, msp %08x, psp %08x\n",
				GET_CON(), GET_SP(), GET_KSP(), GET_USP());
		msleep(500);
	}
}

static void sys_init()
{
	extern char _init_func_list;
	unsigned long *p = (unsigned long *)&_init_func_list;

	while (*p)
		((void (*)())*p++)();
}

int stdin, stdout, stderr;

static void console_init()
{
	open(USART, O_RDWR | O_NONBLOCK);

	stdin = stdout = stderr = USART;
}

#include <kernel/page.h>

int main()
{
	sys_init();
	mm_init();
#ifdef CONFIG_DEVMAN
	devman_init();
#endif
	sei();

	console_init();
	systick_init();
	scheduler_init();

	load_user_task();
	init_task();

	return 0;
}
