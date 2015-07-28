#include <kernel/sched.h>
#include <kernel/task.h>
#include <kernel/jiffies.h>
#include <kernel/softirq.h>
#include "fair.h"
#ifdef CONFIG_REALTIME
#include "rt.h"
#endif
#ifdef CONFIG_DEBUG
#include <foundation.h>
#endif

static struct scheduler cfs;
#ifdef CONFIG_REALTIME
static struct scheduler rts;
#endif

static inline void runqueue_add_core(struct task *new)
{
#ifdef CONFIG_DEBUG
	/* stack overflow */
	if ((new->mm.base[HEAP_SIZE / WORD_SIZE] != STACK_SENTINEL) ||
			(new->mm.kernel.base[0] != STACK_SENTINEL))
	{
		debug(("stack overflow\n"));
		return;
	}
#endif
	if (is_task_realtime(new)) {
#ifdef CONFIG_REALTIME
		rts_rq_add(&rts, new);
#endif
	} else
		cfs_rq_add(&cfs, new);
}

/* As each processor has its own scheduler and it runs in an interrupt context,
 * we are rid of concern about synchronization. */

void schedule_core()
{
	struct task *next;

	if (softirq.pending) {
		if (get_task_state(softirqd) && (current != softirqd)) {
			set_task_pri(softirqd, get_task_pri(current));
			set_task_state(softirqd, TASK_RUNNING);
			runqueue_add_core(softirqd);
		}
	}

#ifdef CONFIG_REALTIME
	if (rts.nr_running) {
		/* rq_add() and rq_del() of real time scheduler must keep
		 * the highest priority amongst tasks in `pri` variable.
		 * `pri` has the least priority when no task in runqueue. */
		if ( !is_task_realtime(current) ||
				(rts.pri <= get_task_pri(current)) ) {
rts_next:
			if ((next = rts_pick_next(&rts)) == NULL)
				goto rts_out;

			runqueue_add_core(current);
			current = next;
			rts_rq_del(&rts, next);

			/* count 1 for `current` */
			if (!rts.nr_running)
				rts.nr_running = 1;
		}

		/* If not runnable when nr_running is 1,
		 * there is no real task to run */
		if (!get_task_state(current))
			goto adjust_vruntime;
		else if (rts.nr_running > 1)
			goto rts_next;

rts_out:
		/* Now it's time for CFS */
		rts.nr_running = 0;
	}
#endif

	if (!(next = cfs_pick_next(&cfs))) {
		if (get_task_state(current)) /* no task to schedule */
			current = &init; /* turn to init task */

		goto adjust_vruntime;
	}

	/* add `current` back to runqueue after picking the next */
	/* and remove the next task from runqueue */
	cfs_rq_add(&cfs, current);
	cfs_rq_del(&cfs, next);

	current = next;

adjust_vruntime:
	/* Update newly selected task's start time because it is stale
	 * as much as the one has been waiting for. */
	current->se.exec_start = get_jiffies_64();
}

/* Calling update_curr() as soon as the system timer interrupt occurs would be
 * the best chance other than elsewhere not to count scheduling overhead but to
 * count only its running time, as long as jiffies gets updated asynchronous. */
void inline update_curr()
{
	uint64_t clock = get_jiffies_64();
	unsigned delta_exec;

	delta_exec = clock - current->se.exec_start;
	current->se.vruntime += delta_exec;
	current->se.sum_exec_runtime += delta_exec;
	current->se.exec_start = clock;

	if (is_task_realtime(current))
		return;

	struct task *task;

	cfs.vruntime_base = current->se.vruntime;

	/* pick the least vruntime in runqueue for vruntime_base
	 * to keep order properly. */
	if (((struct list *)cfs.rq)->next != cfs.rq) { /* if it's not empty */
		task = get_container_of( ((struct list *)cfs.rq)->next,
				struct task, rq );
		if (cfs.vruntime_base > task->se.vruntime)
			cfs.vruntime_base = task->se.vruntime;
	}
}

void inline runqueue_add(struct task *new)
{
	unsigned int irqflag;
	irq_save(irqflag);
	local_irq_disable();
	runqueue_add_core(new);
	irq_restore(irqflag);
}

void inline runqueue_del(struct task *task)
{
	unsigned int irqflag;
	irq_save(irqflag);
	local_irq_disable();

	if (is_task_realtime(task)) {
#ifdef CONFIG_REALTIME
		rts_rq_del(&rts, task);
#endif
	} else
		cfs_rq_del(&cfs, task);

	irq_restore(irqflag);
}

#include <kernel/init.h>

void __init scheduler_init()
{
	cfs_init(&cfs);
#ifdef CONFIG_REALTIME
	rts_init(&rts);
#endif

	schedule_on();
}

void sys_yield()
{
	set_task_state(current, TASK_SLEEPING);
	sys_schedule();
}

#ifdef CONFIG_DEBUG
#include <foundation.h>

void print_rq()
{
	struct list *rq = ((struct list *)cfs.rq)->next;
	struct task *p;

//	int i;

	while (rq != cfs.rq) {
		p = get_container_of(rq, struct task, rq);

		printf("[%08x] state = %x, type = %x, pri = %x, vruntime = %d "
				"exec_runtime = %d (%d sec)\n",
				p->addr, p->state, p->flags, p->pri,
				(unsigned)p->se.vruntime,
				(unsigned)p->se.sum_exec_runtime,
				(unsigned)p->se.sum_exec_runtime / HZ);

//		for (i = 0; i < NR_CONTEXT; i++)
//			printf(("%x : %x", p->sp + i, *(p->sp + i));

		rq = rq->next;
	}
}
#endif
