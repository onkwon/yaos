#include <kernel/timer.h>
#include <kernel/systick.h>
#include <kernel/task.h>
#include <kernel/lock.h>
#include <error.h>
#include <stdlib.h>
#include "worklist.h"

#ifdef CONFIG_TIMER
struct timer_queue timerq;
struct task *timerd;

static DEFINE_WORKLIST_HEAD(work_list_head);
static struct task *add_timerd;

static void add_timer_core(struct ktimer *tm)
{
	struct worklist *work;
	struct link *curr, *prev;
	struct ktimer *ref;
	int stamp, ret, new;

	set_task_pri(current, DEFAULT_PRIORITY);
	add_timerd = current;

loop:
	while (!worklist_empty(&work_list_head)) {
		work = getwork(&work_list_head);
		tm = work->data;
		kfree(work);

		ret = 0;
		stamp = (int)systick;
		tm->expires -= 1; /* as it uses time_after() not time_equal */

		if (time_after(tm->expires, stamp)) {
			ret = -ERR_RANGE;
		} else {
			/* priority inversion as run_timer has the highest
			 * priority */
			set_task_pri(current, HIGHEST_PRIORITY);
			mutex_lock(&timerq.mutex);

			new = (int)tm->expires - stamp;
			prev = &timerq.list;
			curr = prev->next;

			while (curr) {
				ref = (struct ktimer *)curr;
				if (new < ((int)ref->expires - stamp))
					break;

				prev = curr;
				curr = curr->next;
			}

			if (new < ((int)timerq.next - stamp))
				timerq.next = tm->expires;

			link_add(&tm->list, prev);
			timerq.nr++;

			lock_atomic(&tm->task->lock);
			link_add(&tm->link, &tm->task->timer);
			unlock_atomic(&tm->task->lock);

			mutex_unlock(&timerq.mutex);
			set_task_pri(current, DEFAULT_PRIORITY);
		}

		__set_retval(tm->task, ret);
		//sum_curr_stat(tm->task);
		go_run(tm->task);
	}

	yield();
	goto loop;
}
REGISTER_TASK(add_timer_core, TASK_KERNEL, HIGHEST_PRIORITY, STACK_SIZE_MIN);

void __del_timer_if_match(struct task *task, void *addr)
{
	struct ktimer *timer;
	struct link *curr;
	unsigned int pri;
	bool deleted = false;

	/* priority inversion as run_timer has the highest priority */
	pri = get_task_pri(current);
	set_task_pri(current, HIGHEST_PRIORITY);

	mutex_lock(&timerq.mutex);
	lock_atomic(&task->lock);

	for (curr = task->timer.next; curr; curr = curr->next) {
		timer = get_container_of(curr, struct ktimer, link);

		if (timer->event == addr) {
			link_del(&timer->list, &timerq.list);
			timerq.nr--;
			link_del(&timer->link, &task->timer);
			deleted = true;
			break;
		}
	}

	mutex_unlock(&timerq.mutex);
	set_task_pri(current, pri);

	if (deleted)
		__free(timer, task);

	/* hold lock until free() to avoid the task killed before free */
	unlock_atomic(&task->lock);
}

static void do_run_timer(struct ktimer *timer)
{
	timer->event(timer);
	//TODO: sum_curr_stat(timer->task);

	kill(current);
	freeze(); /* never reaches here */
}

static void run_timer()
{
	struct ktimer *timer;
	struct link *curr, *prev;
	struct task *thread;
	unsigned int flags;

infinite:
	mutex_lock(&timerq.mutex);

	prev = &timerq.list;
	curr = prev->next;

	while (curr) {
		timer = (struct ktimer *)curr;

		if (time_before(timer->expires, systick)) {
			timerq.next = timer->expires;
			break;
		}

		link_del(curr, prev);
		curr = prev->next;
		timerq.nr--;

		lock_atomic(&timer->task->lock);
		link_del(&timer->link, &timer->task->timer);
		unlock_atomic(&timer->task->lock);

		flags = (get_task_flags(timer->task) & ~TASK_STATIC) | TF_ATOMIC;

		if ((thread = make(flags, STACK_SIZE_DEFAULT, do_run_timer,
						timer->task)) == NULL)
			break;

		put_arguments(thread, timer, NULL, NULL, NULL);
		go_run(thread);
	}

	mutex_unlock(&timerq.mutex);

	yield();
	goto infinite;
}

static void sleep_callback(struct ktimer *timer)
{
	struct task *task = timer->data;

	/* A trick to enter privileged mode */
	if (!(get_task_flags(current) & TASK_PRIVILEGED)) {
		set_task_flags(current, get_task_flags(current) | TASK_PRIVILEGED);
		schedule();
	}

	go_run_if(task, TASK_SLEEPING);
}

#include <kernel/init.h>

int __init timer_init()
{
	timerq.nr = 0;
	link_init(&timerq.list);
	mutex_init(&timerq.mutex);

	if ((timerd = make(TASK_KERNEL | STACK_SHARED, STACK_SIZE_MIN,
					run_timer, &init)) == NULL)
		return -ERR_ALLOC;

	timerd->name = "timerd";
	set_task_pri(timerd, HIGHEST_PRIORITY);
	go_run_atomic(timerd);

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

int add_timer(int ms, void (*func)(struct ktimer *timer))
{
	struct ktimer *tm;

	if ((ms == INF) || !ms)
		return 0;

	if ((tm = malloc(sizeof(*tm))) == NULL)
		return -ERR_ALLOC;

	tm->expires = systick + msec_to_ticks(ms);
	tm->event = func;
	tm->data = current;

	return __add_timer(tm);
}

int __add_timer(struct ktimer *new)
{
	return syscall(SYSCALL_TIMER_CREATE, new);
}

#include <foundation.h>

void sleep(unsigned int sec)
{
#ifdef CONFIG_TIMER
	struct ktimer tm;

	tm.expires = systick + sec_to_ticks(sec);
	tm.event = sleep_callback;
	tm.data = current;
	if (!__add_timer(&tm))
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
	struct ktimer tm;

	tm.expires = systick + msec_to_ticks(ms);
	tm.event = sleep_callback;
	tm.data = current;
	if (!__add_timer(&tm))
		yield();
#else
	unsigned int timeout = systick + msec_to_ticks(ms);
	timer_nr++;
	while (time_before(timeout, systick));
	timer_nr--;
#endif
}

int sys_timer_create(struct ktimer *new)
{
#ifdef CONFIG_TIMER
	struct worklist *work;

	if (!add_timerd || (work = kmalloc(sizeof(*work))) == NULL)
		return -ERR_ALLOC;

	new->task = current;
	work->data = new;

	worklist_add(work, &work_list_head);
	syscall_delegate(current, add_timerd);

	return 0;
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
