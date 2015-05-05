#include <foundation.h>
#include <stdlib.h>
#include <kernel/sched.h>

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
				IS_TASK_REALTIME(p)? "REALTIME" : "NORMAL  ",
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
	init.state      = LEAST_PRIORITY;
	init.addr       = init_task;
	init.se         = (struct sched_entity){ 0, 0, 0 };
	LIST_LINK_INIT(&init.rq);
	alloc_user_stack(&init);
	SET_SP(init.sp);

	cleanup();

	printf("ibox %s %s\n", VERSION, MACHINE);

	current = &init;
	/* ensure that scheduler gets activated after everything ready. */
	schedule_on();

	while (1) {
		printf("init()\n");
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

#include <kernel/device.h>

static void devman_init()
{
	__devman_init();

	extern char _device_list, _device_list_end;
	struct device_t *dev = (struct device_t *)&_device_list;

	while ((unsigned long)dev < (unsigned long)&_device_list_end) {
		link_device(dev->id, dev);
		dev++;
	}
}

int stdin, stdout, stderr;
static void console_init()
{
	open(USART, 115200);

	stdin = stdout = stderr = USART;
}

#include <kernel/mm.h>

int main()
{
	sys_init();
	mm_init();
	devman_init();
	sei();

	console_init();
	systick_init();
	scheduler_init();

	load_user_task();
	init_task();

	return 0;
}
