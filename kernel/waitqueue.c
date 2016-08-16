#include <kernel/waitqueue.h>
#include <kernel/lock.h>
#include <kernel/sched.h>
#include <kernel/task.h>
#include <error.h>

void sleep_in_waitqueue(struct waitqueue_head *q)
{
	unsigned int irqflag;
	DEFINE_WAIT(new);

	lock_atomic(&q->lock);
	irq_save(irqflag);
	local_irq_disable();

	if (!list_empty(&new.link)) {
		debug(MSG_ERROR, "already linked to 0x%x - 0x%x",
				new.link.prev, new.link.next);
		goto out;
	}

	list_add(&new.link, q->list.prev);
	set_task_state(current, TASK_WAITING);

out:
	unlock_atomic(&q->lock);
	irq_restore(irqflag);

	schedule();
}

void shake_waitqueue_out(struct waitqueue_head *q)
{
	struct task *task = NULL;
	struct list *next;
	unsigned int irqflag;

	lock_atomic(&q->lock);
	irq_save(irqflag);
	local_irq_disable();

	for (next = q->list.next; next != &q->list; next = next->next) {
		task = get_container_of(next, struct waitqueue, link)->task;

		if (get_task_state(task) == TASK_WAITING) {
			list_del(next);
			set_task_state(task, TASK_RUNNING);
			break;
		}
	}

	unlock_atomic(&q->lock);
	irq_restore(irqflag);

	if (next == &q->list)
		return;

	if (task)
		runqueue_add(task);
}

void wq_wait(struct waitqueue_head *q)
{
	unsigned int irqflag;
	DEFINE_WAIT(wait);

	spin_lock_irqsave(&q->lock, irqflag);
	if (list_empty(&wait.link))
		list_add(&wait.link, q->list.prev);

	set_task_state(current, TASK_WAITING);
	spin_unlock_irqrestore(&q->lock, irqflag);

	schedule();
}

void wq_wake(struct waitqueue_head *q, int nr_task)
{
	struct list *p = q->list.next;
	struct task *task;
	unsigned int irqflag;

	spin_lock_irqsave(&q->lock, irqflag);
	while (p != &q->list && nr_task) {
		task = get_container_of(p, struct waitqueue, link)->task;
		set_task_state(task, TASK_RUNNING);
		runqueue_add(task);
		list_del(p);

		p = q->list.next;
		nr_task--;
	}
	spin_unlock_irqrestore(&q->lock, irqflag);
}
