#include <kernel/sched.h>
#include <kernel/task.h>
#include <kernel/jiffies.h>
#include <kernel/softirq.h>

static struct sched_t cfs;
#ifdef CONFIG_REALTIME
static struct sched_t rts;
#endif

static inline void runqueue_add_core(struct task_t *new)
{
	if (is_realtime(new)) {
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
	struct task_t *next;

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
		if ( !is_realtime(current) ||
				(rts.pri <= get_task_pri(current)) ) {
rts_next:
			next = rts_pick_next(&rts);
			runqueue_add_core(current);
			current = next;
			rts_rq_del(&rts, next);

			/* count 1 for `current` */
			if (!rts.nr_running)
				rts.nr_running = 1;
		}

		/* If not runnable when nr_running is 1,
		 * it means no real task to run */
		if (!get_task_state(current))
			goto adjust_vruntime;
		else if (rts.nr_running > 1)
			goto rts_next;

		/* Now it's time for CFS */
		rts.nr_running = 0;
	}
#endif

	if (!(next = cfs_pick_next(&cfs))) {
		if (get_task_state(current)) {
			/* no task to schedule. wake the init task up */
			set_task_state(&init, TASK_RUNNING);
			current = &init;
		}
		goto adjust_vruntime;
	}

	/* add `current` back into runqueue after picking next */
	cfs_rq_add(&cfs, current);

	current = next;

	/* remove the next task from runqueue */
	cfs_rq_del(&cfs, next);

adjust_vruntime:
	/* Update newly selected task's start time because it is stale
	 * as much as the one has been waiting for. */
	current->se.exec_start = get_jiffies_64_core();
}

/* Calling update_curr() as soon as the system timer interrupt occurs would be
 * the best chance other than elsewhere not to count scheduling overhead but to
 * count only its running time, as long as jiffies gets updated asynchronous. */
void inline update_curr()
{
	uint64_t clock = get_jiffies_64_core();
	unsigned delta_exec;

	delta_exec = clock - current->se.exec_start;
	current->se.vruntime += delta_exec;
	current->se.sum_exec_runtime += delta_exec;
	current->se.exec_start = clock;

	if (is_realtime(current))
		return;

	struct task_t *task;

	cfs.vruntime_base = current->se.vruntime;

	/* pick the least vruntime in runqueue for vruntime_base
	 * to keep order properly. */
	if (((struct list_t *)cfs.rq)->next != cfs.rq) { /* if it's not empty */
		task = get_container_of( ((struct list_t *)cfs.rq)->next,
				struct task_t, rq );
		if (cfs.vruntime_base > task->se.vruntime)
			cfs.vruntime_base = task->se.vruntime;
	}
}

void inline runqueue_add(struct task_t *new)
{
	runqueue_add_core(new);
}

#include <kernel/init.h>

void __init scheduler_init()
{
	extern struct list_t cfs_rq;

	cfs.vruntime_base = 0;
	cfs.nr_running    = 0;
	cfs.rq            = (void *)&cfs_rq;

#ifdef CONFIG_REALTIME
	extern struct list_t rts_rq[RT_LEAST_PRIORITY+1];

	rts.nr_running = 0;
	rts.pri        = RT_LEAST_PRIORITY;
	rts.rq         = (void *)rts_rq;

	int i;

	for (i = 0; i <= RT_LEAST_PRIORITY; i++) {
		list_link_init(&rts_rq[i]);
	}
#endif

	schedule_on();
}

#ifdef CONFIG_DEBUG
#include <foundation.h>

void print_rq()
{
	struct list_t *rq = ((struct list_t *)cfs.rq)->next;
	struct task_t *p;

//	int i;

	while (rq != cfs.rq) {
		p = get_container_of(rq, struct task_t, rq);

		printk("[%08x] state = %x, type = %x, pri = %x, vruntime = %d"
				"exec_runtime = %d (%d sec)\n",
				p->addr, p->state, p->flags, p->pri,
				(unsigned)p->se.vruntime,
				(unsigned)p->se.sum_exec_runtime,
				(unsigned)p->se.sum_exec_runtime / HZ);

//		for (i = 0; i < NR_CONTEXT; i++)
//			DEBUG(("%x : %x", p->sp + i, *(p->sp + i)));

		rq = rq->next;
	}
}
#endif