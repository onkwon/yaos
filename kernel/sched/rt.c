#include "rt.h"
#include <kernel/task.h>

static struct list rts_rq[RT_PRIORITY+1];

struct task *rts_pick_next(struct scheduler *q)
{
	if ((q->pri > RT_PRIORITY) ||
			(&rts_rq[q->pri] == rts_rq[q->pri].next))
		return NULL;

	return get_container_of(rts_rq[q->pri].next, struct task, rq);
}

void rts_rq_add(struct scheduler *q, struct task *new)
{
	if (!new || get_task_state(new))
		return;

	struct list *rq_head;
	int pri;

	pri     = get_task_pri(new);
	rq_head = &rts_rq[pri];

	/* real time run queue always holds the most priority in `pri` */
	if (pri < q->pri)
		q->pri = pri;

	/* add new in the end of list */
	list_add(&new->rq, rq_head->prev);
	q->nr_running++;
}

void rts_rq_del(struct scheduler *q, struct task *p)
{
	if (list_empty(&p->rq)) return;

	list_del(&p->rq);
	barrier();
	list_link_init(&p->rq);
	q->nr_running--;

	unsigned int i;
	q->pri = RT_PRIORITY + 1;
	for (i = 0; i <= RT_PRIORITY; i++) {
		if (rts_rq[i].next != &rts_rq[i]) {
			q->pri = i;
			break;
		}
	}
}

void rts_init(struct scheduler *rts)
{
	unsigned int i;

	rts->nr_running = 0;
	rts->pri        = RT_PRIORITY;
	rts->rq         = (void *)rts_rq;

	for (i = 0; i <= RT_PRIORITY; i++)
		list_link_init(&rts_rq[i]);
}
