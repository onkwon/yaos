#include <io.h>
#include <kernel/task.h>
#include "fair.h"

/* TODO: Implement cfs
 * It is still just a round robin scheduler though the vruntime notion is
 * injected */

/* TODO: Make the runqueue's head to point to the init task
 * The cost of turning to the idle task when no task running could be reduced
 * in the way, I guess. think about it */
static DEFINE_LINKS_HEAD(cfs_rq);

struct task *cfs_pick_next(struct scheduler *cfs)
{
	struct links *head = cfs->rq;

	if (head->next == head) /* empty */
		return NULL;

	return get_container_of(head->next, struct task, rq);
}

void cfs_rq_add(struct scheduler *cfs, struct task *new)
{
	if (!new || get_task_state(new))
		return;

	struct links *curr, *head, *to;
	struct task *task;

	head = cfs->rq;
	to   = head->prev;

	/* TODO: Improve O(N) `cfs_rq_add()`, the more tasks the slower */
	for (curr = head->next; curr != head; curr = curr->next) {
		task = get_container_of(curr, struct task, rq);

		if (task->se.vruntime > new->se.vruntime) {
			to = curr->prev;
			break;
		}
	}

	links_add(&new->rq, to);
	cfs->nr_running++;
}

void cfs_rq_del(struct scheduler *cfs, struct task *task)
{
	if (links_empty(&task->rq))
		return;

	/* NOTE: The links must get back to empty state after removing from the
	 * runqueue, or the runqueue list will get screwed up removing the
	 * links more than once. Actually you don't need to have `links_init()`
	 * here as long as the init task doesn't need to be added to the
	 * runqueue in `unlink_task()`. */
	links_del(&task->rq);
	links_init(&task->rq);
	cfs->nr_running--;
}

void cfs_init(struct scheduler *cfs)
{
	cfs->vruntime_base = 0;
	cfs->nr_running    = 0;
	cfs->rq            = (void *)&cfs_rq;
}
