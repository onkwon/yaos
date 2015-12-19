#include <kernel/timer.h>
#include <kernel/systick.h>
#include <kernel/task.h>
#include <kernel/lock.h>
#include <error.h>

#ifdef CONFIG_TIMER
struct timer_queue timerq;
struct task *timerd;

static inline int __add_timer(struct timer *new)
{
	struct list *curr;
	int stamp = (int)systick;

	new->expires -= 1; /* as it uses time_after() not time_equal */

	if (time_after(new->expires, stamp))
		return -ERR_RANGE;

	new->task = current;

	unsigned int irqflag;
	spin_lock_irqsave(timerq.lock, irqflag);

	for (curr = timerq.list.next; curr != &timerq.list; curr = curr->next) {
		if (((int)new->expires - stamp) <
				((int)((struct timer *)curr)->expires - stamp))
			break;
	}

	if (((int)new->expires - stamp) < ((int)timerq.next - stamp))
		timerq.next = new->expires;

	list_add(&new->list, curr->prev);
	timerq.nr++;

	spin_unlock_irqrestore(timerq.lock, irqflag);

	return 0;
}

static void run_timer()
{
	struct timer *timer;
	struct list *curr;
	unsigned int irqflag;
	int tid;

infinite:
	for (curr = timerq.list.next; curr != &timerq.list; curr = curr->next) {
		timer = (struct timer *)curr;

		if (time_before(timer->expires, systick)) {
			timerq.next = timer->expires;
			break;
		}

		tid = clone(STACK_SHARED | (get_task_flags(timer->task) &
						TASK_PRIVILEGED), &init);
		if (tid > 0) {
			/* Note that it is running at HIGHEST_PRIORITY just
			 * like its parent, run_timer(). Change the priority to
			 * its own tasks' to do job at the right priority. */
			set_task_pri(current, get_task_pri(timer->task));
			schedule();

			if (timer->event)
				timer->event(timer->data);

			/* A trick to enter privileged mode */
			if (!(get_task_flags(current) & TASK_PRIVILEGED)) {
				set_task_flags(current, get_task_flags(current)
						| TASK_PRIVILEGED);
				schedule();
			}

			sum_curr_stat(timer->task);

			kill((unsigned int)current);
			freeze();
		} else if (tid != 0) /* error if not parent */
			break;

		spin_lock_irqsave(timerq, irqflag);
		list_del(curr);
		timerq.nr--;
		spin_unlock_irqrestore(timerq, irqflag);
	}

	yield();
	goto infinite;
}

static void sleep_callback(unsigned int data)
{
	struct task *task = (struct task *)data;

	/* A trick to enter privileged mode */
	if (!(get_task_flags(current) & TASK_PRIVILEGED)) {
		set_task_flags(current, get_task_flags(current)
				| TASK_PRIVILEGED);
		schedule();
	}

	if (get_task_state(task) == TASK_SLEEPING) {
		set_task_state(task, TASK_RUNNING);
		runqueue_add(task);
	}
}

#include <kernel/init.h>

int __init timer_init()
{
	timerq.nr = 0;
	list_link_init(&timerq.list);
	lock_init(&timerq.lock);

	if ((timerd = make(TASK_KERNEL | STACK_SHARED, run_timer, &init))
			== NULL)
		return -ERR_ALLOC;

	set_task_pri(timerd, HIGHEST_PRIORITY);

	return 0;
}
#else
unsigned int timer_nr = 0;
#endif /* CONFIG_TIMER */

unsigned int get_timer_nr()
{
#ifdef CONFIG_TIMER
	return timerq.nr;
#else
	return timer_nr;
#endif
}

int add_timer(struct timer *new)
{
	return syscall(SYSCALL_TIMER_CREATE, new);
}

#include <foundation.h>

void sleep(unsigned int sec)
{
#ifdef CONFIG_TIMER
	struct timer tm;

	tm.expires = systick + sec_to_ticks(sec);
	tm.event = sleep_callback;
	tm.data = (unsigned int)current;
	if (!add_timer(&tm))
		yield();
#else
	unsigned int timeout = systick + sec_to_ticks(sec);
	timer_nr++;
	while (time_before(timeout, systick));
	timer_nr--;
#endif
}

void msleep(unsigned int ms)
{
#ifdef CONFIG_TIMER
	struct timer tm;

	tm.expires = systick + msec_to_ticks(ms);
	tm.event = sleep_callback;
	tm.data = (unsigned int)current;
	if (!add_timer(&tm))
		yield();
#else
	unsigned int timeout = systick + msec_to_ticks(ms);
	timer_nr++;
	while (time_before(timeout, systick));
	timer_nr--;
#endif
}

int sys_timer_create(struct timer *new)
{
#ifdef CONFIG_TIMER
	return __add_timer(new);
#else
	return -ERR_UNDEF;
#endif
}

void set_timeout(unsigned int *tv, unsigned int ms)
{
	*tv = systick + msec_to_ticks(ms);
}

int is_timeout(unsigned int goal)
{
	if (time_after(goal, systick))
		return 1;

	return 0;
}

#define MHZ		1000000
void udelay(unsigned int us)
{
	volatile unsigned int counter;
	unsigned int goal, stamp;

	stamp = get_sysclk();
	goal  = get_sysclk_freq() / MHZ;
	goal  = goal * us;

	if (goal > get_sysclk_max())
		return;

	do {
		counter = stamp - get_sysclk();
		if ((int)counter < 0)
			counter = get_sysclk_max() - ((int)counter * -1);
	} while (counter < goal);
}

void mdelay(unsigned int ms)
{
	unsigned int tout;
	set_timeout(&tout, ms);
	while (!is_timeout(tout));
}
