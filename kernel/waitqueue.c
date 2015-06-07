#include <kernel/waitqueue.h>
#include <kernel/lock.h>
#include <kernel/sched.h>
#include <kernel/task.h>

void wq_wait(struct waitqueue_head *q, struct waitqueue *wait)
{
	unsigned irqflag;

	spin_lock_irqsave(q->lock, irqflag);
	if (list_empty(&wait->link))
		list_add(&wait->link, q->list.prev);

	set_task_state(current, TASK_WAITING);
	spin_unlock_irqrestore(q->lock, irqflag);

	schedule();
}

void wq_wake(struct waitqueue_head *head, int nr_task)
{
	struct list *p = head->list.next;
	struct task *task;
	unsigned irqflag;

	spin_lock_irqsave(head->lock, irqflag);
	while (p != &head->list && nr_task) {
		task = get_container_of(p, struct waitqueue, link)->task;
		set_task_state(task, TASK_RUNNING);
		runqueue_add(task);
		list_del(p);

		p = head->list.next;
		nr_task--;
	}
	spin_unlock_irqrestore(head->lock, irqflag);
}
