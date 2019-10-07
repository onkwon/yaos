/*
 * The timer handler MUST be the only highest priority task not being preempted
 * by other tasks but only by an interrupt. And `timer_create()` only can be
 * called in thread mode, never call it in an interrupt. A lock must take place
 * if any other task need to be running over timer handler.
 *
 * TODO: improve time complexity
 *
 * timer_create() timer_delete() timer_handler() timer_nearest()
 *      O(N)           O(N)           O(1)            O(N)
 */
#if defined(CONFIG_TIMER)

#include "kernel/timer.h"
#include "kernel/systick.h"
#include "kernel/task.h"
#include "kernel/sched.h"
#include "kernel/syscall.h"
#include "list.h"
#include "heap.h"
#include "syslog.h"
#include "compiler.h"

#include <errno.h>

typedef struct ktimer {
	uint8_t run; /* 0 when it's done or removed otherwise counted down
			each time run. in case of TIMER_MAX_RERUN it reruns
			forever, not counting down */
	uint8_t slot;
	uint16_t round;
	uint32_t interval;
	uint32_t expires; /* for latency compensation */
	void (*cb)(void *arg);
	void *task;

	struct list q;
} ktimer_t;

static struct list timers[TIMER_NR_SLOTS];

static inline int calc_left(uint32_t goal, uint32_t now)
{
	int left;

	if (goal < now) {
		left = (uint32_t)-1 - now + goal;
	} else {
		left = goal - now;
	}

	return left;
}

static inline int get_slot(int left, uint32_t now)
{
	return (now + left - 1) % TIMER_NR_SLOTS;
}

static inline uint16_t get_round(int left)
{
	return (left - 1) / TIMER_NR_SLOTS;
}

static inline int update_timer(ktimer_t *timer, uint32_t interval_ticks, uint32_t now)
{
	uint32_t goal;
	int next_slot, left;

	goal = now + interval_ticks;
	left = calc_left(goal, now);
	left = min((uint32_t)left, interval_ticks); // in case time passed
	next_slot = get_slot(left, now);

	assert(next_slot >= 0 && next_slot <= TIMER_MAX_NR_SLOTS);

	timer->round = get_round(left);
	timer->interval = interval_ticks;
	timer->expires = goal;
	timer->slot = next_slot;

	assert(!(timer->run == 0));

	return next_slot;
}

static inline void insert(ktimer_t *timer, int slot, uint32_t now)
{
	assert(&timer->q != timers[slot].next);
	struct list *head, **curr;

	head = &timers[slot];
	curr = &head;

	while ((*curr)->next) {
		curr = &(*curr)->next;

		ktimer_t *p = container_of((*curr), ktimer_t, q);
		if (calc_left(timer->expires, now) < calc_left(p->expires, now)) {
			timer->q.next = *curr;
			*curr = &timer->q;
			return;
		} else if ((*curr)->next == NULL) {
			break;
		}
	}

	timer->q.next = (*curr)->next;
	(*curr)->next = &timer->q;
}

static inline void delete(ktimer_t *timer, void *slot)
{
	int res = list_del(&timer->q, slot);
	assert(res == 0);
}

/* a trick to change task authority to avoid security vulnerability, taking
 * context switch overhead */
static inline void turn_task_permission_from(struct task *task)
{
	if (get_task_flags(current) != get_task_flags(task) ||
			get_task_pri(current) != get_task_pri(task)) {
		set_task_flags(current, get_task_flags(task));
		set_task_pri(current, get_task_pri(task));
		yield();
	}
}

static inline void turn_task_permission_to(unsigned long flags, int pri)
{
	if (get_task_flags(current) != flags ||
			get_task_pri(current) != pri) {
		set_task_flags(current, flags);
		set_task_pri(current, pri);
		yield();
	}
}

STATIC void timer_handler(uint32_t now)
{
	unsigned long flags = get_task_flags(current);
	int pri = get_task_pri(current);

	struct list *slot, *node;
	int current_slot;

	current_slot = now % TIMER_NR_SLOTS;
	slot = &timers[current_slot];
	node = slot->next;

	while (node) {
		ktimer_t *timer = container_of(node, ktimer_t, q);
		int left = calc_left(timer->expires, now);
		uint16_t round = get_round(left);

		node = node->next;

		if (timer->run == 0) {
			delete(timer, slot);
			free_to(timer, timer->task);
			continue;
		} else if (left >= TIMER_NR_SLOTS && round) {
			return;
		}

		delete(timer, slot);

		turn_task_permission_from(timer->task);
		timer->cb(timer->task);
		turn_task_permission_to(flags, pri);

		if (timer->run != TIMER_REPEAT)
			timer->run--;
		if (timer->run) {
			insert(timer, update_timer(timer, timer->interval,
						timer->expires), timer->expires);
			continue;
		}

		free_to(timer, timer->task);
	}
}

static void timer_process(void)
{
	uint32_t prev;

	prev = get_systick();

	while (1) {
		for (uint32_t now = get_systick(); prev != now; prev++) {
			timer_handler(prev + 1);
		}

		set_task_state(current, TASK_WAITING);
		yield();
	}
}
REGISTER_TASK(timer_process, TASK_KERNEL | TF_MANUAL, TASK_PRIORITY_HIGHEST,
		STACK_SIZE_DEFAULT);

/* TODO: call only when a timer to run exists, reducing system timer interrupt
 * overhead which results in power saving */
int timer_run(void)
{
	if (get_task_state(&task_timer_process) == TASK_RUNNING) {
		/* NOTE: timers registered are not being processed in time
		 * because it exceeds number of timers that cpu power can
		 * handle at a time. do either of reducing workload or
		 * increasing cpu power. slower systick or higher cpu clock
		 * would help with it. */
		return -EAGAIN;
	}

	set_task_state(&task_timer_process, TASK_RUNNING);
	runqueue_add_core(&task_timer_process);

	return 0;
}

/**
 * @return timer ID, a memory pointer to allocated timer in fact
*/
int timer_create_core(uint32_t interval_ticks, void (*cb)(void *arg), uint8_t run)
{
	ktimer_t *new;

	if (interval_ticks == 0 || run == 0 || !cb)
		return -EINVAL;

	if ((new = malloc(sizeof(*new))) == NULL)
		return -ENOMEM;

	uint32_t now = get_systick();
	int next_slot;

	new->run = run;
	new->cb = cb;
	next_slot = update_timer(new, interval_ticks, now);
	new->task = current;
	list_init(&new->q);

	insert(new, next_slot, now);

	return (int)new;
}

int timer_delete_core(int timerid)
{
	ktimer_t *p = (ktimer_t *)timerid;

	if (p) {
		p->run = 0;
		struct list *slot = &timers[p->slot];
		delete(p, slot);
		free(p);

		return 0;
	}

	return -EINVAL;
}

int32_t timer_nearest_core(void)
{
	int32_t nearest = TIMER_EMPTY;
	uint32_t now = get_systick();

	for (unsigned int i = 0; i < TIMER_NR_SLOTS; i++) {
		struct list *slot = &timers[i];
		if (slot->next == NULL)
			continue;

		ktimer_t *p = container_of(slot->next, ktimer_t, q);
		int32_t diff = p->expires - now;

		if (diff < 0)
			diff = (uint32_t)-1 - now + p->expires;

		if (diff == TIMER_EMPTY)
			diff -= 1;

		if (nearest > diff)
			nearest = diff;
	}

	return nearest;
}

void timer_init(void)
{
	for (unsigned int i = 0; i < TIMER_NR_SLOTS; i++) {
		list_init(&timers[i]);
	}
}

static void cb_wake(void *arg)
{
	struct task *task = arg;

	set_task_state(task, TASK_RUNNING);
	runqueue_add(task);
}

void sleep(size_t sec)
{
	set_task_state(current, TASK_SLEEPING);
	timer_create(SEC_TO_TICKS(sec), cb_wake, 1);
	yield();
}

void msleep(size_t msec)
{
	set_task_state(current, TASK_SLEEPING);
	timer_create(MSEC_TO_TICKS(msec), cb_wake, 1);
	yield();
}

#else /* !CONFIG_TIMER */
#include "kernel/timer.h"

int timer_run(void) { return 0; }
void timer_init(void) {}

#endif /* CONFIG_TIMER */
