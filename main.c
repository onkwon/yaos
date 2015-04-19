#include <foundation.h>
#include <stdlib.h>
#include <kernel/sched.h>

static unsigned *alloc_user_stack(struct task_t *p)
{
	if ( (p->stack_size <= 0) || !(p->stack = (unsigned *)malloc(p->stack_size)) )
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
	int i;

	while (p->state) {
		/* sanity check */
		if (!p->addr) continue;
		if (!alloc_user_stack(p)) continue;

		/* initialize task register set */
		*(p->sp--) = 0x01000000;		/* psr */
		*(p->sp--) = (unsigned)p->addr;		/* pc */
		for (i = 2; i < (CONTEXT_NR-1); i++) {	/* lr */
			*p->sp = 0;			/* . */
			p->sp--;			/* . */
		}					/* . */
		*p->sp = (unsigned)EXC_RETURN_MSPT;	/* lr */

		set_task_state(p, TASK_RUNNING);

		/* initial state of all tasks are runnable, add into runqueue */
		runqueue_add(p);

		p++;
	}
}

static void cleanup()
{
	/* Clean up redundant, one time code and date that only used during initializing */
}

struct task_t init;

/* when no task in runqueue, this init_task takes place.
 * do some power saving things */
static void init_task()
{
	cleanup();

	/* set init_task() into struct task_t init */
	init.state = LEAST_PRIORITY;
	init.sp    = (unsigned *)GET_SP();
	init.addr  = init_task;
	init.se    = (struct sched_entity){ 0, 0, 0 };
	LIST_LINK_INIT(&init.rq);

	current    = &init;

	/* ensure that scheduler is not activated prior
	 * until all things to be ready. */
	schedule_on();

	DBUG(("psr : %x sp : %x int : %x control : %x lr : %x\n", GET_PSR(), GET_SP(), GET_INT(), GET_CON(), GET_LR()));
	DBUG(("PC = %x\n", GET_PC()));
	while (1) {
		printf("init()\n");
		mdelay(500);
	}
}

void sys_init()
{
	extern int _init_func_list;
	unsigned *p = (unsigned *)&_init_func_list;

	while (*p) ((void (*)())*p++)();
}

#include <driver/usart.h>

int main()
{
	sys_init();

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
