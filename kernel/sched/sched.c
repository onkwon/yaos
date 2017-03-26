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
	unsigned int delta_exec;

	/* if delta_exec zero, runtime is finer than `sysfreq` granularity */
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
	if (((struct links *)cfs.rq)->next != cfs.rq) { /* if it's not empty */
		task = get_container_of( ((struct links *)cfs.rq)->next,
				struct task, rq );
		if (cfs.vruntime_base > task->se.vruntime)
			cfs.vruntime_base = task->se.vruntime;
	}
}

void runqueue_add_core(struct task *new)
{
	if (is_task_realtime(new)) {
#ifdef CONFIG_REALTIME
		rts_rq_add(&rts, new);
#endif
	} else
		cfs_rq_add(&cfs, new);
}

void runqueue_del_core(struct task *task)
{
	if (is_task_realtime(task)) {
#ifdef CONFIG_REALTIME
		rts_rq_del(&rts, task);
#endif
	} else
		cfs_rq_del(&cfs, task);
}

/* As each processor has its own scheduler and it runs in an interrupt context,
 * where interrupts disabled, we are rid of concern about synchronization. */

#include <kernel/timer.h>

static inline void run_softirq()
{
	if (softirq.pending && current != softirqd)
		go_run_atomic(softirqd);

#ifdef CONFIG_TIMER
	extern struct timer_queue timerq;
	extern struct task *timerd;

	if (timerq.nr && time_after(timerq.next, systick) && current != timerd)
		go_run_atomic_if(timerd, TASK_SLEEPING);
#endif
}

void schedule_core()
{
#ifdef CONFIG_DEBUG
	/* stack overflow */
	if ((current->mm.base[0] != STACK_SENTINEL) ||
			(current->mm.kernel.base[0] != STACK_SENTINEL))
	{
		error("stack overflow %x(%x)", current, current->addr);
		unsigned int i;
		for (i = 0; i < NR_CONTEXT; i++)
			error("%08x", current->mm.sp[i]);
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
	cfs_rq_del(&cfs, next);
	cfs_rq_add(&cfs, current);
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

	if (is_task_realtime(new)) {
#ifdef CONFIG_REALTIME
		rts_rq_add(&rts, new);
#endif
	} else
		cfs_rq_add(&cfs, new);

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
	update_curr();
	to->se.sum_exec_runtime += current->se.sum_exec_runtime;
}

#ifdef CONFIG_DEBUG_SCHED
int sched_overhead;
#endif

void __attribute__((naked, used)) __schedule()
{
#ifdef CONFIG_DEBUG_SCHED
	/* FIXME: do not use any registers that are not saved yet
	 * make sure that registers used here must be the ones saved already */
	sched_overhead = get_sysclk();
#endif

	dsb();
	__context_save(current);

	schedule_core();

	__context_restore(current);
	dsb();
	isb();

#ifdef CONFIG_DEBUG_SCHED
	sched_overhead -= get_sysclk();
#endif
	__ret();
}

static void __attribute__((naked, used)) __schedule_dummy()
{
	__ret();
}

#include <asm/io.h>

void run_scheduler(bool run)
{
	void (*func)();

	if (!is_interrupt_disabled() &&
			get_current_rank() == TF_USER &&
			!which_context()) {
		error("no permission");
		return;
	}

	if (run)
		func = __schedule;
	else
		func = __schedule_dummy;

	register_isr(NVECTOR_PENDSV, func);
	isb();
}

#include <kernel/init.h>

void __init scheduler_init()
{
	cfs_init(&cfs);
#ifdef CONFIG_REALTIME
	rts_init(&rts);
#endif

	run_scheduler(true);
}

unsigned int nr_running()
{
	unsigned int total = cfs.nr_running;
#ifdef CONFIG_REALTIME
	total += rts.nr_running;
#endif
	return total;
}

void sys_yield()
{
	set_task_state(current, TASK_SLEEPING);
	resched();
}

#include <foundation.h>
void print_rq()
{
	struct links *rq = ((struct links *)cfs.rq)->next;
	struct task *p;

//	int i;

	printf("   ADDR    STATE   TYPE  PRI    PARENT              VTIME EXEC\n");

	while (rq != cfs.rq) {
		p = get_container_of(rq, struct task, rq);

		printf("0x%08x 0x%-4x 0x%-4x %3d 0x%08x(0x%08x) %d %d(%d sec)\n",
				p->addr, p->state, p->flags, p->pri,
				p->parent, p->parent->addr,
				(unsigned int)p->se.vruntime,
				(unsigned int)p->se.sum_exec_runtime,
				(unsigned int)p->se.sum_exec_runtime / sysfreq);

//		for (i = 0; i < NR_CONTEXT; i++)
//			printf(("%x : %x", p->sp + i, *(p->sp + i));

		/* run queue list gets changed continuously so referencing link
		 * here may be broken. Make exception condition to avoid
		 * infinite loop */
		if (rq == rq->next) break;

		rq = rq->next;
	}
}
