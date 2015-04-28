#include <foundation.h>
#include <kernel/sched.h>

/* A register that is not yet saved in stack gets used by compiler optimization.
 * If I put all the registers that are not yet saved in clobber list,
 * it changes code ordering that ruins stack, showing weird behavior.
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

void __attribute__((naked)) sys_schedule()
{
	SCB_ICSR |= 1 << 28; /* raising pendsv for scheduling */
	__asm__ __volatile__("bx lr");
}

#include <syscall.h>

void __attribute__((naked)) svc_handler()
{
	/* What all those inline assembly do is
	 * `syscall_table[((char *)sp[6])[-2]]();` in C. */

	__asm__ __volatile__(
			//"ldr	r3, [sp, #24]		\n\t" /* get the `svc` parameter */
			//"ldrb	r3, [r3, #-2]		\n\t"
			//"cmp	r3, %0			\n\t" /* if nr >= SYSCALL_NR */
			//"it	ge			\n\t" /* then nr = 0 */
			//"movge	r3, #0			\n\t"
			//"ldr	r12, =syscall_table	\n\t" /* get the syscall address */
			//"ldr	r12, [r12, r3, lsl #2]	\n\t"
			"cmp	r0, %0			\n\t" /* if nr >= SYSCALL_NR */
			"it	ge			\n\t" /* then nr = 0 */
			"movge	r0, #0			\n\t"
			"ldr	r3, =syscall_table	\n\t" /* get the syscall address */
			"ldr	r3, [r3, r0, lsl #2]	\n\t"

			//"ldr	r3, [sp, #12]		\n\t" /* arguments in place */
			"ldr	r2, [sp, #12]		\n\t"
			"ldr	r1, [sp, #8]		\n\t"
			"ldr	r0, [sp, #4]		\n\t"
			"push	{lr}			\n\t"
			"blx	r3			\n\t"
			"pop	{lr}			\n\t"
			"str	r0, [sp]		\n\t" /* store return value */
			"bx	lr			\n\t"
			:: "I"(SYSCALL_NR));
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
