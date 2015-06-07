#include <kernel/task.h>
#include "fair.h"

/* runqueue's head can be init task. thnik about it */
DEFINE_LIST_HEAD(cfs_rq);

struct task *cfs_pick_next(struct scheduler *cfs)
{
	struct list *rq_head = cfs->rq;

	if (rq_head->next == rq_head) /* empty */
		return NULL;

	return get_container_of(rq_head->next, struct task, rq);
}

void cfs_rq_add(struct scheduler *cfs, struct task *new)
{
	if (!new || get_task_state(new))
		return;

	struct list *p, *rq_head;
	struct task *task;

	rq_head = cfs->rq;
	p       = rq_head->next;

	cfs->nr_running++;

	if (p == rq_head) { /* empty */
		list_add(&new->rq, rq_head);
		return;
	}

	do {
		task = get_container_of(p, struct task, rq);

		if (task->se.vruntime > new->se.vruntime) {
			list_add(&new->rq, p->prev);
			break;
		}

		p = p->next;
	} while (p != rq_head);

	if (p == rq_head) /* the last entry */
		list_add(&new->rq, rq_head->prev);
}

void cfs_rq_del(struct scheduler *cfs, struct task *p)
{
	cfs->nr_running--;

	list_del(&p->rq);
}
