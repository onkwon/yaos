#include <foundation.h>
#include <kernel/sched.h>

static struct sched_t cfs;
#ifdef CONFIG_REALTIME
static struct sched_t rts;
#endif

struct task_t *current;

/* As a scheduler works only for a processor and runs in an interrupt context,
 * we are rid of concern about synchronization. */

#include <time.h>
#include <kernel/cfs.h>

void schedule_core()
{
	struct task_t *next;

#ifdef CONFIG_REALTIME
	/* Real time scheduler */

	if (rts.nr_running) {
		/* rq_add() and rq_del() of real time scheduler must keep
		 * the highest priority amongst tasks in `pri` variable.
		 * `pri` has the least priority when no task in runqueue. */
		if ( !IS_TASK_REALTIME(current) ||
				(rts.pri <= GET_PRIORITY(current)) ) {
rts_next:
			next = rts_pick_next(&rts);

			if (!IS_TASK_REALTIME(current))
				cfs_rq_add(&cfs, current);
			else
				rts_rq_add(&rts, current);

			current = next;

			rts_rq_del(&rts, next);

			/* count 1 for `current` */
			rts.nr_running++;
		}

		/* If not runnable, it means the last real time task finished
		 * or went to sleep. Now it's time for CFS. */
		if (get_task_state(current) & TASK_RUNNING)
			return;
		else if (rts.nr_running > 1)
			goto rts_next;

		rts.nr_running = 0;
	}
#endif

	/* Completely fair scheduler */

	/* Find a better way to minimize scheduling overhead
	 * when there is only one task, `current`. */
	next = cfs_pick_next(&cfs);

	/* add `current` back into runqueue after picking next */
	cfs_rq_add(&cfs, current);

	if (!next) { /* no task to schedule */
		/* wake up the init task */
	}

	current = next;

	/* remove the next task from runqueue */
	cfs_rq_del(&cfs, next);

	/* Update newly selected task's start time because it is stale
	 * as much as how long the one has been waiting for. */
	current->se.exec_start = __get_jiffies_64();
}

/* Calling update_curr() as soon as the system timer interrupt occurs would be
 * the best chance other than elsewhere not to count scheduling overhead but to
 * count only its running time, as long as jiffies gets updated asynchronous. */
void update_curr()
{
	uint64_t clock = __get_jiffies_64();
	unsigned delta_exec;

	delta_exec = clock - current->se.exec_start;
	current->se.vruntime += delta_exec;
	current->se.sum_exec_runtime += delta_exec;
	current->se.exec_start = clock;

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

void runqueue_add(struct task_t *new)
{
	if (IS_TASK_REALTIME(new)) { /* a real time task */
#ifdef CONFIG_REALTIME
		rts_rq_add(&rts, new);
#endif
	} else /* a normal task */
		cfs_rq_add(&cfs, new);
}

void scheduler_init()
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
		LIST_LINK_INIT(&rts_rq[i]);
	}
#endif
}

void print_rq()
{
	struct list_t *rq = ((struct list_t *)cfs.rq)->next;
	struct task_t *p;

//	int i;

	while (rq != cfs.rq) {
		p = get_container_of(rq, struct task_t, rq);

		printf("[%08x] state = %x, vruntime = %d, exec_runtime = %d (%d sec)\n",
				p->addr, p->state, (unsigned)p->se.vruntime,
				(unsigned)p->se.sum_exec_runtime,
				(unsigned)p->se.sum_exec_runtime / HZ);

//		for (i = 0; i < CONTEXT_NR; i++)
//			DEBUG(("%x : %x\n", p->sp + i, *(p->sp + i)));

		rq = rq->next;
	}
}
