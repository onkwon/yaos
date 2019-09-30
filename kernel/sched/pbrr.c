#include "pbrr.h"
#include "list.h"
#include "kernel/task.h"

static struct listq_head runq_head[TASK_PRIORITY_MAX];

int pbrr_enqueue(struct scheduler *sched, void *node)
{
	struct task *new = node;
	struct listq_head *q;
	int pri;

	if (!sched || !new || get_task_state(new))
		return -EINVAL;

	pri = get_task_pri(new);
	assert((pri >= 0) && (pri < TASK_PRIORITY_MAX));
	q = sched->rq;
	q = &q[pri];

	listq_push(&new->rq, q);
	atomic_faa(&sched->nr_running, 1);

	return 0;
}

void *pbrr_dequeue(struct scheduler *sched)
{
	struct listq_head *q;
	int pri;

	if (!sched)
		return NULL;

	for (pri = TASK_PRIORITY_MAX - 1; pri >= 0; pri--) {
		struct list *node;

		q = sched->rq;
		q = &q[pri];
		node = listq_pop(q);
		if (node) {
			struct task *task
				= container_of(node, struct task, rq);
			atomic_faa(&sched->nr_running, -1);

			return task;
		}
	}

	return NULL;
}

void pbrr_init(struct scheduler *pbrr)
{
	pbrr->nr_running = 0;
	pbrr->rq = &runq_head;

	for (int i = 0; i < TASK_PRIORITY_MAX; i++) {
		listq_init(&runq_head[i]);
	}

	pbrr->add = pbrr_enqueue;
	pbrr->next = pbrr_dequeue;
}
