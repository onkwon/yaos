#include <waitqueue.h>
#include <kernel/sched.h>

void wait_in(struct waitqueue_head_t *q, struct waitqueue_t *wait)
{
	unsigned irq_flag;

	spin_lock_irqsave(q->lock, irq_flag);

	if (list_empty(&wait->link))
		list_add(&wait->link, q->list.prev);

	set_task_state(current, TASK_WAITING);

	spin_unlock_irqrestore(q->lock, irq_flag);

	schedule();
}

void wake_up(struct waitqueue_head_t *head, int nr_task)
{
	struct list_t *p = head->list.next;
	struct task_t *task;
	unsigned irq_flag;

	spin_lock_irqsave(head->lock, irq_flag);

	while (p != &head->list && nr_task) {
		task = get_container_of(p, struct waitqueue_t, link)->task;
		set_task_state(task, TASK_RUNNING);
		runqueue_add(task);
		list_del(p);

		p = head->list.next;
		nr_task--;
	}

	spin_unlock_irqrestore(head->lock, irq_flag);
}
