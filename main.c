#include "foundation.h"
#include "io.h"
#include "sched.h"

static void __attribute__((naked)) isr_systick()
{
	SYSTICK_FLAG(); /* clear flag */
	__asm__ __volatile__("b __schedule");
}

static void systick_init()
{
	ISR_REGISTER(15, isr_systick);

	RESET_SYSTICK();
	SET_SYSTICK(get_stkclk(get_hclk(get_sysclk())) / HZ - 1);
	SYSTICK(ON | SYSTICK_INT);
}

#include "stdlib.h"

static unsigned *alloc_user_stack(struct task_t *p)
{
	if ( (p->stack_size <= 0) || !(p->stack = (unsigned *)malloc(p->stack_size)) )
		return NULL;

	/* make it to point out the highest memory address */
	p->stack += (p->stack_size >> 2) - 1;
	return p->stack;
}

static void load_user_task()
{
	extern int _user_task_list;
	struct task_t *p = (struct task_t *)&_user_task_list;
	int i;

	while (p->flags) {
		/* sanity check */
		if (!p->addr) continue;
		if (!alloc_user_stack(p)) continue;

		/* initialize task register set */
		*(p->stack--) = 0x01000000;		/* psr */
		*(p->stack--) = (unsigned)p->addr;	/* pc */
		for (i = 2; i < (CONTEXT_NR-1); i++) {	/* lr */
			*p->stack = 0;			/* . */
			p->stack--;			/* . */
		}					/* . */
		*p->stack = (unsigned)EXC_RETURN_MSPT;	/* lr */

		/* initial state for all tasks are runnable, add into runqueue */
		runqueue_add(p);
		p++;
	}
}

/* when no task in runqueue, this init_task gets the cpu.
 * do some power saving things */
static void init_task()
{
	/* ensure that systick is not activated 
	 * prior to task and scheduler ready to run. */
	systick_init();

#ifdef DEBUG
	SET_PORT_CLOCK(ENABLE, PORTD);
	SET_PORT_PIN(PORTD, 2, PIN_OUTPUT_50MHZ);
	kprintf("psr : %x sp : %x int : %x control : %x lr : %x\n", GET_PSR(), GET_SP(), GET_INT(), GET_CON(), GET_LR());
	kprintf("PC = %x\n", GET_PC());
	while (1) {
		printf("init()\n");
		mdelay(500);
	}
#endif
}

#include "driver/usart.h"

int main()
{
	usart_open(USART1, (struct usart_t) {
			.brr  = brr2reg(115200, get_sysclk()),
			.gtpr = 0,
			.cr3  = 0,
			.cr2  = 0,
			.cr1  = (1 << 13) /* UE    : USART enable */
			| (1 << 5)        /* RXNEIE: RXNE interrupt enable */
			| (1 << 3)        /* TE    : Transmitter enable */
			| (1 << 2)});     /* RE    : Receiver enable */

	load_user_task();
	init_task();

	return 0;
}
