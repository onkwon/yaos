#include <kernel/waitqueue.h>
#include <kernel/lock.h>
#include <kernel/sched.h>
#include <kernel/task.h>
#include <error.h>

/* TODO: Replace doubly linked list with singly linked list in waitqueue */

/* NOTE: Do not use sleep_in_waitqeue() and its pair, shake_waitqueue_out(), in
 * interrupt context to avoid deadlock. Use wq_wait() and its pair, wq_wake(),
 * instead in interrupt context. */
void sleep_in_waitqueue(struct waitqueue_head *q)
{
	/* its own stack would never change since the task is entering in
	 * waitqueue. so to use local variable for waitqueue list here
	 * absolutely fine. */
	DEFINE_WAIT(new);

	assert(!is_locked(current->lock));

	set_task_state(current, TASK_WAITING);

	lock_atomic(&q->lock);
	links_add(&new.list, q->list.prev);
	unlock_atomic(&q->lock);

	schedule();
}

/* TODO: Support `WQ_ALL` option at runtime */
void shake_waitqueue_out(struct waitqueue_head *q)
{
	struct task *task = NULL;
	struct links *next;

	if (links_empty(&q->list))
		return;

	lock_atomic(&q->lock);

#if 1 /* WQ_EXCLUSIVE */
	next = q->list.next;
	assert(next != &q->list);
	links_del(next);

	task = get_container_of(next, struct waitqueue, list)->task;
	assert(get_task_state(task) == TASK_WAITING);
	assert(!is_locked(task->lock));

	set_task_state(task, TASK_RUNNING);
	runqueue_add(task);
#else /* WQ_ALL */
	for (next = q->list.next; next != &q->list && nr_task; next = next->next) {
		links_del(next);

		task = get_container_of(next, struct waitqueue, list)->task;
		assert(get_task_state(task) == TASK_WAITING);
		assert(!is_locked(task->lock));

		set_task_state(task, TASK_RUNNING);
		runqueue_add(task);

		nr_task--;
	}
#endif

	unlock_atomic(&q->lock);
}

void wq_wait(struct waitqueue_head *q)
{
	unsigned int irqflag;
	DEFINE_WAIT(wait);

	spin_lock_irqsave(&q->lock, irqflag);

	if (links_empty(&wait.list))
		links_add(&wait.list, q->list.prev);

	assert(!is_locked(current->lock));
	set_task_state(current, TASK_WAITING);

	spin_unlock_irqrestore(&q->lock, irqflag);

	schedule();
}

void wq_wake(struct waitqueue_head *q, int nr_task)
{
	struct links *p = q->list.next;
	struct task *task;
	unsigned int irqflag;

	spin_lock_irqsave(&q->lock, irqflag);
	while (p != &q->list && nr_task) {
		task = get_container_of(p, struct waitqueue, list)->task;

		assert(!is_locked(task->lock));
		set_task_state(task, TASK_RUNNING);
		runqueue_add_core(task);
		links_del(p);

		p = q->list.next;
		nr_task--;
	}
	spin_unlock_irqrestore(&q->lock, irqflag);
}
