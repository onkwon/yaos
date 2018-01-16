#include "include/context.h"
#include <kernel/task.h>

void set_task_context_hard(struct task *p, void *addr)
{
	unsigned int i;

	*(--p->mm.sp) = DEFAULT_PSR;		/* psr */
	*(--p->mm.sp) = (unsigned int)addr;	/* pc */
	for (i = 2; i < NR_CONTEXT_HARD; i++)
		*(--p->mm.sp) = 0;
}

#ifdef CONFIG_FPU
void set_task_context_soft(struct task *p)
{
	unsigned int i;

	*(--p->mm.sp) = EXC_RETURN_PSPT;
	for (i = 1; i < NR_CONTEXT_SOFT; i++)
		*(--p->mm.sp) = 0;
}

void __attribute__((naked, used)) ISR_schedule()
{
	dsb();

	/* context save */
	__asm__ __volatile__(
			"mrs	r12, psp		\n\t"
			"tst	lr, #0x10		\n\t"
			"it	eq			\n\t"
			"vstmdbeq	r12!, {s16-s31}	\n\t"
			"stmdb	r12!, {r4-r11, lr}	\n\t"
			::: "r4", "r5", "r6", "r7", "r8",
			"r9", "r10", "r11", "r12", "memory");
	__asm__ __volatile__(
			"mov	%0, r12			\n\t"
			: "=&r"(current->mm.sp)
			:: "r12", "memory");

	schedule_core();

	/* context restore */
	__asm__ __volatile__(
			"msr	msp, %0			\n\t"
			"mov	r12, #3			\n\t"
			"tst	%1, %2			\n\t"
			"it	ne			\n\t"
			"movne	r12, #2			\n\t"
			:: "r"(current->mm.kernel.sp)
			, "r"(get_task_flags(current))
			, "I"(TF_PRIVILEGED)
			: "r12", "memory");
	__asm__ __volatile__(
			"msr	control, r12		\n\t"
			"ldmia	%0!, {r4-r11, r12}	\n\t"
			"ldr	lr, =0xffffffed		\n\t"
			"orr	lr, r12			\n\t"
			"tst	r12, #0x10		\n\t"
			"it	eq			\n\t"
			"vldmiaeq	%0!, {s16-s31}	\n\t"
			"msr	psp, %0			\n\t"
			:: "r"(current->mm.sp)
			: "r4", "r5", "r6", "r7", "r8", "r9",
			"r10", "r11", "r12", "lr", "memory");

	dsb();
	isb();

	__ret();
}
#else
void set_task_context_soft(struct task *p)
{
	unsigned int i;

	for (i = 0; i < NR_CONTEXT_SOFT; i++)
		*(--p->mm.sp) = 0;
}
#endif

void set_task_context(struct task *p, void *addr)
{
	set_task_context_hard(p, addr);
	set_task_context_soft(p);
}
