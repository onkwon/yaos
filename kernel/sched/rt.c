#include "rt.h"
#include <kernel/task.h>

struct list rts_rq[RT_LEAST_PRIORITY+1];

struct task *rts_pick_next(struct scheduler *q)
{
	return get_container_of(rts_rq[q->pri].next, struct task, rq);
}

void rts_rq_add(struct scheduler *q, struct task *new)
{
	if (!new || get_task_state(new))
		return;

	int pri = get_task_pri(new);
	struct list *rq_head = &rts_rq[pri];

	q->nr_running++;

	/* keep the highest priority of tasks residing in runqueue */
	if (pri < q->pri)
		q->pri = pri;

	/* add new in the end of list */
	list_add(&new->rq, rq_head->prev);
}

void rts_rq_del(struct scheduler *q, struct task *p)
{
	int i;

	if (--q->nr_running > 1) {
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
