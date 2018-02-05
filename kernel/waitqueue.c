#include <kernel/waitqueue.h>
#include <kernel/lock.h>
#include <kernel/sched.h>
#include <kernel/task.h>
#include <error.h>

/* TODO: Replace doubly linked list with singly linked list in waitqueue */

#include <kernel/timer.h>
#include <stdlib.h>

static void wake_callback(struct ktimer *timer)
{
	struct waitqueue_head *q = timer->task->args;
	struct links *curr;
	struct task *task;
	unsigned int flags;
	bool deleted = false;

	lock_atomic(&q->lock);
	for (curr = q->list.next; curr != &q->list; curr = curr->next) {
		task = get_container_of(curr, struct waitqueue, list)->task;
		if (task == timer->task) {
			links_del(curr);
			deleted = true;
			break;
		}
	}
	unlock_atomic(&q->lock);

	if (deleted) {
		/* A trick to enter privileged mode */
		flags = get_task_flags(current);
		if (!(flags & TF_PRIVILEGED)) {
			set_task_flags(current, flags | TASK_PRIVILEGED);
			schedule();
		}

		assert(!is_locked(timer->task->lock));

		timer->task->args = (void *)-ETIME;
		go_run(timer->task);
		__free(timer, timer->task);

		/* return to the original mode */
		if (!(flags & TF_PRIVILEGED)) {
			set_task_flags(current, flags);
			schedule();
		}
	}
}

/* NOTE: Do not use sleep_in_waitqeue() and its pair, shake_waitqueue_out(), in
 * interrupt context to avoid deadlock. Use wq_wait() and its pair, wq_wake(),
 * instead in interrupt context. */
int __attribute__((used)) sleep_in_waitqueue(struct waitqueue_head *q, int ms)
{
	/* its own stack would never change since the task is entering in
	 * waitqueue. so to use local variable for waitqueue list here
	 * absolutely fine. */
	DEFINE_WAIT(new);

	if (!ms)
		return -ETIME;

	lock_atomic(&q->lock);

	/* keep the pointer of its own queue in temp space of task structure */
	current->args = q;
	if ((ms != INF) && add_timer(ms, wake_callback)) {
		unlock_atomic(&q->lock);
		return -ETIME;
	}

	links_add(&new.list, q->list.prev);

	/* NOTE: Lock the task first before releasing queue. Otherwise waking
	 * job can be done first even before waiting job to be done. */
	lock_atomic(&current->lock);
	unlock_atomic(&q->lock);

	assert(get_task_state(current) == TASK_RUNNING);
	assert(is_locked(current->lock));

	set_task_state(current, TASK_WAITING);
	unlock_atomic(&current->lock);

	schedule();

	return (int)current->args;
}

/* TODO: Support `WQ_ALL` option at runtime */
void __attribute__((used)) shake_waitqueue_out(struct waitqueue_head *q)
{
	struct task *task = NULL;
	struct links *next;
	unsigned int flags;

	lock_atomic(&q->lock);

	if (links_empty(&q->list)) {
		unlock_atomic(&q->lock);
		return;
	}

	next = q->list.next;
	assert(next != &q->list);
	links_del(next);

	task = get_container_of(next, struct waitqueue, list)->task;

	/* NOTE: Lock the task first before releasing queue. Otherwise waking
	 * job can be done first even before waiting job to be done. */
	lock_atomic(&task->lock);
	unlock_atomic(&q->lock);

	assert(get_task_state(task) == TASK_WAITING);
	assert(is_locked(task->lock));

	/* there is only one wake_callback registered at a time if timer
	 * registered. so if being woken up earlier than timer, remove the
	 * timer registered prior being executed. */
	__del_timer_if_match(task, wake_callback);

	/* A trick to enter privileged mode */
	flags = get_task_flags(current);
	if (!(flags & TF_PRIVILEGED)) {
		set_task_flags(current, flags | TASK_PRIVILEGED);
		schedule();
	}

	set_task_state(task, TASK_RUNNING);
	runqueue_add(task);

	unlock_atomic(&task->lock);

	/* return to the original mode */
	if (!(flags & TF_PRIVILEGED)) {
		set_task_flags(current, flags);
		schedule();
	}
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
