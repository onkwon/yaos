#include <foundation.h>
#include <stdlib.h>
#include <kernel/sched.h>

static unsigned *alloc_user_stack(struct task_t *p)
{
	if ( (p->stack_size <= 0) || !(p->stack = (unsigned *)kmalloc(p->stack_size)) )
		return NULL;

	/* make its stack pointer to point out the highest memory address */
	/* (p->stack_size >> 2) == (p->stack_size / sizeof(long)) */
	p->sp = p->stack + ((p->stack_size >> 2) - 1);

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
}

struct task_t init;

/* when no task in runqueue, this init_task takes place.
 * do some power saving */
static void init_task()
{
	cleanup();

	printk("ibox %s %s\n", VERSION, MACHINE);

	/* set init_task() into struct task_t init */
	init.stack_size = DEFAULT_STACK_SIZE;
	init.state      = LEAST_PRIORITY;
	init.addr       = init_task;
	init.se         = (struct sched_entity){ 0, 0, 0 };
	LIST_LINK_INIT(&init.rq);
	alloc_user_stack(&init);
	SET_SP(init.sp);

	current = &init;

	/* ensure that scheduler is not activated prior
	 * until everything to be ready. */
	schedule_on();

	while (1) {
		printf("init()\n");
		mdelay(500);
	}
}

void sys_init()
{
	extern int _init_func_list;
	unsigned *p = (unsigned *)&_init_func_list;

	while (*p)
		((void (*)())*p++)();
}

#include <driver/usart.h>
#include <kernel/mm.h>

int main()
{
	sys_init();
	mm_init();

	usart_open(USART1, (struct usart_t) {
			.brr  = brr2reg(115200, get_sysclk()),
			.gtpr = 0,
			.cr3  = 0,
			.cr2  = 0,
			.cr1  = (1 << 13) /* UE    : USART enable */
			| (1 << 5)        /* RXNEIE: RXNE interrupt enable */
			| (1 << 3)        /* TE    : Transmitter enable */
			| (1 << 2)});     /* RE    : Receiver enable */

	systick_init();
	scheduler_init();

	load_user_task();
	init_task();

	return 0;
}
