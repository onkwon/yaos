#ifdef CONFIG_REALTIME
#include <kernel/rts.h>

struct list_t rts_rq[RT_LEAST_PRIORITY+1];

struct task_t *rts_pick_next(struct sched_t *q)
{
	return get_container_of(rts_rq[q->pri].next, struct task_t, rq);
}

void rts_rq_add(struct sched_t *q, struct task_t *new)
{
	if ( !new || !(get_task_state(new) & TASK_RUNNING) )
		return;

	int pri = GET_PRIORITY(new);
	struct list_t *rq_head = &rts_rq[pri];

	q->nr_running++;

	/* keep the highest priority of tasks residing in runqueue */
	if (pri < q->pri)
		q->pri = pri;

	/* add new in the end of list */
	list_add(&new->rq, rq_head->prev);
}

void rts_rq_del(struct sched_t *q, struct task_t *p)
{
	int i;

	if (--q->nr_running) {
		for (i = 0; i <= RT_LEAST_PRIORITY; i++) {
			if (rts_rq[i].next != &rts_rq[i]) {
				q->pri = i;
				break;
			}
		}
	} else {
		q->pri = RT_LEAST_PRIORITY + 1;
	}

	list_del(&p->rq);
}
#endif
