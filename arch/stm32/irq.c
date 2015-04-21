#include <foundation.h>
#include <kernel/sched.h>

/* A register that is not yet saved in stack gets used by compiler optimization.
 * If I put all the registers that are not yet saved in clobber list,
 * it changes code ordering that ruins stack showing weird behavior.
 * Make it in an assembly file or do some study. */
void __attribute__((naked, used, optimize("O0"))) pendsv_handler()
{
	/* schedule_prepare() saves the current context and
	 * guarantees not to be preempted while schedule_finish()
	 * does the opposite. */
	schedule_prepare();
	schedule_core();
	schedule_finish();
	__asm__ __volatile__("bx lr");
}

static void __attribute__((naked, used)) svc_handler(unsigned *sp)
{
	/* we have return address(pc) stacked in sp[6].
	 * svc parameter can be gotten by accessing the prior code of return
	 * address, pc[-2] while pc[-1] of 'df'(code of svc mnemonic).
	 *
	 * e.g.
	 *  [-2][-1]
	 *   `df 40      	svc	64`
	 *   `f3 ef 83 03 	mrs	r3, PSR` # code of return address
	 *    [0][1][2][3]
	 */

	switch ( ((char *)sp[6])[-2] ) {
	case 0:
		SCB_ICSR |= 1 << 28; /* raising pendsv for scheduling */
		break;
	default:
		__asm__ __volatile__("push {lr}" ::: "memory");
		DEBUG(("no handler!\n"));
		__asm__ __volatile__("pop {lr}" ::: "memory");
		break;
	}

	__asm__ __volatile__("bx lr");
}

void __attribute__((naked)) __svc_handler()
{
	__asm__ __volatile__(
		"mov	r0, sp		\n\t"
		"b	svc_handler	\n\t"
		/* never reach out up to the code below */
		"bx	lr		\n\t"
	);
}

void __attribute__((naked)) isr_default()
{
	unsigned sp, lr, psr;

	sp  = GET_SP ();
	psr = GET_PSR();
	lr  = GET_LR ();

	/* EXC_RETURN:
	 * 0xFFFFFFF1 - MSP, return to handler mode
	 * 0xFFFFFFF9 - MSP, return to thread mode
	 * 0xFFFFFFFD - PSP, return to thread mode */
	kprintf("\nSP             0x%08x\n"
		"Stacked PSR    0x%08x\n"
		"Stacked PC     0x%08x\n"
		"Stacked LR     0x%08x\n"
		"Current LR     0x%08x\n"
		"Current PSR    0x%08x(vector number:%d)\n", sp,
		*(unsigned *)(sp + 28),
		*(unsigned *)(sp + 24),
		*(unsigned *)(sp + 20),
		lr, psr, psr & 0x1ff);

	kprintf("\ncurrent->sp         0x%08x\n"
		"current->stack      0x%08x\n"
		"current->stack_size 0x%08x\n"
		"current->state      0x%08x\n"
		"current->primask    0x%08x\n"
		, current->sp, current->stack, current->stack_size
		, current->state, current->primask);

	/* led for debugging */
	SET_PORT_CLOCK(ENABLE, PORTD);
	SET_PORT_PIN(PORTD, 2, PIN_OUTPUT_50MHZ);
	while (1) {
		PUT_PORT(PORTD, GET_PORT(PORTD) ^ 4);
		mdelay(100);
	}
}
