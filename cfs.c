#include <kernel/cfs.h>

/* runqueue's head can be init task. thnik about it */
LIST_HEAD(cfs_rq);

struct task_t *cfs_pick_next(struct sched_t *cfs)
{
	struct list_t *rq_head = cfs->rq;

	if (rq_head->next == rq_head) /* empty */
		return NULL;

	return get_container_of(rq_head->next, struct task_t, rq);
}

void cfs_rq_add(struct sched_t *cfs, struct task_t *new)
{
	if ( !new || !(get_task_state(new) & TASK_RUNNING) )
		return;

	struct list_t *p, *rq_head;
	struct task_t *task;

	rq_head = cfs->rq;
	p       = rq_head->next;

	cfs->nr_running++;

	if (p == rq_head) { /* empty */
		list_add(&new->rq, rq_head);
		return;
	}

	do {
		task = get_container_of(p, struct task_t, rq);

		if (task->se.vruntime > new->se.vruntime) {
			list_add(&new->rq, p->prev);
			break;
		}

		p = p->next;
	} while (p != rq_head);

	if (p == rq_head) /* the last entry */
		list_add(&new->rq, rq_head->prev);
}

void cfs_rq_del(struct sched_t *cfs, struct task_t *p)
{
	cfs->nr_running--;

	list_del(&p->rq);
}
