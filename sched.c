#include "foundation.h"
#include "io.h"
#include "sched.h"

LIST_HEAD(runqueue);

static struct task_t init = { .rq = {&runqueue, &runqueue} };
struct task_t *current = &init;

void __attribute__((naked)) __schedule()
{
	/* interrupt disable. schedule() must not be reentrant */
	cli();
	context_save(current->stack);

#ifdef DEBUG_CONTEXT
	int i;
	for (i = 0; i < CONTEXT_NR+8; i++) kprintf("%02d: %x = %x\n", i+1, (current->stack + i), *(current->stack + i));
	kprintf("prev %x ", current->addr);
#endif

	static struct list_t *rq = &runqueue; /* rq counter */

	rq = rq->next;
	if (rq == &runqueue) { /* turnaround */
		if ((rq = rq->next) == &runqueue) { /* no task to schedule */
			/* wake up the init_task */
			rq = &init.rq;
		}
	}

	current = get_container_of(rq, struct task_t, rq);

#ifdef DEBUG_CONTEXT
	kprintf("next %x\n", current->addr);
	for (i = 0; i < CONTEXT_NR+8; i++) kprintf("%02d: %x = %x\n", i+1, (current->stack + i), *(current->stack + i));
#endif

	context_restore(current->stack);

	/* interrut enable; preempt enable */
	sei();
	__asm__ __volatile__("bx lr");
}

void runqueue_add(struct task_t *p)
{
	/* newest is always head->next */
	list_add(&p->rq, &runqueue);
}

void runqueue_del(struct task_t *p)
{
	list_del(&p->rq);

	/* release all related to the task */
	schedule();
}

void print_rq()
{
	struct list_t *rq = runqueue.next;
	struct task_t *p;

#ifdef DEBUG_RQ
	int i;
#endif

	while (rq != &runqueue) {
		p = get_container_of(rq, struct task_t, rq);

		kprintf("flags = %d, stack size = %d, addr = 0x%x\n", p->flags, p->stack_size, p->addr);
#ifdef DEBUG_RQ
		for (i = 0; i < CONTEXT_NR; i++)
			kprintf("%x : %x\n", p->stack + i, *(p->stack + i));
#endif

		rq = rq->next;
	}
}
