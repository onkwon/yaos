#include <kernel/waitqueue.h>
#include <kernel/lock.h>
#include <kernel/sched.h>
#include <kernel/task.h>

void sleep_in_waitqueue(struct waitqueue_head *q)
{
	unsigned int irqflag;
	DEFINE_WAIT(new);

	lock_dec_spinning(&q->lock);

	if (list_empty(&new.link))
		list_add(&new.link, q->list.prev);

	lock_inc_spinning(&q->lock);

	irq_save(irqflag);
	local_irq_disable();
	set_task_state(current, TASK_WAITING);
	irq_restore(irqflag);

	schedule();
}

void shake_waitqueue_out(struct waitqueue_head *q)
{
	return;
	struct task *task;
	unsigned int irqflag;

	lock_dec_spinning(&q->lock);

	if (q->list.next == &q->list) {
		lock_inc_spinning(&q->lock);
		return;
	}

	task = get_container_of(q->list.next, struct waitqueue, link)->task;
	list_del(q->list.next);

	lock_inc_spinning(&q->lock);

	irq_save(irqflag);
	local_irq_disable();
	set_task_state(task, TASK_RUNNING);
	irq_restore(irqflag);
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
