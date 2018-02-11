/*
 * "[...] Sincerity (comprising truth-to-experience, honesty towards the self,
 * and the capacity for human empathy and compassion) is a quality which
 * resides within the laguage of literature. It isn't a fact or an intention
 * behind the work [...]"
 *
 *             - An introduction to Literary and Cultural Theory, Peter Barry
 *
 *
 *                                                   o8o
 *                                                   `"'
 *     oooo    ooo  .oooo.    .ooooo.   .oooo.o     oooo   .ooooo.
 *      `88.  .8'  `P  )88b  d88' `88b d88(  "8     `888  d88' `88b
 *       `88..8'    .oP"888  888   888 `"Y88b.       888  888   888
 *        `888'    d8(  888  888   888 o.  )88b .o.  888  888   888
 *         .8'     `Y888""8o `Y8bod8P' 8""888P' Y8P o888o `Y8bod8P'
 *     .o..P'
 *     `Y8P'                   Kyunghwan Kwon <kwon@yaos.io>
 *
 *  Welcome aboard!
 */

#include <kernel/timer.h>
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
		ret = -ETIME;
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

#if 1
	/* in case the caller has not yet turned into waiting state by being
	 * preempted, missing its chance to run until higher realtime tasks get
	 * their own work done. */
	if (get_task_state(tm->task) == TASK_RUNNING) {
		unsigned int pri;
		runqueue_del(tm->task);
		pri = get_task_pri(tm->task);
		set_task_pri(tm->task, get_task_pri(current));
		runqueue_add(tm->task);

		while (get_task_state((volatile struct task *)tm->task) !=
				TASK_WAITING)
			resched();
		set_task_pri(tm->task, pri);
	}
#endif
	assert(get_task_state(tm->task) == TASK_WAITING);

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

	sys_kill_core(current, current);
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

		flags = (get_task_flags(timer->task) & ~TASK_STATIC) | TF_ATOMIC;
		if ((thread = make(flags, STACK_SIZE_MIN, do_run_timer,
						timer->task)) == NULL)
			break;

		link_del(curr, prev);
		curr = prev->next;
		q->nr--;

		lock_atomic(&timer->task->lock);
		link_del(&timer->link, &timer->task->timer_head);
		unlock_atomic(&timer->task->lock);

		set_task_pri(thread, get_task_pri(timer->task));
		put_arguments(thread, timer, NULL, NULL, NULL);
		go_run(thread);
	}

	mutex_unlock(&q->mutex);
}

struct ktimer *get_timer_nearest()
{
	return (struct ktimer *)timerq.list.next;
}

static void sleep_callback(struct ktimer *timer)
{
	struct task *task = timer->data;

	/* A trick to enter privileged mode */
	if (!(get_task_flags(current) & TASK_PRIVILEGED)) {
		set_task_flags(current, get_task_flags(current) | TASK_PRIVILEGED);
		schedule();
	}

	/* this callback may get run first before caller gets into sleeping if
	 * sleep time was really short. so give it time to fall asleep */
	while (get_task_state((volatile struct task *)task) != TASK_SLEEPING);
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
		return -ERANGE;
	}

	if ((nsoftirq_add = request_softirq(add_timerd, RT_HIGHEST_PRIORITY))
			>= SOFTIRQ_MAX) {
		error("out of softirq");
		return -ERANGE;
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

#ifdef CONFIG_TIMER
#ifdef CONFIG_SYSCALL_THREAD
static void create_timer(struct ktimer *new)
{
	new->task = current->parent;
	while (raise_softirq(nsoftirq_add, new) == false) ;

	sys_kill_core(current, current);
	freeze();
}

int sys_timer_create(struct ktimer *new)
{
#if 1
	struct task *thread;

	if ((thread = make(TASK_HANDLER | STACK_SHARED, STACK_SIZE_MIN,
					create_timer, current)) == NULL)
		return -ENOMEM;

	syscall_put_arguments(thread, new, NULL, NULL, NULL);
	syscall_delegate(current, thread);
#else
	new->task = current;
	raise_softirq(nsoftirq_add, new);
	set_task_state(current, TASK_WAITING);
	resched();
#endif

	return 0;
}
#else
static void create_timer(struct ktimer *new)
{
	new->task = current;
	while (raise_softirq(nsoftirq_add, new) == false) ;

	set_task_state(current, TASK_WAITING);
	resched();
}

int sys_timer_create(struct ktimer *new)
{
	syscall_delegate_atomic(create_timer, &current->mm.sp, &current->flags);

	(void)new;
	return 0;
}
#endif /* CONFIG_SYSCALL_THREAD */
#else
int sys_timer_create(struct ktimer *new)
{
	return -EFAULT;
}
#endif /* CONFIG_TIMER */

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
		return -ENOMEM;

	tm->expires = systick + msec_to_ticks(ms);
	tm->event = func;
	tm->data = current;

	if (__add_timer(tm)) {
		free(tm);
		return -ENOMEM;
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
	*tv = systick + tick - 1;
}

bool is_timeout(unsigned int goal)
{
	if (time_after(goal, (unsigned int)*(volatile unsigned int *)&systick))
		return true;

	return false;
}

void mdelay(unsigned int ms)
{
	unsigned int tout;
	set_timeout(&tout, msec_to_ticks(ms));
	while (!is_timeout(tout));
}

/* TODO: make it work for user */
/* NOTE: be aware that it can be delayed more than requested up to
 * `HZ * number of tasks` as context switch occurs every HZ. */
void udelay(unsigned int us)
{
	int goal, stamp, prev, elapsed;

	if (!is_honored()) {
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
