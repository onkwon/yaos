#include "pbrr.h"

#include "queue.h"
#include "syslog.h"
#include "kernel/lock.h"
#include "kernel/task.h"

#include <assert.h>
#include <errno.h>

static queue_t runqueue[TASK_PRIORITY_MAX];

int pbrr_enqueue(struct scheduler *sched, void *node)
{
	struct task *new = node;
	queue_t *q;
	int pri, err;

	if (!sched || !new || get_task_state(new))
		return -EINVAL;

	pri = get_task_pri(new);
	assert((pri >= 0) && (pri < TASK_PRIORITY_MAX));
	q = sched->rq;
	q = &q[pri];

	if ((err = enqueue(q, &new))) {
		error("failed adding in queue: %d", err);
		return -ENOSPC;
	}

	atomic_faa(&sched->nr_running, 1);

	return 0;
}

void *pbrr_dequeue(struct scheduler *sched)
{
	queue_t *q;
	int pri;
	struct task *task = NULL;

	if (!sched)
		return NULL;

	for (pri = 0; pri < TASK_PRIORITY_MAX; pri++) {
		q = sched->rq;
		q = &q[pri];
		if (dequeue(q, &task) == 0) {
			atomic_faa(&sched->nr_running, -1);
			return task;
		}
	}

	return NULL;
}

void pbrr_init(struct scheduler *pbrr)
{
	static uintptr_t arr[TASK_PRIORITY_MAX][MAX_NR_TASKS];

	pbrr->nr_running = 0;
	pbrr->rq = &runqueue;

	for (int i = 0; i < TASK_PRIORITY_MAX; i++)
		queue_init_static(&runqueue[i], arr[i], MAX_NR_TASKS,
				sizeof(**arr));

	pbrr->add = pbrr_enqueue;
	pbrr->next = pbrr_dequeue;
}
