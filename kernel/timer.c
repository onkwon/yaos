#include <kernel/timer.h>
#include <kernel/systick.h>
#include <kernel/task.h>
#include <kernel/lock.h>
#include <kernel/softirq.h>
#include <error.h>
#include <stdlib.h>
#include <asm/sysclk.h>

#ifdef CONFIG_TIMER
struct timer_queue timerq;
int nsoftirq_timerd;
static int nsoftirq_add;

static void add_timerd(void *args)
{
	struct ktimer *tm = args;
	struct link *curr, *prev;
	struct ktimer *ref;
	int stamp, ret, new;

	ret = 0;
	stamp = (int)systick;
	tm->expires -= 1; /* as it uses time_after() not time_equal */

	if (time_after(tm->expires, stamp)) {
		ret = -ETIMEDOUT;
	} else {
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
		link_add(&tm->link, &tm->task->timer_head);
		unlock_atomic(&tm->task->lock);

		mutex_unlock(&timerq.mutex);
	}

	__set_retval(tm->task, ret);
	//sum_curr_stat(tm->task);
	go_run(tm->task);
}

/* This function should be called after taking lock for the task */
void __del_timer_if_match(struct task *task, void *addr)
{
	struct ktimer *timer;
	struct link *curr;
	unsigned int pri;
	bool deleted = false;

	/* priority inversion as timerd has the highest priority */
	pri = get_task_pri(current);
	set_task_pri(current, RT_HIGHEST_PRIORITY);

	mutex_lock(&timerq.mutex);

	for (curr = task->timer_head.next; curr; curr = curr->next) {
		timer = get_container_of(curr, struct ktimer, link);

		if (timer->event == addr) {
			link_del(&timer->list, &timerq.list);
			timerq.nr--;
			link_del(&timer->link, &task->timer_head);
			deleted = true;
			break;
		}
	}

	mutex_unlock(&timerq.mutex);
	set_task_pri(current, pri);

	/* hold lock until free() to avoid the task killed before free */
	if (deleted)
		__free(timer, task);
}

static void do_run_timer(struct ktimer *timer)
{
	timer->event(timer);
	//TODO: sum_curr_stat(timer->task);

	kill(current);
	freeze(); /* never reaches here */
}

static void run_timerd(void *args)
{
	struct timer_queue *q = args;
	struct ktimer *timer;
	struct link *curr, *prev;
	struct task *thread;
	unsigned int flags;

	mutex_lock(&q->mutex);

	prev = &q->list;
	curr = prev->next;

	while (curr) {
		timer = (struct ktimer *)curr;

		if (time_before(timer->expires, systick)) {
			q->next = timer->expires;
			break;
		}

		link_del(curr, prev);
		curr = prev->next;
		q->nr--;

		lock_atomic(&timer->task->lock);
		link_del(&timer->link, &timer->task->timer_head);
		unlock_atomic(&timer->task->lock);

		flags = (get_task_flags(timer->task) & ~TASK_STATIC) | TF_ATOMIC;

		if ((thread = make(flags, STACK_SIZE_DEFAULT, do_run_timer,
						timer->task)) == NULL)
			break;

		put_arguments(thread, timer, NULL, NULL, NULL);
		go_run(thread);
	}

	mutex_unlock(&q->mutex);
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

	if ((nsoftirq_timerd = request_softirq(run_timerd, RT_HIGHEST_PRIORITY))
			>= SOFTIRQ_MAX) {
		error("out of softirq");
		return ERANGE;
	}

	if ((nsoftirq_add = request_softirq(add_timerd, RT_HIGHEST_PRIORITY))
			>= SOFTIRQ_MAX) {
		error("out of softirq");
		return ERANGE;
	}

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

static void create_timer(struct ktimer *new)
{
	new->task = current->parent;
	while (raise_softirq(nsoftirq_add, new) == false) ;

	sys_kill_core(current, current);
	freeze();
}

int sys_timer_create(struct ktimer *new)
{
#ifdef CONFIG_TIMER
#if 1
	struct task *thread;

	if ((thread = make(TASK_HANDLER | STACK_SHARED, STACK_SIZE_MIN,
					create_timer, current)) == NULL)
		return ENOMEM;

	syscall_put_arguments(thread, new, NULL, NULL, NULL);
	syscall_delegate(current, thread);
#else
	new->task = current;
	raise_softirq(nsoftirq_add, new);
	set_task_state(current, TASK_WAITING);
	resched();
#endif

	return 0;
#else
	return EFAULT;
#endif
}

int __add_timer(struct ktimer *new)
{
	return syscall(SYSCALL_TIMER_CREATE, new);
}

int add_timer(int ms, void (*func)(struct ktimer *timer))
{
	struct ktimer *tm;

	if ((ms == INF) || !ms)
		return 0;

	if ((tm = malloc(sizeof(*tm))) == NULL)
		return ENOMEM;

	tm->expires = systick + msec_to_ticks(ms);
	tm->event = func;
	tm->data = current;

	if (__add_timer(tm)) {
		free(tm);
		return ENOMEM;
	}

	return 0;
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

void set_timeout(unsigned int *tv, unsigned int tick)
{
	*tv = systick + tick;
}

int is_timeout(unsigned int goal)
{
	if (time_after(goal, systick))
		return 1;

	return 0;
}

static inline void set_timeout_ms(unsigned int *tv, unsigned int ms)
{
	*tv = systick_ms + ms;
}

static inline bool is_timeout_ms(unsigned int goal)
{
	if (time_after(goal, systick_ms))
		return true;

	return false;
}

void mdelay(unsigned int ms)
{
	unsigned int tout;
	set_timeout_ms(&tout, ms);
	while (!is_timeout_ms(tout));
}

/* TODO: make it work for user */
/* NOTE: be aware that it can be delayed more than requested up to
 * `HZ * number of tasks` as context switch occurs every HZ. */
void udelay(unsigned int us)
{
	int goal, stamp, prev, elapsed;

	if (get_current_rank() != TF_PRIVILEGED) {
		error("no permission");
		return;
	}

	prev = get_sysclk();
	goal = get_sysclk_freq() / MHZ;
	goal = goal * us;

	do {
		stamp = get_sysclk();
		elapsed = prev - stamp; /* downcounter */
		prev = stamp;
		if (elapsed < 0)
			goal -= get_sysclk_max() + elapsed;
		else
			goal -= elapsed;
	} while (goal > 0);
}
