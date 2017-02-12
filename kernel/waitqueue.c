#include <kernel/waitqueue.h>
#include <kernel/lock.h>
#include <kernel/sched.h>
#include <kernel/task.h>
#include <error.h>

/* TODO: Replace doubly linked list with singly linked list in waitqueue */

void sleep_in_waitqueue(struct waitqueue_head *q)
{
	/* its own stack would never change since the task is entering in
	 * waitqueue. so to use local variable for waitqueue list here
	 * absolutely fine. */
	DEFINE_WAIT(new);
	unsigned int irqflag;

	assert(!is_locked(current->lock));
	spin_lock(&current->lock);
	set_task_state(current, TASK_WAITING);
	spin_unlock(&current->lock);

	spin_lock_irqsave(&q->lock, irqflag);
	links_add(&new.list, q->list.prev);
	spin_unlock_irqrestore(&q->lock, irqflag);

	schedule();
}

/* TODO: Support `WQ_ALL` option at runtime */
void shake_waitqueue_out(struct waitqueue_head *q)
{
	struct task *task = NULL;
	struct links *next;
	unsigned int irqflag;

	if (links_empty(&q->list))
		return;

	spin_lock_irqsave(&q->lock, irqflag);

#if 1 /* WQ_EXCLUSIVE */
	next = q->list.next;
	assert(next != &q->list);
	links_del(next);

	task = get_container_of(next, struct waitqueue, list)->task;
	assert(get_task_state(task) == TASK_WAITING);

	spin_lock(&task->lock);
	set_task_state(task, TASK_RUNNING);
	runqueue_add(task);
	spin_unlock(&task->lock);
#else /* WQ_ALL */
	for (next = q->list.next; next != &q->list && nr_task; next = next->next) {
		links_del(next);

		task = get_container_of(next, struct waitqueue, list)->task;
		assert(get_task_state(task) == TASK_WAITING);

		spin_lock(&task->lock);
		set_task_state(task, TASK_RUNNING);
		runqueue_add(task);
		spin_unlock(&task->lock);

		nr_task--;
	}
#endif

	spin_unlock_irqrestore(&q->lock, irqflag);
}

void wq_wait(struct waitqueue_head *q)
{
	unsigned int irqflag;
	DEFINE_WAIT(wait);

	spin_lock_irqsave(&q->lock, irqflag);
	if (links_empty(&wait.list))
		links_add(&wait.list, q->list.prev);

	spin_lock(&current->lock);
	set_task_state(current, TASK_WAITING);
	spin_unlock(&current->lock);

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

		spin_lock(&task->lock);
		set_task_state(task, TASK_RUNNING);
		runqueue_add_core(task);
		links_del(p);
		spin_unlock(&task->lock);

		p = q->list.next;
		nr_task--;
	}
	spin_unlock_irqrestore(&q->lock, irqflag);
}
