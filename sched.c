#include "foundation.h"
#include "io.h"
#include "sched.h"

/* runqueue's head can be init task. think about it */
LIST_HEAD(runqueue);

static struct task_t init = { .rq = {&runqueue, &runqueue} };
struct task_t *current = &init;

void __attribute__((naked)) __schedule()
{
	irq_save(current->primask);
	cli();

	context_save(current->stack);

	static struct list_t *rq = &runqueue; /* rq counter */

	rq = rq->next;
	if (rq == &runqueue) { /* turnaround */
		if ((rq = rq->next) == &runqueue) { /* no task to schedule */
			/* wake up the init_task */
			rq = &init.rq;
		}
	}

	current = get_container_of(rq, struct task_t, rq);

	context_restore(current->stack);

	irq_restore(current->primask);

	__asm__ __volatile__("bx lr");
}

DEFINE_SPINLOCK(rq_lock);

void runqueue_add(struct task_t *p)
{
	unsigned long flags;
	spinlock_irqsave(&rq_lock, &flags);

	/* newest is always head->next */
	list_add(&p->rq, &runqueue);

	spinlock_irqrestore(&rq_lock, &flags);
}

void runqueue_del(struct task_t *p)
{
	unsigned long flags;
	spinlock_irqsave(&rq_lock, &flags);

	list_del(&p->rq);

	spinlock_irqrestore(&rq_lock, &flags);

	/* release all related to the task */

	schedule();
}

void print_rq()
{
	struct list_t *rq = runqueue.next;
	struct task_t *p;

	int i;

	while (rq != &runqueue) {
		p = get_container_of(rq, struct task_t, rq);

		DBUG(("flags = %d, stack size = %d, addr = 0x%x\n", p->flags, p->stack_size, p->addr));

		for (i = 0; i < CONTEXT_NR; i++)
			DBUG(("%x : %x\n", p->stack + i, *(p->stack + i)));

		rq = rq->next;
	}
}
