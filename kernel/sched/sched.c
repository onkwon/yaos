#include <kernel/sched.h>
#include <kernel/task.h>
#include <kernel/systick.h>
#include <kernel/softirq.h>
#include <error.h>
#include "fair.h"
#ifdef CONFIG_REALTIME
#include "rt.h"
#endif

static struct scheduler cfs;
#ifdef CONFIG_REALTIME
static struct scheduler rts;
#endif

/* Calling update_curr() as soon as the system timer interrupt occurs would be
 * the best chance other than in elsewhere not to count scheduling overhead but
 * to count only its running time, as long as ticks gets updated
 * asynchronously. */
static inline void update_curr()
{
	uint64_t clock = get_systick64();
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

static inline void runqueue_add_core(struct task *new)
{
	if (is_task_realtime(new)) {
#ifdef CONFIG_REALTIME
		rts_rq_add(&rts, new);
#endif
	} else
		cfs_rq_add(&cfs, new);
}

/* As each processor has its own scheduler and it runs in an interrupt context,
 * where interrupts disabled, we are rid of concern about synchronization. */

#include <kernel/timer.h>

static inline void run_softirq()
{
	if (softirq.pending) {
		if (get_task_state(softirqd)) {
			set_task_state(softirqd, TASK_RUNNING);
			if (current != softirqd)
				runqueue_add_core(softirqd);
		}
	}

#ifdef CONFIG_TIMER
	extern struct timer_queue timerq;
	extern struct task *timerd;

	if (timerq.nr && time_after(timerq.next, systick)) {
		if (get_task_state(timerd)) {
			set_task_state(timerd, TASK_RUNNING);
			if (current != timerd)
				runqueue_add_core(timerd);
		}
	}
#endif
}

void schedule_core()
{
#ifdef CONFIG_DEBUG
	/* stack overflow */
	if ((current->mm.base[HEAP_SIZE / WORD_SIZE] != STACK_SENTINEL) ||
			(current->mm.kernel.base[0] != STACK_SENTINEL))
	{
		debug("stack overflow %x(%x)", current, current->addr);
		unsigned int i;
		for (i = 0; i < NR_CONTEXT; i++)
			debug("%08x", current->mm.sp[i]);
		return;
	}
#endif
	update_curr();
	run_softirq();

	struct task *next;

#ifdef CONFIG_REALTIME
	if (rts.nr_running) {
		/* The realtime runqueue always holds the most priority in
		 * `pri` variable. when no task in run queue it goes down to
		 * `RT_PRIORITY + 1`, the normal priority level. */
		if (rts.pri <= get_task_pri(current)) {
rts_next:
			if ((next = rts_pick_next(&rts))) {
				rts_rq_del(&rts, next);
				runqueue_add_core(current);
				current = next;

				/* count 1 for `current` */
				if (!rts.nr_running)
					rts.nr_running = 1;
			}
		}

		/* If not runnable when nr_running is 1,
		 * there is no real task to run */
		if (!get_task_state(current))
			goto adjust_vruntime;
		else if (rts.nr_running > 1)
			goto rts_next;

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
	current->se.exec_start = get_systick64();
}

void runqueue_add(struct task *new)
{
	unsigned int irqflag;
	irq_save(irqflag);
	local_irq_disable();
	runqueue_add_core(new);
	irq_restore(irqflag);
}

void runqueue_del(struct task *task)
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

void sum_curr_stat(struct task *to)
{
	/* make sure task `to` is still alive not to access stale address */

	unsigned int irqflag;
	irq_save(irqflag);
	local_irq_disable();

	update_curr();
	to->se.sum_exec_runtime += current->se.sum_exec_runtime;

	irq_restore(irqflag);
}

#ifdef CONFIG_DEBUG
int sched_overhead;
#endif

void __attribute__((naked, used, optimize("O0"))) __schedule()
{
#ifdef CONFIG_DEBUG
	/* make sure that registers used here must be the ones saved already */
	sched_overhead = get_sysclk();
#endif
	/* schedule_prepare() saves the current context and
	 * guarantees not to be preempted while schedule_finish()
	 * does the opposite. */
	schedule_prepare();
	schedule_core();
	schedule_finish();
#ifdef CONFIG_DEBUG
	sched_overhead -= get_sysclk();
#endif
	__ret();
}

#include <kernel/init.h>

void __init scheduler_init()
{
	cfs_init(&cfs);
#ifdef CONFIG_REALTIME
	rts_init(&rts);
#endif

	run_scheduler();
}

void sys_yield()
{
	set_task_state(current, TASK_SLEEPING);
	resched();
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
