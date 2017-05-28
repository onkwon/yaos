#include <kernel/task.h>
#include <context.h>

void set_task_context_hard(struct task *p, void *addr)
{
	unsigned int i;

	*(--p->mm.sp) = DEFAULT_PSR;		/* psr */
	*(--p->mm.sp) = (unsigned int)addr;	/* pc */
	for (i = 2; i < NR_CONTEXT_HARD; i++)
		*(--p->mm.sp) = 0;
}

void set_task_context_soft(struct task *p)
{
	unsigned int i;

	*(--p->mm.sp) = EXC_RETURN_PSPT;
	for (i = 1; i < NR_CONTEXT_SOFT; i++)
		*(--p->mm.sp) = 0;
}

void set_task_context(struct task *p, void *addr)
{
	set_task_context_hard(p, addr);
	set_task_context_soft(p);
}
