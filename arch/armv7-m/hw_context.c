#include "include/hw_context.h"
#include "kernel/sched.h"
#include "kernel/interrupt.h"
#include "io.h"
#include "kernel/task.h"

void ISR_svc_pend(void)
{
	dsb();

	/* context save */
#if defined(CONFIG_FPU)
	__asm__ __volatile__(
			"mrs	r12, psp		\n\t"
			"tst	lr, #0x10		\n\t"
			"it	eq			\n\t"
			"vstmdbeq	r12!, {s16-s31}	\n\t"
			"stmdb	r12!, {r4-r11, lr}	\n\t"
			::: "r4", "r5", "r6", "r7", "r8",
			"r9", "r10", "r11", "r12", "memory");
#else /* !defined(CONFIG_FPU) */
	__asm__ __volatile__(
			"mrs	r12, psp		\n\t"
			"stmdb	r12!, {r4-r11, lr}	\n\t"
			::: "r4", "r5", "r6", "r7", "r8",
			"r9", "r10", "r11", "r12", "memory");
#endif
	__asm__ __volatile__(
			"mov	%0, r12			\n\t"
			: "=&r"(current->stack.p)
			:: "r12", "memory");

	schedule();

	/* context restore */
	__asm__ __volatile__(
			"msr	msp, %0			\n\t"
			"mov	r12, #3			\n\t"
			"tst	%1, %2			\n\t"
			"it	ne			\n\t"
			"movne	r12, #2			\n\t"
			:: "r"(current->kstack.p)
			, "r"(get_task_flags(current))
			, "I"(TF_PRIVILEGED)
			: "r12", "memory");
#if defined(CONFIG_FPU)
	__asm__ __volatile__(
			"msr	control, r12		\n\t"
			"ldmia	%0!, {r4-r11, r12}	\n\t"
			"ldr	lr, =0xffffffed		\n\t"
			"orr	lr, r12			\n\t"
			"tst	r12, #0x10		\n\t"
			"it	eq			\n\t"
			"vldmiaeq	%0!, {s16-s31}	\n\t"
			"msr	psp, %0			\n\t"
			:: "r"(current->stack.p)
			: "r4", "r5", "r6", "r7", "r8", "r9",
			"r10", "r11", "r12", "lr", "memory");
#else /* !defined(CONFIG_FPU) */
	__asm__ __volatile__(
			"msr	control, r12		\n\t"
			"ldmia	%0!, {r4-r11, lr}	\n\t"
			"msr	psp, %0			\n\t"
			"ldr	lr, =0xfffffffd		\n\t"
			:: "r"(current->stack.p)
			: "r4", "r5", "r6", "r7", "r8", "r9",
			"r10", "r11", "r12", "lr", "memory");
#endif

	dsb();
	isb();

	__ret();
}

void set_task_context_hard(struct task *p, void *addr)
{
	uintptr_t *sp = p->stack.p;

	*(--sp) = DEFAULT_PSR;		/* psr */
	*(--sp) = (uintptr_t)addr;	/* pc */
	for (int i = 2; i < NR_CONTEXT_HARD; i++)
		*(--sp) = 0;

	p->stack.p = sp;
}

#if defined(CONFIG_FPU)
void set_task_context_soft(struct task *p)
{
	uintptr_t *sp = p->stack.p;

	*(--sp) = EXC_RETURN_PSPT;
	for (int i = 1; i < NR_CONTEXT_SOFT; i++)
		*(--sp) = 0;

	p->stack.p = sp;
}
#else
void set_task_context_soft(struct task *p)
{
	uintptr_t *sp = p->stack.p;

	for (int i = 0; i < NR_CONTEXT_SOFT; i++)
		*(--sp) = 0;

	p->stack.p = sp;
}
#endif

void set_task_context(struct task *p, void *addr)
{
	set_task_context_hard(p, addr);
	set_task_context_soft(p);
}
