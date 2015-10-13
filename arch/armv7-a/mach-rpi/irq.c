#include <foundation.h>
#include <kernel/task.h>
#include "irq.h"

static void (*irq_table[IRQ_MAX])();

int register_isr(unsigned int nirq, void (*func)())
{
	irq_table[nirq] = func;
	dsb();
	return 0;
}

static int need_resched = 0;

void resched()
{
	need_resched = 1;
}

void irq_handler_c()
{
	struct interrupt_controller *intcntl
		= (struct interrupt_controller *)INTCNTL_BASE;

	if (intcntl->basic_pending) {
		struct general_timer *gtimer
				= (struct general_timer *)GENTIMER_BASE;
		unsigned int i;
		for (i = 0; intcntl->basic_pending >> (i+1); i++) ;

		switch (i) {
		case IRQ_TIMER:
			gtimer->irq_clear = 1;
			break;
		case IRQ_PEND1:
			for (i = 0; intcntl->pending1 >> (i+1); i++) ;
			break;
		case IRQ_PEND2:
			for (i = 0; intcntl->pending2 >> (i+1); i++) ;
			i += 32;
			break;
		}

		irq_table[i]();
	}
}

void __attribute__((naked, optimize("O0"))) irq_handler()
{
	__asm__ __volatile__("sub lr, lr, #4");
	schedule_prepare();

	__asm__ __volatile__("bl irq_handler_c");

	/* schedule if sched_req is set */
	__asm__ __volatile__(
			"cmp	%0, #0			\n\t"
			"ldrne	r12, =sys_schedule	\n\t"
			"blxne	r12			\n\t"
			:: "r"(need_resched)
			: "r12", "memory");

	schedule_finish();
	__ret_from_exc(0);
}

void __attribute__((naked)) sys_schedule()
{
	need_resched = 0;
	__asm__ __volatile__("b schedule_core");

	/* it never comes back here */
	schedule_finish();
	__ret_from_exc(0);
}

#ifdef CONFIG_SYSCALL
#include <kernel/syscall.h>

#ifdef CONFIG_DEBUG
unsigned int syscall_count = 0;
#endif

void __attribute__((naked, optimize("O0"))) svc_handler()
{
	schedule_prepare();

	/* branch to interrupt service routine */
	__asm__ __volatile__(
			"mov	r4, %0			\n\t"
			:: "r"(current->mm.sp) : "memory");
	__asm__ __volatile__(
			"ldr	r0, [r4, #4]		\n\t"
			/* if nr >= SYSCALL_NR */
			"cmp	r0, %0			\n\t"
			/* then nr = 0 */
			"movge	r0, #0			\n\t"
			/* get the syscall address */
			"ldr	r12, =syscall_table	\n\t"
			"ldr	r12, [r12, r0, lsl #2]	\n\t"
			/* arguments in place */
			"ldr	r0, [r4, #12]		\n\t"
			"ldr	r1, [r4, #16]		\n\t"
			"ldr	r2, [r4, #20]		\n\t"
			"blx	r12			\n\t"
			/* r0 now holds the return value */
			"str	r0, [r4, #4]		\n\t"
			:: "I"(SYSCALL_NR) : "memory");

	/* schedule if sched_req is set */
	__asm__ __volatile__(
			"cmp	%0, #0			\n\t"
			"ldrne	r12, =sys_schedule	\n\t"
			"blxne	r12			\n\t"
			:: "r"(need_resched)
			: "r12", "memory");

	schedule_finish();
	__ret_from_exc(0);
}
#endif /* CONFIG_SYSCALL */

#include <kernel/init.h>

void isr_null() { debug("ISR is not registered yet"); }

void __init irq_init()
{
	unsigned int i;
	for (i = 0; i < IRQ_MAX; i++)
		irq_table[i] = isr_null;
	dsb();
}
REGISTER_INIT(irq_init, 1);
