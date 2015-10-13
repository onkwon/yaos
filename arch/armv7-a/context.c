#include <kernel/task.h>
#include <context.h>

void set_task_context_hard(struct task *p, void *addr)
{
	unsigned int i, sp;

	sp = (unsigned int)p->mm.sp;

	*(--p->mm.sp) = (unsigned int)addr;		/* pc */
	for (i = 4; i < NR_CONTEXT_HARD; i++)		/* r1 ~ r14 */
		*(--p->mm.sp) = 0;
	*(--p->mm.sp) = sp;				/* sp */
	*(--p->mm.sp) = 0;				/* r0 */
	if (get_task_flags(p) & TASK_PRIVILEGED)
		*(--p->mm.sp) = INIT_PSR_SYS;		/* psr */
	else
		*(--p->mm.sp) = INIT_PSR;
}

void set_task_context_soft(struct task *p)
{
}

void set_task_context(struct task *p, void *addr)
{
	set_task_context_hard(p, addr);
	set_task_context_soft(p);
}
